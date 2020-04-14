#include <stdio.h>
#include "LPC802.h"
#include "i2c.h"
#include "arduino.h"

#define LPC_I2C0BUFFERSize 	(35)
#define LPC_I2C0BAUDRate 	(100000)// 100kHz
//volatile uint8_t g_I2C0DataBuf[LPC_I2C0BUFFERSize];
//volatile uint8_t g_I2C0DataCnt;
#define WKT_FREQ 	1000000 // Use if the WKT is clocked by the LPOSC
#define WKT_RELOAD 	100000//10000000 // Reload value for the WKT down counter

//uint32_t g_WKT_RELOAD = WKT_RELOAD; // counter reload value for WKT

#define Display_I2CAddress (0x27)

// Define values for I2C registers that aren't in the header file. // Table 195 of LPC802 User Manual
#define MSTCTL_CONTINUE 	(1UL << 0)// Bit 0 of MSTCTL set
#define MSTCTL_START 		(1UL << 1)// Bit 1 of MSTCTL set
#define MSTCTL_STOP 		(1UL << 2)// Bit 2 of MSTTCL set #define CTL_SLVCONTINUE(1UL << 0)
#define CTL_SLVNACK 		(1UL << 1)
#define MASTER_STATE_MASK		 (0x7<<1)// bits 3:1 of STAT register.
#define I2C_STAT_MSTST_IDLE		 ((0b000)<<1)// from LPC802 user manual table 187
#define I2C_STAT_MSTST_RXRDY	 ((0b001)<<1)// from LPC802 user manual table 187
#define I2C_STAT_MSTST_TXRDY	 ((0b010)<<1)// from LPC802 user manual table 187
#define I2C_STAT_MSTST_NACK_ADD	 ((0b011)<<1)// from LPC802 user manual table 187
#define I2C_STAT_MSTST_NACK_DATA ((0b100)<<1)// from LPC802 user manual table 187

//LCD define values
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

#define I2C_DISPLAY_SDA_PIN		(11)
#define I2C_DISPLAY_SCL_PIN		(13)

/*

 CHAR LCD PINOUT
 1 VSS 0V Ground
 2 VDD 5V Supply Voltage for logic
 3 VO (Variable) Operating voltage for LCD

 4 RS H/L H: DATA, L: Instruction code
 5 R/W H/L H: Read(MPU?Module) L: Write(MPU?Module)
 6 E H->L Chip enable signal

 7  DB0 H/L Data bus line
 8  DB1 H/L Data bus line
 9  DB2 H/L Data bus line
 10 DB3 H/L Data bus line
 11 DB4 H/L Data bus line
 12 DB5 H/L Data bus line
 13 DB6 H/L Data bus line
 14 DB7 H/L Data bus line

 15 A 5V LED +
 16 K 0V LED-

 I2C BIT ORDER
 0:	RS
 1:	RW
 2:	E	(POS EDGE- CLOCK BASICALLY)
 3:	-   (BACKLIGHT BIT)
 4:	D4
 5:	D5
 6:	D6
 7:  D7
 */

class Display {

public:
	Display(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows,
			uint8_t charsize = LCD_5x8DOTS);
	void init(void);
	void clear();
	void home();
	void setCursor(uint8_t col, uint8_t row);
	void display();
	void noDisplay();
	void noBacklight(void);
	void backlight(void);
	void print(uint8_t value);
	void printString(char *string);
private:
	void command(uint8_t value);
	void send(uint8_t data, uint8_t mode);
	void writeAndUpdate(uint8_t data);
	void writeDevice(uint8_t data);
	//void send(uint8_t, uint8_t);
	//void write4bits(uint8_t);
	//void expanderWrite(uint8_t);
	//void pulseEnable(uint8_t);
	uint8_t _addr;
	uint8_t _displayfunction;
	uint8_t _displaycontrol;
	uint8_t _displaymode;
	uint8_t _cols;
	uint8_t _rows;
	uint8_t _charsize;
	uint8_t _backlightval;
};

//Display display = Display(0x27, 20, 4, 0);



