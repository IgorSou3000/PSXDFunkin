/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gfvs.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

#include "speaker.h"


//GFVS character structure
enum
{
	GFVS_ArcMain_BopLeft,
	GFVS_ArcMain_BopRight,
	GFVS_ArcMain_BopRigh2,
	GFVS_ArcMain_Cry,
	GFVS_ArcMain_Happy,
	GFVS_ArcMain_Left,
	GFVS_ArcMain_Down,
	GFVS_ArcMain_Up,
	GFVS_ArcMain_Right,
	
	GFVS_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[GFVS_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Speaker
	Speaker speaker;
	
	//Pico test
	u16 *pico_p;
} Char_GFVS;

//GFVS character definitions
static const CharFrame char_gfvs_frame[] = {
	{GFVS_ArcMain_BopLeft, {  0,   0,  85, 116}, { 48,  86}}, //0 bop left 1
	{GFVS_ArcMain_BopLeft, { 85,   0,  85, 114}, { 48,  86}}, //1 bop left 2
	{GFVS_ArcMain_BopLeft, {170,   0,  84, 114}, { 48,  86}}, //2 bop left 3
	{GFVS_ArcMain_BopLeft, {  0, 116,  84, 114}, { 48,  80}}, //3 bop left 4
	{GFVS_ArcMain_BopLeft, { 86, 115,  84, 114}, { 35,  81}}, //4 bop left 5
	{GFVS_ArcMain_BopLeft, {172, 115,  84, 114}, { 35,  81}}, //5 bop left 6
	
	{GFVS_ArcMain_BopRight, {  0,   0,  90, 120}, { 36,  87}}, //6 bop right 1
	{GFVS_ArcMain_BopRight, { 92,   0,  90, 120}, { 37,  87}}, //7 bop right 2
	{GFVS_ArcMain_BopRight, {  0, 125,  90, 120}, { 35,  89}}, //8 bop right 3
	{GFVS_ArcMain_BopRight, { 92, 125,  90, 120}, { 35,  89}}, //9 bop right 4

	{GFVS_ArcMain_BopRigh2, {  0,  0, 100, 116}, { 46,  87}}, //10 bop right 5
	
	{GFVS_ArcMain_Cry, {  0,   0,  86, 103}, { 48,  73}}, //12 cry
	{GFVS_ArcMain_Cry, { 87,   0,  86, 103}, { 48,  73}}, //13 cry

	{GFVS_ArcMain_Happy, {  0,   0,  86, 115}, { 36,  86}}, //12 cry
	{GFVS_ArcMain_Happy, { 87,   0,  86, 115}, { 36,  86}}, //13 cry

	{GFVS_ArcMain_Left, {  0,   0,  86, 110}, { 50,  81}}, //12 cry
	{GFVS_ArcMain_Left, { 87,   0,  86, 110}, { 45,  82}}, //13 cry

	{GFVS_ArcMain_Down, {  0,   0,  92, 110}, { 44,  80}}, //12 cry
	{GFVS_ArcMain_Down, { 90,   0,  90, 110}, { 41,  80}}, //13 cry

	{GFVS_ArcMain_Up, {  0,   0,  92, 110}, { 40,  82}}, //12 cry
	{GFVS_ArcMain_Up, { 92,   0,  92, 110}, { 41,  82}}, //13 cry

	{GFVS_ArcMain_Right, {  0,   0,  92, 110}, { 37,  82}}, //12 cry
	{GFVS_ArcMain_Right, { 91,   0,  89, 110}, { 41,  82}}, //13 cry
};

static const Animation char_gfvs_anim[CharAnim_Max] = {
	{2, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                           //CharAnim_Idle
	{1, (const u8[]){ 15, 15,  16, 16, 16, ASCR_BACK, 1}},  //CharAnim_Left
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}},                           //CharAnim_LeftAlt
	{1, (const u8[]){ 17, 17, 18, 18, 18, ASCR_BACK, 1}},                              //CharAnim_Down
	{2, (const u8[]){11, 12, ASCR_REPEAT}},                            //CharAnim_DownAlt
	{1, (const u8[]){ 19, 19, 20, 20, 20, ASCR_BACK, 1}},                            //CharAnim_Up
	{1, (const u8[]){13, 14, 13, 14, 14, 13, 13, ASCR_BACK, 1}},                          //CharAnim_UpAlt
	{1, (const u8[]){21, 21, 22, 22, 22, ASCR_BACK, 1}},  //CharAnim_Right
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 10, ASCR_BACK, 1}},                        //CharAnim_RightAlt
};

//GFVS character functions
void Char_GFVS_SetFrame(void *user, u8 frame)
{
	Char_GFVS *this = (Char_GFVS*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gfvs_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_GFVS_Tick(Character *character)
{
	Char_GFVS *this = (Char_GFVS*)character;

	
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	{
		Character_CheckEndSing(character);

		if (this->pico_p != NULL)
	{
		if (stage.note_scroll >= 0)
		{
			//Scroll through Pico chart
			u16 substep = stage.note_scroll >> FIXED_SHIFT;
			while (substep >= ((*this->pico_p) & 0x7FFF))
			{
				//Play animation and bump speakers
				character->set_anim(character, ((*this->pico_p) & 0x8000) ? CharAnim_Right : CharAnim_LeftAlt);
				Speaker_Bump(&this->speaker);
				this->pico_p++;
			}
		}
	}

	   else
	   {
		
		if (stage.flag & STAGE_FLAG_JUST_STEP)
			{
			if ((Animatable_Ended(&character->animatable) || character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_RightAlt) &&
				(character->animatable.anim != CharAnim_Left &&
				 character->animatable.anim != CharAnim_Down &&
				 character->animatable.anim != CharAnim_Up &&
				 character->animatable.anim != CharAnim_Right) &&
				(stage.song_step & 0x3) == 0)
			    character->set_anim(character, CharAnim_Idle);
		

				//Bump speakers
				Speaker_Bump(&this->speaker);
			}
	     }
	}
		

	//Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
		
				case StageId_1_4: //Bopeebo peace
					if (stage.song_step > 64 && stage.song_step < 192 && (stage.song_step & 0x3F) == 60)
						character->set_anim(character, CharAnim_UpAlt);
					break;
				default:
					break;
			}
		}
	

		
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GFVS_SetFrame);
	Character_Draw(character, &this->tex, &char_gfvs_frame[this->frame]);
	
	//Tick speakers
	Speaker_Tick(&this->speaker, character->x, character->y);
}

void Char_GFVS_SetAnim(Character *character, u8 anim)
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


void Char_GFVS_Free(Character *character)
{
	Char_GFVS *this = (Char_GFVS*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_GFVS_New(fixed_t x, fixed_t y)
{
	//Allocate GFVS object
	Char_GFVS *this = Mem_Alloc(sizeof(Char_GFVS));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_GFVS_New] Failed to allocate GFVS object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_GFVS_Tick;
	this->character.set_anim = Char_GFVS_SetAnim;
	this->character.free = Char_GFVS_Free;
	
	Animatable_Init(&this->character.animatable, char_gfvs_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 12;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-45,1);
	this->character.focus_zoom = FIXED_DEC(13,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GFVS.ARC;1");
	
	const char **pathp = (const char *[]){
		"bopleft.tim",  //GFVS_ArcMain_BopLeft
		"bopright.tim", //GFVS_ArcMain_BopRight
		"boprigh2.tim", //GFVS_ArcMain_BopRight 2
		"cry.tim",      //GFVS_ArcMain_Cry
		"happy.tim",      //GFVS_ArcMain_Cry
		"left.tim",      //GFVS_ArcMain_Cry
		"down.tim",      //GFVS_ArcMain_Cry
		"up.tim",      //GFVS_ArcMain_Cry
		"right.tim",      //GFVS_ArcMain_Cry
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

		this->pico_p = NULL;
	
	return (Character*)this;
}
