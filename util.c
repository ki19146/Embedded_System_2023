/*
 *  ======== util.c ========
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

int tdiff,tdiff_final;

int sample, gtz_out[8];
int flag = 0;

short coef[8] =
		{ 0x6D02, 0x68AD, 0x63FC, 0x5EE7, 0x4A70, 0x4090, 0x3290, 0x23CE }; // goertzel coefficients
void task1_dtmfDetect();
void task2_dtmfGenerate(char* keys);
extern short* buffer;

void task1_dtmfDetect() {
	int i, n;
	char pad[4][4] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
	char result[8];

	for(n=0;n<8;n++) {
		while (!flag) Task_sleep(210);
		/* TODO 3. Complete code to detect the 8 digits based on the GTZ output */
		/* ========================= */
		int DigitPosition[2]={0};
		int count=0;	//count stored GTZ positions
		for(i = 0; i<8; i++){
			if(gtz_out[i] != 0){
				DigitPosition[count++] = i;}	 //store 2 non zero GTZ positions
		}
		if (count==2){
			result[n]=pad[DigitPosition[0]][DigitPosition[1]-4];	//decode 1 digit with its corresponding 2 freq 
			}
		printf("%c\n", result[n]);
		System_flush();
		flag = 0;

		/* result[n] = ... */
		/* ========================= */


	}
	printf("\nDetection finished\n");
	printf("Generating audio\n");
	task2_dtmfGenerate(result);
	printf("Finished\n");
}

void task2_dtmfGenerate(char* keys)
{
	int fs = 10000;
	float tone_length = 0.5;
	int n_tones = 8;
	int samples_per_tone = (int) (tone_length * fs);
	int samples_total = samples_per_tone * n_tones;
	int i, n;
	char digit;
	int frow[4] = { 697, 770, 852, 941 }; // 1st tone
	int fcol[4] = { 1209, 1336, 1477, 1633 }; // 2nd tone
	char pad[4][4] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
	int tone1;
	int tone2;
	int row;
	int col;
	int output;
	for(n=0;n<n_tones;n++) {
		digit = keys[n];
		/* TODO 4. Complete the DTMF algorithm to generate audio signal based on the digits */
		/* ========================= */

		for (row = 0; row < 4; row++) {
		    for (col = 0; col < 4; col++) {
		      // If the element matches the target digit, return its position
		      if (pad[row][col] == digit) {
		        tone1 = frow[row];	//get tone1 from the position
		        tone2 = fcol[col];	//get tone2 from the position	

		      }
		    }
		  }
		for (i=0; i<5000; i++){
			output = (int) (16384.0*sin(2.0*PI*tone1*i/fs) + 16384.0*sin(2.0*PI*tone2*i/fs));
			buffer[n*samples_per_tone+i] = output; 	// 5000 samples per tone
		}


		/* buffer[..] = ... */
		/* ========================= */

}

	/* Writing the data to a wav file */

	FILE* fp = fopen("../answer.wav", "wb");
	int datasize = samples_total*2;
	int filesize = 36+datasize;
	int headersize = 16;
	int datatype = 1;
	int nchannel = 1;
	int byterate = fs*2;
	int align = 2;
	int bitpersample = 16;

	fwrite("RIFF", 1, 4, fp);
	fwrite(&filesize, 4, 1, fp);
	fwrite("WAVE", 1, 4, fp);
	fwrite("fmt ", 1, 4, fp);
	fwrite(&headersize, 4, 1, fp);
	fwrite(&datatype, 2, 1, fp);
	fwrite(&nchannel, 2, 1, fp);
	fwrite(&fs, 4, 1, fp);
	fwrite(&byterate, 4, 1, fp);
	fwrite(&align, 2, 1, fp);
	fwrite(&bitpersample, 2, 1, fp);
	fwrite("data", 1, 4, fp);
	fwrite(&datasize, 4, 1, fp);
	fwrite(buffer, 2, samples_total, fp);
	fclose(fp);
}

