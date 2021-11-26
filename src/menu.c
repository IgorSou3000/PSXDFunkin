/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "menu.h"

#include "mem.h"
#include "main.h"
#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "archive.h"
#include "mutil.h"

#include "font.h"
#include "trans.h"
#include "loadscr.h"

#include "stage.h"
#include "character/gf.h"
#include "character/bfgf.h"

//Menu messages
static const char *funny_messages[][2] = {
	{"PSX PORT BY CUCKYDEV", "YOU KNOW IT"},
	{"PORTED BY IGORSOU3000", "YEAH"},
	{"FUNKIN", "FOREVER"},
	{"WHAT THE HELL", "D SIDE PSX"},
	{"LIKE PARAPPA", "BUT COOLER"},
	{"THE JAPI", "EL JAPI"},
	{"THIS IS", "THE POSSUM EVER"},
	{"OPENGL BACKEND", "BY CLOWNACY"},
	{"CUCKYFNF", "SETTING STANDARDS"},
	{"lool", "inverted colours"},
	{"NEVER LOOK AT", "THE ISSUE TRACKER"},
	{"PSXDEV", "HOMEBREW"},
	{"ZERO POINT ZERO TWO TWO EIGHT", "ONE FIVE NINE ONE ZERO FIVE"},
	{"DOPE ASS GAME", "PLAYSTATION MAGAZINE"},
	{"NEWGROUNDS", "FOREVER"},
	{"NO FPU", "NO PROBLEM"},
	{"OK OKAY", "WATCH THIS"},
	{"ITS MORE MALICIOUS", "THAN ANYTHING"},
	{"USE A CONTROLLER", "LOL"},
	{"SNIPING THE KICKSTARTER", "HAHA"},
	{"SHITS UNOFFICIAL", "NOT A PROBLEM"},
	{"SYSCLK", "RANDOM SEED"},
	{"THEY DIDNT HIT THE GOAL", "STOP"},
	{"FCEFUWEFUETWHCFUEZDSLVNSP", "PQRYQWENQWKBVZLZSLDNSVPBM"},
	{"PSXFUNKIN BY CUCKYDEV", "SUCK IT DOWN"},
	{"PLAYING ON EPSXE HUH", "YOURE THE PROBLEM"},
	{"NEXT IN LINE", "ATARI"},
	{"HAXEFLIXEL", "COME ON"},
	{"HAHAHA", "I DONT CARE"},
	{"GET ME TO STOP", "TRY"},
	{"FNF MUKBANG GIF", "THATS UNRULY"},
	{"OPEN SOURCE", "FOREVER"},
	{"ITS A PORT", "ITS WORSE"},
	{"WOW GATO", "WOW GATO"},
	{"BALLS FISH", "BALLS FISH"},
};

//Menu state
static struct
{
	//Menu state
	u8 page, next_page;
	boolean page_swap;
	u8 select, next_select;
	
	fixed_t scroll;
	fixed_t trans_time;
	
	//Page specific state
	union
	{
		struct
		{
			u8 funny_message;
		} opening;
		struct
		{
			fixed_t logo_bump;
			fixed_t fade, fadespd;
		} title;
		struct
		{
			fixed_t fade, fadespd;
		} story;
	} page_state;
	
	union
	{
		struct
		{
			u8 id, diff;
			boolean story;
		} stage;
	} page_param;
	
	//Menu assets
	Gfx_Tex tex_back, tex_cre0, tex_cre1, tex_cre2, tex_cre3, tex_cre4, tex_cre5, tex_cre6, tex_cre7, tex_cre8, tex_cre9, tex_cre10, tex_cre11, tex_story, tex_title;
	FontData font_bold;
	
	Character *bfgf; //Title Girlfriend
} menu;

//Internal menu functions
char menu_text_buffer[0x100];

static const char *Menu_LowerIf(const char *text, boolean lower)
{
	//Copy text
	char *dstp = menu_text_buffer;
	if (lower)
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'A' && *srcp <= 'Z')
				*dstp++ = *srcp | 0x20;
			else
				*dstp++ = *srcp;
		}
	}
	else
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'a' && *srcp <= 'z')
				*dstp++ = *srcp & ~0x20;
			else
				*dstp++ = *srcp;
		}
	}
	
	//Terminate text
	*dstp++ = '\0';
	return menu_text_buffer;
}

static void Menu_DrawBack(boolean flash, s32 scroll, u8 r0, u8 g0, u8 b0, u8 r1, u8 g1, u8 b1)
{
	RECT back_src = {0, 0, 255, 255};
	RECT back_dst = {0, -scroll - SCREEN_WIDEADD2, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};
	
	if (flash || (animf_count & 4) == 0)
		Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r0, g0, b0);
	else
		Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r1, g1, b1);
}

static void Menu_DifficultySelector(s32 x, s32 y)
{
	//Change difficulty
	if (menu.next_page == menu.page && Trans_Idle())
	{
		if (pad_state.press & PAD_LEFT)
		{
			if (menu.page_param.stage.diff > StageDiff_Easy)
				menu.page_param.stage.diff--;
			else
				menu.page_param.stage.diff = StageDiff_Hard;
		}
		if (pad_state.press & PAD_RIGHT)
		{
			if (menu.page_param.stage.diff < StageDiff_Hard)
				menu.page_param.stage.diff++;
			else
				menu.page_param.stage.diff = StageDiff_Easy;
		}
	}
	
	//Draw difficulty arrows
	static const RECT arrow_src[2][2] = {
		{{224, 64, 16, 32}, {224, 96, 16, 32}}, //left
		{{240, 64, 16, 32}, {240, 96, 16, 32}}, //right
	};
	
	Gfx_BlitTex(&menu.tex_story, &arrow_src[0][(pad_state.held & PAD_LEFT) != 0], x - 40 - 16, y - 16);
	Gfx_BlitTex(&menu.tex_story, &arrow_src[1][(pad_state.held & PAD_RIGHT) != 0], x + 40, y - 16);
	
	//Draw difficulty
	static const RECT diff_srcs[] = {
		{  0, 96, 64, 18},
		{ 64, 96, 80, 18},
		{144, 96, 64, 18},
	};
	
	const RECT *diff_src = &diff_srcs[menu.page_param.stage.diff];
	Gfx_BlitTex(&menu.tex_story, diff_src, x - (diff_src->w >> 1), y - 9 + ((pad_state.press & (PAD_LEFT | PAD_RIGHT)) != 0));
}

static void Menu_DrawWeek(const char *week, s32 x, s32 y)
{
	//Draw label
	if (week == NULL)
	{
		//Tutorial
		RECT label_src = {0, 0, 112, 32};
		Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
	}
	else
	{
		//Week
		RECT label_src = {0, 32, 80, 32};
		Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
		
		//Number
		x += 80;
		for (; *week != '\0'; week++)
		{
			//Draw number
			u8 i = *week - '0';
			
			RECT num_src = {128 + ((i & 3) << 5), ((i >> 2) << 5), 32, 32};
			Gfx_BlitTex(&menu.tex_story, &num_src, x, y);
			x += 32;
		}
	}
}

//Menu functions
void Menu_Load(MenuPage page)
{
	//Load menu assets
	IO_Data menu_arc = IO_Read("\\MENU\\MENU.ARC;1");
	Gfx_LoadTex(&menu.tex_back,  Archive_Find(menu_arc, "back.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre0,  Archive_Find(menu_arc, "cre0.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre1,  Archive_Find(menu_arc, "cre1.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre2,  Archive_Find(menu_arc, "cre2.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre3,  Archive_Find(menu_arc, "cre3.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre4,  Archive_Find(menu_arc, "cre4.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre5,  Archive_Find(menu_arc, "cre5.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre6,  Archive_Find(menu_arc, "cre6.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre7,  Archive_Find(menu_arc, "cre7.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre8,  Archive_Find(menu_arc, "cre8.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre9,  Archive_Find(menu_arc, "cre9.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre10,  Archive_Find(menu_arc, "cre10.tim"),  0);
	Gfx_LoadTex(&menu.tex_cre11,  Archive_Find(menu_arc, "cre11.tim"),  0);
	Gfx_LoadTex(&menu.tex_story, Archive_Find(menu_arc, "story.tim"), 0);
	Gfx_LoadTex(&menu.tex_title, Archive_Find(menu_arc, "title.tim"), 0);
	Mem_Free(menu_arc);
	
	FontData_Load(&menu.font_bold, Font_Bold);
	
	menu.bfgf = Char_BFGF_New(FIXED_DEC(16,1), FIXED_DEC(50,1));
	stage.camera.x = stage.camera.y = FIXED_DEC(0,1);
	stage.camera.bzoom = FIXED_UNIT;
	stage.bfgf_speed = 4;
	
	//Initialize menu state
	menu.select = menu.next_select = 0;
	
	switch (menu.page = menu.next_page = page)
	{
		case MenuPage_Opening:
			//Get funny message to use
			//Do this here so timing is less reliant on VSync
			#ifdef PSXF_PC
				menu.page_state.opening.funny_message = time(NULL) % COUNT_OF(funny_messages);
			#else
				menu.page_state.opening.funny_message = ((*((volatile u32*)0xBF801120)) >> 3) % COUNT_OF(funny_messages); //sysclk seeding
			#endif
			break;
		default:
			break;
	}
	menu.page_swap = true;
	
	menu.trans_time = 0;
	Trans_Clear();
	
	stage.song_step = 0;
	
	//Play menu music
	Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
	Audio_WaitPlayXA();
	
	//Set background colour
	Gfx_SetClear(0, 0, 0);
}

void Menu_Unload(void)
{
	//Free title Girlfriend
	Character_Free(menu.bfgf);
}

void Menu_Tick(void)
{
	//Clear per-frame flags
	stage.flag &= ~STAGE_FLAG_JUST_STEP;
	
	//Get song position
	u16 next_step = Audio_TellXA_Milli() / 147; //100 BPM
	if (next_step != stage.song_step)
	{
		if (next_step >= stage.song_step)
			stage.flag |= STAGE_FLAG_JUST_STEP;
		stage.song_step = next_step;
	}
	
	//Handle transition out
	if (Trans_Tick())
	{
		//Change to set next page
		menu.page_swap = true;
		menu.page = menu.next_page;
		menu.select = menu.next_select;
	}
	
	//Tick menu page
	MenuPage exec_page;
	switch (exec_page = menu.page)
	{
		case MenuPage_Opening:
		{
			u16 beat = stage.song_step >> 2;
			
			//Start title screen if opening ended
			if (beat >= 16)
			{
				menu.page = menu.next_page = MenuPage_Title;
				menu.page_swap = true;
				//Fallthrough
			}
			else
			{
				//Start title screen if start pressed
				if (pad_state.held & PAD_START)
					menu.page = menu.next_page = MenuPage_Title;
				
				//Draw different text depending on beat
				const char **funny_message = funny_messages[menu.page_state.opening.funny_message];
				
				switch (beat)
				{
					case 3:
						menu.font_bold.draw(&menu.font_bold, "PRESENT", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 32, FontAlign_Center);
				//Fallthrough
					case 2:
					case 1:
						menu.font_bold.draw(&menu.font_bold, "NINJAMUFFIN",   SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "PHANTOMARCADE", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "KAWAISPRITE",   SCREEN_WIDTH2, SCREEN_HEIGHT2,      FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "EVILSKER",      SCREEN_WIDTH2, SCREEN_HEIGHT2 + 16, FontAlign_Center);
						break;
					
					case 7:
						menu.font_bold.draw(&menu.font_bold, "LOL",    SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
				//Fallthrough
					case 6:
					case 5:
						menu.font_bold.draw(&menu.font_bold, "PSXFUNKIN", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "MODDED",           SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
						break;
					
					case 11:
						menu.font_bold.draw(&menu.font_bold, funny_message[1], SCREEN_WIDTH2, SCREEN_HEIGHT2, FontAlign_Center);
				//Fallthrough
					case 10:
					case 9:
						menu.font_bold.draw(&menu.font_bold, funny_message[0], SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
						break;
					
					case 15:
						menu.font_bold.draw(&menu.font_bold, "BABYYY", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 8, FontAlign_Center);
				//Fallthrough
					case 14:
						menu.font_bold.draw(&menu.font_bold, "D SIDE", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 8, FontAlign_Center);
				//Fallthrough
					case 13:
						menu.font_bold.draw(&menu.font_bold, "FRIDAY NIGHT FUNKIN", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 24, FontAlign_Center);
						break;
				}
				break;
			}
		}
	//Fallthrough
		case MenuPage_Title:
		{
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(90,1);
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Go to main menu when start is pressed
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if ((pad_state.press & PAD_START) && menu.next_page == menu.page && Trans_Idle())
			{
				menu.trans_time = FIXED_UNIT;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(300,1);
				menu.next_page = MenuPage_Main;
				menu.next_select = 0;
			}
			
			//Draw Friday Night Funkin' logo
			if ((stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0x3) == 0 && menu.page_state.title.logo_bump == 0)
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
			
			static const fixed_t logo_scales[] = {
				FIXED_DEC(1,1),
				FIXED_DEC(101,100),
				FIXED_DEC(102,100),
				FIXED_DEC(103,100),
				FIXED_DEC(105,100),
				FIXED_DEC(110,100),
				FIXED_DEC(97,100),
			};
			fixed_t logo_scale = logo_scales[(menu.page_state.title.logo_bump * 24) >> FIXED_SHIFT];
			u32 x_rad = (logo_scale * (200 >> 1)) >> FIXED_SHIFT;
			u32 y_rad = (logo_scale * (133 >> 1)) >> FIXED_SHIFT;
			
			RECT logo_src = {0, 0, 180, 119};
			RECT logo_dst = {
				100 - x_rad + (SCREEN_WIDEADD2 >> 1),
				90 - y_rad,
				x_rad << 1,
				y_rad << 1
			};
			Gfx_DrawTex(&menu.tex_title, &logo_src, &logo_dst);
			
			if (menu.page_state.title.logo_bump > 0)
				if ((menu.page_state.title.logo_bump -= timer_dt) < 0)
					menu.page_state.title.logo_bump = 0;
			
			//Draw "Press Start to Begin"
			if (menu.next_page == menu.page)
			{
				//Blinking blue
				s16 press_lerp = (MUtil_Cos(animf_count << 3) + 0x100) >> 1;
				u8 press_r = 51 >> 1;
				u8 press_g = (58  + ((press_lerp * (255 - 58))  >> 8)) >> 1;
				u8 press_b = (206 + ((press_lerp * (255 - 206)) >> 8)) >> 1;
				
				RECT press_src = {0, 120, 256, 30};
				Gfx_BlitTexCol(&menu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48, press_r, press_g, press_b);
			}
			else
			{
				//Flash white
				RECT press_src = {0, (animf_count & 1) ? 152 : 120, 256, 30};
				Gfx_BlitTex(&menu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48);
			}
			
			//Draw Boyfriend and Girlfriend
			menu.bfgf->tick(menu.bfgf);
			break;
		}
		case MenuPage_Main:
		{
			static const char *menu_options[] = {
				"STORY MODE",
				"FREEPLAY",
				"CREDITS",
				"OPTIONS",
			};
			
			//Initialize page
			if (menu.page_swap)
				menu.scroll = menu.select * FIXED_DEC(8,1);
			
			//Draw version identification
			menu.font_bold.draw(&menu.font_bold,
				"PSXFUNKIN BY CUCKYDEV",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					switch (menu.select)
					{
						case 0: //Story Mode
							menu.next_page = MenuPage_Story;
							break;
						case 1: //Freeplay
							menu.next_page = MenuPage_Freeplay;
							break;
						case 2: //Mods
							menu.next_page = MenuPage_Mods;
							break;
						case 3: //Options
							menu.next_page = MenuPage_Options;
							break;
					}
					menu.next_select = 0;
					menu.trans_time = FIXED_UNIT;
				}
				
				//Return to title screen if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Title;
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(8,1);
			menu.scroll += (next_scroll - menu.scroll) >> 2;
			
			if (menu.next_page == menu.page || menu.next_page == MenuPage_Title)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(menu_options[i], menu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i << 5) - 48 - (menu.scroll >> FIXED_SHIFT),
						FontAlign_Center
					);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				menu.font_bold.draw(&menu.font_bold,
					menu_options[menu.select],
					SCREEN_WIDTH2,
					SCREEN_HEIGHT2 + (menu.select << 5) - 48 - (menu.scroll >> FIXED_SHIFT),
					FontAlign_Center
				);
			}
			
			//Draw background
			Menu_DrawBack(
				menu.next_page == menu.page || menu.next_page == MenuPage_Title,
				menu.scroll >> (FIXED_SHIFT + 1),
				253 >> 1, 231 >> 1, 113 >> 1,
				253 >> 1, 113 >> 1, 155 >> 1
			);
			break;
		}
		case MenuPage_Story:
		{
			static const struct
			{
				const char *week;
				StageId stage;
				const char *name;
				const char *tracks[4];
			} menu_options[] = {
				{NULL, StageId_1_4, "TUTORIAL", {"TUTORIAL", NULL, NULL}},
				{"1", StageId_1_1, "GUEST STAR", {"BOPEEBO", "FRESH", "DADBATTLE", NULL}},
				{"2", StageId_2_1, "SWEET SCARES", {"SPOOKEEZ", "SOUTH", "GHASTLY", "MONSTER"}},
				{"3", StageId_3_1, "BAD HAIR DAY", {"PICO", "PHILLY NICE", "BLAMMED", NULL}},
				{"4", StageId_4_1, "RED HEAD MOMMA", {"SATIN PANTIES", "HIGH", "AWOOGA", "MILF"}},
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = 0;
				menu.page_param.stage.diff = StageDiff_Normal;
				menu.page_state.title.fade = FIXED_DEC(0,1);
				menu.page_state.title.fadespd = FIXED_DEC(0,1);
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Draw difficulty selector
			Menu_DifficultySelector(SCREEN_WIDTH - 75, 80);
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					menu.next_page = MenuPage_Stage;
					menu.page_param.stage.id = menu_options[menu.select].stage;
					menu.page_param.stage.story = true;
					menu.trans_time = FIXED_UNIT;
					menu.page_state.title.fade = FIXED_DEC(255,1);
					menu.page_state.title.fadespd = FIXED_DEC(510,1);
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 0; //Story Mode
					Trans_Start();
				}
			}
			
			//Draw week name and tracks
			menu.font_bold.draw(&menu.font_bold,
				menu_options[menu.select].name,
				SCREEN_WIDTH - 16,
				24,
				FontAlign_Right
			);
			
			const char * const *trackp = menu_options[menu.select].tracks;
			for (size_t i = 0; i < COUNT_OF(menu_options[menu.select].tracks); i++, trackp++)
			{
				if (*trackp != NULL)
					menu.font_bold.draw(&menu.font_bold,
						*trackp,
						SCREEN_WIDTH - 16,
						SCREEN_HEIGHT - (4 * 24) + (i * 24),
						FontAlign_Right
					);
			}
			
			//Draw upper strip
			RECT name_bar = {0, 16, SCREEN_WIDTH, 32};
			Gfx_DrawRect(&name_bar, 249, 207, 81);
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(48,1);
			menu.scroll += (next_scroll - menu.scroll) >> 3;
			
			if (menu.next_page == menu.page || menu.next_page == MenuPage_Main)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					s32 y = 64 + (i * 48) - (menu.scroll >> FIXED_SHIFT);
					if (y <= 16)
						continue;
					if (y >= SCREEN_HEIGHT)
						break;
					Menu_DrawWeek(menu_options[i].week, 48, y);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				Menu_DrawWeek(menu_options[menu.select].week, 48, 64 + (menu.select * 48) - (menu.scroll >> FIXED_SHIFT));
			}
			
			break;
		}
		case MenuPage_Freeplay:
		{
			static const struct
			{
				StageId stage;
				const char *text;
			} menu_options[] = {
				{StageId_1_4, "TUTORIAL"},
				{StageId_1_1, "BOPEEBO"},
				{StageId_1_2, "FRESH"},
				{StageId_1_3, "DADBATTLE"},
				{StageId_2_1, "SPOOKEEZ"},
				{StageId_2_2, "SOUTH"},
				{StageId_2_3, "GHASTLY"},
				{StageId_2_4, "MONSTER"},
				{StageId_3_1, "PICO"},
				{StageId_3_2, "PHILLY NICE"},
				{StageId_3_3, "BLAMMED"},
				{StageId_4_1, "SATIN PANTIES"},
				{StageId_4_2, "HIGH"},
				{StageId_4_3, "AWOOGA"},
				{StageId_4_4, "MILF"},
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_Normal;
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"FREEPLAY",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw difficulty selector
			Menu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					menu.next_page = MenuPage_Stage;
					menu.page_param.stage.id = menu_options[menu.select].stage;
					menu.page_param.stage.story = false;
					Trans_Start();
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 1; //Freeplay
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				146 >> 1, 113 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_Mods:
 
		  {

			 static u16 change = 1;
        
			    //Swap Main if opening ended
			  {
				if (change >= 16)
				menu.page = menu.next_page = MenuPage_Main;
				menu.page_swap = true;	

		 	    }

		
				  if (change>=16)
				  change = 1;

			    //Fallthrough
			{
				//Swap Main if circle pressed
			
				if (pad_state.press & PAD_CIRCLE)
				 {
				  menu.page = menu.next_page = MenuPage_Main;
				  menu.page_swap = true;
					(change) = 1;
				 }

				
				//Draw different text 
				RECT src_cre0 = {0, 0, 128, 128};
				RECT src_cre1 = {48, 0, 128, 128};
				RECT src_cre2 = {112, 0, 128, 128};
				RECT src_cre3 = {95, 0, 128, 128};
				RECT src_cre4 = {90, 130, 128, 128};
				RECT src_cre5 = {80, 130,  20, 20};
				RECT src_cre6 = {80, 150,  20, 20};
				RECT src_cre7 = {80, 170,  20, 20};
				RECT src_cre8 = {80, 190,  20, 20};
				RECT src_cre9 = {80, 210,  20, 20};
				RECT src_cre10 = {80, 230, 20, 20};
				RECT src_cre11 = {120,130, 128,128};
				
			   if (pad_state.press & (PAD_START | PAD_CROSS))
			    change++;

				switch (change)
					{
					case 2:

						menu.font_bold.draw(&menu.font_bold, "cuckydev", SCREEN_WIDTH2, SCREEN_HEIGHT2 -48,  FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre1, &src_cre1, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);
				//Fallthrough
					case 1:
					{
						menu.font_bold.draw(&menu.font_bold, "PSXFUNKIN",   SCREEN_WIDTH2, SCREEN_HEIGHT2 - 80, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "BY", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						break;
					}
					
					case 4:
						menu.font_bold.draw(&menu.font_bold, "igorsou",    SCREEN_WIDTH2, SCREEN_HEIGHT2 -48 , FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre0, &src_cre0, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);
					
				//Fallthrough
					case 3:
						menu.font_bold.draw(&menu.font_bold, "PORTED", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 80, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "BY",           SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						break;

				   case 6:
						menu.font_bold.draw(&menu.font_bold, "dark_moon", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre2, &src_cre2, (SCREEN_WIDTH - 128) >> 1, SCREEN_HEIGHT2 - 18);
					 
					case 5:
						menu.font_bold.draw(&menu.font_bold, "ARTIST OF THE PORT", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						break;
                    
					 case 8:
					   menu.font_bold.draw(&menu.font_bold, "john paul", SCREEN_WIDTH2 +70, SCREEN_HEIGHT2 - 48, FontAlign_Center);
					Gfx_BlitTex(&menu.tex_cre3, &src_cre3, (SCREEN_WIDTH +20) >> 1, SCREEN_HEIGHT2 - 18);
					menu.font_bold.draw(&menu.font_bold, "lord scout", SCREEN_WIDTH2 -90, SCREEN_HEIGHT2 -48 , FontAlign_Center);
					Gfx_BlitTex(&menu.tex_cre4, &src_cre4, (SCREEN_WIDTH - 356) >> 1, SCREEN_HEIGHT2 - 18);

					case 7:
						menu.font_bold.draw(&menu.font_bold, "PLAYTESTER", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 80, FontAlign_Center);
						break;
					 
					 case 10:
					    menu.font_bold.draw(&menu.font_bold, "dastardlydeacon", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 80, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre5, &src_cre5, (SCREEN_WIDTH +200) >> 1, SCREEN_HEIGHT2 - 80);
						menu.font_bold.draw(&menu.font_bold, "philiplol", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 60,  FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre6, &src_cre6, (SCREEN_WIDTH +120) >> 1, SCREEN_HEIGHT2 -60);
						menu.font_bold.draw(&menu.font_bold, "gc", SCREEN_WIDTH2, SCREEN_HEIGHT2 -44, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "lolika", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 26 , FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "fifleo", SCREEN_WIDTH2, SCREEN_HEIGHT2 +16, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre7, &src_cre7, (SCREEN_WIDTH +120) >> 1, SCREEN_HEIGHT2 + 16);
						menu.font_bold.draw(&menu.font_bold, "ellisbros", SCREEN_WIDTH2, SCREEN_HEIGHT2 +32, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "aninternetsecho", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 48, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "squish", SCREEN_WIDTH2, SCREEN_HEIGHT2 +64, FontAlign_Center);

					case 9:
						menu.font_bold.draw(&menu.font_bold, "D SIDE MOD CREATORS", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 96, FontAlign_Center);
						break;
					
					 case 12:
                	    menu.font_bold.draw(&menu.font_bold, "unstopable", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre8, &src_cre8, (SCREEN_WIDTH +150) >> 1, SCREEN_HEIGHT2 - 16);
						menu.font_bold.draw(&menu.font_bold, "zeriben", SCREEN_WIDTH2, SCREEN_HEIGHT2 +8, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre9, &src_cre9, (SCREEN_WIDTH +120) >> 1, SCREEN_HEIGHT2 + 8);
						menu.font_bold.draw(&menu.font_bold, "g3yt", SCREEN_WIDTH2, SCREEN_HEIGHT2 +40, FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre10, &src_cre10, (SCREEN_WIDTH +80) >> 1, SCREEN_HEIGHT2 + 40);
						menu.font_bold.draw(&menu.font_bold, "psxfunkin discord server", SCREEN_WIDTH2 , SCREEN_HEIGHT2 +60, FontAlign_Center);

					case 11:
						menu.font_bold.draw(&menu.font_bold, "SPECIAL THANKS", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
						break;

					case 15:
						menu.font_bold.draw(&menu.font_bold, "THIS IS THE POSSUM EVER", SCREEN_WIDTH2, SCREEN_HEIGHT2 , FontAlign_Center);
						Gfx_BlitTex(&menu.tex_cre11, &src_cre11, (SCREEN_WIDTH -100 ) >> 1, SCREEN_HEIGHT2 +20);

						case 14:
						menu.font_bold.draw(&menu.font_bold, "BEFORE YOU GO", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
				
					
						case 13:
						menu.font_bold.draw(&menu.font_bold, "THANKS FOR PLAYING!", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
						break;
				}
				break;
			

			}
		}
	
		case MenuPage_Options:
		{
			static const char *gamemode_strs[] = {"NORMAL", "SWAP", "TWO PLAYER"};
			static const struct
			{
				enum
				{
					OptType_Boolean,
					OptType_Enum,
				} type;
				const char *text;
				void *value;
				union
				{
					struct
					{
						int a;
					} spec_boolean;
					struct
					{
						s32 max;
						const char **strs;
					} spec_enum;
				} spec;
			} menu_options[] = {
				{OptType_Enum,    "GAMEMODE", &stage.mode, {.spec_enum = {StageMode_Max, gamemode_strs}}},
				//{OptType_Boolean, "INTERPOLATION", &stage.expsync},
				{OptType_Boolean, "GHOST TAP ", &stage.ghost, {.spec_boolean = {0}}},
				{OptType_Boolean, "DOWNSCROLL", &stage.downscroll, {.spec_boolean = {0}}},
			};
			
			//Initialize page
			if (menu.page_swap)
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"OPTIONS",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Handle option changing
				switch (menu_options[menu.select].type)
				{
					case OptType_Boolean:
						if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							*((boolean*)menu_options[menu.select].value) ^= 1;
						break;
					case OptType_Enum:
						if (pad_state.press & PAD_LEFT)
							if (--*((s32*)menu_options[menu.select].value) < 0)
								*((s32*)menu_options[menu.select].value) = menu_options[menu.select].spec.spec_enum.max - 1;
						if (pad_state.press & PAD_RIGHT)
							if (++*((s32*)menu_options[menu.select].value) >= menu_options[menu.select].spec.spec_enum.max)
								*((s32*)menu_options[menu.select].value) = 0;
						break;
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 3; //Options
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				char text[0x80];
				switch (menu_options[i].type)
				{
					case OptType_Boolean:
						sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "ON" : "OFF");
						break;
					case OptType_Enum:
						sprintf(text, "%s %s", menu_options[i].text, menu_options[i].spec.spec_enum.strs[*((s32*)menu_options[i].value)]);
						break;
				}
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Menu_DrawBack(
				true,
				8,
				253 >> 1, 113 >> 1, 155 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_Stage:
		{
			//Unload menu state
			Menu_Unload();
			
			//Load new stage
			LoadScr_Start();
			Stage_Load(menu.page_param.stage.id, menu.page_param.stage.diff, menu.page_param.stage.story);
			gameloop = GameLoop_Stage;
			LoadScr_End();
			break;
		}
		default:
			break;
	}
	
	//Clear page swap flag
	menu.page_swap = menu.page != exec_page;
}
