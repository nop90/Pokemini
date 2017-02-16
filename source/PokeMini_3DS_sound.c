/////////////////////////////////////////////////////////////////////////////////
//                     PokeMini 3DS - A Pokemon Mini Emulator                  //
//							Ported to 3DS by nop90 - 2015                      //
//                       Everything not covered by PokeMini                    //
//                          Copyrights is Public Domain                        //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

//#include <stdlib>
#include <string.h>
//#include <ctime>
//#include <cctype>
//#include <malloc.h>

#include <3ds.h>
#include "PokeMini_3ds_sound.h"
#include "PokeMini.h"


u8 *stream;

int bufferpos=0;

int soundstate=0;

void pm_3ds_sound_callback(int len)
{
int buffertail;
    if(soundstate) {
        if (bufferpos+len<=SOUND_BUFFER_SIZE) {
			MinxAudio_GetSamplesU8(stream+bufferpos, len);  // I call a modded func that return S8 instead of U8
			GSPGPU_FlushDataCache(stream+bufferpos, len);
		} else {
			buffertail = SOUND_BUFFER_SIZE - bufferpos;
			MinxAudio_GetSamplesU8(stream+bufferpos, buffertail);
			MinxAudio_GetSamplesU8(stream, len-buffertail);
			GSPGPU_FlushDataCache(stream+bufferpos, buffertail);
			GSPGPU_FlushDataCache(stream, len-buffertail);
		}
	bufferpos= (bufferpos+len) % (unsigned int) SOUND_BUFFER_SIZE;
    }
}

void pm_3ds_sound_start(int freq, int len)
{
	bufferpos = 0;
	soundstate=1;
	pm_3ds_sound_callback(len*1.2); // Filling first frame with +25% of samples to avoid buffer get empty
	GSPGPU_FlushDataCache(stream, SOUND_BUFFER_SIZE);
	csndPlaySound(0x8, SOUND_REPEAT | SOUND_FORMAT_8BIT, freq, 1.0, 0.0, (u32*)stream, (u32*)stream,  SOUND_BUFFER_SIZE);
}

void pm_3ds_sound_pause(void)
{
	if (soundstate) {
		CSND_SetPlayState(0x8, 0);//Stop audio playback.
		csndExecCmds(0);
		
		soundstate=0;
	}
}

int pm_3ds_sound_getstate(void)
{
	return soundstate;
}

int pm_3ds_sound_init(void)
{
    
	if(csndInit()) return 0;
    
    stream = (u8*)linearAlloc(SOUND_BUFFER_SIZE);
    if (!stream) {
        printf("ERROR : Couldn't malloc stream\n");
        return 0;
    } 
	bufferpos=0;
    return 1;
}

void pm_3ds_sound_quit(void)
{
		CSND_SetPlayState(0x8, 0);//Stop audio playback.
		csndExecCmds(0);

   if (stream) {
		linearFree(stream);
    }
	csndExit();
}
