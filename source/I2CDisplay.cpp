#include "LPC802.h"
#include "I2CDisplay.h"
#include "i2c.h"

//Contructor
Display::Display(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows,
		uint8_t charsize) {
	_addr = lcd_addr;
	_cols = lcd_cols;
	_rows = lcd_rows;
	_charsize = charsize;
	_backlightval = LCD_BACKLIGHT;
}

//MOSTLY INITS I2C
void Display::init(void) {

	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	if (_rows > 1) {
		_displayfunction |= LCD_2LINE;
	}
	// for some 1 line displays you can select a 10 pixel high font
	if ((_charsize != 0) && (_rows == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	writeDevice(0b00000000);	//SET ALL BITS TO 0

	//ACTUALLY INIT DISPLAY
	// D7 D6 D5 D4 -  E  RW RS
	// 7  6  5  4  3  2  1  0

	// ALWAYS OUTPUTTING
	// 0  0  1  0  1  1  1  1
	delayMilli(20);				//WAIT >15ms
	writeDevice(0b00110000);
	delayMilli(5);
	writeDevice(0b00110100);	//Enable Pulse
	delayMilli(5);
	writeDevice(0b00110000);
	delayMilli(5);				//WAIT >4.1ms
	writeDevice(0b00110100);	//Enable Pulse
	delayMilli(5);
	writeDevice(0b00110000);
	delayMicro(100);				//WAIT >100us
	writeDevice(0b00110100);	//Enable Pulse
	delayMicro(100);
	writeDevice(0b00110000);

	writeAndUpdate(0b00100000);
	writeAndUpdate(0b00100000);
	writeAndUpdate(0b10000000);
	writeAndUpdate(0b10000000);
	writeAndUpdate(0b00000000);
	writeAndUpdate(0b10001000);
	writeAndUpdate(0b01110000);	//N F ID S
	//Display is now initalized.

	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	clear();

	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

	home();
}

void Display::printString(char *string) {
	for (int i = 0; i < _cols; i++) {
		if (string[i] == '\0')
			break;
		print(string[i]);
	}
}

void Display::clear() {
	command(LCD_CLEARDISPLAY);	// clear display, set cursor position to zero
	delayMicro(2000);  // this command takes a long time!
}

void Display::home() {
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicro(2000);  // this command takes a long time!
}

void Display::setCursor(uint8_t col, uint8_t row) {
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > _rows) {
		row = _rows - 1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void Display::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Display::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void Display::noBacklight(void) {
	_backlightval = LCD_NOBACKLIGHT;
}

void Display::backlight(void) {
	_backlightval = LCD_BACKLIGHT;
}

void Display::print(uint8_t value) {
	send(value, 1);
}

//Specific funtion for sending control commands
void Display::command(uint8_t value) {
	send(value, 0);
}
//Takes 8 bit char  or command and sends it using 4 bit mode
void Display::send(uint8_t data, uint8_t mode) {
	uint8_t highnib = data & 0xf0;	//Puts 4 high bits into last 4 bits 1010xxxx
	uint8_t lownib = (data << 4) & 0xf0;//Puts 4 low bits into last 4 bits  1010xxxx
	writeAndUpdate((highnib) | mode);		//Writes high bits
	writeAndUpdate((lownib) | mode);		//Writes low bits
}

//Pulses enable bit for you and sends command
void Display::writeAndUpdate(uint8_t data) {
	data |= (1UL << 2);	//xxxxx1xx			//Forces enable bit HIGH
	writeDevice((data | _backlightval));	//Push data to device (HIGH BITS)
	delayMicro(45);		//Wait for  37us + 4 us for display to process command
	data &= ~(1UL << 2);					//xxxxx0xx //Forces enable bit LOW
	writeDevice((data | _backlightval));	//Push data to device (LOW BITS)
	delayMicro(45);		//Wait for  37us + 4 us for display to process command
}

//pushes it to display
void Display::writeDevice(uint8_t data) {
	I2CChangePin(I2C_DISPLAY_SDA_PIN, I2C_DISPLAY_SCL_PIN);
	WaitI2CMasterState(I2C0, I2C_STAT_MSTST_IDLE); // Wait for the master state to be idle
	I2C0->MSTDAT = (Display_I2CAddress << 1) | 0; // Address with 0 for RWn bit (WRITE)
	I2C0->MSTCTL = MSTCTL_START; //Start the transaction by setting the MSTSTART bit to 1
								 //in the Master control register.

	WaitI2CMasterState(I2C0, I2C_STAT_MSTST_TXRDY); // Wait for the address to be ACK'd
	I2C0->MSTDAT = data;				//Put 1 byte of data into buffer
	I2C0->MSTCTL = MSTCTL_CONTINUE;

	WaitI2CMasterState(I2C0, I2C_STAT_MSTST_TXRDY); // Wait for the address to be ACK'd
	I2C0->MSTCTL = MSTCTL_STOP;

	while ((I2C0->STAT & MASTER_STATE_MASK) != I2C_STAT_MSTST_IDLE);
}
