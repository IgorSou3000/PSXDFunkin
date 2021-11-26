/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "monster.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Monster character structure
enum
{
	Monster_ArcMain_Idle0,
	Monster_ArcMain_Idle1,
	Monster_ArcMain_Idle2,
	Monster_ArcMain_Idle3,
	Monster_ArcMain_Idle4,
	Monster_ArcMain_Idle5,
	Monster_ArcMain_Idle6,
	Monster_ArcMain_Idle7,
	Monster_ArcMain_Left0,
	Monster_ArcMain_Left1,
	Monster_ArcMain_Left2,
	Monster_ArcMain_Left3,
	Monster_ArcMain_Down0,
	Monster_ArcMain_Down1,
	Monster_ArcMain_Down2,
	Monster_ArcMain_Down3,
	Monster_ArcMain_Up0,
	Monster_ArcMain_Up1,
	Monster_ArcMain_Up2,
	Monster_ArcMain_Up3,
	Monster_ArcMain_Right0,
	Monster_ArcMain_Right1,
	Monster_ArcMain_Right2,
	Monster_ArcMain_Right3,
	
	Monster_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Monster_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Monster;

//Monster character definitions
static const CharFrame char_monster_frame[] = {
	{Monster_ArcMain_Idle0, {  0,   0, 192, 177}, { 42, 157}}, //0 idle 1
	{Monster_ArcMain_Idle1, {  0,   0, 192, 177}, { 43, 155}}, //1 idle 2
	{Monster_ArcMain_Idle2, {  0,   0, 192, 177}, { 42, 155}}, //2 idle 3
	{Monster_ArcMain_Idle3, {  0,   0, 192, 192}, { 41, 155}}, //3 idle 4
	
	{Monster_ArcMain_Idle4, {  0,   0, 192, 192}, { 44, 167}}, //0 idle 1
	{Monster_ArcMain_Idle5, {  0,   0, 192, 192}, { 45, 169}}, //1 idle 2
	{Monster_ArcMain_Idle6, {  0,   0, 192, 195}, { 44, 169}}, //2 idle 3
	{Monster_ArcMain_Idle7, {  0,   0, 192, 195}, { 43, 172}}, //3 idle 4

	{Monster_ArcMain_Left0, {  0,   0, 188, 191}, { 45, 170}}, //0 idle 1
	{Monster_ArcMain_Left1, {  0,   0, 188, 191}, { 43, 170}}, //1 idle 2
	{Monster_ArcMain_Left2, {  0,   0, 188, 191}, { 43, 170}}, //2 idle 3
	{Monster_ArcMain_Left3, {  0,   0, 188, 191}, { 43, 170}}, //3 idle 4

	{Monster_ArcMain_Down0, {  0,   0, 188, 179}, { 41, 154}}, //0 idle 1
	{Monster_ArcMain_Down1, {  0,   0, 188, 179}, { 37, 158}}, //1 idle 2
	{Monster_ArcMain_Down2, {  0,   0, 188, 179}, { 41, 158}}, //2 idle 3
	{Monster_ArcMain_Down3, {  0,   0, 188, 179}, { 41, 158}}, //3 idle 4

	{Monster_ArcMain_Up0, {  0,   0, 188, 205}, { 37, 185}}, //0 idle 1
	{Monster_ArcMain_Up1, {  0,   0, 188, 205}, { 45, 180}}, //1 idle 2
	{Monster_ArcMain_Up2, {  0,   0, 188, 205}, { 45, 180}}, //2 idle 3
	{Monster_ArcMain_Up3, {  0,   0, 188, 205}, { 43, 180}}, //3 idle 4

	{Monster_ArcMain_Right0, {  0,   0, 188, 191}, { 40, 170}}, //0 idle 1
	{Monster_ArcMain_Right1, {  0,   0, 188, 191}, { 42, 170}}, //1 idle 2
	{Monster_ArcMain_Right2, {  0,   0, 188, 191}, { 42, 170}}, //2 idle 3
	{Monster_ArcMain_Right3, {  0,   0, 188, 191}, { 41, 170}}, //3 idle 4
};

static const Animation char_monster_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5,  6,  7, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 8,  9, 10, 11, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){12, 13, 14, 15, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){16, 17, 18, 19, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){20, 21, 22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Monster character functions
void Char_Monster_SetFrame(void *user, u8 frame)
{
	Char_Monster *this = (Char_Monster*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_monster_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Monster_Tick(Character *character)
{
	Char_Monster *this = (Char_Monster*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Monster_SetFrame);
	Character_Draw(character, &this->tex, &char_monster_frame[this->frame]);
}

void Char_Monster_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Monster_Free(Character *character)
{
	Char_Monster *this = (Char_Monster*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Monster_New(fixed_t x, fixed_t y)
{
	//Allocate Monster object
	Char_Monster *this = Mem_Alloc(sizeof(Char_Monster));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Monster_New] Failed to allocate Monster object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Monster_Tick;
	this->character.set_anim = Char_Monster_SetAnim;
	this->character.free = Char_Monster_Free;
	
	Animatable_Init(&this->character.animatable, char_monster_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 6;
	
	this->character.focus_x = FIXED_DEC(88,1);
	this->character.focus_y = FIXED_DEC(-110,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MONSTER.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Monster_ArcMain_Idle0
		"idle1.tim", //Monster_ArcMain_Idle1
		"idle2.tim", //Monster_ArcMain_Idle0
		"idle3.tim", //Monster_ArcMain_Idle1
		"idle4.tim", //Monster_ArcMain_Idle0
		"idle5.tim", //Monster_ArcMain_Idle1
		"idle6.tim", //Monster_ArcMain_Idle0
		"idle7.tim", //Monster_ArcMain_Idle1
		"left0.tim", //Monster_ArcMain_Idle1
		"left1.tim", //Monster_ArcMain_Idle1
		"left2.tim", //Monster_ArcMain_Idle1
		"left3.tim", //Monster_ArcMain_Idle1
		"down0.tim", //Monster_ArcMain_Idle1
		"down1.tim", //Monster_ArcMain_Idle1
		"down2.tim", //Monster_ArcMain_Idle1
		"down3.tim", //Monster_ArcMain_Idle1
		"up0.tim", //Monster_ArcMain_Idle1
		"up1.tim", //Monster_ArcMain_Idle1
		"up2.tim", //Monster_ArcMain_Idle1
		"up3.tim", //Monster_ArcMain_Idle1
		"right0.tim", //Monster_ArcMain_Idle1
		"right1.tim", //Monster_ArcMain_Idle1
		"right2.tim", //Monster_ArcMain_Idle1
		"right3.tim", //Monster_ArcMain_Idle1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
