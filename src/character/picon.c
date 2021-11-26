/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "picon.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//PicoN character structure
enum
{
	PicoN_ArcMain_Idle,
	PicoN_ArcMain_Hit0,
	PicoN_ArcMain_Up,
	PicoN_ArcMain_Right,
	PicoN_ArcMain_Reload0,
	PicoN_ArcMain_Reload1,
	
	PicoN_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[PicoN_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_PicoN;

//PicoN character definitions
static const CharFrame char_picon_frame[] = {
	{PicoN_ArcMain_Idle, {  0,   0, 118, 132}, { 64, 104}}, //0 idle 1
	{PicoN_ArcMain_Idle, {119,   0, 112, 126}, { 64,  97}}, //1 idle 2
	{PicoN_ArcMain_Idle, {  0, 132, 132, 128}, { 64,  96}}, //2 idle 3
	{PicoN_ArcMain_Idle, {133, 128, 132, 132}, { 64,  99}}, //3 idle 4
	
	{PicoN_ArcMain_Hit0, {  0,   0, 126, 130}, { 80,  99}}, //4 left 1
	{PicoN_ArcMain_Hit0, {126,   0, 130, 130}, { 74, 100}}, //5 left 2
	
	{PicoN_ArcMain_Hit0, {  0, 128, 128, 114}, { 56,  85}}, //6 down 1
	{PicoN_ArcMain_Hit0, {128, 128, 128, 114}, { 54,  86}}, //7 down 2
	
	{PicoN_ArcMain_Up, {  0,   0, 110, 144}, { 62, 114}}, //8 up 1
	{PicoN_ArcMain_Up, {110,   0, 110, 144}, { 70, 114}}, //9 up 2
	
	{PicoN_ArcMain_Right, {  0,  0, 99, 139}, { 54,110}}, //10 right 1
	{PicoN_ArcMain_Right, {100,  0, 98, 139}, { 54,110}}, //11 right 2

	{PicoN_ArcMain_Reload0, {  0,  0, 112, 134}, { 56,105}}, //10 right 1
	{PicoN_ArcMain_Reload0, {112,  0, 108, 134}, { 50,105}}, //10 right 1

	{PicoN_ArcMain_Reload1, {  0,  0, 111, 134}, { 52,105}}, //10 right 1
	{PicoN_ArcMain_Reload1, {111,  0, 109, 134}, { 50,105}}, //11 right 2
};

static const Animation char_picon_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},       //CharAnim_Left
	{2, (const u8[]){12, 13,  14,  15, 12, ASCR_BACK, 1}} ,   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//PicoN character functions
void Char_PicoN_SetFrame(void *user, u8 frame)
{
	Char_PicoN *this = (Char_PicoN*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_picon_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_PicoN_Tick(Character *character)
{
	Char_PicoN *this = (Char_PicoN*)character;

	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_Right))
		Character_CheckEndSing(character);
	
	//Perform idle dance
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
			if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_PicoN_SetFrame);
	Character_Draw(character, &this->tex, &char_picon_frame[this->frame]);
}

void Char_PicoN_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_PicoN_Free(Character *character)
{
	Char_PicoN *this = (Char_PicoN*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_PicoN_New(fixed_t x, fixed_t y)
{
	//Allocate PicoN object
	Char_PicoN *this = Mem_Alloc(sizeof(Char_PicoN));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_PicoN_New] Failed to allocate PicoN object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_PicoN_Tick;
	this->character.set_anim = Char_PicoN_SetAnim;
	this->character.free = Char_PicoN_Free;
	
	Animatable_Init(&this->character.animatable, char_picon_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 3;
	
	this->character.focus_x = FIXED_DEC(20,1);
	this->character.focus_y = FIXED_DEC(-95,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\PICON.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim", //PicoN_ArcMain_Idle0
		"hit0.tim", //PicoN_ArcMain_Hit0
		"up.tim", //PicoN_ArcMain_Up
		"right.tim", //PicoN_ArcMain_Right
		"reload0.tim", //PicoN_ArcMain_Right
		"reload1.tim", //PicoN_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
