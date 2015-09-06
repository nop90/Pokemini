/* Porting to 3DS by Nop90 - adapted from PSP and NDS versions*/

/*
  PokeMini - Pokémon-Mini Emulator
  Copyright (C) 2009-2015  JustBurn

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <3ds.h>
#include <sf2d.h>


#include "PokeMini.h"
#include "Hardware.h"
#include "Joystick.h"

#include "UI.h"
#include "Video_x1.h"
#include "Video_x2.h"
#include "PokeMini_BG3.h"
#include "PokeMini_3DS.h"
#include "PokeMini_3DS_sound.h"

#define RUMBLEOFFSET	8

const char *AppName = "PokeMini " PokeMini_Version " 3DS";

FS_archive sdmcArchive;
Handle dirHandle;

// For the emulator loop and video
int emurunning = 1;
int pm_offsetX,pm_offsetY;

void setup_screen();
sf2d_texture *screentex1, *screentex2;
int PixPitch,PixPitchUI;
float scaleX, scaleY,scaleX2,scaleY2;

int zoom_old;

const char *clc_zoom_txt[] = {
	"0x (Illegal)",
	"1x ( 96x 64)",
	"1x Fit Height",
	"1x Stretch ",
	"2x (192x128)",
	"2x Fit Height",
	"2x Stretch",
};


// Joystick names and mapping (NEW IN 0.5.0)
char *Joy_KeysNames[] = {
	"Off",		// -1
	"Select",	// 0
	"Start",	// 1
	"Up",		// 2
	"Down",		// 3
	"Left",		// 4
	"Right",	// 5
	"A",		// 6
	"B",		// 7
	"X",		// 8
	"Y",		// 9
	"L",		// 10
	"R"			// 11
};
int Joy_KeysMapping[] = {
	0,		// Menu
	6,		// A
	7,		// B
	8,		// C
	2,		// Up
	3,		// Down
	4,		// Left
	5,		// Right
	1,		// Power
	10		// Shake
};

// Custom command line (NEW IN 0.5.0)
int clc_zoom = 6;
int clc_displayfps = 0;

const TCommandLineCustom CustomConf[] = {
	{ "zoom", &clc_zoom, COMMANDLINE_INT, 1, 4 },
	{ "displayfps", &clc_displayfps, COMMANDLINE_BOOL },
	{ "", NULL, COMMANDLINE_EOL }
};

// Platform menu (REQUIRED >= 0.4.4)
int UIItems_PlatformC(int index, int reason);
TUIMenu_Item UIItems_Platform[] = {
	PLATFORMDEF_GOBACK,
	{ 0,  1, "Zoom: %s", UIItems_PlatformC },
	{ 0,  2, "Display FPS: %s", UIItems_PlatformC },
	{ 0,  9, "Define Joystick...", UIItems_PlatformC },
	PLATFORMDEF_SAVEOPTIONS,
	PLATFORMDEF_END(UIItems_PlatformC)
};
int UIItems_PlatformC(int index, int reason)
{
	int zoomchanged = 0;
	if (reason == UIMENU_OK) {
		reason = UIMENU_RIGHT;
	}
	if (reason == UIMENU_CANCEL) {
		UIMenu_PrevMenu();
	}
	if (reason == UIMENU_LEFT) {
		switch (index) {
			case 1: // Zoom
				clc_zoom--;
				if (clc_zoom < 1) clc_zoom = 6;
				zoomchanged = 1;
				break;
			case 2: // Display FPS
				clc_displayfps = !clc_displayfps;
				break;
		}
	}
	if (reason == UIMENU_RIGHT) {
		switch (index) {
			case 1: // Zoom
				clc_zoom++;
				if (clc_zoom > 6) clc_zoom = 1;
				zoomchanged = 1;
				break;
			case 2: // Display FPS
				clc_displayfps = !clc_displayfps;
				break;
			case 9: // Define joystick
				JoystickEnterMenu();
				break;
		}
	}
	UIMenu_ChangeItem(UIItems_Platform, 1, "Zoom: %s", clc_zoom_txt[clc_zoom]);
	UIMenu_ChangeItem(UIItems_Platform, 2, "Display FPS: %s", clc_displayfps ? "Yes" : "No");
	if (zoomchanged) setup_screen();
	return 1;
}

// Setup screen
void setup_screen()
{
	TPokeMini_VideoSpec *videospec;

	if (clc_zoom == 1) {
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video1x1;
		pm_offsetX = 152;
		pm_offsetY = 88;
		scaleX = 1;
		scaleY = 1;
	} else if (clc_zoom == 2) {
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video1x1;
		pm_offsetX = 20;
		pm_offsetY = 0;
		scaleY = 240.0 / 64;
		scaleX = scaleY;
	} else if (clc_zoom == 3) {
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video1x1;
		pm_offsetX = 0;
		pm_offsetY = 0;
		scaleX = 400.0 / 96;
		scaleY = 240.0 / 64;
	} else if (clc_zoom == 4) {
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video2x2;
		pm_offsetX = 104;
		pm_offsetY = 56;
		scaleX = 1;
		scaleY = 1;
	} else if (clc_zoom == 5) {
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video2x2;
		pm_offsetX = 20;
		pm_offsetY = 0;
		scaleY = 240.0 / 128;
		scaleX = scaleY;
	} else {
		clc_zoom = 6;
		videospec = (TPokeMini_VideoSpec *)&PokeMini_Video2x2;
		pm_offsetX = 0;
		pm_offsetY = 0;
		scaleX = 400.0 / 192;
		scaleY = 240.0 / 128;
	}

	// Set video spec and check if is supported
	if (!PokeMini_SetVideo(videospec, 32, CommandLine.lcdfilter, CommandLine.lcdmode)) {
		PokeDPrint(POKEMSG_ERR, "Couldn't set video spec\n");
		exit(1);
	}
}

void system_checkPolls() {
    APP_STATUS status;

	while((status=aptGetStatus()) != APP_RUNNING) {

        if(status == APP_SUSPENDING)
        {
            aptReturnToMenu();
        }
        else if(status == APP_PREPARE_SLEEPMODE)
        {
			aptSignalReadyForSleep();
            aptWaitStatusEvent();
        }
        else if (status == APP_SLEEPMODE) {
        }
        else if (status == APP_EXITING) {
			emurunning = 0;
        }

    }
}

// Handle keyboard and quit events
void handleevents()
{
    hidScanInput();
    u32 keydown = hidKeysDown();
		if (keydown & KEY_SELECT){ 
			JoystickButtonsEvent(0, 1);
		} else if (keydown & KEY_START){ 
			JoystickButtonsEvent(1, 1);
		} else if (keydown & KEY_UP){ 
			JoystickButtonsEvent(2, 1);
		} else if (keydown & KEY_DOWN){ 
			JoystickButtonsEvent(3, 1);
		} else if (keydown & KEY_LEFT){ 
			JoystickButtonsEvent(4, 1);
		} else if (keydown & KEY_RIGHT){ 
			JoystickButtonsEvent(5, 1);
		} else if (keydown & KEY_A){ 
			JoystickButtonsEvent(6, 1);
		} else if (keydown & KEY_B){ 
			JoystickButtonsEvent(7, 1);
		} else if (keydown & KEY_X){ 
			JoystickButtonsEvent(8, 1);
		} else if (keydown & KEY_Y){ 
			JoystickButtonsEvent(9, 1);
		} else if (keydown & KEY_L){ 
			JoystickButtonsEvent(10, 1);
		} else if (keydown & KEY_R){
			JoystickButtonsEvent(11, 1);
		}

    u32 keyup = hidKeysUp();
		if (keyup & KEY_SELECT){ 
			JoystickButtonsEvent(0, 0);
		} else if (keyup & KEY_START){
			JoystickButtonsEvent(1, 0);
		} else if (keyup & KEY_UP){ 
			JoystickButtonsEvent(2, 0);
		} else if (keyup & KEY_DOWN){ 
			JoystickButtonsEvent(3, 0);
		} else if (keyup & KEY_LEFT){ 
			JoystickButtonsEvent(4, 0);
		} else if (keyup & KEY_RIGHT){ 
			JoystickButtonsEvent(5, 0);
		} else if (keyup & KEY_A){ 
			JoystickButtonsEvent(6, 0);
		} else if (keyup & KEY_B){ 
			JoystickButtonsEvent(7, 0);
		} else if (keyup & KEY_X){ 
			JoystickButtonsEvent(8, 0);
		} else if (keyup & KEY_Y){ 
			JoystickButtonsEvent(9, 0);
		} else if (keyup & KEY_L){ 
			JoystickButtonsEvent(10, 0);
		} else if (keyup & KEY_R){ 
			JoystickButtonsEvent(11, 0);
		}
	system_checkPolls();
}

// Used to fill the sound buffer
//void emulatorsound(void *unused, u8 *stream, int len)
//{
//	MinxAudio_GetSamplesS16((int16_t *)stream, len>>1);
//}

// Enable / Disable sound
void enablesound(int sound)
{
	MinxAudio_ChangeEngine(sound);
	if (pm_3ds_sound_getstate()) pm_3ds_sound_pause();
}

void Screen_Flip(void) {

    GSPGPU_FlushDataCache(NULL, (u8*)screentex1->data, 128*256*4);

	screentex1->tiled = 0;
    sf2d_texture_tile32(screentex1);
        
    sf2d_start_frame(GFX_TOP, GFX_LEFT);
	
    switch (clc_zoom) {
		case 1:
			sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, 1, 1); 
			break;
		case 2:
			sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, scaleY, scaleY); 
			break;
		case 3:
			sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, scaleX, scaleY); 
			break;
		case 4:
			sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, 1, 1); 
			break;
		case 5:
			sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, scaleY, scaleY); 
			break;
		case 6:
		default:
			sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, scaleX, scaleY); 
			break;
	}
	
    sf2d_end_frame();
    gfxFlushBuffers();

    sf2d_swapbuffers();
}

void Screen_FlipUI(void) {
	
	GSPGPU_FlushDataCache(NULL, (u8*)screentex2->data, 512*256*4);

	screentex2->tiled = 0;
    sf2d_texture_tile32(screentex2);
        
    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_draw_texture_rotate_cut_scale(screentex2, 0, 0, 0, 0, 0, 288, 192, scaleX2, scaleY2); 
    sf2d_end_frame();
    gfxFlushBuffers();

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
	
	if(zoom_old<4){
		switch (clc_zoom) {
			case 1:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, 1, 1); 
				break;
			case 2:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, scaleY, scaleY); 
				break;
			case 3:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, scaleX, scaleY); 
				break;
			case 4:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, 2, 2); 
				break;
			case 5:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, scaleY*2, scaleY*2); 
				break;
			case 6:
			default:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 96, 64, scaleX*2, scaleY*2); 
				break;
		}
	} else {
		switch (clc_zoom) {
			case 1:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, 0.5, 0.5); 
				break;
			case 2:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, scaleY/2, scaleY/2); 
				break;
			case 3:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, scaleX/2, scaleY/2); 
				break;
			case 4:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, 1, 1); 
				break;
			case 5:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, scaleY, scaleY); 
				break;
			case 6:
			default:
				sf2d_draw_texture_rotate_cut_scale(screentex1, pm_offsetX, pm_offsetY, 0, 0, 0, 192, 128, scaleX, scaleY); 
				break;
		}
	}

    sf2d_end_frame();
    gfxFlushBuffers();

    sf2d_swapbuffers();
}


// Menu loop
void menuloop()
{

	// Stop sound
	enablesound(0);

	// Update EEPROM
	PokeMini_SaveFromCommandLines(0);

	// Menu's loop
	while (emurunning && (UI_Status == UI_STATUS_MENU)) {

		// Process UI
		UIMenu_Process();

		// Screen rendering
			// Render the menu or the game screen
			UIMenu_Display_32(screentex2->data, PixPitchUI);
			Screen_FlipUI();

		// Handle events
		handleevents();
	}

	// Apply configs
	PokeMini_ApplyChanges();
	if (UI_Status == UI_STATUS_EXIT) emurunning = 0;
	else enablesound(CommandLine.sound);
}

// Main function
int main()
{
	char fpstxt[16];
	char buffer[PMTMPV];

	u8  isN3DS = 0;

	aptOpenSession();
	APT_SetAppCpuTimeLimit(NULL, 30); // enables syscore usage
	aptCloseSession();

	APT_CheckNew3DS(NULL, &isN3DS);

    sdmcArchive = (FS_archive){ARCH_SDMC, (FS_path){PATH_EMPTY, 1, (u8*)""}};
    FSUSER_OpenArchive(NULL, &sdmcArchive);

	printf("%s\n\n", AppName);

	strncpy(buffer, "sdmc:/Pokemini", PMTMPV-1);
	
	chdir(buffer);
	PokeMini_GetCurrentDir();
	strcpy(PokeMini_ExecDir, PokeMini_CurrDir);

	PokeMini_GetCurrentDir();
	
	CommandLineInit();

	if (isN3DS) 
		CommandLine.synccycles = 8; // best performance on n3ds
	else
		CommandLine.synccycles = 16; // default for o3DS to reach 72fps
		
	CommandLineConfFile("pokemini.cfg", NULL, NULL);
	JoystickSetup("3DS", 0, 30000, Joy_KeysNames, 12, Joy_KeysMapping);

	// Initialize GPU lib
    sf2d_init();

    sf2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xff));
    sf2d_set_vblank_wait(1);

	// Initialize the display

	setup_screen();

	UIMenu_SetDisplay(288, 192, PokeMini_RGB32, (u8 *)PokeMini_BG3, (uint16_t *)PokeMini_BG3_PalRGB16, (uint32_t *)PokeMini_BG3_PalRGB32);
	scaleX2 = 320.0 / 288;
	scaleY2 = 240.0 / 192;

    screentex1 = sf2d_create_texture(192, 128, TEXFMT_RGBA8, SF2D_PLACE_RAM);
    screentex2 = sf2d_create_texture(400, 256, TEXFMT_RGBA8 , SF2D_PLACE_RAM);
	
	PixPitch = 256;
	PixPitchUI = 512;
	
	// Initialize the 3ds sound 
	
	pm_3ds_sound_init();

	// Initialize the emulator
	printf("Starting emulator...\n");
	if (!PokeMini_Create(0, PMSOUNDBUFF)) {
		printf("Error while initializing emulator.\n");
	}

	// Setup palette and LCD mode
	PokeMini_VideoPalette_Init(PokeMini_RGB16, 1);
	PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
	PokeMini_ApplyChanges();

	// Load stuff
	PokeMini_UseDefaultCallbacks();
	if (!PokeMini_LoadFromCommandLines("Using FreeBIOS", "EEPROM data will be discarded!")) {
		UI_Status = UI_STATUS_MENU;
	}

	// Enable sound & init UI
	printf("Starting emulator...\n");
	UIMenu_Init();
	enablesound(CommandLine.sound);

	// initialize timers
	u64 tickcurr;
	u64 fpsticknext,frmticknext,frmtickold;
	
	float deltafrmtick;

	int fps = 72, fpscnt = 0;
	strcpy(fpstxt, "");
	tickcurr=svcGetSystemTick();
	fpsticknext = tickcurr + TICKS_PER_SEC;
	frmticknext = tickcurr + TICKS_PER_FRAME;
	deltafrmtick = (float)tickcurr + TICKS_PER_FRAME - frmticknext;
	
		// Emulator's loop
	while (emurunning) {

		// Handle events
		handleevents();

		PokeMini_EmulateFrame();
		PokeMini_EmulateFrame(); // Skip 1 frame rendering: for performance on Old 3DS and to reach 72 emulated FPS on New 3ds (vsync limit speed to 60 rendered FPS)

		if (pm_3ds_sound_getstate())
			pm_3ds_sound_callback(SOUND_SAMPLES_PER_FRAME);  
		else
			pm_3ds_sound_start(SOUND_FREQUENCY,SOUND_SAMPLES_PER_FRAME);  

		// Screen rendering: render the game screen
		if (PokeMini_Rumbling) {
			PokeMini_VideoBlit(screentex1->data + RUMBLEOFFSET, PixPitch);
		} else {
			PokeMini_VideoBlit(screentex1->data, PixPitch);
		}

		// Display FPS counter
		if (clc_displayfps) {
			UIDraw_String_32((uint32_t *)screentex1->data, PixPitch, 4, 4, 10, fpstxt, UI_Font1_Pal32);
		}

		LCDDirty = 0;
		Screen_Flip();

		svcSleepThread((float)(frmticknext - svcGetSystemTick()) / TICKS_PER_NSEC);  // sync to 72 FPS

		tickcurr=svcGetSystemTick();
		frmtickold = frmticknext;
		frmticknext = frmticknext + deltafrmtick + TICKS_PER_FRAME;
		deltafrmtick = (float)frmtickold + deltafrmtick + TICKS_PER_FRAME - (float)frmticknext;
		fpscnt++;
		if (tickcurr >= fpsticknext) {
			fpsticknext = fpsticknext + TICKS_PER_SEC;
			fps = fpscnt;
			fpscnt = 0;
			sprintf(fpstxt, "%i FPS", fps*2);
		} 


		zoom_old = clc_zoom;
		// Menu
		if (UI_Status == UI_STATUS_MENU){

			menuloop();

			// tricky way to turn black the two bottom framebuffer while running the rom without 
			// having to draw on them at every frame (for performance)
			// may not work with CIA going to menu and returning to program (framebuffer addresses may change)
			sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
			sf2d_draw_rectangle(0,0,288,192,0x000000ff);
			sf2d_end_frame();
			gfxFlushBuffers();
			sf2d_swapbuffers();
			sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
			sf2d_draw_rectangle(0,0,288,192,0x000000ff);
			sf2d_end_frame();
			gfxFlushBuffers();
			sf2d_swapbuffers();
			
			// clear timers
			strcpy(fpstxt, "");
			fpscnt = 0;
			tickcurr=svcGetSystemTick();
			fpsticknext = tickcurr + TICKS_PER_SEC;
			frmticknext = tickcurr + TICKS_PER_FRAME;
			deltafrmtick = (float)tickcurr + TICKS_PER_FRAME - frmticknext;
		}
	}

	// Disable sound & free UI
	enablesound(0);
	UIMenu_Destroy();

	// Save Stuff
	PokeMini_SaveFromCommandLines(1);

	// Terminate...
	printf("Shutdown emulator...\n");
	pm_3ds_sound_quit();
	PokeMini_VideoPalette_Free();
	PokeMini_Destroy();
    sf2d_free_texture(screentex1);
    sf2d_free_texture(screentex2);
    sf2d_fini();
	aptExit();

	return 0;
}

