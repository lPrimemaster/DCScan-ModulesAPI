#include <stdio.h>
#include <iostream>
#include <chrono>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main()
{
	int         error=0;
	TaskHandle  taskHandle=0;
	uInt32      data;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("", &taskHandle));
	DAQmxErrChk (DAQmxCreateCICountEdgesChan(taskHandle, "PXI_Slot2/ctr0", "", DAQmx_Val_Rising, 0, DAQmx_Val_CountUp));
    DAQmxErrChk (DAQmxSetCICountEdgesTerm(taskHandle, "PXI_Slot2/ctr0", "/PXI_Slot2/PFI0"));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

    auto start = std::chrono::steady_clock::now();

    FILE* f = fopen("DAQ_CountFailureNoBuffer_5hours.csv", "w");
    fputs("Time, SDiff Mean, Failure Ratio (ppm), Failures, Cycles", f);

	while( std::chrono::steady_clock::now() - start < std::chrono::hours(5) ) {
		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadCounterScalarU32(taskHandle, 10.0, &data, NULL));

        static uInt32 last = 0;
        static uInt32 cycles = 0;
        static uInt32 overflows = 0;

        if(std::chrono::steady_clock::now() - start == std::chrono::hours(1))
        {
            char buffer[256];
            sprintf(buffer, "Time, SDiff Mean, Failure Ratio (ppm), Failures, Cycles");
        }

        if(data - last > 1)
        {
            overflows++;
            cycles++;
            printf("Count overflow [size diff = %d]... Failure ratio = %u/%u = %f ppm.\n", 
                data - last, overflows, cycles, 
                (float)overflows/cycles * 1000000.0);

            

            DAQmxErrChk (DAQmxReadCounterScalarU32(taskHandle, 10.0, &data, NULL));
        }
        else if(data - last == 1)
        {
            cycles++;
        }


        last = data;

		//printf("\rCount: %u", (unsigned int)data);
		//fflush(stdout);
	}

    fclose(f);

Error:
	puts("");
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n", errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}