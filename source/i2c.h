#define LPC_I2C0BUFFERSize 	(35)
#define LPC_I2C0BAUDRate 	(100000)// 100kHz

#define WKT_FREQ 	1000000 // Use if the WKT is clocked by the LPOSC
#define WKT_RELOAD 	100000//10000000 // Reload value for the WKT down counter

// Define values for I2C registers that aren't in the header file. // Table 195 of LPC802 User Manual
#define MSTCTL_CONTINUE 	(1UL << 0)// Bit 0 of MSTCTL set
#define MSTCTL_START 		(1UL << 1)// Bit 1 of MSTCTL set
#define MSTCTL_STOP 		(1UL << 2)// Bit 2 of MSTTCL set
#define CTL_SLVCONTINUE     (1UL << 0)
#define CTL_SLVNACK 		(1UL << 1)
#define MASTER_STATE_MASK		 (0x7<<1)// bits 3:1 of STAT register.
#define I2C_STAT_MSTST_IDLE		 ((0b000)<<1)// from LPC802 user manual table 187
#define I2C_STAT_MSTST_RXRDY	 ((0b001)<<1)// from LPC802 user manual table 187
#define I2C_STAT_MSTST_TXRDY	 ((0b010)<<1)// from LPC802 user manual table 187
#define I2C_STAT_MSTST_NACK_ADD	 ((0b011)<<1)// from LPC802 user manual table 187
#define I2C_STAT_MSTST_NACK_DATA ((0b100)<<1)// from LPC802 user manual table 187
#define I2C_DEFAULT_SDA_PIN		(10)
#define I2C_DEFAULT_SCL_PIN		(16)


/**
 * @brief Initalizes I2C module with default pins
 * 
 * Initalizes I2C module by: 
 * - setting the clock to on
 * - adjust the switch matrix
 * - fully reseting I2C module
 * - setting baud rate to default 100Khz, 12Mhz
 * 
 * I2C_MasterSetBaudRate() command must be called again to adjust
 * baude rate or declare new FRO speed.
 */
void I2Cinit(void);

/**
 * @brief Used to force program to wait until I2C bus is in correct state
 * 
 * Waits for I2C mater pending, then compares the master
 * state parameter. If compare fails, enter a while(1) loop.
 * If compare passes, return.
 * 
 * @param ptr_LPC_I2C A pointer to an I2C instance
 * @param state       One of the 3-bit Master function state codes of the I2C - Refer to LPC802 manual table 187
 */
void WaitI2CMasterState(I2C_Type * ptr_LPC_I2C, uint32_t state);

/**
 * @brief Sets the Baud rate of the I2C bus
 * 
 * @param baudRate_Bps in Hz (default 100Khz)
 * @param srcClock_Hz  FRO clock
 */
void I2C_MasterSetBaudRate(uint32_t baudRate_Bps, uint32_t srcClock_Hz);


/**
 * @brief Changes pins assossiated with I2C bus
 * 
 * This command can be called multiple times given bus
 * is idle when switched (does not check)
 * 
 * Allows for multiple slave devices to work completly isolated
 * from each other. Useful in some senerios
 * 
 * @param SDApin Serial Data Pin
 * @param SCLpin Serial Clock Pin
 */
void I2CChangePin(uint8_t SDApin, uint8_t SCLpin);

