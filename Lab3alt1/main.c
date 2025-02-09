/*
 * Lab3alt1.c
 *lab 1 solution
 * Created: 05/02/2025 11:07:52
 * Author : Clara & Nader
 */ 

#include <stdint.h>
#include <avr/io.h>
#include <avr/portpins.h>
#include <stdio.h>
#include <stdlib.h>
#include "tinythreads.h"
#include <stdbool.h>

// trying to fix primes,  button and all-in-One

mutex mut = MUTEX_INIT;


int numbers[10] =
{
	0x1551,		// 0
	0x0118,		// 1
	0x1e11,		// 2
	0x1b11,		// 3
	0x0b50,		// 4
	0x1b41,		// 5
	0x1f41,		// 6
	0x4009,		// 7
	0x1f51,		// 8
	0x1b51,		// 9
};


void disableClockPrescaler() {
	// Disable the CPU clock prescaler by writing 0x80 followed by 0x00 to CLKPR
	CLKPR = 0x80;
	CLKPR = 0x00;
}

void initLCD(void) {
	// Initial LCD setup, from p. 216 in ATmega168 manual.
	// Drive time: 300 microseconds; contrast control voltage: 3.35 V,
	// 1/3 bias, 1/4 duty cycle, 25 segments enabled, prescaler setting N=16,
	// clock divider setting D=8, LCD enabled, low power waveform, no frame interrupt, no blanking. 
	
    LCDCRB = (1 << LCDCS) |(1 << LCDMUX0) | (1 << LCDMUX1)| (1 << LCD2B) | (1 << LCDPM2) | (1 << LCDPM1) | (1 << LCDPM0);   
		
    LCDFRR = (1 << LCDCD0) | (1 << LCDCD1) | (1 << LCDCD2);
	
    LCDCCR = (1 << LCDCC3) | (1 << LCDCC2) | (1 << LCDCC1) | (1 << LCDCC0);

    LCDCRA = (1 << LCDAB) | (1 << LCDEN);
}


void writeChar(char ch, int pos) {
	uint8_t nibble = 0x0;
	volatile uint8_t *lcdaddr = &LCDDR0;
	lcdaddr += pos/2 ;
	uint8_t c = ch;
	c-= 0x30;
	uint16_t number = numbers[c];
	if(pos > 5 || pos < 0) return;
	uint8_t  mask = pos % 2 == 0 ? 0xf0 : 0x0f;

	for (int x=0; x<4; x++) {
		nibble = number & 0xf;
		number = number >> 4;
		if(pos % 2 != 0) {
			nibble = nibble << 4;
		}
		*lcdaddr = (*lcdaddr & mask) | nibble;
		lcdaddr += 5;
	}
}


void writeLong(long i){
    for (int counter = 5; i != 0 && counter >= 0; counter--){
        writeChar(i % 10, counter);				// write the least significant digit of the long to the display
        i = i/10;								// extract the least significant digit of the long
    }
}

int is_prime(long i){ 
	for(int j = 2; j < i; j++){
		if (i % j == 0) return 0;				// if is divisible for more numbers than the number itself and 1 is not prime, return false
	}
	return 1;
}


//To make the assignment slightly more interesting, 
//we will limit prime number printouts to LCD positions 0 and 1 (the left-most positions), 
//and require the accumulated key press count to be displayed at LCD at positions 4 and 5 (the right-most positions). 
//Function printAt() from lab 2 can preferably be used for this purpose 
//(restored to its original definition, of course, involving no global variables). 
void printAt(long num, int pos) {

	int pp = pos;
	writeChar( (num % 100) / 10 + '0', pp);
	pp++;
	writeChar( num % 10 + '0', pp);
	
}



void computePrimes(int pos) {
	long n;
	for(n = 1; ; n++) {
		if (is_prime(n)) {
			printAt(n, pos);
		}
	}
}


void blink(void){
	TCCR1B = TCCR1B|0x04; //this changes CS12 to 1
	int *LCDADDR18 = &LCDDR18;
	unsigned short VAR_REG = TCCR1B;  //unsigned integer

	while(1){
		if(TCNT1 == VAR_REG) {
			VAR_REG= VAR_REG +(8000000 / 256); //Timer use the 8 MHz with a prescaling factor of 256
			*LCDADDR18 = *LCDADDR18 ^ 0x01; //XOR FOR turn on and off
		}
	}
}

void button() {
	static long button_count = 0;        // Holds the accumulated button presses
	PORTB = (1<<PB7); // set bit 7 to high
	int knappen = 0;

	while(1) {
		if(PINB>>7 == 1) {
			knappen = 0;
		}
		if(!knappen && PINB>>7 == 0) {
			knappen = 1;               //we change mode when the button is pressed down
			// Increment the counter
			button_count++;
           
            printAt(button_count, 4);	/// Display the last two digits at positions 4 (tens) and 5 (ones)

			if (LCDDR0 & 0x03)  {
				LCDDR0 = LCDDR0 & 0xfc; // turn off bit 0 and bit 1
				LCDDR1 = LCDDR1 | 0x80; // turn on bit 7
		    } else {
				LCDDR0 = LCDDR0 | 0x03; // turn on bit 0 and bit 1
				LCDDR1 = LCDDR1 & 0x7f; // turn off bit 7
			}
		}
	}
}


void allInOne(void){ 
	long num = 25000;
	TCCR1B = TCCR1B|0x04; 
	int *LCDADDR18 = &LCDDR18;
	unsigned short VAR_REG = TCCR1B;  
	PORTB = (1<<PB7); 
	int knappen = 0;
	static long button_count = 0;  

	
	while(1){	
		if(is_prime(num)){	
			printAt(num, 0);			//limit prime number printouts to LCD positions 0 and 1 (the left-most positions)
		}
		//should we use is prime here or computePrimes? different outputs
		
		//computePrimes(0);
		num += 1;
		
		if(PINB>>7 == 1) {
			knappen = 0;
		}
		
		if(!knappen && PINB>>7 == 0) {
			knappen = 1;    
			button_count++;
			printAt(button_count, 4);           
			if (LCDDR0 & 0x03)  {
				LCDDR0 = LCDDR0 & 0xfc; 
				LCDDR1 = LCDDR1 | 0x80; 
		    } else {
				LCDDR0 = LCDDR0 | 0x03; 
				LCDDR1 = LCDDR1 & 0x7f; 
			}
		}
		
		if(TCNT1 >= VAR_REG) {
			VAR_REG= VAR_REG +(8000000 / (256*2)); 
			*LCDADDR18 = *LCDADDR18 ^ 0x01; 
		}
	}
	
}



int main(void)
{

	disableClockPrescaler();
	initLCD();
	
	//computePrimes(0);
	
	//button();
	blink();
	//allInOne();
}
