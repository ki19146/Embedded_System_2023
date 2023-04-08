/*
 *  ======== gtz.c ========
 */    

#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gtz.h"

void clk_SWI_Generate_DTMF(UArg arg0);
void clk_SWI_GTZ_0697Hz(UArg arg0);

extern void task0_dtmfGen(void);
extern void task1_dtmfDetect(void);

extern char digit;
extern int sample, mag1, mag2, freq1, freq2, gtz_out[8];
extern short coef[8];



/*
 *  ======== main ========
 */
void main(void)
{


	System_printf("\n I am in main :\n");
	System_flush();
	/* Create a Clock Instance */
    Clock_Params clkParams;

    /* Initialise Clock Instance with time period and timeout (system ticks) */
    Clock_Params_init(&clkParams); //set before period to ensure period will be innitialized to 0
    clkParams.period = 1;
    clkParams.startFlag = TRUE;

    /* Instantiate ISR for tone generation  */
	Clock_create(clk_SWI_Generate_DTMF, TIMEOUT, &clkParams, NULL);

    /* Instantiate 8 parallel ISRs for each of the eight Goertzel coefficients */
	Clock_create(clk_SWI_GTZ_0697Hz, TIMEOUT, &clkParams, NULL);

	mag1 = 32768.0; mag2 = 32768.0; freq1 = 697; // I am setting freq1 = 697Hz to test my GTZ algorithm with one frequency.

	/* Start SYS_BIOS */
    BIOS_start();
}

/*
 *  ====== clk0Fxn =====
 *  Dual-Tone Generation
 *  ====================
 */
void clk_SWI_Generate_DTMF(UArg arg0)
{
	static int tick;


	tick = Clock_getTicks();

//	sample = (int) 32768.0*sin(2.0*PI*freq1*TICK_PERIOD*tick) + 32768.0*sin(2.0*PI*freq2*TICK_PERIOD*tick);
	sample = (int) 32768.0*sin(2.0*PI*freq1*TICK_PERIOD*tick) + 32768.0*sin(2.0*PI*0*TICK_PERIOD*tick);
	sample = sample >>12;
}

/*
 *  ====== clk_SWI_GTZ =====
 *  gtz sweep
 *  ====================
 */
void clk_SWI_GTZ_0697Hz(UArg arg0)
{
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;

   	short input, coef_1; //coef_1 = 0x6D02* for 0697Hz

   	/*Read audio and turn into short form, then scale */
   	R_in = mcbsp0_read(); //function to read from audio input source & store as R_in
   	input = (short) R_in;
   	input = input >>4; //downscale coef is 4, need to be calculated
   	                   // scale factor = 2^(q-1) which means should be shifted to right by q-1 bits
   	                   // q is the number of bits used to represent the input data

   	/*implement the Goertzel Algorithm*/
   	prod1 = (delay_1*coeff_1) >>14; //avoid overflow
   	delay = input + (short)prod1 - dalay_2;
   	delay_2 = delay_1;
   	delay_1 = delay;
   	N++;

   	/*handling the output after setted number of samples*/
   	if (N==206)
   	{
   		prod1 = (delay_1*delay_1);
   		prod2 = (delay_2*delay_2);
   		prod3 = (delay_1*coeff_1)>>14;
   		prod3 = prod3*delay_2;
   		Geortzel_Value = (prod1 + prod2 - prod3) >>15;
   		Geortzel_Value << = 4; //upscale coef is 4, multiples it by 16
   		N =0;
   		delay_1 = delay_2 = 0; //ready to start a new cycle
   	}

   	output = (((short) R_in) * ((short)Geortzel_Value)) >>15; // q = 16 here, means 16-bit signed integer by 'short'
   	mcbsp0_write(output& Oxfffffffe); //send signal out with the least significant bit cleared
   	return;

    //gtz_out[0] = Goertzel_Value; DONT KNOW WHATS THIS FOR?


}
