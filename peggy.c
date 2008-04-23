/*
Title:			peggy.c 
Date Created:   1/16/08
Last Modified:  2/17/08
Target:			Atmel ATmega164P 
Environment:	AVR-GCC

Purpose: Drive multiplexed LED array 
This version:  Fade in and out, every other time with retro scanlines added-- "bad video" effect.


Fuse settings: 8 MHz internal RC oscillator, clock prescaler turned off (divide by 1)



Written by Windell Oskay, http://www.evilmadscientist.com/

Copyright 2008 Windell H. Oskay
Distributed under the terms of the GNU General Public License, please see below.

Additional license terms may be available; please contact us for more information.



More information about this project is at 
http://www.evilmadscientist.com/article.php/peggy



-------------------------------------------------
USAGE: How to compile and install



A makefile is provided to compile and install this program using AVR-GCC and avrdude.

To use it, follow these steps:
1. Update the header of the makefile as needed to reflect the type of AVR programmer that you use.
2. Open a terminal window and move into the directory with this file and the makefile.  
3. At the terminal enter
		make clean   <return>
		make all     <return>
		make install <return>
4. Make sure that avrdude does not report any errors.  If all goes well, the last few lines output by avrdude
should look something like this:

avrdude: verifying ...
avrdude: XXXX bytes of flash verified

avrdude: safemode: lfuse reads as E2
avrdude: safemode: hfuse reads as D9
avrdude: safemode: efuse reads as FF
avrdude: safemode: Fuses OK

avrdude done.  Thank you.


If you a different programming environment, make sure that you copy over 
the fuse settings from the makefile.


-------------------------------------------------

This code should be relatively straightforward, so not much documentation is provided.  If you'd like to ask 
questions, suggest improvements, or report success, please use the evilmadscientist forum:
http://www.evilmadscientist.com/forum/

-------------------------------------------------


Revision hitory:

1/22/2008 

Initial version
-------------------------------------------------


 This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


*/

#include <avr/io.h> 
#include <avr/interrupt.h>
#include "pwm.h"

void delayLong()
{
			asm("nop");  
			asm("nop");  
			asm("nop");  
			asm("nop");  
			asm("nop"); 
			asm("nop");  
			asm("nop");  
			asm("nop");  
			asm("nop");  
			asm("nop"); 
}



/*
void delayLong() // Alternate scheme
{
unsigned int delayvar;

			delayvar = 0; 

//Uncomment the following section to watch the charlieplexed display
//do its stuff, one line at a time.
		
			delayvar = 0; 
	while (delayvar <=  65000U)		
		{ 
			asm("nop");  
			asm("nop");  
			asm("nop");  
			asm("nop");  
			asm("nop");  
			delayvar++;
		} 
		

}
*/






SIGNAL(SIG_PIN_CHANGE0)
{ 		
	DDRD  = 0; // All D Inputs
	PORTD = 0;

	DDRC  = 0; // All C Inputs
	PORTC = 0;

	DDRB  = 0; // All B Inputs
	PORTB = 0;

	DDRA  = 0; // All A Inputs
	PORTA = 0;

	ADCSRA = 0; // Kill ADC

	SMCR = 5U;
	asm("sleep");		//Go to sleep!
}

void light_row(int row_num){
	if(row_num<8){
		PORTD = 1;
		delayLong();
	}else if(row_num<16){
		
	}else if(row_num<24){

	}else{

	}
}

void update_pwm_m(struct pwm_manager * pm){
	pm->ResOffCt++;
	if (pm->ResOffCt > pm->loop_length){ // only increment the pwm every so often
		pm->ResOffCt = 0;
		if (pm->SweepDir == 1){
			pm->PWMVal++;
			if (pm->PWMVal > 127){
				pm->PWMVal = 127;
				pm->SweepDir = 0;
			}
		}else{
			pm->PWMVal--;
			if (pm->PWMVal == 0){
				pm->SweepDir = 1;
			}
		}
	}
	
	pm->ResOffCtLocal++;		// PWM variable
	if (pm->ResOffCtLocal > 126){
		pm->ResOffCtLocal = 0;
	}
	
	if (pm->ResOffCtLocal > pm->PWMVal){
		pm->ResOnLocal = 0;		// Turn LEDs off for part of the time.
	}else{
		pm->ResOnLocal = 0;
		if (pm->phase == 0){
			pm->ResOnLocal = 1;
		}
	}
}

int main (void)
{ 
	int num_rows = 25;
	struct pwm_manager pwm_m[num_rows], *pm;
	int i=0;
	
	asm("cli");		// DISABLE global interrupts
	DDRA  = 0; // All A Inputs
	PORTA = 0;

	// General Hardware Initialization:

	MCUCR |= (1 << 4); // Disable pull-ups

	PCMSK0 = 4U; // Pin change interrupt mask
	PCICR = 1; // Enable PCINTs on PCMSK0

		
	DDRD  = 0; // All D Inputs
	PORTD = 255U;

	DDRC  = 0; // All C Inputs
	PORTC = 255U;

	DDRB  = 0; // All B Inputs
	PORTB = 255U;

	DDRA  = 0; // All A Inputs
	PORTA = 255U;
	ADMUX = 5U;		// Channel 3 only
	ADCSRA = _BV(ADEN) |_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0)  ;  // Enable ADC, prescale at 128.

	ADCSRA |= _BV(ADSC);	// Start initial ADC cycle
	while ((ADCSRA & _BV(ADSC)) != 0){;;}		// Wait until conversion has finished
	ADMUX = 3U;		// Channel 3 only

	//Initialization for Multiplexed drive
	DDRC  = 255; // All C Inputs
	PORTC = 64U;

	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;

	DDRA = 192U;
	DDRB = 255U;
	DDRC = 255U;
	DDRD = 255U;
	
	for(i=0;i<num_rows;i++){
		pwm_m[i].ResOffCt = 0; 
		pwm_m[i].ResOffCtLocal = 0;
		pwm_m[i].phase = 0;
		pwm_m[i].ResOn = 1; 
		pwm_m[i].PWMVal = 8;
		pwm_m[i].SweepDir = 1;
		pwm_m[i].loop_length = 375-i*10;		
	}

	// pwm_m.ResOffCt = 0; 
	// pwm_m.ResOffCtLocal = 0;
	// pwm_m.phase = 0;
	// pwm_m.ResOn = 1; 
	// pwm_m.PWMVal = 8;
	// pwm_m.SweepDir = 1;
	// pwm_m.loop_length = 375;
	asm("sei");		// ENABLE global interrupts
	for (;;){
		
		PORTA = 64;
		delayLong();
		PORTA = 128;
		delayLong();
		PORTA = 0;

		PORTB = 0;
		PORTC = 0;
		PORTD = 0;
		
		for(i=0;i<12;i++){
			pm = &pwm_m[i];        			/* put the address of 'pwm_m[0]' into 'pm' */
			update_pwm_m(pm);
			if(i<4){
				if(pwm_m[i].ResOnLocal){
					PORTB ^= 0b000011 << i*2;
				}
			}else if(i<8){
				if(pwm_m[i].ResOnLocal){
					PORTC ^= 0b000011 << (i-4)*2;
				}
			}else{
				if(pwm_m[i].ResOnLocal){
					PORTD ^= 0b000011 << (i-8)*2;
				}
			}
		}
		 
		
		// if (pwm_m[0].ResOnLocal) {
		// 	PORTB = 0b11111111;
		// }
		// if (pwm_m[4].ResOnLocal) {
		// 	PORTC = 0b11111111;
		// }
		// if (pwm_m[8].ResOnLocal) {
		// 	PORTD = 0b11111111;
		// }
		delayLong();
		delayLong();
		delayLong();
		delayLong();
		delayLong();
		delayLong();
		delayLong();
		delayLong();
	}	//End main loop.
	return 0;
}
