/*
 *  ======== gtz.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gtz.h"

void clk_SWI_Read_Data(UArg arg0);
void clk_SWI_GTZ_All_Freq(UArg arg0);

extern void task0_dtmfGen(void);
extern void task1_dtmfDetect(void);

extern int sample, tdiff, tdiff_final, gtz_out[8];
extern short coef[8];                          //8 FREQS TO BE CALCULATED
extern int flag;

short data[NO_OF_SAMPLES];
short *buffer;

/*
 *  ======== main ========
 */
int main() {
	System_printf("\n System Start\n");
	System_flush();

	/* Read binary data file */
	FILE* fp = fopen("../data.bin", "rb"); //OPEN DATA FILE AND READ IN BINARY
	if(fp==0)                              //CHECK WHETHER FILE WAS OPENED SUCCESSFUL
	{
		System_printf("Error: Data file not found\n");
		System_flush();
		return 1;                      //STATUS CODE OF 1
	}
	fread(data, 2, NO_OF_SAMPLES, fp);     //READ BINARY CODE INTO BUFFER 'DATA'
	buffer = (short*)malloc(2*8*10000);

	/* Create a Clock Instance */
	Clock_Params clkParams;

	/* Initialise Clock Instance with time period and timeout (system ticks) */
	Clock_Params_init(&clkParams);
	clkParams.period = 1;
	clkParams.startFlag = TRUE;

	/* Instantiate ISR for tone generation  */
	Clock_create(clk_SWI_Read_Data, TIMEOUT, &clkParams, NULL);

	/* Instantiate 8 parallel ISRs for each of the eight Goertzel coefficients */
	Clock_create(clk_SWI_GTZ_All_Freq, TIMEOUT, &clkParams, NULL);

	/* Start SYS_BIOS */
	BIOS_start();
}

/*
 *  ====== clk_SWI_Generate_DTMF =====
 *  Dual-Tone Generation
 *  ==================================
 */
void clk_SWI_Read_Data(UArg arg0) {
	static int tick;
	tick = Clock_getTicks();
	sample = data[tick%NO_OF_SAMPLES];
}

/*
 *  ====== clk_SWI_GTZ =====
 *  gtz sweep
 *  ========================
 */
void clk_SWI_GTZ_All_Freq(UArg arg0) {
	// define variables for timing
	static int start, stop;

	// define feedback times as N
	static int N = 0;

   	//Record start time
	start = Timestamp_get32();

	static int Goertzel_Value = 0;   //INITIALISE GOERTZEL VALUE
	sample = sample >>4;             //SAMPLE SHFIT TO RIGHT 4 BITS TO PREVENT OVERFLOW
	short input = (short) (sample);  //MAKE SAMPLE INTO SHORT FORM
	static short delay[8];           //MAKE DELAY ARRAY
	static short delay_1[8]={0};     //MAKE DELAY_1 ARRAY
	static short delay_2[8]={0};     //MAKE DELAY_2 ARRAY
	short coef[8] =                  //REFERING TO FREQS IN main_Onefreq.c FILE
			{ 0x6D02, 0x68AD, 0x63FC, 0x5EE7, 0x4A70, 0x4090, 0x3290, 0x23CE }; // goertzel coefficients
	int prod1[8], prod2[8], prod3[8];
	
	/* TODO 1. Complete the feedback loop of the GTZ algorithm*/
	/* ========================= */
	int i;
	for (i=0;i<8;i++){
		prod1[i] = (delay_1[i]*coef[i])>>14;
	   	delay[i] = input + (short)prod1[i] - delay_2[i];
	   	delay_2[i] = delay_1[i];
	   	delay_1[i] = delay[i];
	}

	/* ========================= */
	N++;

	//Record stop time
	stop = Timestamp_get32();
	//Record elapsed time
	tdiff = stop-start;

	if (N == 206) {
	   	//Record start time
		start = Timestamp_get32();

		/* TODO 2. Complete the feedforward loop of the GTZ algorithm*/
		/* ========================= */
		for (i=0;i<8;i++){
			prod1[i] = (delay_1[i] * delay_1[i]);
		   	prod2[i] = (delay_2[i] * delay_2[i]);
		   	prod3[i] = (delay_1[i] * coef[i])>>14;
		   	prod3[i] = prod3[i] * delay_2[i];
		   	Goertzel_Value = (prod1[i] + prod2[i] - prod3[i]) >> 15;
		   	Goertzel_Value <<= 4; // Scale up value for sensitivity
		   	N = 0;
		   	delay_1[i] = delay_2[i] = 0;
		   	gtz_out[i] = Goertzel_Value;
			}
		/* gtz_out[..] = ... */
		/* ========================= */
		flag = 1;
		N = 0;

		//Record stop time
		stop = Timestamp_get32();
		//Record elapsed time
		tdiff_final = stop-start;
	}
}
