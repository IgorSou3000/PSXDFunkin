/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "spook.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Spook character structure
enum
{
	Spook_ArcMain_Idle0,
	Spook_ArcMain_Idle0a1,
	Spook_ArcMain_Idle1,
	Spook_ArcMain_Idle2,
	Spook_ArcMain_Idle3,
	Spook_ArcMain_Left0,
	Spook_ArcMain_Left1,
	Spook_ArcMain_Left2,
	Spook_ArcMain_Left3,
	Spook_ArcMain_Down0,
	Spook_ArcMain_Down1,
	Spook_ArcMain_Up0,
	Spook_ArcMain_Up1,
	Spook_ArcMain_Right0,
	Spook_ArcMain_Right1,
	
	Spook_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Spook_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Spook;

//Spook character definitions
static const CharFrame char_spook_frame[] = {
	{Spook_ArcMain_Idle0, {  0,   0, 144, 140}, { 73, 125}}, //0 idle 1
   {Spook_ArcMain_Idle0a1,{  0,   0, 144, 140}, { 71, 124}}, //1 idle 2
	{Spook_ArcMain_Idle1, {  0,   0, 109, 125}, { 55, 117}}, //2 idle 3
	{Spook_ArcMain_Idle1, {111,   0, 100, 125}, { 55, 117}}, //3 idle 4
	{Spook_ArcMain_Idle2, {  0,   0, 128, 136}, { 76, 127}}, //4 idle 5
	{Spook_ArcMain_Idle3, {  0,   0, 128, 136}, { 74, 127}}, //5 idle 6
	
	{Spook_ArcMain_Left0, {  0,   0, 124, 150}, {59, 135}}, //6 left 1
	{Spook_ArcMain_Left1, {  0,   0, 124, 151}, {59, 134}}, //7 left 2

	{Spook_ArcMain_Left2, {  0,   0, 124, 151}, {59, 134}}, //6 left 1
	{Spook_ArcMain_Left3, {  0,   0, 124, 151}, {59, 134}}, //7 left 2
	
	{Spook_ArcMain_Down0, {  0,   0, 136, 138}, { 43, 126}}, //8 down 1
	{Spook_ArcMain_Down1, {  0,   0, 136, 138}, { 43, 128}}, //9 down 2
	
	{Spook_ArcMain_Up0, {  0,   0, 136, 153}, { 68, 144}}, //10 up 1
	{Spook_ArcMain_Up1, {  0,   0, 140, 153}, { 67, 143}}, //11 up 2
	
	{Spook_ArcMain_Right0, {  0,   0, 124, 146}, { 42, 131}}, //12 right 1
	{Spook_ArcMain_Right1, {  0,   0, 124, 146}, { 42, 131}}, //13 right 2
};

static const Animation char_spook_anim[CharAnim_Max] = {
	{2, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}}, //CharAnim_Idle
	{2, (const u8[]){ 6,  7,  8,  9, ASCR_BACK, 1}},          //CharAnim_Left
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 1}},  //CharAnim_LeftAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},          //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}}, //CharAnim_DownAlt
	{2, (const u8[]){12, 13, ASCR_BACK, 1}},          //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}}, //CharAnim_UpAlt
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},          //CharAnim_Right
	{2, (const u8[]){ 4,  5,  2,  3, ASCR_BACK, 1}},  //CharAnim_RightAlt
};

//Spook character functions
void Char_Spook_SetFrame(void *user, u8 frame)
{
	Char_Spook *this = (Char_Spook*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_spook_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Spook_Tick(Character *character)
{
	Char_Spook *this = (Char_Spook*)character;
	
	//Perform spooky dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	{
		Character_CheckEndSing(character);
		
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			if ((Animatable_Ended(&character->animatable) || character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_RightAlt) &&
				(character->animatable.anim != CharAnim_Left &&
				 character->animatable.anim != CharAnim_Down &&
				 character->animatable.anim != CharAnim_Up &&
				 character->animatable.anim != CharAnim_Right) &&
				(stage.song_step & 0x3) == 0)
				character->set_anim(character, CharAnim_Idle);
		}
	}
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Spook_SetFrame);
	Character_Draw(character, &this->tex, &char_spook_frame[this->frame]);
}

void Char_Spook_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim == CharAnim_Idle)
	{
		if (character->animatable.anim == CharAnim_LeftAlt)
			anim = CharAnim_RightAlt;
		else
			anim = CharAnim_LeftAlt;
		character->sing_end = FIXED_DEC(0x7FFF,1);
	}
	else
	{
		Character_CheckStartSing(character);
	}
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_Spook_Free(Character *character)
{
	Char_Spook *this = (Char_Spook*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Spook_New(fixed_t x, fixed_t y)
{
	//Allocate spook object
	Char_Spook *this = Mem_Alloc(sizeof(Char_Spook));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Spook_New] Failed to allocate spook object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Spook_Tick;
	this->character.set_anim = Char_Spook_SetAnim;
	this->character.free = Char_Spook_Free;
	
	Animatable_Init(&this->character.animatable, char_spook_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 2;
	
	this->character.focus_x =  FIXED_DEC(38,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SPOOK.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Spook_ArcMain_Idle0
		"idle0a1.tim", //Spook_ArcMain_Idle0
		"idle1.tim", //Spook_ArcMain_Idle1
		"idle2.tim", //Spook_ArcMain_Idle2
		"idle3.tim", //Spook_ArcMain_Idle2
		"left0.tim",  //Spook_ArcMain_Left
		"left1.tim",  //Spook_ArcMain_Left
		"left2.tim",  //Spook_ArcMain_Left
		"left3.tim",  //Spook_ArcMain_Left
		"down0.tim",  //Spook_ArcMain_Down
		"down1.tim",  //Spook_ArcMain_Down
		"up0.tim",    //Spook_ArcMain_Up
		"up1.tim",    //Spook_ArcMain_Up
		"right0.tim", //Spook_ArcMain_Right
		"right1.tim", //Spook_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
