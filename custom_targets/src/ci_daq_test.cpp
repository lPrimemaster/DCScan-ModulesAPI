#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main(void)
{
	int         error=0;
	TaskHandle  taskHandle=0;
	TaskHandle  taskHandle2=0;
	int32       read;
	uInt32      data[1000];
	char        errBuff[2048]={'\0'};

	DAQmxErrChk (DAQmxCreateTask("L2",&taskHandle));
	DAQmxErrChk (DAQmxCreateCICountEdgesChan(taskHandle,"PXI1Slot8/ctr0","",DAQmx_Val_Rising,0,DAQmx_Val_CountUp));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"/PXI1Slot8/PFI32",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));
	// DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"OnBoardClock",100000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));
	
	DAQmxErrChk (DAQmxCreateTask("L",&taskHandle2));
	DAQmxErrChk (DAQmxCreateCOPulseChanFreq(taskHandle2,"PXI1Slot8/ctr1","",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,1000.0,0.50));
	DAQmxErrChk (DAQmxCfgImplicitTiming(taskHandle2,DAQmx_Val_ContSamps,1000));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));
	DAQmxErrChk (DAQmxStartTask(taskHandle2));

	printf("Generating pulse train.\n");

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/

	printf("Continuously reading. Press Ctrl+C to interrupt\n");
	while( 1 ) {
		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadCounterU32(taskHandle,1000,10.0,data,1000,&read,NULL));

		printf("Acquired %d samples\n",(int)read);
		printf("%d %d\n", data[0], data[999]);
		fflush(stdout);
	}

Error:
	puts("");
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( taskHandle2!=0 )
	{
		DAQmxStopTask(taskHandle2);
		DAQmxClearTask(taskHandle2);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}
