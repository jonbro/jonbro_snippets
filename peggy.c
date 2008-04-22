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





int main (void)
{ 
	asm("cli");		// DISABLE global interrupts


	unsigned int ADIn,  ADoutTemp, ADLast;
	unsigned int ADCcount;

	unsigned short LightsOn, EnableLights, AutoMode; 
	unsigned short ADdelay, PWMVal, phase, Cmask, rs, rs2; 

	unsigned int ResOffCt;

	unsigned short ResOn, ResOnLocal, SweepDir, ResOffCtLocal;
	unsigned int AutoStage; 

	unsigned short maskCounter, maskOn, maskRatio, maskRatioBig;


	maskRatioBig = 0;
	maskCounter = 0;

	rs2 = 0;
	Cmask = 0;

	ADdelay = 0;
	ResOffCtLocal = 0;
	ADCcount = 0;
	EnableLights = 1;	// Basic conditions met
	LightsOn = 1;		// actually turn lights on
	AutoMode = 1;   //Whether we are in the auto/start-up/power down sequence
	AutoStage = 0;	// If so, which stage we are in
	// 0: Initialization
	// 1: Initial fading/blinking on
	// 2: steady on for some hours
	// 3: power down until it goes dark again

	DDRA  = 0; // All A Inputs
	PORTA = 0;

	ResOffCt = 0; 

	phase = 0;
	
	rs = 0;	
	
	ADIn = 10000U;

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

	ADoutTemp = 0;


	ADCSRA |= _BV(ADSC);	// Start initial ADC cycle



	while ((ADCSRA & _BV(ADSC)) != 0){;;}		// Wait until conversion has finished

		ADIn = ADCW;	


	ADLast = ADIn;











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

	ResOn = 1; 
	PWMVal = 8;
		
	SweepDir = 1;

	asm("sei");		// ENABLE global interrupts




	for (;;){
		// main loop
		if (ADdelay == 255){
			if ((ADCSRA & _BV(ADSC)) == 0){		// If conversion has finished
				ADIn = ADCW;		
				if (ADIn < 25) {
					EnableLights = 1; 
					ADCcount = 0;
					AutoMode = 0; 
				}	
				else if (ADIn < 975){		// It's Dark out! Turn on the lights!
					if (EnableLights == 1){
						ADCcount++;
					}else{
						ADCcount = 0;					
					}
				}else{	// if (ADIn > 975), meaning that it's DARK out...		
					if (EnableLights == 0){
						ADCcount++;				
					}else{
						ADCcount = 0;				
					}
				}
				if (ADCcount > 1000U){
					ADCcount = 0;
					AutoMode = 1;
					if (ADIn < 975){
						EnableLights = 0;    
					}else{
						EnableLights = 1; 
					}	
				}

				ADCSRA |= _BV(ADSC);	// Start ADC
			}else{
				ADdelay++;			
			}
		}
		ResOffCt++;
		rs++;
		if (rs == 255){
			rs = 0; 			
		}


		// Phases:  0: Standard w/o noise
		//1 delay
		//2: standard with noise
		//3 delay

		if (phase == 1){
			ResOffCt += 5;			
		}
	
		if (phase == 3){
			ResOffCt += 5;			
		}
	

		if (ResOffCt > 375) // Every 375 cycles increment PWM
		{

		rs2++; 
		Cmask = ~(1 << rs2) ; // Generate simple scan lines

		Cmask |= rs;	// Add noise to scan lines

		if (rs2 > 7){
			rs2 = 0;	
		}

		ResOffCt = 0;

		if (SweepDir == 1){
			PWMVal++;
			if (PWMVal > 127){
				PWMVal = 127;
				SweepDir = 0;
			}
		}else{
			PWMVal--;
			if (PWMVal == 0){
				SweepDir = 1;
				phase++;
				if (phase == 4){
					phase = 0;
				}
			}
		}
	}
	


		// Phases:  0: Standard w/o noise
		//1 delay
		//2: standard with noise
		//3 delay


		
		ResOffCtLocal++;		// PWM variable
		if (ResOffCtLocal > 126)
			ResOffCtLocal = 0;
		
		if (ResOffCtLocal > PWMVal)
			ResOnLocal = 0;		// Turn LEDs off for part of the time.
		else	{

				ResOnLocal = 0;
	
			if (phase == 0)
				ResOnLocal = 1;

			if (phase == 2)
				ResOnLocal = 1;
				
		
		
		}


		if (phase == 2) 
			maskRatio  = ( PWMVal) >> 3; 
		else
			maskRatio = 0;
	
		//maskRatio = 15;  // Lots of glitches
		//maskRatio = 0; // No glitches

		
		maskCounter++;		// PWM variable
		if (maskCounter > 15)
			maskCounter = 0;
	
		if (maskCounter > maskRatio)
			maskOn = 0;		// Turn LEDs off for part of the time.
		else
			maskOn = 1;





		if (EnableLights) {
			if (ResOnLocal) {
				PORTA = 64;
				delayLong();
				PORTA = 128;
				delayLong();
				PORTA = 0; 

				PORTB = 1;
				delayLong();
				PORTB = 2;
				delayLong();
				PORTB = 4;
				delayLong();
				PORTB = 8;
				delayLong();
				PORTB = 16;
				delayLong();
				PORTB = 32;
				delayLong();
				PORTB = 64;
				delayLong();
				PORTB = 128;
				delayLong();
				PORTB = 0; 

				PORTC = 0;
				delayLong();
				PORTC = 0;
				delayLong();
				PORTC = 0;
				delayLong();
				PORTC = 0;// Row 8
				delayLong();
				PORTC = 0;
				PORTD = 1;
				delayLong();
				PORTD = 2;
				delayLong();
				PORTD = 4;
				delayLong();
				PORTD = 8;
				delayLong();
				PORTD = 16;
				delayLong();
				PORTD = 32;
				delayLong();
				PORTD = 64;
				delayLong();
				PORTD = 128;
				delayLong();
				PORTD = 0;
			}else{	// ResOff
				PORTC = 1;
				delayLong();
				PORTC = 2;
				delayLong();
				PORTC = 4;
				delayLong();
				PORTC = 8;
				delayLong();
				PORTC = 16;
				delayLong();
				PORTC = 32;
				delayLong();
				PORTC = 64;
				delayLong();
				PORTC = 128;
				delayLong();
				PORTC = 0; 
				
				
		
				PORTD = 0;	// Row 7
				delayLong();
				PORTD = 0;
				delayLong();
				PORTD = 0;
				delayLong();
				PORTD = 0;
				delayLong();
				PORTD = 0;
				delayLong();
				PORTD = 0; 
			}
		} 
	}	//End main loop.
	return 0;
}
