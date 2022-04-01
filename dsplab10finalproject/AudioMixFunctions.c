/*
 * AudioMixFunctions.c
 *
 *  Created on: Oct 20, 2021
 *      Author: Roehl
 */

#include <F28x_Project.h>
#include <F2837xD_Device.h>
#include "SWandLED.h"
#include "SRAMviaSPI.h"
#include "AIC23.h"
#include "InitAIC23.h"

volatile uint32_t circBuffPtr = 0; //& with mask then multiply by 2 to get SRAM address
uint32_t mask = 0x3FFFF;

volatile uint32_t mixAudioPtr = 0;
bool recordFlag = 0;
bool mixAudioFlag = 0;
int16_t storedAudio;
void Record(int16_t audio) {    //three SRAM read/writes may be too slow for MixAudio and Playback to run simultaneously
    if(mixAudioFlag) {
        storedAudio = (int16_t)(SRAM_ReadWord((mixAudioPtr & mask)<<1));
        audio = (storedAudio>>1) + (audio>>1);
        SRAM_WriteWord((mixAudioPtr & mask)<<1,audio);//overwrite the old audio with the mixed audio
        mixAudioPtr++;
    }
    else {
        SRAM_WriteWord((circBuffPtr & mask)<<1,audio);
        circBuffPtr++;
    }
}

volatile uint32_t playStartPtr = 0;
bool playbackFlag = 0;
bool interp4xFlag = 0;
bool decima4xFlag = 1;
uint16_t mask4x = 0x3;
uint16_t interp4xPtr = 0;
int16_t sample1,sample2;
void Playback() {
    if(interp4xFlag) {
        if(!(interp4xPtr & mask4x)) {
            sample1 = (int16_t)(SRAM_ReadWord((playStartPtr & mask)<<1));
            sample2 = (int16_t)(SRAM_ReadWord(((playStartPtr+1) & mask)<<1));
            McbspbRegs.DXR2.all = sample1;
        }
        else {
            McbspbRegs.DXR2.all = sample1 + ((sample2-sample1)*(int16_t)(interp4xPtr & mask4x))>>2;
        }
        interp4xPtr++;
        if(!(interp4xPtr & mask4x))
            playStartPtr++;
    }
    else if(decima4xFlag) {
        McbspbRegs.DXR2.all = (int16_t)(SRAM_ReadWord((playStartPtr & mask)<<1));
        playStartPtr += 4;
    }
    else {
        McbspbRegs.DXR2.all = (int16_t)(SRAM_ReadWord((playStartPtr & mask)<<1));
        //McbspbRegs.DXR1.all = 0x0000; //I2S mode
        playStartPtr++;
    }
}

void ZeroBuffer() {
    circBuffPtr = 0;
    playStartPtr = 0;
    mixAudioPtr = 0;
}

void Check_AudioFlags(int32_t sample_L) {
    if(PB1_Pressed()) {
        recordFlag = !recordFlag;
        if(recordFlag)
            LED_Write(0x01);
        else
            LED_Write(0x00);
    }
    if(PB2_Pressed()) {
        LED_Write(0xFF);
        ZeroBuffer();
    }
    if(PB3_Pressed()) {
        LED_Write(0x03);
        playbackFlag = 1;
    }
    mixAudioFlag = DIP0_Read();
    if(mixAudioFlag && recordFlag) {
        LED_Write(0x09);
        if(mixAudioPtr > circBuffPtr)   //if we surpass the length of the stored sound,
            circBuffPtr = mixAudioPtr;  //then we need to update circBuffPtr for proper play back length
    }

    if(recordFlag) {
        //Record(sample_L >> 16); //record upper word (I2S mode)
        Record(sample_L);         //record word (DSP mode)
    }
    if(playbackFlag && (playStartPtr < circBuffPtr)) {  //if there's still more samples to play, then play them.
        Playback();
    }
    if(playbackFlag && (playStartPtr >=circBuffPtr)) {  //if finished playing,
        LED_Write(0x00);
        mixAudioPtr = circBuffPtr;      //if mixed audio is less than stored audio, then we need to reset to mix pointer
        interp4xPtr = 0;                //reset interpolation mask
        playStartPtr = circBuffPtr;     //reset playStart if we passed end during decimation
        playbackFlag = 0;
    }
    if(!playbackFlag) {         //if not playing back, don't play anything
        McbspbRegs.DXR2.all = 0x0000;
        //McbspbRegs.DXR1.all = 0x0000; //I2S mode
    }
}

uint32_t switches;
void Check_SRControl() {
    uint16_t command;
    if(switches != DIP_ReadAll()) {
        switches = DIP_ReadAll();
        if(switches & 0x08) {
            command = CLKsampleratecontrol (SR48);
            SpiTransmit(command);
            SmallDelay();
            interp4xFlag = 0;
            decima4xFlag = 0;
        }
        else if(switches & 0x04) {
            command = CLKsampleratecontrol (SR32);
            SpiTransmit(command);
            SmallDelay();
            interp4xFlag = 1;
            decima4xFlag = 0;
        }
        else if(switches & 0x02) {
            command = CLKsampleratecontrol (SR8);
            SpiTransmit(command);
            SmallDelay();
            interp4xFlag = 0;
            decima4xFlag = 1;
        }
    }
}
