/*
 * Copyright 2020 Luke Bennett
 * All rights reserved.
 *
 */
 
/**
 * @file    FMradioCPP.cpp
 * @brief   Application entry point.
 */
#include <stdio.h>	//For sprintf
#include "LPC802.h"
#include "I2CDisplay.h"
#include "tuner.h"
#include "arduino.h"
#include "i2c.h"
#include "string.h"

/*
 * @brief   Application entry point.
 */

#define ENCODER_OUTA (4)
#define ENCODER_OUTB (0)
#define ENCODER_BUT (9)

#define BUTTON_USER1 (7)
#define BUTTON_USER2 (12)
#define BUTTON_USER3 (8)
#define BUTTON_USER4 (17)

#define INT_CHAN_0 (0)
#define INT_CHAN_1 (1)
#define INT_CHAN_2 (2)
#define INT_CHAN_3 (3)
#define INT_CHAN_4 (4)
#define INT_CHAN_5 (5)
#define INT_CHAN_6 (6)

volatile int station = START_FREQ;			//Being Displayed on Screen
volatile int readStation = 870;		//Pulled from Tuner
volatile int volume = 15;
int rssi = 0;
volatile int mode = 1;
volatile int preset = 0;
volatile int presets[4] = {925,991,999,1045};	//92.5  99.1  99.9  104.5
char stationString[40];
char volumeString[40];
char stereoString[20];
char rssiString[20];
char presetString[25];
char notificationString[20] = "Welcome!";
volatile bool forceClear = false;
volatile bool triggerTune = false;
volatile bool triggerVolume = false;
volatile int  triggerScan = 0;	//0: Do Nothing  -1:Tune DOWN  1:Tune UP
volatile bool rotaryWait = false; //0: Do Nothing -1: Decrement 1:Increment
volatile bool triggerWaitForScanComplete = false;
volatile bool triggerPresetChange = true;



Tuner tuner(0x10, 10, 16);					//Create Object
Display display(0x27, 20, 4);	//Create Object

int main(void) {
	//Disable interrupts
	__disable_irq();				// turn off globally
	NVIC_DisableIRQ(PIN_INT0_IRQn);	// turn off the PIN INT0 interrupt.
	NVIC_DisableIRQ(PIN_INT1_IRQn); // turn off the PIN INT1 interrupt.
	NVIC_DisableIRQ(PIN_INT2_IRQn); // turn off the PIN INT2 interrupt.
	NVIC_DisableIRQ(PIN_INT3_IRQn); // turn off the PIN INT3 interrupt.
	NVIC_DisableIRQ(PIN_INT4_IRQn); // turn off the PIN INT4 interrupt.
	NVIC_DisableIRQ(PIN_INT5_IRQn); // turn off the PIN INT5 interrupt.
	NVIC_DisableIRQ(PIN_INT6_IRQn); // turn off the PIN INT6 interrupt.
	// -----------------------Begin GPIO setup ------------------------------------
	// Set up a general GPIO for use
	// Only the ISR needs the GPIO.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK |// GPIO is on
								SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);// GPIO Interrupt is on
	// Put 0 in the GPIO and GPIO Interrupt reset bit to reset it.
	// Then put a 1 in the GPIO and GPIO Interrupt reset bit to allow both to operate.
	// manual: Section 6.6.10
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK
							|SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);
	// reset GPIO and GPIO Interrupt (bit=0)
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK
							|SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);// clear reset (bit=1)
	// Config the Pushbutton (GPIO 8) for input and LED (GPIO 9) for output
	// Remember: only bits set to 1 have an effect on DIRCLR and DIRSET registers.
	//bits cleared to 0 are ignored.
	//Therefore, use DIRCLR to select input and DIRSET to select output
	GPIO->DIRCLR[0] = (1UL<<ENCODER_BUT);
	GPIO->DIRCLR[0] = (1UL<<BUTTON_USER1);
	GPIO->DIRCLR[0] = (1UL<<BUTTON_USER2);
	GPIO->DIRCLR[0] = (1UL<<BUTTON_USER3);
	GPIO->DIRCLR[0] = (1UL<<BUTTON_USER4);
	GPIO->DIRCLR[0] = (1UL<<ENCODER_OUTA);
	GPIO->DIRCLR[0] = (1UL<<ENCODER_OUTB);
	// -----------------------end of GPIO setup -----------------------------------
	SYSCON->PINTSEL[0] = ENCODER_BUT;
	SYSCON->PINTSEL[1] = BUTTON_USER1;
	SYSCON->PINTSEL[2] = BUTTON_USER2;
	SYSCON->PINTSEL[3] = BUTTON_USER3;
	SYSCON->PINTSEL[4] = BUTTON_USER4;
	SYSCON->PINTSEL[5] = ENCODER_OUTA;
	SYSCON->PINTSEL[6] = ENCODER_OUTB;
	//INTENTIONALY OMITTED ENCODER_OUTB
	// Configure the Pin interrupt mode register (a.k.a ISEL) for edge-sensitive

	PINT->ISEL = 0x00;// on PINTSEL0.  0 is edge sensitive. 1 is level sensitive.
	// channel 0 bit is 0: is edge sensitive (so are the other channels)
	// Use IENR or IENF (or S/CIENF or S/CIENR) to set edge type
	PINT->CIENR = 0b00011111;	// disable for rising edge for channel[x]
	PINT->SIENF = 0b00011111;	// enable for falling edge for channel[x]
	PINT->CIENF = 0b01100000;	// disable for falling edge for channel[x]
	PINT->SIENR = 0b01100000;	// enable for rising edge for channel[x]

	PINT->IST= 0xFF;			// each bit set to 1 removes any pending flag.

	NVIC_EnableIRQ(PIN_INT0_IRQn);// Enable GPIO CH 0 interrupt
	NVIC_EnableIRQ(PIN_INT1_IRQn);// Enable GPIO CH 1 interrupt
	NVIC_EnableIRQ(PIN_INT2_IRQn);// Enable GPIO CH 2 interrupt
	NVIC_EnableIRQ(PIN_INT3_IRQn);// Enable GPIO CH 3 interrupt
	NVIC_EnableIRQ(PIN_INT4_IRQn);// Enable GPIO CH 4 interrupt
	NVIC_EnableIRQ(PIN_INT5_IRQn);// Enable GPIO CH 5 interrupt
	NVIC_EnableIRQ(PIN_INT6_IRQn);// Enable GPIO CH 6 interrupt

	// -----------------------Begin SysTick setup --------------------------
	NVIC_DisableIRQ(SysTick_IRQn);
	// Clock rate: 12 MHz via FRO. (8.3*10^(-8) sec period)
	// 12,000,000 ticks/sec * 0.2 sec = 2400000 ticks
	// SysTick is a 24 bittimer (max almost 17 million)
	SysTick_Config(2400000);  // 2400000 ticks = 0.2 sec @ 12 MHz
	NVIC_EnableIRQ(SysTick_IRQn);
	// -----------------------end SysTick setup --------------------------

	I2Cinit();

    display.init();
    display.backlight();
    display.setCursor(4, 1);
	display.printString("Booting...");

	delayMilli(300);
    tuner.init();
    delayMilli(300);
    tuner.setVolume(7);
    delayMilli(300);
    tuner.forceMono(true);
    tuner.bassBoost(true);
    tuner.setStation(1071);
    station = 1071;



    /* Force the counter to be placed into memory. */
    volatile static int i = 0 ;
    /* Enter an infinite loop, just incrementing a counter. */
    __enable_irq();  // global enable
    while(1) {
        i++ ;
        asm("nop");
    }
    return 0 ;
}
extern "C"{
void SysTick_Handler(void){
	rotaryWait = false;
	//Tuner API Calls_________________________________________

	readStation = tuner.getStation();

	if(triggerTune){				//Update tuned frequency
		tuner.setStation(station);
		triggerTune = false;
	}

	if(triggerScan == 1){
		tuner.seekUp();
		triggerScan = 0;
		triggerWaitForScanComplete = true;
	}
	if(triggerScan == -1){
		tuner.seekDown();
		triggerScan = 0;
		triggerWaitForScanComplete = true;
	}

	if(triggerPresetChange){
		station = presets[preset];
		tuner.setStation(station);
		triggerPresetChange = false;
	}

	if(triggerVolume){
		tuner.setVolume(volume);
		triggerVolume = false;
	}

	if(triggerWaitForScanComplete){
		memset(notificationString, 0, sizeof notificationString	);
		if(tuner.seekComplete()){
			if(tuner.seekStatus()) strcpy(notificationString , "Scan Fail");
			else strcpy(notificationString ,"Done Scan");
			station = tuner.getStation();
			triggerWaitForScanComplete = false;
		}
		else strcpy(notificationString ,"Scanning..");
	}

	//Display API Call Code__________________________________
	//Clear Display
	if (forceClear) {
		display.clear();
		forceClear = false;
	}
	//Get data ready for display_____________________

	//Preset String Data
	memset(presetString, 0, sizeof presetString);	//Clear String
	strcat(presetString, "Preset 1  2  3  4 ");
	if((station == presets[preset]) && mode){	//Causes Arrows to Disappear
									///When Not Tuned To Preset
		presetString[6+(preset*3)] = '>';
		presetString[8+(preset*3)] = '<';
	}

	//Get Tuned and Stero/Mono Data
	bool stereo = tuner.STstatus();
	memset(stereoString, 0, sizeof stereoString);	//Clear String
	if(stereo) strcat(stereoString, "Stereo");
	else strcat(stereoString, "Mono");

	//Get RSSI Data (Signal Strength)
	rssi = tuner.getSignalStrength();
	rssi /= 16;
	memset(rssiString, 0, sizeof rssiString);
	strcat(rssiString, "Signal [");

	for(int i = 0; i < 8; i++){
		if(i > rssi){
			strcat(rssiString, "_");
			continue;
		}
		char temp[2] = {(char) 0b11111111};
		strcat(rssiString, temp);//THICK BLOCK
	}
	strcat(rssiString, "]");


	//Convert station to sting with MHz Attached to end
	char whole[10];
	char decimal[10];
	sprintf(whole, "%d", (station / 10));
	sprintf(decimal, "%d", (station % 10));
	memset(stationString, 0, sizeof stationString);
	if(mode) strcat(stationString, "_");
	else strcat(stationString, ">");
	strcat(stationString, whole);
	strcat(stationString, ".");
	strcat(stationString, decimal);
	strcat(stationString, "MHz ");

	//Convert Volume to String
	char strVol[4] = {'\0'};
	//memset(strVol, 0, sizeof strVol);
	sprintf(strVol, "%d", volume);
	memset(volumeString, 0, sizeof volumeString);
	if(mode) strcat(volumeString, ">");
	else strcat(volumeString, "_");
	strcat(volumeString, "Vol:");
	strcat(volumeString, strVol);
	strcat(volumeString, " ");

	//Display Data___________________________________
	display.setCursor(0, 0);
	display.printString(stationString);

	display.setCursor(12, 0);
	display.printString(volumeString);

	if(mode){
		display.setCursor(0, 1);
		display.printString(rssiString);

		display.setCursor(0, 2);
		display.printString(stereoString);

		display.setCursor(10, 2);
		display.printString(notificationString);
	}
	else{
		char string1[21] = "Save Station?";
		char string2[21] = "Use Buttons to save.";
		display.setCursor(0, 1);
		display.printString(string1);

		display.setCursor(0, 2);
		display.printString(string2);
	}

	display.setCursor(0, 3);
	display.printString(presetString);
}
}

extern "C"{
//One interrupt method per channel

//CHANGE MODE Button (Rotary Encoder Press Button)_____________
void PIN_INT0_IRQHandler(void){
	// was an IRQ requested for Channel 0 of GPIO INT?
	if(PINT->IST & (1<<INT_CHAN_0)){
		// remove the any IRQ flag for Channel 0 of GPIO INT
		if(mode) mode = 0;
		else mode = 1;
		forceClear = true;
		PINT->IST = (1<<INT_CHAN_0);
	}
	else asm("NOP");  // Place a breakpthere if debugging.
	return;
}
}

extern "C"{
//Decrease Preset Button (A Button)
void PIN_INT1_IRQHandler(void){
	if(PINT->IST & (1<<INT_CHAN_1)){
		PINT->IST = (1<<INT_CHAN_1);
		if((preset > 0) && mode){
			preset--;					//Decrement preset
			triggerPresetChange = true;	//Make SysTick process the station change
			//forceClear = true;
		}
		else if(!mode){
			presets[0] = station;	//Save station to preset 1 (array el. 0)
			mode = 1;
			strcpy(notificationString ,"Saved.");
			forceClear = true;
		}
		PINT->IST = (1<<INT_CHAN_1);
	}
	else asm("NOP");
	return;
}
}

extern "C"{
//Increase Preset Button (B Button)
void PIN_INT2_IRQHandler(void){
	if(PINT->IST & (1<<INT_CHAN_2)){
		if((preset < 3) && mode){
			preset++;					//Increment preset
			triggerPresetChange = true; //Make SysTick process the station change
			//forceClear = true;
		}
		else if(!mode){
			presets[1] = station;	//Save station to preset 2 (array el. 1)
			mode = 1;
			strcpy(notificationString ,"Saved.");
			forceClear = true;
		}
		PINT->IST = (1<<INT_CHAN_2);
	}
	else asm("NOP");
	return;
}
}

extern "C"{
//Scan Down Button (C Button)
void PIN_INT3_IRQHandler(void){
	if(PINT->IST & (1<<INT_CHAN_3)){
		if (mode)
			triggerScan = -1;	//Scan down
		else if(!mode){
			presets[2] = station; //Save station to preset 3 (array el. 2)
			mode = 1;
			strcpy(notificationString ,"Saved.");
			forceClear = true;
		}
		PINT->IST = (1<<INT_CHAN_3);
	}
	else asm("NOP");
	return;
}
}

extern "C"{
//Scan Up Button (D Button)
void PIN_INT4_IRQHandler(void){
	if(PINT->IST & (1<<INT_CHAN_4)){
		if (mode)
			triggerScan = 1;	//Scan up
		else if(!mode){
			presets[3] = station;	//Save station to preset 4 (array el. 3)
			mode = 1;
			strcpy(notificationString ,"Saved.");
			forceClear = true;
		}
		PINT->IST = (1<<INT_CHAN_4);
	}
	else asm("NOP");
	return;
}
}
/*
extern "C"{
//Clockwise:		B then late A
//Counterclockwise: A then late B
void PIN_INT5_IRQHandler(void){
	if(PINT->IST & (1<<INT_CHAN_5)){

		//COUNTER CLOCKWISE
		if(!rotaryWait){
			if(GPIO->B[0][ENCODER_OUTB] && 1UL){
				if(mode && (volume > 0))
					volume--;
				else if(!mode && (station > START_FREQ)){
					station--;
					triggerTune = true;
				}
			}
			//CLOCKWISE
			else{
				if(mode && (volume < 15))
					volume++;
				else if(!mode && (station < 1080)){
					station++;
					triggerTune = true;
				}
			}
			rotaryWait = true;
		}
		PINT->IST = (1<<INT_CHAN_5);
	}
	else asm("NOP");
	return;
}
}
*/
//BUTTON VERSION____________________________
extern "C"{
void PIN_INT5_IRQHandler(void){
	if(PINT->IST & (1<<INT_CHAN_5)){
		if(mode && (volume > 0)){
			volume--;
			triggerVolume = true;
		}
		else if(!mode && (station > START_FREQ)){
			station--;
			triggerTune = true;
		}
		PINT->IST = (1<<INT_CHAN_5);
	}
	else asm("NOP");
	return;
}
}

extern "C"{
void PIN_INT6_IRQHandler(void){
	if(PINT->IST & (1<<INT_CHAN_6)){
		if(mode && (volume < 15)){
			volume++;
			triggerVolume = true;
		}
		else if(!mode && (station < 1080)){
			station++;
			triggerTune = true;
		}

		PINT->IST = (1<<INT_CHAN_6);
	}
	else asm("NOP");
	return;
}
}

