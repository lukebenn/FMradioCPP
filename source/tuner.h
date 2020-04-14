#define LPC_I2C0BUFFERSize 	(35)
#define LPC_I2C0BAUDRate 	(100000)// 100kHz

//FM SETTING DEFAULTS CHECK DATA SHEET ON HOW TO MODIFY
#define CHAN_SPACING (100000UL)
#define CHAN_SPACING_BIT (00UL)	//00: 100KHz	01: 200KHz	10:	50KHz	11:	25KHz	Adjust CHAN_SPACING accordingly
#define BAND_REGION (00UL)		//00: 87-108	01: 76-91	10:	76-108	11:	65-76
#define START_FREQ (870)
#define STOP_FREQ (1080)

#define TOTALWRITE_REG (6)	//Number of Registers (0:6)
#define TOTALREAD_REG (5)	//Number of Registers (0:5)

//MASKS
#define SEEKDIR_MASK 			(0b00000010)
#define SEEK_TRIGGER_MASK 		(0b00000001)
#define SEEK_THRESHOLD_MASK		(0b00001111)

#define TUNE_TRIGGER_MASK 		(0b00010000)
#define TUNE_COMPLETE_MASK 		(0b01000000)
#define BAND_MASK 				(0b00001100)
#define STERO_INDICATOR_MASK 	(0b00000100)
#define SEEK_FAIL_MASK			(0b00100000)
#define SEEK_COMPLETE_MASK		(0b01000000)

#define VOLUME_MASK 			(0b00001111)
#define MUTE_MASK				(0b01000000)
#define MONO_MASK				(0b00100000)
#define BASS_MASK				(0b00010000)

#define RESET_MASK				(0b00000010)
#define ENABLE_MASK				(0b00000001)

#define RSSI_MASK 				(0b11111110)
#define FM_TRUE_MASK			(0b00000001)

#define READCHAN_MASK_LOW  		(0b11111111)
#define READCHAN_MASK_HIGH 		(0b00000011)
#define WRITECHAN_MASK_LOW  	(0b11000000)
#define WRITECHAN_MASK_HIGH 	(0b11111111)

class Tuner {
	uint8_t TunerReadBuf[12];	/*!< 6 16-bit registers with a high and low byte separated */
	uint8_t TunerWriteBuf[12];  /*!< 6 16-bit registers with a high and low byte separated */
	u_int16_t rdsBlocks[4];		/*!< 4 16-bit registers */
	uint8_t _addr = 0x10;		/*!< I2C Address */
	uint8_t _SDApin;			/*!< I2C Serial Data Pin Number */
	uint8_t _SCLpin;			/*!< I2C Serial Clock Pin Number */
public:
	/**
	 * @brief Construct a new Tuner object
	 * 
	 * @param address I2C Address
	 * @param SDA Serial Data Pin
	 * @param SCL Serial Clock Pin
	 */
	Tuner(uint8_t address, uint8_t SDA, uint8_t SCL) {
		_addr = address;
		_SDApin = SDA;
		_SCLpin = SCL;
		resetBuffer();
	}

	//High level commands_____________________________________

	/**
	 * @brief Resets chip, enables chip, and updates both buffers
	 * 
	 */
	void init(void);

	/**
	 * @brief Sends updated buffers to tuner chip
	 * 
	 * All chages made with other API calls
	 * will not take affect until this command
	 * is called.
	 * 
	 */
	void updateDevice(void);

	/**
	 * @brief Set the station to tune to
	 * 
	 * @param station (Station in MHz * 10)
	 */
	
	void setStation(int station);

	/**
	 * @brief Get channel chip is tuned to currently
	 * 
	 * @return int station (Station in MHz * 10)
	 */
	int getStation();

	/**
	 * @brief Set volume of chip audio amplifier
	 * 
	 * Will return error if integer given is out of range
	 * 
	 * @param volume (0 to 15)
	 * @return int
	 */
	int setVolume(uint8_t volume);

	/**
	 * @brief Get the Volume value from buffer
	 * 
	 * @return uint8_t volume
	 */
	uint8_t getVolume();

	/**
	 * @brief Get the Signal Strength as a 7-bit int
	 * 
	 * @return uint8_t strength
	 */
	uint8_t getSignalStrength();
	
	/**
	 * @brief Calling this function preforms soft reset
	 * 
	 * @param resetReg Also reset internal buffers as well?
	 */
	void resetChip(bool resetReg);

	/**
	 * @brief Enables receiver audio amplifier of chip
	 * 
	 */
	void enableChip();

	/**
	 * @brief Disables receiver audio amplifier of chip
	 * 
	 */
	void disableChip();

	/**
	 * @brief Set the Seek Sensitivity 
	 * 
	 * Send a 4-bit number to set the signal
	 * to noise ratio of detected signal
	 * 
	 * @param threshold (default: 1000)
	 */
	void setSeekSensitiviy(uint8_t threshold);

	/**
	 * @brief Command chip to preform Auto Seek Upwards
	 * 
	 */
	void seekUp();

	/**
	 * @brief Command chip to preform Auto Seek Downwards
	 * 
	 */
	void seekDown();

	/**
	 * @brief Status of most recent seek
	 * 
	 * @return true: Last seek Succeeded
	 * @return false: Last seek Failed
	 */
	bool seekStatus(void);

	/**
	 * @brief Status of if scan is complete
	 * 
	 * @return true: Scan Complete
	 * @return false: Scan in progress
	 */
	bool seekComplete(void);

	/**
	 * @brief Is the chip receiving a stereo signal?
	 * 
	 * @return true:  Currently receiving stereo signal 
	 * @return false: Currently receiving mono signal
	 */
	bool STstatus();

	/**
	 * @brief Is current frequency a station?
	 * 
	 * @return true: 	Chip is Tuned to Station
	 * @return false: 	Chip is Tuned to Station
	 */
	bool TunedStatus();

	/**
	 * @brief Force mono signal only
	 * 
	 * @param mono 
	 */
	void forceMono(bool mono);

	/**
	 * @brief enable bass boost option
	 * 
	 * @param bass 
	 */
	void bassBoost(bool bass);

	/**
	 * @brief Toggle mute option, chip will still respond
	 * 
	 * @param mute 
	 */
	void mute(bool mute);


private:
	//Low level commands______________________________________
	/**
	 * @brief Reads read-only buffer from chip
	 * 
	 */
	void readDevice(void);

	/**
	 * @brief Sends updated write buffer to chip
	 * 
	 */
	void writeDevice(void);

	/**
	 * @brief Converts station to bytes for use by chip
	 * 
	 * The 
	 * 
	 * @param station (in MHz * 10)
	 * @return uint16_t 
	 */
	inline uint16_t StationToBytes(int station);

	/**
	 * @brief Converts bytes in buffer to station
	 * 
	 * @param CHAN 
	 * @return int (station in MHz * 10)
	 */
	inline int StationFromBytes(uint16_t CHAN);

	/**
	 * @brief Resets write buffer to original state
	 * 
	 */
	void resetBuffer(void);
};

