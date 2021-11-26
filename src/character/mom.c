/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mom.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../timer.h"

//Mom character structure
enum
{
	Mom_ArcMain_Idle0,
	Mom_ArcMain_Idle1,
	Mom_ArcMain_Left0,
	Mom_ArcMain_Left1,
	Mom_ArcMain_Left2,
	Mom_ArcMain_Left3,
	Mom_ArcMain_Down0,
	Mom_ArcMain_Down1,
	Mom_ArcMain_Up0,
	Mom_ArcMain_Up1,
	Mom_ArcMain_Right0,
	Mom_ArcMain_Right1,
	
	Mom_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Mom_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
} Char_Mom;

//Mom character definitions
static const CharFrame char_mom_frame[] = {
	{Mom_ArcMain_Idle0, {  0,   0,  95, 201}, { 41, 164}}, //0 idle 1
	{Mom_ArcMain_Idle0, { 95,   0,  93, 201}, { 38, 164}}, //1 idle 2
	{Mom_ArcMain_Idle1, {  0,   0,  94, 201}, { 41, 164}}, //2 idle 3
	{Mom_ArcMain_Idle1, { 94,   0,  94, 201}, { 40, 164}}, //3 idle 4
	
	{Mom_ArcMain_Left0, {  0,   0, 140, 199}, { 65, 156}}, //4 left 1
	{Mom_ArcMain_Left1, {  0,   0, 140, 199}, { 65, 156}}, //4 left 1
	{Mom_ArcMain_Left2, {  0,   0, 140, 199}, { 65, 156}}, //4 left 1
	{Mom_ArcMain_Left3, {  0,   0, 140, 199}, { 65, 156}}, //4 left 1
	
	{Mom_ArcMain_Down0, {  0,   0, 119, 133}, { 45, 111}}, //6 down 1
	{Mom_ArcMain_Down0, {120,   0, 120, 133}, { 42, 111}}, //7 down 2
	{Mom_ArcMain_Down1, {  0,   0, 118, 133}, { 45, 111}}, //6 down 1
	{Mom_ArcMain_Down1, {120,   0, 120, 133}, { 42, 111}}, //7 down 2
	
	{Mom_ArcMain_Up0, {  0,   0, 129, 214}, { 47, 185}}, //8 up 1
	{Mom_ArcMain_Up0, {129,   0, 127, 214}, { 45, 182}}, //9 up 2
	{Mom_ArcMain_Up1, {  0,   0, 128, 214}, { 47, 185}}, //8 up 1
	{Mom_ArcMain_Up1, {128,   0, 128, 214}, { 45, 182}}, //9 up 2
	
	{Mom_ArcMain_Right0, {  0,   0, 124, 195}, { 62, 155}}, //10 right 1
	{Mom_ArcMain_Right0, {126,   0, 126, 195}, { 61, 156}}, //11 right 2
	{Mom_ArcMain_Right1, {  0,   0, 124, 195}, { 65, 155}}, //10 right 1
	{Mom_ArcMain_Right1, {126,   0, 126, 195}, { 62, 155}}, //11 right 2
};

static const Animation char_mom_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0,  0,  1,  1,  2,  3,  2,  3,  2,  3,  2,  3, ASCR_BACK, 2}}, //CharAnim_Idle
	{1, (const u8[]){ 4,  5,  6,  7,  6,  7, ASCR_BACK, 2}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{1, (const u8[]){ 8,  9, 10, 11, 10, 11, 10, 11, 10, 11, ASCR_BACK, 2}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{1, (const u8[]){12, 13, 14, 15, 14, 15, 14, 15, 14, 15, ASCR_BACK, 2}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{1, (const u8[]){16, 17, 18, 19, 18, 19, 18, 19, ASCR_BACK, 2}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Mom character functions
void Char_Mom_SetFrame(void *user, u8 frame)
{
	Char_Mom *this = (Char_Mom*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_mom_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Mom_Tick(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		{
		Character_CheckEndSing(character);
		
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			if ((Animatable_Ended(&character->animatable)) &&
				(character->animatable.anim != CharAnim_Left &&
				 character->animatable.anim != CharAnim_Down &&
				 character->animatable.anim != CharAnim_Up &&
				 character->animatable.anim != CharAnim_Right) &&
				(stage.song_step & 0x7) == 0)
				character->set_anim(character, CharAnim_Idle);
		}
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Mom_SetFrame);
	Character_Draw(character, &this->tex, &char_mom_frame[this->frame]);
	
}

void Char_Mom_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Mom_Free(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Mom_New(fixed_t x, fixed_t y)
{
	//Allocate mom object
	Char_Mom *this = Mem_Alloc(sizeof(Char_Mom));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Mom_New] Failed to allocate mom object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Mom_Tick;
	this->character.set_anim = Char_Mom_SetAnim;
	this->character.free = Char_Mom_Free;
	
	Animatable_Init(&this->character.animatable, char_mom_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 4;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MOM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Mom_ArcMain_Idle0
		"idle1.tim", //Mom_ArcMain_Idle1
		"left0.tim",  //Mom_ArcMain_Left
		"left1.tim",  //Mom_ArcMain_Left
		"left2.tim",  //Mom_ArcMain_Left
		"left3.tim",  //Mom_ArcMain_Left
		"down0.tim",  //Mom_ArcMain_Down0
		"down1.tim",  //Mom_ArcMain_Down1
		"up0.tim",    //Mom_ArcMain_Up
		"up1.tim",    //Mom_ArcMain_Up
		"right0.tim", //Mom_ArcMain_Right
		"right1.tim", //Mom_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
