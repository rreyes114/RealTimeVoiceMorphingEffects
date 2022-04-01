//DSP Lab 10

#include <F28x_Project.h>
#include <F2837xD_Device.h>
#include "SWandLED.h"
#include "OneToOneI2CDriver.h"
#include "LCDviaI2C.h"
#include "SPIDriver.h"
#include "SRAMviaSPI.h"
//#include "Timer1andADCA.h"
#include "AIC23.h"
#include "InitAIC23.h"
//#include "AudioMixFunctions.h"
//#include "RFFTSetup.h"
#include "CFFTSetup.h"
//#include "RFFTandICFFT.h"
#include <math.h>
#define PI2 2*3.14596f
float32 w1, w2;

#include <stdlib.h>
#include <time.h>

//declare global variables
#define SAMP_FREQ 48000
int16_t sample_L, sample_R, sample_avg;
int16_t sample_out, sample_out2;
char* mode0 = "None";
char* mode1 = "Robot";
char* mode2 = "Chorus";
char* mode3 = "Pitch: ";

//internal SRAM circular buffer
#define BUFF_SIZE CFFT_SIZE*2
//#define BUFF_SIZE RFFT_SIZE
#define BUFF_MASK BUFF_SIZE-1
#define BIN_SIZE SAMP_FREQ/BUFF_SIZE

#pragma DATA_SECTION(rec1Buff,"ramgs0")
float rec1Buff[BUFF_SIZE];
#pragma DATA_SECTION(rec2Buff,"ramgs1")
float rec2Buff[BUFF_SIZE];
#pragma DATA_SECTION(rec3Buff,"ramgs2")
float rec3Buff[BUFF_SIZE];
#pragma DATA_SECTION(rec4Buff,"ramgs3")
float rec4Buff[BUFF_SIZE];

float32* x_samples = &CFFTin2Buff[0];
float32* x_next = &CFFTin1Buff[0];
float32* y_output = &rec2Buff[0];
float32* y_next = &rec1Buff[0];

float32* x_samples2 = &CFFTin4Buff[0]; //2nd set used for overlap
float32* x_next2 = &CFFTin3Buff[0];
float32* y_output2 = &rec4Buff[0];
float32* y_next2 = &rec3Buff[0];

float32* temptr; //used for swapping the pointers

uint16_t ptr = 0;
uint16_t ptr2 = BUFF_SIZE/2;

//int16_t output_monitor[BUFF_SIZE];

interrupt void Mcbsp_RxINTB_ISR(void) {
    sample_L = McbspbRegs.DRR2.all;     //read high word
    sample_R = McbspbRegs.DRR1.all;     //read high word
    sample_avg = (sample_L>>1) + (sample_R>>1);

    w1 = 0.5f * (1 - cosf(PI2*(float32)(ptr) / ((BUFF_SIZE - 1))));
    w2 = 0.5f * (1 - cosf(PI2*(float32)(ptr2) / ((BUFF_SIZE - 1))));

    x_samples[ptr&BUFF_MASK] = w1*(float32)(sample_avg); //store sample in buffer
    x_samples2[ptr2&BUFF_MASK] = w2*(float32)(sample_avg); //store sample in buffer

    sample_out = (int16_t)y_output[ptr&BUFF_MASK];
    sample_out2 = (int16_t)y_output2[ptr2&BUFF_MASK];

    //output_monitor[ptr&BUFF_MASK] = sample_out + sample_out2;

    ptr++;
    ptr2++;

    McbspbRegs.DXR2.all = sample_out + sample_out2;
    McbspbRegs.DXR1.all = sample_out + sample_out2;
    //McbspbRegs.DXR1.all = sample_avg; //use this line ONLY for debugging purposes
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP6; //documentation says to clear, not set, so idk why it is like this
}

void McbspISR_Init() {
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;              //enable the PIE
    PieVectTable.MCBSPB_RX_INT = &Mcbsp_RxINTB_ISR; //write ISR vector to vector table
    PieCtrlRegs.PIEIER6.bit.INTx7 = 1;              //set the PIEIER6.7 bit
    IER |= 0x20;                                    //Set CPU IER bit for group 6
}

#pragma DATA_SECTION(magBuff,"ramgs0")
float magBuff[BUFF_SIZE];
#pragma DATA_SECTION(phsBuff,"ramgs1")
float phsBuff[BUFF_SIZE];

void downSample(float32 factor) {
    uint16_t maxIndex = 0;
    for(float32 i = 0.0f; i < (float32)BUFF_SIZE; i+=factor) {  //grab every N index
        magBuff[maxIndex] = magBuff[(uint16_t)i];
        phsBuff[maxIndex] = phsBuff[(uint16_t)i];
        maxIndex++;
    }
    for(uint16_t i = maxIndex+1; i < BUFF_SIZE; i++) {  //zero out rest of buffer
        magBuff[i] = 0.0f;
        phsBuff[i] = 0.0f;
    }
}

#pragma DATA_SECTION(tempMagBuff,"ramgs2")
float tempMagBuff[BUFF_SIZE];
#pragma DATA_SECTION(tempPhsBuff,"ramgs3")
float tempPhsBuff[BUFF_SIZE];
void upSample(float32 factor) {
    for(uint16_t i = 0; i < BUFF_SIZE; i++) {   //zero out temp buffers
        tempMagBuff[i] = 0.0f;
        tempPhsBuff[i] = 0.0f;
    }
    uint16_t maxIndex = 0;
    for(float32 i = 0.0f; i < (float32)BUFF_SIZE/2; i+=factor) {  //space out each sample by N
        tempMagBuff[(uint16_t)i] = magBuff[maxIndex];
        tempPhsBuff[(uint16_t)i] = phsBuff[maxIndex];
        maxIndex++;
    }
    for(uint16_t i = 0; i < BUFF_SIZE/2; i++) {   //interpolate zeros, ignoring ends
        if(tempMagBuff[i] == 0.0f) {
            magBuff[i] = 0.5*(tempMagBuff[i-1] + tempMagBuff[i+1]);
            phsBuff[i] = 0.5*(tempPhsBuff[i-1] + tempPhsBuff[i+1]);
        }
        else {
            magBuff[i] = tempMagBuff[i];
            phsBuff[i] = tempPhsBuff[i];
        }
    }
}

float ratio;
void applyTransform(){
    //calculate FFT
    CFFT_f32t(hnd_cfft);
    CFFT_f32_unpack(hnd_cfft);  //output in CurrentOutPtr

    //calculate magnitude and phase
    temptr = hnd_cfft->CurrentInPtr;            //switch pointers
    hnd_cfft->CurrentInPtr = hnd_cfft->CurrentOutPtr;
    hnd_cfft->CurrentOutPtr = temptr;
    temptr = hnd_cfft->CurrentOutPtr;           //save the output pointer
    hnd_cfft->CurrentOutPtr = &magBuff[0];      //point output to magnitude buffer
    CFFT_f32_mag_TMU0(hnd_cfft);                //calculate magnitude
    hnd_cfft->CurrentOutPtr = &phsBuff[0];      //point output to phase buffer
    CFFT_f32_phase_TMU0(hnd_cfft);              //calculate phase
    hnd_cfft->CurrentOutPtr = temptr;           //return pointer to original output

    //do some processing
    if(PB2_Pressed()) {     //robotization
        for(uint16_t i = 0; i < BUFF_SIZE/2; i++)   //set phase to zero
            phsBuff[i] = 0.0f;
    }
    else if(PB1_Pressed()) {     //chorus effect
        for(uint16_t i = 0; i < BUFF_SIZE/2; i++)   //set phase to random
            phsBuff[i] = ((float)rand()/(float)(RAND_MAX)) * PI2 - PI2/2;
    }
    else if(PB0_Pressed()) {     //time stretch
        ratio = (float)(DIP_ReadAll()>>1)/7.0f + 1.0f;
        if(DIP0_Read())
            upSample(ratio);
        else
            downSample(ratio);
    }

    //reconstruct complex spectrum
    for(uint16_t i = 0; i < BUFF_SIZE/2; i++) {  //reconstruct complex spectrum
        hnd_cfft->CurrentOutPtr[2*i] = magBuff[i]*cosf(phsBuff[i]);
        hnd_cfft->CurrentOutPtr[2*i+1] = magBuff[i]*sinf(phsBuff[i]);
    }

    //calculate the ICFFT
    temptr = hnd_cfft->CurrentInPtr;
    hnd_cfft->CurrentInPtr = hnd_cfft->CurrentOutPtr;
    hnd_cfft->CurrentOutPtr = temptr;
    CFFT_f32_pack(hnd_cfft);
    hnd_cfft->InPtr  = hnd_cfft->CurrentOutPtr;
    hnd_cfft->OutPtr = hnd_cfft->CurrentInPtr;
    ICFFT_f32t(hnd_cfft);       //output in CurrentInPtr
}

int main(void)
{
    InitSysCtrl();  //initialize clocks and set board to run at 200MHz
    DINT;           // Disable interrupts globally
    InitPieCtrl();  //Init PIE
    IER = 0x0000;   // Clear CPU interrupt register
    IFR = 0x0000;   // Clear CPU interrupt flag register
    InitPieVectTable();
    EALLOW;
    McbspISR_Init();    // Initialize McBSP interrupt
    //Timer1_Init();      // Initialize CPU timer 1 interrupt
    //ADCA_Init();        // Initialize ADC A channel 0
    InitSPIA();         //initializes SPIA interface with AIC23
    InitMcBSPb();       //initializes McBsp, enables interrupt on the peripheral
    InitAIC23();        //sends initializing commands to AIC23 via SPI
    LCD_Init();         //initialize I2C to communicate with LCD
    SPI_Master_Init();  //initialize SPIB interface with SRAM
    GPIO_Init();        //initialize GPIO for switches and LEDs
    //DMA_Init();         //initialize DMA to do sampling
    //RFFT_Init();        //initialize RFFT structure
    CFFT_Init();        //initialize CFFT structure
    srand((unsigned int)time(NULL));
    EnableInterrupts(); //Global interrupt enable for PIE and CPU
    while(1){
        if(!(ptr&BUFF_MASK) && ptr > 0) { //if sample buffer full, switch to another and process full buffer
            //point cfft to newly filled input data
            hnd_cfft->InPtr = x_samples;
            hnd_cfft->OutPtr  = CFFToutBuff;

            //switch input pointers
            temptr = x_samples;
            x_samples = x_next;
            x_next = temptr;

            //switch output pointers
            temptr = y_output;
            y_output = y_next;
            y_next = temptr;

            //apply transform
            applyTransform();

            //copy result into next output
            for(uint16_t i = 0; i < BUFF_SIZE; i++)
                y_next[i] = hnd_cfft->CurrentInPtr[i];
        }
        if(!(ptr2&BUFF_MASK) && ptr2 > BUFF_SIZE) { //do the same thing for the overlap buffer
            //point cfft to newly filled input data
            hnd_cfft->InPtr = x_samples2;
            hnd_cfft->OutPtr  = CFFToutBuff;

            //switch input pointers
            temptr = x_samples2;
            x_samples2 = x_next2;
            x_next2 = temptr;

            //switch output pointers
            temptr = y_output2;
            y_output2 = y_next2;
            y_next2 = temptr;

            //apply transform
            applyTransform();

            //copy result into next output
            for(uint16_t i = 0; i < BUFF_SIZE; i++)
                y_next2[i] = hnd_cfft->CurrentInPtr[i];
        }
    }
}
