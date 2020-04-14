#include <stdio.h>
#include "LPC802.h"
#include <math.h>
#include <string.h>
#include "i2c.h"
#include "tuner.h"
#include "arduino.h"

void Tuner::init(void) {
	resetChip(true);
	writeDevice();
	readDevice();
	enableChip();
}

void Tuner::updateDevice(void) {
	writeDevice();
	readDevice();
}

inline uint16_t Tuner::StationToBytes(int station) {
	//Example 107.3MHz would be inputted as station = 1703
	//1073 = CHAN_SPACING * CHAN + START FREQ
	uint16_t CHAN = (uint16_t) ((station - START_FREQ) * CHAN_SPACING/100000);

	return CHAN;
}

inline int Tuner::StationFromBytes(uint16_t CHAN) {
	int station = (int) ((CHAN_SPACING/100000 * CHAN) + START_FREQ);
	return station;
}

void Tuner::resetBuffer(void){
	//INITAL VALUES FOR TUNER REGISTER
	TunerWriteBuf[0] = 0b11000000;	//REG 0x02 High Bit
	TunerWriteBuf[1] = 0b00000101;	//REG 0x02 Low Bit

	TunerWriteBuf[2] = 0b00011110;	//REG 0x03 High Bit
	TunerWriteBuf[3] = 0b01000000;  //REG 0x03 Low Bit		DEFAULT CHAN @ BOOT: 99.1MHz ASSUMING US REGION

	TunerWriteBuf[4] = 0b00000000;	//REG 0x04 High Bit
	TunerWriteBuf[5] = 0b00000000;	//REG 0x04 Low Bit

	TunerWriteBuf[6] = 0b11001000;	//REG 0x05 High Bit     1000 seek threshold = 0110
	TunerWriteBuf[7] = 0b00001111;	//REG 0x05 Low Bit

	TunerWriteBuf[8] = 0b00000000;	//REG 0x06 High Bit
	TunerWriteBuf[9] = 0b00000000;	//REG 0x06 Low Bit

	TunerWriteBuf[10] = 0b01000010;	//REG 0x07 High Bit
	TunerWriteBuf[11] = 0b00000010;	//REG 0x07 Low Bit
}

void Tuner::setStation(int station) {
	uint16_t CHAN = StationToBytes(station);

	//MASK AND SET CHANNEL BITS TO 0
	TunerWriteBuf[2] &= ~(WRITECHAN_MASK_HIGH);
	TunerWriteBuf[3] &= ~(WRITECHAN_MASK_LOW);

	//Insert Intended Channel Code	
	//High Byte		Low Byte
	//11111111 		11xxxxxx
	TunerWriteBuf[2] |= (uint8_t) (CHAN >> 2); //11111111  Set upper 8 bits of Chan Select
	TunerWriteBuf[3] |= (uint8_t) (CHAN << 6); //11xxxxxx  Set lower 2 bits of Chan Select

	TunerWriteBuf[3] |= (1UL << 4);	//Set tune bit high to actually tune to selected channel
	writeDevice();
	TunerWriteBuf[3] &= ~(1UL << 4);//Resets tune bit
									//Tuner reset bit internally
									//but buffer needs to be reset as well
	return;
}

int Tuner::getStation(void) {
	readDevice();
	uint8_t highbyte = TunerReadBuf[0];
			highbyte &= READCHAN_MASK_HIGH;

	uint8_t lowbyte = TunerReadBuf[1];
			lowbyte &= READCHAN_MASK_LOW;

	uint16_t readCHAN = 0;
			 readCHAN = lowbyte;
			 readCHAN |= (highbyte << 8);

	int station = StationFromBytes(readCHAN);
	return station;
}

//16 Volume Levels, 0 is mute, 15 is full volume
int Tuner::setVolume(uint8_t volume) {
	if (volume >= 0 && volume < 16) {
		//Given that the top half of this bit is reserved
		//we can just equal instead of preform bitwise operations
		TunerWriteBuf[7] &= ~(VOLUME_MASK);
		TunerWriteBuf[7] |= (volume & VOLUME_MASK);
		writeDevice();
		return 0;
	} else {
		return 1;
	}
}

uint8_t Tuner::getVolume() {
	return (TunerWriteBuf[7] & VOLUME_MASK);
}

uint8_t Tuner::getSignalStrength() {
	return (TunerReadBuf[2] >> 1);
}

void Tuner::seekUp() {
	TunerWriteBuf[0] |= SEEKDIR_MASK;	//1 = seekUp
	TunerWriteBuf[0] |= SEEK_TRIGGER_MASK;
	writeDevice();
	TunerWriteBuf[0] &= ~(SEEK_TRIGGER_MASK);
}

void Tuner::seekDown() {
	TunerWriteBuf[0] &= ~(SEEKDIR_MASK);	//0 = seekDown
	TunerWriteBuf[0] |= SEEK_TRIGGER_MASK;
	writeDevice();
	TunerWriteBuf[0] &= ~(SEEK_TRIGGER_MASK);
}

//Did the seek fail?
// 0: Success
// 1: Fail
bool Tuner::seekStatus(void) {
	readDevice();
	bool status = ((TunerReadBuf[0] & SEEK_FAIL_MASK) >> 5);
	return status;
}

//Did the seek finish / scan through everything
// 0: incomplete
// 1: complete
bool Tuner::seekComplete(void) {
	readDevice();
	bool status = ((TunerReadBuf[0] & SEEK_COMPLETE_MASK) >> 6);
	return status;
}

void Tuner::setSeekSensitiviy(uint8_t threshold) {
	threshold &= SEEK_THRESHOLD_MASK;			//Ensure threshold is within range
	TunerReadBuf[6] |= SEEK_THRESHOLD_MASK;		//Set bits to xxxx1111
	TunerReadBuf[6] &= ~(SEEK_THRESHOLD_MASK); 	//Clear bits to xxxx0000
	TunerReadBuf[6] |= threshold;				//Assign to bits
	writeDevice();								//Update Device
}

bool Tuner::STstatus() {
	bool status = false;
	status = ((TunerReadBuf[0] & STERO_INDICATOR_MASK) >> 2);
	return status;
}

bool Tuner::TunedStatus() {
	readDevice();
	if((TunerReadBuf[2] & FM_TRUE_MASK) > 0)
		return true;
	else
		return false;
}

void Tuner::forceMono(bool mono) {
	if (mono)
		TunerWriteBuf[0] |= (MONO_MASK);
	else
		TunerWriteBuf[0] &= ~(MONO_MASK);
	writeDevice();
}

void Tuner::bassBoost(bool bass) {
	if (bass)
		TunerWriteBuf[0] |= (BASS_MASK);
	else
		TunerWriteBuf[0] &= ~(BASS_MASK);
	writeDevice();
}

void Tuner::resetChip(bool resetReg) {
	TunerWriteBuf[1] |= RESET_MASK;
	writeDevice();
	delayMilli(300);
	TunerWriteBuf[1] &= ~(RESET_MASK);
	if(resetReg) resetBuffer();
}

void Tuner::enableChip() {
	TunerWriteBuf[1] |= ENABLE_MASK;
	writeDevice();
}

void Tuner::disableChip() {
	TunerWriteBuf[1] &= ~(ENABLE_MASK);
	writeDevice();
}


void Tuner::mute(bool mute) {
	if (mute)
		TunerWriteBuf[0] |= (MUTE_MASK);
	else
		TunerWriteBuf[0] &= ~(MUTE_MASK);
	writeDevice();

}



void Tuner::readDevice(void) {
	I2CChangePin(_SDApin, _SCLpin);
	/*
	 WaitI2CMasterState(I2C0, I2C_STAT_MSTST_IDLE); // Wait for the master state to be idle
	 I2C0->MSTDAT = (TUNER_I2CAddress<<1) | 0; // Address with 0 for RWn bit (WRITE)
	 I2C0->MSTCTL = MSTCTL_START; //Start the transaction by setting the MSTSTART bit to 1 in the Master control register.

	 WaitI2CMasterState(I2C0, I2C_STAT_MSTST_TXRDY); // Wait for the address to be ACK'd
	 I2C0->MSTDAT = 0x00;							//Put LM75 into config mode (I THINK)
	 I2C0->MSTCTL = MSTCTL_CONTINUE;
	 */
	WaitI2CMasterState(I2C0, I2C_STAT_MSTST_IDLE);
	I2C0->MSTDAT = (_addr << 1) | 1; // Address with 1 for RWn bit (READ)
	I2C0->MSTCTL = MSTCTL_START; 		// Start the transaction by setting the
										// MSTSTART bit to 1 in the Master control register.
	for (int i = 0; i < TOTALREAD_REG; i++) { //6 registers to read from
		//WaitI2CMasterState(I2C0, I2C_STAT_MSTST_TXRDY); // Wait for the address to be ACK'd
		//WaitI2CMasterState(I2C0, I2C_STAT_MSTST_IDLE);
		//WaitI2CMasterState(I2C0, I2C_STAT_MSTST_RXRDY);
		while ((I2C0->STAT & MASTER_STATE_MASK) != I2C_STAT_MSTST_RXRDY)
			; // WAIT FOR THIS CONDITION TO BE TRUE // MASTER_STATE_MASK is (0x7<<1)
		TunerReadBuf[2 * i] = I2C0->MSTDAT;	//STORE HIGH BIT
		I2C0->MSTCTL = MSTCTL_CONTINUE;

		WaitI2CMasterState(I2C0, I2C_STAT_MSTST_RXRDY); // Wait for the address to be ACK'd
		// MASTER_STATE_MASK is (0x7<<1)
		while ((I2C0->STAT & MASTER_STATE_MASK) != I2C_STAT_MSTST_RXRDY)
			; //WAIT FOR THIS CONDITION TO BE TRUE
		TunerReadBuf[(2 * i) + 1] = I2C0->MSTDAT; //STORE LOW BIT

		if (i != TOTALREAD_REG - 1)
			I2C0->MSTCTL = MSTCTL_CONTINUE;
	}
	I2C0->MSTCTL = MSTCTL_STOP;

	// MASTER_STATE_MASK is (0x7<<1)
	while ((I2C0->STAT & MASTER_STATE_MASK) != I2C_STAT_MSTST_IDLE)
		;

}

//Uses buffer and pushes it to tuner
void Tuner::writeDevice(void) {
	I2CChangePin(_SDApin, _SCLpin);
	WaitI2CMasterState(I2C0, I2C_STAT_MSTST_IDLE); // Wait for the master state to be idle
	I2C0->MSTDAT = (_addr << 1) | 0; // Address with 0 for RWn bit (WRITE)
	I2C0->MSTCTL = MSTCTL_START; //Start the transaction by setting the MSTSTART bit to 1 in the Master control register.
	for (int i = 0; i < TOTALWRITE_REG; i++) {
		WaitI2CMasterState(I2C0, I2C_STAT_MSTST_TXRDY); // Wait for the address to be ACK'd
		while ((I2C0->STAT & MASTER_STATE_MASK) != I2C_STAT_MSTST_TXRDY)
			; //WAIT FOR THIS CONDITION TO BE TRUE
		I2C0->MSTDAT = TunerWriteBuf[2 * i];//Put LM75 into config mode (I THINK)
		I2C0->MSTCTL = MSTCTL_CONTINUE;

		WaitI2CMasterState(I2C0, I2C_STAT_MSTST_TXRDY); // Wait for the High Bit to be ACK'd
		while ((I2C0->STAT & MASTER_STATE_MASK) != I2C_STAT_MSTST_TXRDY)
			; //WAIT FOR THIS CONDITION TO BE TRUE
		I2C0->MSTDAT = TunerWriteBuf[(2 * i) + 1];//Put LM75 into config mode (I THINK)

		//if(i != TOTALWRITE_REG-1)
		I2C0->MSTCTL = MSTCTL_CONTINUE;

	}
	WaitI2CMasterState(I2C0, I2C_STAT_MSTST_TXRDY); // Wait for the Low bit to be ACK'd
	//while ((I2C0->STAT & MASTER_STATE_MASK) != I2C_STAT_MSTST_TXRDY); //WAIT FOR THIS CONDITION TO BE TRUE
	I2C0->MSTCTL = MSTCTL_STOP;
	while ((I2C0->STAT & MASTER_STATE_MASK) != I2C_STAT_MSTST_IDLE)
		;
}
