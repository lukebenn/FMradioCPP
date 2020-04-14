#include "LPC802.h"
#include "i2c.h"

void I2Cinit(void){
	// Step 1: Connect I2C module to outer pins via Switch Matrix
	I2CChangePin(I2C_DEFAULT_SDA_PIN, I2C_DEFAULT_SDA_PIN);
	// ------------------------------------------------------------
	// Step 2: Turn on the I2C module via the SYSCON clock enable pin
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_I2C0_MASK );// I2C CLOCK is on

	// Put 0 in the GPIO, GPIO Interrupt and I2C reset bit to reset it.
	// Then put a 1 in the GPIO, GPIO Interrupt and I2C reset bit to allow them to operate.
	// manual: Section 6.6.10
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_I2C0_RST_N_MASK );// reset I2C(bit = 0)
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_I2C0_RST_N_MASK);// remove i2C reset (bit = 1)

	// ------------------------------------------------------------
	// Step 3: Choose the source of the I2C timing clock
	// see: Fig 7 of User Manual
	// ------------------------------------------------------------
	// Provide main_clk as function clock to I2C0
	// bits 2:0 are 0x1.
	// FRO is 0b000; main clock is 0b001, frg0clk is 0b010, fro_div is 0b100.
	// Set i2c to FRO (12 MHz)
	// (only bits 2:0 are used; other 29 bits are ignored)
	SYSCON->I2C0CLKSEL = 0b000;// put 000 in bits 2:1.
	// confirmed in reg view that this is FRO.
	// ------------------------------------------------------------
	// Step 4: enable master (but not slave) functionality in the i2c module.
	//
	// ------------------------------------------------------------
	// Configure the I2C0 CFG register:
	// Master enable = true
	// Slave enable = true
	// Monitor enable = false
	// Time-out enable = false
	// Monitor function clock stretching = false
	//
	// Only config I2C0 as a master
	I2C0->CFG = (I2C_CFG_MSTEN_MASK);// only as master

	// Comment out for now: therefore, no interruptions.
	// Enable the I2C0 slave pending interrupt
	// I2C0->INTENSET = I2C_INTENSET_SLVPENDINGEN_MASK; // STAT_SLVPEND;

	// ------------------------------------------------------------
	// Step 5: set the i2c clock rate. (20Hz to 400kHz is typical)
	// I2C_MasterSetBaudRate() function is helpful.
	// ------------------------------------------------------------
	// Set i2C bps to 100kHz assuming an input clock of 12MHz
	I2C_MasterSetBaudRate(LPC_I2C0BAUDRate, 12000000);
	// after this, CLKDIV is 0x9 and MSTTIME is 0x44.
	// MSTTIME = 0x44 means: MSTSCLLOW [2:0] is CLOCKS_6 // MSTCLHIGH [6:4] is CLOCKS_6
}
/*****************************************************************************
** Function name: WaitI2CMasterState
**
**  Description: 	Waits for I2C mater pending, then compares the master
** 					state parameter. If compare fails, enter a while(1) loop.
** 					If compare passes, return.
**
**	Parameters:
**					ptr_LPC_I2C: A pointer to an I2C instance
**					state: ONe of the 3-bit Master function state codes of the I2C
**
**  Returned value: None
**
** *****************************************************************************/
void WaitI2CMasterState(I2C_Type * ptr_LPC_I2C, uint32_t state) {
	// Check the Master Pending bit (bit 0 of the i2c stat register)
	while(!(ptr_LPC_I2C->STAT & I2C_STAT_MSTPENDING_MASK)); // Wait for MSTPENDING bit set in STAT register
	// Check to see that the state is correct.
	// if it is not, then turn on PIO0_9 to indicate a problem
	// Master's state is in bits 3:1. MASTER_STATE_MASK is (0x7<<1)
	if((ptr_LPC_I2C->STAT & MASTER_STATE_MASK) != state){ // If master state mismatch ...
		//GPIO->DIRCLR[0] = (1UL<<LED_USER2);// turn on LED on PIO0_9 (LED_USER2)
		while(1); // die here and debug the problem
	}
	return; // If no mismatch, return
}

// --------------------------------------------------------------------------------
// I2C_MasterSetBaudRate() //
// from SDK for LPC802; fsl_i2c.c
// input: baudRate_Bps & srcClock_Hz
// output: no explicit output. Two I2C config registers set.
// --------------------------------------------------------------------------------
void I2C_MasterSetBaudRate(uint32_t baudRate_Bps, uint32_t srcClock_Hz){
	uint32_t scl, divider;
	uint32_t best_scl, best_div;
	uint32_t err, best_err;
	best_err = 0;

	for (scl = 9; scl >= 2; scl--){
		/* calculated ideal divider value for given scl */
		divider = srcClock_Hz / (baudRate_Bps * scl * 2u);

		/* adjust it if it is out of range */
		divider = (divider > 0x10000u) ? 0x10000 : divider;

		/* calculate error */
		err = srcClock_Hz - (baudRate_Bps * scl * 2u * divider);
		if ((err < best_err) || (best_err == 0)){
			best_div = divider; best_scl = scl; best_err = err;
		}
		if ((err == 0) || (divider >= 0x10000u)){
			/* either exact value was found
			or divider is at its max (it would even greater in the next iteration for sure) */
		break;
		}
	}
	// Assign Clock Divider value, using macro included in LPC802.h
	I2C0->CLKDIV = I2C_CLKDIV_DIVVAL(best_div - 1);
	// Assign Master timing configuration, using two macros include in LPC802.h
	I2C0->MSTTIME = I2C_MSTTIME_MSTSCLLOW(best_scl - 2u) | I2C_MSTTIME_MSTSCLHIGH(best_scl - 2u);
}

//	Dynamic Pin Change For Fixing an Issue
// 	With Finicky FM Chip
void I2CChangePin(uint8_t SDApin, uint8_t SCLpin){
	// PIO0_16 is connected to the SCL line
	// PIO0_10 is connected to the SDA line
	// PINASSIGN5 bits 15:8 are for SCL. Therefore fill with value 16
	// PINASSIGN5 bits 7:0 are for SDA. Therefore fill with value 10 //
	// enable switch matrix.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_SWM_MASK);
	// Set switch matrix
	// PINASSIGN5: clear bits 15:0 to permit the two i2c lines to be assigned.
	SWM0->PINASSIGN.PINASSIGN5 &= ~(SWM_PINASSIGN5_I2C0_SCL_IO_MASK | SWM_PINASSIGN5_I2C0_SDA_IO_MASK); // clear 15:0
	SWM0->PINASSIGN.PINASSIGN5 |= ((SCLpin<<SWM_PINASSIGN5_I2C0_SCL_IO_SHIFT)|// Put 16 in bits 15:8
	 (SDApin<<SWM_PINASSIGN5_I2C0_SDA_IO_SHIFT));// put 10 in bits 7:0 // PINASSIGN5 should be 0xffff100a after this.
	// disable the switch matrix
	SYSCON->SYSAHBCLKCTRL0 &= ~(SYSCON_SYSAHBCLKCTRL0_SWM_MASK);
	return;
}

