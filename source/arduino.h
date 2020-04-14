
#define MAIN_CLK 12000000

/*
 * TODO:
 * Delay function
 * LED BULLSHIT
 * BUTTON BULLSHIT
 */

int bit_test(char bit, char byte);
//returns true or false depending on state of bit
int bit_true(char bit, char byte);//sets selected bit to 0
void bit_reset(char bit, char *byte);
void delayMicro(int value);
void delayMilli(int value);
