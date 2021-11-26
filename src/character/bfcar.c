/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfcar.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_bfcar_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	BFCar_ArcMain_Idle0,
	BFCar_ArcMain_Idle1,
	BFCar_ArcMain_Hit0,  //Left Down
	BFCar_ArcMain_Miss0, //Left Down
	BFCar_ArcMain_Hit0a2, //left down 2
	BFCar_ArcMain_Hit1,  //Up Right
	BFCar_ArcMain_Miss1, //Up Right
	BFCar_ArcMain_Hit1a2,  //Up Right 2
	BFCar_ArcMain_Dead0, //BREAK
	BFCar_ArcMain_Shoot0, //shoot left and down
	BFCar_ArcMain_Shoot1, //shoot up0
	BFCar_ArcMain_Shoot2, //shoot up1
	BFCar_ArcMain_Shoot3, //shoot right
	
	BFCar_ArcMain_Max,
};

enum
{
	BFCar_ArcDead_Dead1, //Mic Drop
	BFCar_ArcDead_Dead2, //Twitch
	BFCar_ArcDead_Retry, //Retry prompt
	
	BFCar_ArcDead_Max,
};

#define BFCar_Arc_Max BFCar_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFCar_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfcar_skull)];
	u8 skull_scale;
} Char_BFCar;

//Boyfriend player definitions
static const CharFrame char_bfcar_frame[] = {
	{BFCar_ArcMain_Idle0, {  0,   0, 116, 128}, { 48,  93}}, //0 idle 1
	{BFCar_ArcMain_Idle0, {114,   0, 116, 126}, { 48,  93}}, //1 idle 2
	{BFCar_ArcMain_Idle0, {  0, 126, 116, 128}, { 47,  93}}, //2 idle 3
	{BFCar_ArcMain_Idle0, {112, 126, 117, 128}, { 52,  93}}, //3 idle 4
	{BFCar_ArcMain_Idle1, {  0,   0, 114, 128}, { 50,  93}}, //2 idle 3
	{BFCar_ArcMain_Idle1, {114,   0, 114, 128}, { 50,  93}}, //3 idle 4

	
	{BFCar_ArcMain_Hit0,  {  0,   0, 122, 122}, { 42,  93}}, //4 left 1
	{BFCar_ArcMain_Hit0,  {122,   0, 122, 122}, { 46,  93}}, //5 left 2
	{BFCar_ArcMain_Miss0, {  0,   0, 122, 122}, { 42,  93}}, //6 left miss 1
	{BFCar_ArcMain_Miss0, {122,   0, 122, 122}, { 46,  93}}, //7 left miss 2
	
	{BFCar_ArcMain_Hit0,  {  0, 122, 122, 122}, { 39,  84}}, //8 down 1
	{BFCar_ArcMain_Hit0,  {122, 122, 122, 122}, { 39,  83}}, //9 down 2
	{BFCar_ArcMain_Miss0, {  0, 122, 122, 122}, { 37,  83}}, //10 down miss 1
	{BFCar_ArcMain_Miss0, {122, 122, 122, 122}, { 37,  83}}, //11 down miss 2

	{BFCar_ArcMain_Hit0a2,  {  0,   0, 122, 122}, { 42,  93}}, //4 left 2 1
	{BFCar_ArcMain_Hit0a2,  {122,   0, 122, 122}, { 46,  93}}, //5 left 2 2
	
	{BFCar_ArcMain_Hit0a2,  {  0, 122, 122, 122}, { 39,  84}}, //8 down 2 1
	{BFCar_ArcMain_Hit0a2,  {122, 122, 122, 122}, { 39,  84}}, //9 down 2 2
	
	{BFCar_ArcMain_Hit1,  {  0,   0, 122, 122}, { 34,  99}}, //12 up 1
	{BFCar_ArcMain_Hit1,  {122,   0, 122, 122}, { 34,  99}}, //13 up 2
	{BFCar_ArcMain_Miss1, {  0,   0, 122, 122}, { 34,  99}}, //14 up miss 1
	{BFCar_ArcMain_Miss1, {122,   0, 122, 122}, { 34,  99}}, //15 up miss 2
	
	{BFCar_ArcMain_Hit1,  {  0, 124, 114, 122}, { 43,  95}}, //16 right 1
	{BFCar_ArcMain_Hit1,  {114, 124, 124, 122}, { 44,  94}}, //17 right 2
	{BFCar_ArcMain_Miss1, {  0, 124, 114, 122}, { 42,  94}}, //18 right miss 1
	{BFCar_ArcMain_Miss1, {114, 124, 124, 122}, { 42,  94}}, //19 right miss 2

	{BFCar_ArcMain_Hit1a2,  {  0,   0, 122, 122}, { 34,  99}}, //12 up 2 1
	{BFCar_ArcMain_Hit1a2,  {122,   0, 122, 122}, { 34,  99}}, //13 up 2 2
	
	{BFCar_ArcMain_Hit1a2,  {  0, 124, 114, 122}, { 43,  95}}, //16 right 2 1
	{BFCar_ArcMain_Hit1a2,  {114, 124, 124, 122}, { 44,  94}}, //17 right 2 2
	
	{BFCar_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //23 dead0 0
	{BFCar_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //24 dead0 1
	{BFCar_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //25 dead0 2
	{BFCar_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //26 dead0 3
	
	{BFCar_ArcDead_Dead1, {  0,   0, 128, 127}, { 53,  98}}, //27 dead1 0
	{BFCar_ArcDead_Dead1, {128,   0, 128, 127}, { 53,  98}}, //28 dead1 1
	{BFCar_ArcDead_Dead1, {  0, 128, 128, 128}, { 58,  96}}, //29 dead1 2
	{BFCar_ArcDead_Dead1, {128, 128, 128, 128}, { 56,  96}}, //30 dead1 3
	
	{BFCar_ArcDead_Dead2, {  0,   0, 128, 128}, { 58,  98}}, //31 dead2 body twitch 0
	{BFCar_ArcDead_Dead2, {128,   0, 128, 128}, { 56,  98}}, //32 dead2 body twitch 1
	{BFCar_ArcDead_Dead2, {  0, 128, 128, 128}, { 58,  98}}, //33 dead2 balls twitch 0
	{BFCar_ArcDead_Dead2, {128, 128, 128, 128}, { 56, 100}}, //34 dead2 balls twitch 1

	{BFCar_ArcMain_Shoot0, {  0,  0, 125, 102}, { 42,  84}}, //18 right miss 1
	{BFCar_ArcMain_Shoot0, {125,  0, 124, 102}, { 42,  84}}, //19 right miss 2
	
	{BFCar_ArcMain_Shoot0, {  0, 128, 125, 118}, { 52, 100}}, //18 right miss 1
	{BFCar_ArcMain_Shoot0, {125, 128, 125, 118}, { 42, 100}}, //19 right miss 2

	{BFCar_ArcMain_Shoot1, {  0,  0, 136, 116}, { 42,  100}}, //18 right miss 1
	{BFCar_ArcMain_Shoot2, {  0,  0, 136, 116}, { 42,  100}}, //19 right miss 2

	{BFCar_ArcMain_Shoot3, {  0,  0, 128, 100}, { 42,  84}}, //18 right miss 1
	{BFCar_ArcMain_Shoot3, {128,  0, 128, 100}, { 42,  84}}, //19 right miss 2
};

static const Animation char_bfcar_anim[PlayerAnim_Max] = {
	{1, (const u8[]){ 0,  1,  2,  3,  2,  3,  2,  3,  4,  5, ASCR_BACK, 2}}, //CharAnim_Idle
	{1, (const u8[]){ 6,  7, 14, 15, 14, 15, 14, 15, ASCR_BACK, 2}},             //CharAnim_Left
	{1, (const u8[]){35, 36, ASCR_BACK, 1}},     //CharAnim_LeftAlt
	{1, (const u8[]){10, 11, 16, 17, 16, 17, 16, 17, ASCR_BACK, 2}},             //CharAnim_Down
	{1, (const u8[]){37, 38, ASCR_BACK, 1}},     //CharAnim_DownAlt
	{1, (const u8[]){18, 19, 26, 27, 26, 27, 26, 27, ASCR_BACK, 2}},             //CharAnim_Up
	{1, (const u8[]){39, 40, ASCR_BACK, 1}},     //CharAnim_UpAlt
	{1, (const u8[]){22, 23, 28, 29, 28, 29, 28, 29, ASCR_BACK, 2}},             //CharAnim_Right
	{1, (const u8[]){41, 42, ASCR_BACK, 1}},     //CharAnim_RightAlt

	{1, (const u8[]){ 6,  8,  9,  9, ASCR_BACK, 1}},     //CharAnim_LeftAlt
	{1, (const u8[]){10, 12, 13, 13, ASCR_BACK, 1}},     //CharAnim_DownAlt
	{1, (const u8[]){18, 20, 21, 21, ASCR_BACK, 1}},     //CharAnim_UpAlt
	{1, (const u8[]){22, 24, 25, 25, ASCR_BACK, 1}},     //CharAnim_RightAlt


	{2, (const u8[]){20, 21, 22, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //PlayerAnim_Sweat
	
	{5, (const u8[]){23+7, 24+7, 25+7, 26+7, 26+7, 26+7, 26+7, 26+7, 26+7, 26+7, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){26+7, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){27+7, 28+7, 29+7, 30+7, 30+7, 30+7, 30+7, 30+7, 30+7, 30+7, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){30+7, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){31+7, 32+7, 30+7, 30+7, 30+7, 30+7, 30+7, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){33+7, 34+7, 30+7, 30+7, 30+7, 30+7, 30+7, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){30+7, 30+7, 30+7, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){33+7, 34+7, 30+7, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_BFCar_SetFrame(void *user, u8 frame)
{
	Char_BFCar *this = (Char_BFCar*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfcar_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFCar_Tick(Character *character)
{
	Char_BFCar *this = (Char_BFCar*)character;
	
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
		
		//Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_1_4: //Tutorial peace
					if (stage.song_step > 64 && stage.song_step < 192 && (stage.song_step & 0x3F) == 60)
						character->set_anim(character, PlayerAnim_Peace);
					break;
				case StageId_1_1: //Bopeebo peace
					if ((stage.song_step & 0x1F) == 28)
						character->set_anim(character, PlayerAnim_Peace);
					break;
				default:
					break;
			}
		}
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFCar, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(12,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BFCar_SetFrame);
	Character_Draw(character, &this->tex, &char_bfcar_frame[this->frame]);
}

void Char_BFCar_SetAnim(Character *character, u8 anim)
{
	Char_BFCar *this = (Char_BFCar*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //BFCar_ArcDead_Dead1
				"dead2.tim", //BFCar_ArcDead_Dead2
				"retry.tim", //BFCar_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BFCar_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFCar_Free(Character *character)
{
	Char_BFCar *this = (Char_BFCar*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BFCar_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BFCar *this = Mem_Alloc(sizeof(Char_BFCar));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFCar_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFCar_Tick;
	this->character.set_anim = Char_BFCar_SetAnim;
	this->character.free = Char_BFCar_Free;
	
	Animatable_Init(&this->character.animatable, char_bfcar_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;

	if (stage.stage_id >= StageId_2_1 && stage.stage_id <= StageId_2_4 )
	    {
		 this->character.focus_x = FIXED_DEC(-20, 1);
		 this->character.focus_y = FIXED_DEC(-68, 1);
		 this->character.focus_zoom = FIXED_DEC(1, 1);
	    }
    else if (stage.stage_id >= StageId_3_1 && stage.stage_id <= StageId_3_2 )
	    {
		 this->character.focus_x = FIXED_DEC(-30, 1);
		 this->character.focus_y = FIXED_DEC(-88, 1);
		 this->character.focus_zoom = FIXED_DEC(1, 1);
	    }
		
	else
    {
	 this->character.focus_x = FIXED_DEC(-47,1);
	 this->character.focus_y = FIXED_DEC(-65,1);
	 this->character.focus_zoom = FIXED_DEC(1,1);
    }
	

     this->character.health_i = 0;
	
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFCAR.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",  //BFCar_ArcMain_Idle
		"idle1.tim",  //BFCar_ArcMain_Idle
		"hit0.tim",  //BFCar_ArcMain_Hit0
		"miss0.tim", //BFCar_ArcMain_Miss0
		"hit0a2.tim",  //BFCar_ArcMain_Hit0a2
		"hit1.tim",  //BFCar_ArcMain_Hit1
		"miss1.tim", //BFCar_ArcMain_Miss1
		"hit1a2.tim",  //BFCar_ArcMain_Hit1a2
		"dead0.tim", //BFCar_ArcMain_Dead0
		"shoot0.tim", //BFCar_ArcMain_Shoot0
		"shoot1.tim", //BFCar_ArcMain_Shoot1
		"shoot2.tim", //BFCar_ArcMain_Shoot0
		"shoot3.tim", //BFCar_ArcMain_Shoot1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_bfcar_skull, sizeof(char_bfcar_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFCar, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}



