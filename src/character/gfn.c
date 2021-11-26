/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gfn.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

#include "speaker.h"

#include "../stage/week7.h"

//GFN character structure
enum
{
	GFN_ArcMain_BopLeft,
	GFN_ArcMain_BopRight,
	GFN_ArcMain_BopRigh2,
	GFN_ArcMain_Cry,
	
	GFN_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[GFN_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Speaker
	Speaker speaker;
	
	//Pico test
	u16 *pico_p;
} Char_GFN;

//GFN character definitions
static const CharFrame char_gfn_frame[] = {
	{GFN_ArcMain_BopLeft, {  0,   0,  85, 116}, { 48,  86}}, //0 bop left 1
	{GFN_ArcMain_BopLeft, { 85,   0,  85, 114}, { 48,  86}}, //1 bop left 2
	{GFN_ArcMain_BopLeft, {170,   0,  84, 114}, { 48,  86}}, //2 bop left 3
	{GFN_ArcMain_BopLeft, {  0, 116,  84, 114}, { 48,  80}}, //3 bop left 4
	{GFN_ArcMain_BopLeft, { 86, 115,  84, 114}, { 35,  81}}, //4 bop left 5
	{GFN_ArcMain_BopLeft, {172, 115,  84, 114}, { 35,  81}}, //5 bop left 6
	
	{GFN_ArcMain_BopRight, {  0,   0,  90, 120}, { 36,  87}}, //6 bop right 1
	{GFN_ArcMain_BopRight, { 92,   0,  90, 120}, { 37,  87}}, //7 bop right 2
	{GFN_ArcMain_BopRight, {  0, 125,  90, 120}, { 35,  89}}, //8 bop right 3
	{GFN_ArcMain_BopRight, { 92, 125,  90, 120}, { 35,  89}}, //9 bop right 4

	{GFN_ArcMain_BopRigh2, {  0,  0, 100, 116}, { 46,  87}}, //10 bop right 5
	
	{GFN_ArcMain_Cry, {  0,   0,  86, 103}, { 48,  73}}, //12 cry
	{GFN_ArcMain_Cry, { 87,   0,  86, 103}, { 48,  73}}, //13 cry
};

static const Animation char_gfn_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Idle
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_LeftAlt
	{2, (const u8[]){11, 12, ASCR_REPEAT}},                                  //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_UpAlt
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 10, ASCR_BACK, 1}}, //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_RightAlt
};

//GFN character functions
void Char_GFN_SetFrame(void *user, u8 frame)
{
	Char_GFN *this = (Char_GFN*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gfn_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_GFN_Tick(Character *character)
{
	Char_GFN *this = (Char_GFN*)character;
	
	//Initialize Pico test
	if (stage.stage_id == StageId_7_3 && stage.back != NULL && this->pico_p == NULL)
		this->pico_p = ((Back_Week7*)stage.back)->pico_chart;
	
	if (this->pico_p != NULL)
	{
		if (stage.note_scroll >= 0)
		{
			//Scroll through Pico chart
			u16 substep = stage.note_scroll >> FIXED_SHIFT;
			while (substep >= ((*this->pico_p) & 0x7FFF))
			{
				//Play animation and bump speakers
				character->set_anim(character, ((*this->pico_p) & 0x8000) ? CharAnim_Right : CharAnim_Left);
				Speaker_Bump(&this->speaker);
				this->pico_p++;
			}
		}
	}
	else
	{
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			//Perform dance
			if ((stage.song_step % stage.gf_speed) == 0)
			{
				//Switch animation
				if (character->animatable.anim == CharAnim_Left)
					character->set_anim(character, CharAnim_Right);
				else
					character->set_anim(character, CharAnim_Left);
				
				//Bump speakers
				Speaker_Bump(&this->speaker);
			}
		}
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GFN_SetFrame);
	Character_Draw(character, &this->tex, &char_gfn_frame[this->frame]);
	
	//Tick speakers
	Speaker_Tick(&this->speaker, character->x, character->y);
}

void Char_GFN_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_GFN_Free(Character *character)
{
	Char_GFN *this = (Char_GFN*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_GFN_New(fixed_t x, fixed_t y)
{
	//Allocate GFN object
	Char_GFN *this = Mem_Alloc(sizeof(Char_GFN));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_GFN_New] Failed to allocate GFN object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_GFN_Tick;
	this->character.set_anim = Char_GFN_SetAnim;
	this->character.free = Char_GFN_Free;
	
	Animatable_Init(&this->character.animatable, char_gfn_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-50,1);
	this->character.focus_zoom = FIXED_DEC(13,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GFN.ARC;1");
	
	const char **pathp = (const char *[]){
		"bopleft.tim",  //GFN_ArcMain_BopLeft
		"bopright.tim", //GFN_ArcMain_BopRight
		"boprigh2.tim", //GFN_ArcMain_BopRight 2
		"cry.tim",      //GFN_ArcMain_Cry
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize speaker
	Speaker_Init(&this->speaker);
	
	//Initialize Pico test
	if (stage.stage_id == StageId_7_3 && stage.back != NULL)
		this->pico_p = ((Back_Week7*)stage.back)->pico_chart;
	else
		this->pico_p = NULL;
	
	return (Character*)this;
}
