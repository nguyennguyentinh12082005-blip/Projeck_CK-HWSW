

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <xtime_l.h> // Timer for execution time calculations
#include "xaxidma.h"
#include "xparameters.h"
#define FFT_Size 8
#define DMA_IDLE_MASK 0x00000002
#define DMA_MM2S_RegOffset 0x04
#define DMA_S2MM_RegOffset 0x34

const float complex twiddle_factors[FFT_Size/2] = {1-0*I, 0.7071067811865476-0.7071067811865475*I, 0.0-1*I,  -0.7071067811865475-0.7071067811865476*I};

// This function reorders the input to get the output in the normal order
// Refer the handout for the desired input order
const int input_reorder[FFT_Size] = {0, 4, 2, 6, 1, 5, 3, 7};
void InputReorder(float complex dataIn[FFT_Size], float complex dataOut[FFT_Size])
{
	for (int i = 0 ; i < FFT_Size ; i++)
	{
		dataOut[i] = dataIn[input_reorder[i]];
	}
}

// For FFT of size FFT_Size, the number of butterfly stages are 2^stages = FFT_Size.
// For 8-point FFT, there are three butterfly stages.
void FFTStages(float complex FFT_input[FFT_Size], float complex FFT_output[FFT_Size])
{
	float complex stage1_out[FFT_Size], stage2_out[FFT_Size];
	// Stage 1
	for (int i = 0 ; i < FFT_Size ; i=i+2)
	{
		stage1_out[i] = FFT_input[i] + FFT_input[i+1];
		stage1_out[i+1] = FFT_input[i] - FFT_input[i+1];
	}
	// Stage 2
	for (int i = 0 ; i < FFT_Size ; i=i+4)
	{
		for (int j = 0; j < 2; ++j)
		{
			stage2_out[i+j] = stage1_out[i+j] + twiddle_factors[2*j]*stage1_out[i+j+2];
			stage2_out[i+2+j] = stage1_out[i+j] - twiddle_factors[2*j]*stage1_out[i+j+2];
		}

	}
	// Stage 3
	for (int i = 0 ; i < FFT_Size/2 ; i++)
	{
		FFT_output[i] = stage2_out[i] + twiddle_factors[i]*stage2_out[i+4];
		FFT_output[i+4] = stage2_out[i] - twiddle_factors[i]*stage2_out[i+4];
	}

}
u32 checkDMAIdle(u32 BaseAddress, u32 offset){
	u32 status = XAxiDma_ReadReg(BaseAddress,offset) & DMA_IDLE_MASK;
	return status;
}

int main()
{
	// For FFT_Size point FFT, define the input.
	// You may modify the code to take the input from user via UART
    float complex FFT_input[FFT_Size] = {11+23*I,32+10*I,91+94*I,15+69*I,47+96*I,44+12*I,96+17*I,49+58*I};
    // FFT PS output will be stored in this variable
    float complex FFT_output[FFT_Size];
    // Variable for intermediate outputs
    float complex FFT_rev[FFT_Size];

    // FFT PL output will be stored in this variable
        float complex FFT_output_PL[FFT_Size];
    int status;
    // to store the time at which certain processes starts and ends
    XTime time_PS_start , time_PS_end;
    XTime time_PL_start , time_PL_end;

    // Print the FFT input on the UART
    printf("\n FFT input: \r\n");
	  for (int i = 0 ; i < FFT_Size ; i++)
	  {
		  printf("%f %f\n", crealf(FFT_input[i]), cimagf(FFT_input[i]));
	  }

	 XTime_SetTime(0);
     XTime_GetTime(&time_PS_start); // Capture the timer value at the start
    // As discussed in the handout, FFT involves two tasks:
    // 1) Reorder of the inputs to get output in the normal order
    // 2) Multiplications using multi-stage butterfly approach
	InputReorder(FFT_input, FFT_rev); // Task 1
    FFTStages(FFT_rev, FFT_output); // Task 2
    XTime_GetTime(&time_PS_end); // Capture the timer value at the STOP
    XAxiDma_Config *DMA_confptr;
	XAxiDma AxiDMA;

    DMA_confptr = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
    status = XAxiDma_CfgInitialize(&AxiDMA,DMA_confptr);

    if(status != XST_SUCCESS){
    	printf("DMA Initialization Failed\r\n");
    	return XST_FAILURE;
    }

    XTime_SetTime(0);
    XTime_GetTime(&time_PL_start);

    Xil_DCacheFlushRange ((UINTPTR) FFT_input, (sizeof(float complex) *FFT_Size)); //cacheflu
    Xil_DCacheFlushRange ((UINTPTR) FFT_output_PL, (sizeof(float complex) *FFT_Size));

    status = XAxiDma_SimpleTransfer(&AxiDMA,(UINTPTR) FFT_output_PL, (sizeof(float complex) *FFT_Size ), XAXIDMA_DEVICE_TO_DMA);
    status = XAxiDma_SimpleTransfer(&AxiDMA,(UINTPTR) FFT_input, (sizeof(float complex) *FFT_Size ), XAXIDMA_DMA_TO_DEVICE);


    status= checkDMAIdle(XPAR_AXIDMA_0_BASEADDR, DMA_MM2S_RegOffset);
    while(status != DMA_IDLE_MASK)
    {
    	status= checkDMAIdle(XPAR_AXIDMA_0_BASEADDR, DMA_MM2S_RegOffset);
    }

    status= checkDMAIdle(XPAR_AXIDMA_0_BASEADDR, DMA_S2MM_RegOffset);
        while(status != DMA_IDLE_MASK)
        {
        	status= checkDMAIdle(XPAR_AXIDMA_0_BASEADDR, DMA_S2MM_RegOffset);
        }
        XTime_GetTime(&time_PL_end);



     ///////////////////////////////////////
        //compare ps and pl
        // Compare PS and PL output
        int j;
        int err_flag = 0; //error flag to check for mismatch between transmitted and received data
        for (j=0 ; j < FFT_Size; j++)
        {
        	printf("PS Output: %f + I%f , PL output: %f + I%f \n",
        	       crealf(FFT_output[j]), cimagf(FFT_output[j]),
        	       crealf(FFT_output_PL[j]), cimagf(FFT_output_PL[j]));
        //printing the output of PS and PL
        float diff1 = abs (crealf(FFT_output[j]) - crealf (FFT_output_PL[j]));
        float diff2 = abs(cimagf(FFT_output[j]) - cimagf(FFT_output_PL[j]));

        if (diff1 >= 0.0001 && diff2 >= 0.0001)
        {
        err_flag = 1; //error flag asserted for any mismatch
        break;
        }
        }

        if(err_flag == 1)
        	printf("Data Mismatch found at %d \r\n",j);
        else
        	printf("Functionality is verified successfully \r\n");



    // Print the FFT output on the UART


    printf("\n-----------EXECUTION TIME--------------------\n");
    float time_processor = 0;
	time_processor = (float)1.0 * (time_PS_end - time_PS_start) / (COUNTS_PER_SECOND/1000000);
	printf("Execution Time for PS in Micro-Seconds : %f\n" , time_processor);

	float time_PL = 0;
		time_PL = (float)1.0 * (time_PL_end - time_PL_start) / (COUNTS_PER_SECOND/1000000);
		printf("Execution Time for PL in Micro-Seconds : %f\n" , time_PL);

    // Modify this code for large size FFT
    // How you can generalize the code for any FFT size (limited to power of two)
    // Receive the FFT size and FFT input from User
}


// Equivalent Matlab Code for FFT
// clc;
// clear;
// FFT_Size =8;
// input_real =[11,32,91,15,47,44,96,49];
// input_imag =[23,10,94,69,96,12,17,58];
// FFT_input =complex(input_real,input_imag);
// FFT_out=fft(FFT_input,FFT_Size);
// disp(real(FFT_out));
// disp(imag(FFT_out));
