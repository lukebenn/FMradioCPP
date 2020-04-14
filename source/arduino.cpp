#include "LPC802.h"
#include "arduino.h"

int bit_test(char bit, char byte) {
    bit = 1 << bit;     //Creates Mask
    return(bit & byte); //ANDs mask and byte
}

//returns true or false depending on state of bit
int bit_true(char bit, char byte) {
    int temp = bit_test(bit, byte);
    if (temp != 0)  return 1;
    else            return 0;
}

//sets selected bit to 0
void bit_reset(char bit, char *byte){
    bit = 1 << bit;
    bit ^= 0xff;
    *byte = *byte & bit;
}

void delayMicro(int value){
	int maxVal = ((value/2) * MAIN_CLK/12000000);
	for(int i = 0; i < maxVal; i++)
		asm("nop");
}

void delayMilli(int value){
	int maxVal = ((value/2) * MAIN_CLK/11600);
	for(int i = 0; i < maxVal; i++)
		asm("nop");
}

