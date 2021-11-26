/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "pico.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Pico character structure
enum
{
	Pico_ArcMain_Idle,
	Pico_ArcMain_Hit0,
	Pico_ArcMain_Up,
	Pico_ArcMain_Right,
	Pico_ArcMain_Reload0,
	Pico_ArcMain_Reload1,
	Pico_ArcMain_Shoot0,
	Pico_ArcMain_Shoot1,
	Pico_ArcMain_Shoot2,
	
	Pico_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Pico_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Pico;

//Pico character definitions
static const CharFrame char_pico_frame[] = {
	{Pico_ArcMain_Idle, {  0,   0, 118, 132}, { 64, 104}}, //0 idle 1
	{Pico_ArcMain_Idle, {119,   0, 112, 126}, { 64,  97}}, //1 idle 2
	{Pico_ArcMain_Idle, {  0, 132, 132, 128}, { 64,  96}}, //2 idle 3
	{Pico_ArcMain_Idle, {133, 128, 132, 132}, { 64,  99}}, //3 idle 4
	
	{Pico_ArcMain_Hit0, {  0,   0, 126, 130}, { 80,  99}}, //4 left 1
	{Pico_ArcMain_Hit0, {126,   0, 130, 130}, { 74, 100}}, //5 left 2
	
	{Pico_ArcMain_Hit0, {  0, 128, 128, 114}, { 56,  85}}, //6 down 1
	{Pico_ArcMain_Hit0, {128, 128, 128, 114}, { 54,  86}}, //7 down 2
	
	{Pico_ArcMain_Up, {  0,   0, 110, 140}, { 62, 114}}, //8 up 1
	{Pico_ArcMain_Up, {110,   0, 110, 140}, { 70, 114}}, //9 up 2
	
	{Pico_ArcMain_Right, {  0,  0, 99, 139}, { 54,110}}, //10 right 1
	{Pico_ArcMain_Right, {100,  0, 98, 139}, { 54,110}}, //11 right 2

	{Pico_ArcMain_Reload0, {  0,  0, 112, 134}, { 56,105}}, //10 right 1
	{Pico_ArcMain_Reload0, {112,  0, 108, 134}, { 50,105}}, //10 right 1

	{Pico_ArcMain_Reload1, {  0,  0, 111, 134}, { 52,105}}, //10 right 1
	{Pico_ArcMain_Reload1, {111,  0, 109, 134}, { 50,105}}, //11 right 2

	{Pico_ArcMain_Shoot0, {  0,  0, 168, 137}, { 52,105}}, //10 right 1
	{Pico_ArcMain_Shoot1, {  0,  0, 200, 137}, { 52,105}}, //10 right 1
	{Pico_ArcMain_Shoot2, {  0,  0, 204, 137}, { 52,105}}, //10 right 1
};

static const Animation char_pico_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},       //CharAnim_Left
	{2, (const u8[]){14, 15, 12, 13, ASCR_BACK, 1}} ,   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){16, 17, 18, ASCR_BACK, 1}},   //CharAnim_RightAlt
};

//Pico character functions
void Char_Pico_SetFrame(void *user, u8 frame)
{
	Char_Pico *this = (Char_Pico*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_pico_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Pico_Tick(Character *character)
{
	Char_Pico *this = (Char_Pico*)character;

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
	Animatable_Animate(&character->animatable, (void*)this, Char_Pico_SetFrame);
	Character_Draw(character, &this->tex, &char_pico_frame[this->frame]);
}

void Char_Pico_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);

}

void Char_Pico_Free(Character *character)
{
	Char_Pico *this = (Char_Pico*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Pico_New(fixed_t x, fixed_t y)
{
	//Allocate pico object
	Char_Pico *this = Mem_Alloc(sizeof(Char_Pico));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Pico_New] Failed to allocate pico object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Pico_Tick;
	this->character.set_anim = Char_Pico_SetAnim;
	this->character.free = Char_Pico_Free;
	
	Animatable_Init(&this->character.animatable, char_pico_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 3;
	
	this->character.focus_x = FIXED_DEC(40,1);
	this->character.focus_y = FIXED_DEC(-95,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\PICO.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim", //Pico_ArcMain_Idle0
		"hit0.tim", //Pico_ArcMain_Hit0
		"up.tim", //Pico_ArcMain_Up
		"right.tim", //Pico_ArcMain_Right
		"reload0.tim", //Pico_ArcMain_Right
		"reload1.tim", //Pico_ArcMain_Right
		"shoot0.tim", //Pico_ArcMain_Right
		"shoot1.tim", //Pico_ArcMain_Right
		"shoot2.tim", //Pico_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
