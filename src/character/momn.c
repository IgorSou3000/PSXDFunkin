/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "momn.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../timer.h"

//Momn character structure
enum
{
	Momn_ArcMain_Idle0,
	Momn_ArcMain_Idle1,
	Momn_ArcMain_Left0,
	Momn_ArcMain_Left1,
	Momn_ArcMain_Left2,
	Momn_ArcMain_Left3,
	Momn_ArcMain_Down0,
	Momn_ArcMain_Down1,
	Momn_ArcMain_Up0,
	Momn_ArcMain_Up1,
	Momn_ArcMain_Right0,
	Momn_ArcMain_Right1,
	
	Momn_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Momn_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
} Char_Momn;

//Momn character definitions
static const CharFrame char_momn_frame[] = {
	{Momn_ArcMain_Idle0, {  0,   0,  95, 201}, { 41, 164}}, //0 idle 1
	{Momn_ArcMain_Idle0, { 95,   0,  93, 201}, { 38, 164}}, //1 idle 2
	{Momn_ArcMain_Idle1, {  0,   0,  94, 201}, { 41, 164}}, //2 idle 3
	{Momn_ArcMain_Idle1, { 94,   0,  94, 201}, { 40, 164}}, //3 idle 4
	
	{Momn_ArcMain_Left0, {  0,   0, 140, 199}, { 65, 156}}, //4 left 1
	{Momn_ArcMain_Left1, {  0,   0, 140, 199}, { 65, 156}}, //4 left 1
	{Momn_ArcMain_Left2, {  0,   0, 140, 199}, { 65, 156}}, //4 left 1
	{Momn_ArcMain_Left3, {  0,   0, 140, 199}, { 65, 156}}, //4 left 1
	
	{Momn_ArcMain_Down0, {  0,   0, 119, 133}, { 45, 111}}, //6 down 1
	{Momn_ArcMain_Down0, {120,   0, 120, 133}, { 42, 111}}, //7 down 2
	{Momn_ArcMain_Down1, {  0,   0, 118, 133}, { 45, 111}}, //6 down 1
	{Momn_ArcMain_Down1, {120,   0, 120, 133}, { 42, 111}}, //7 down 2
	
	{Momn_ArcMain_Up0, {  0,   0, 129, 214}, { 47, 185}}, //8 up 1
	{Momn_ArcMain_Up0, {129,   0, 127, 214}, { 45, 182}}, //9 up 2
	{Momn_ArcMain_Up1, {  0,   0, 128, 214}, { 47, 185}}, //8 up 1
	{Momn_ArcMain_Up1, {128,   0, 128, 214}, { 45, 182}}, //9 up 2
	
	{Momn_ArcMain_Right0, {  0,   0, 124, 195}, { 62, 155}}, //10 right 1
	{Momn_ArcMain_Right0, {126,   0, 126, 195}, { 61, 156}}, //11 right 2
	{Momn_ArcMain_Right1, {  0,   0, 124, 195}, { 65, 155}}, //10 right 1
	{Momn_ArcMain_Right1, {126,   0, 126, 195}, { 60, 155}}, //11 right 2
};

static const Animation char_momn_anim[CharAnim_Max] = {
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

//Momn character functions
void Char_Momn_SetFrame(void *user, u8 frame)
{
	Char_Momn *this = (Char_Momn*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_momn_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Momn_Tick(Character *character)
{
	Char_Momn *this = (Char_Momn*)character;
	
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
	Animatable_Animate(&character->animatable, (void*)this, Char_Momn_SetFrame);
	Character_Draw(character, &this->tex, &char_momn_frame[this->frame]);
	
}

void Char_Momn_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Momn_Free(Character *character)
{
	Char_Momn *this = (Char_Momn*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Momn_New(fixed_t x, fixed_t y)
{
	//Allocate Momn object
	Char_Momn *this = Mem_Alloc(sizeof(Char_Momn));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Momn_New] Failed to allocate Momn object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Momn_Tick;
	this->character.set_anim = Char_Momn_SetAnim;
	this->character.free = Char_Momn_Free;
	
	Animatable_Init(&this->character.animatable, char_momn_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 4;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MOMN.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Momn_ArcMain_Idle0
		"idle1.tim", //Momn_ArcMain_Idle1
		"left0.tim",  //Momn_ArcMain_Left
		"left1.tim",  //Momn_ArcMain_Left
		"left2.tim",  //Momn_ArcMain_Left
		"left3.tim",  //Momn_ArcMain_Left
		"down0.tim",  //Momn_ArcMain_Down0
		"down1.tim",  //Momn_ArcMain_Down1
		"up0.tim",    //Momn_ArcMain_Up
		"up1.tim",    //Momn_ArcMain_Up
		"right0.tim", //Momn_ArcMain_Right
		"right1.tim", //Momn_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
