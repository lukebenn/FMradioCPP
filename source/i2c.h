#define LPC_I2C0BUFFERSize 	(35)
#define LPC_I2C0BAUDRate 	(100000)// 100kHz
#define WKT_FREQ 	1000000 // Use if the WKT is clocked by the LPOSC
#define WKT_RELOAD 	100000//10000000 // Reload value for the WKT down counter


//#define Display_I2CAddress (0x10)

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

void I2Cinit(void);
void WaitI2CMasterState(I2C_Type * ptr_LPC_I2C, uint32_t state);
void I2C_MasterSetBaudRate(uint32_t baudRate_Bps, uint32_t srcClock_Hz);
void I2CChangePin(uint8_t SDApin, uint8_t SCLpin);

