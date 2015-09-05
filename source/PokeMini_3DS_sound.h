/////////////////////////////////////////////////////////////////////////////////
//                     PokeMini 3DS - A Pokemon Mini Emulator                  //
//							Ported to 3DS by nop90 - 2015                      //
//                       Everything not covered by PokeMini                    //
//                          Copyrights is Public Domain                        //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


#ifndef __PM_3DS_SOUND_H__
#define __PM_3DS_SOUND_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <3ds.h>

// Sound buffer size
#define SOUND_BUFFER_SIZE	2048

u8 *stream;

int  pm_3ds_sound_init(void);
void pm_3ds_sound_quit(void);
void pm_3ds_sound_start(int freq, int len);
void pm_3ds_sound_callback(int len);
void pm_3ds_sound_pause(void);
int  pm_3ds_sound_getstate(void);

#endif