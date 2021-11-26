/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfgf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"


//BFGF character structure
enum
{
	BFGF_ArcMain_Dance0,
	BFGF_ArcMain_Dance1,
	BFGF_ArcMain_Dance2,
	BFGF_ArcMain_Dance3,
	BFGF_ArcMain_Dance4,
	BFGF_ArcMain_Dance5,
	BFGF_ArcMain_Dance6,
	BFGF_ArcMain_Dance7,
	BFGF_ArcMain_Dance8,
	
	BFGF_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BFGF_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	

} Char_BFGF;

//BFGF character definitions
static const CharFrame char_bfgf_frame[] = {
	{BFGF_ArcMain_Dance0, {  0,   0, 200, 161}, { 46,  86}}, //0 bop left 1
	{BFGF_ArcMain_Dance1, {  0,   0, 200, 161}, { 46,  86}}, //1 bop left 2
	{BFGF_ArcMain_Dance2, {  0,   0, 200, 161}, { 46,  86}}, //2 bop left 3
	{BFGF_ArcMain_Dance3, {  0,   0, 200, 161}, { 46,  86}}, //3 bop left 4
	{BFGF_ArcMain_Dance4, {  0,   0, 200, 161}, { 46,  86}}, //4 bop left 5
	{BFGF_ArcMain_Dance5, {  0,   0, 200, 161}, { 46,  86}}, //5 bop left 6
	{BFGF_ArcMain_Dance6, {  0,   0, 200, 161}, { 46,  86}}, //6 bop right 1
	{BFGF_ArcMain_Dance7, {  0,   0, 200, 161}, { 46,  86}}, //7 bop right 2
	{BFGF_ArcMain_Dance8, {  0,   0, 200, 161}, { 46,  86}}, //8 bop right 3
	
	
};

static const Animation char_bfgf_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Idle
	{1, (const u8[]){ 0,   1,   2,   3,   7,  8,   4,   5,  6,  6, ASCR_BACK,1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_LeftAlt
	{1, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_UpAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}}, //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_RightAlt
};

//BFGF character functions
void Char_BFGF_SetFrame(void *user, u8 frame)
{
	Char_BFGF *this = (Char_BFGF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfgf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFGF_Tick(Character *character)
{
	Char_BFGF *this = (Char_BFGF*)character;
	
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			
			//Perform dance
			if ((stage.song_step % 0x4 == 0))
			
				//Switch animation
				if (character->animatable.anim == CharAnim_Left)
					character->set_anim(character, CharAnim_Left);
	

			}
		
	
		
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_BFGF_SetFrame);
	Character_Draw(character, &this->tex, &char_bfgf_frame[this->frame]);
	
}

void Char_BFGF_SetAnim(Character *character, u8 anim)
{

	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_BFGF_Free(Character *character)
{
	Char_BFGF *this = (Char_BFGF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BFGF_New(fixed_t x, fixed_t y)
{
	//Allocate BFGF object
	Char_BFGF *this = Mem_Alloc(sizeof(Char_BFGF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFGF_New] Failed to allocate BFGF object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFGF_Tick;
	this->character.set_anim = Char_BFGF_SetAnim;
	this->character.free = Char_BFGF_Free;
	
	Animatable_Init(&this->character.animatable, char_bfgf_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-50,1);
	this->character.focus_zoom = FIXED_DEC(13,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFGF.ARC;1");
	
	const char **pathp = (const char *[]){
		"dance0.tim",  //BFGF_ArcMain_Dance
		"dance1.tim", //BFGF_ArcMain_Dance
		"dance2.tim",  //BFGF_ArcMain_Dance
		"dance3.tim", //BFGF_ArcMain_Dance
		"dance4.tim",  //BFGF_ArcMain_Dance
		"dance5.tim", //BFGF_ArcMain_Dance
		"dance6.tim",  //BFGF_ArcMain_Dance
		"dance7.tim", //BFGF_ArcMain_Dance
		"dance8.tim",  //BFGF_ArcMain_Dance
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;

	
	return (Character*)this;
}
