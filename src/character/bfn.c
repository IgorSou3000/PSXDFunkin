/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfn.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_BFN_skull[15] = {
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
	BFN_ArcMain_Idle,
	BFN_ArcMain_Hit0,  //Left Down
	BFN_ArcMain_Miss0, //Left Down
	BFN_ArcMain_Hit1,  //Up Right
	BFN_ArcMain_Miss1, //Up Right
	BFN_ArcMain_Peace,
	BFN_ArcMain_Dead0, //BREAK
	BFN_ArcMain_Shoot0, //shoot left and down
	BFN_ArcMain_Shoot1, //shoot up0
	BFN_ArcMain_Shoot2, //shoot up1
	BFN_ArcMain_Shoot3, //shoot right
	
	BFN_ArcMain_Max,
};

enum
{
	BFN_ArcDead_Dead1, //Mic Drop
	BFN_ArcDead_Dead2, //Twitch
	BFN_ArcDead_Retry, //Retry prompt
	
	BFN_ArcDead_Max,
};

#define BFN_Arc_Max BFN_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFN_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_BFN_skull)];
	u8 skull_scale;
} Char_BFN;

//Boyfriend player definitions
static const CharFrame char_BFN_frame[] = {
	{BFN_ArcMain_Idle, {  0,   0, 116, 126}, { 50,  93}}, //0 idle 1
	{BFN_ArcMain_Idle, {116,   0, 140, 128}, { 53,  93}}, //1 idle 2
	{BFN_ArcMain_Idle, {  0, 126, 116, 128}, { 52,  93}}, //2 idle 3
	{BFN_ArcMain_Idle, {116, 126, 140, 128}, { 55,  93}}, //3 idle 4
	
	{BFN_ArcMain_Hit0,  {  0,   0, 122, 122}, { 42,  93}}, //4 left 1
	{BFN_ArcMain_Hit0,  {122,   0, 122, 122}, { 46,  93}}, //5 left 2
	{BFN_ArcMain_Miss0, {  0,   0, 122, 122}, { 42,  93}}, //6 left miss 1
	{BFN_ArcMain_Miss0, {122,   0, 122, 122}, { 46,  93}}, //7 left miss 2
	
	{BFN_ArcMain_Hit0,  {  0, 122, 122, 122}, { 39,  84}}, //8 down 1
	{BFN_ArcMain_Hit0,  {122, 122, 122, 122}, { 43,  83}}, //9 down 2
	{BFN_ArcMain_Miss0, {  0, 122, 122, 122}, { 37,  83}}, //10 down miss 1
	{BFN_ArcMain_Miss0, {122, 122, 122, 122}, { 37,  83}}, //11 down miss 2
	
	{BFN_ArcMain_Hit1,  {  0,   0, 122, 122}, { 34,  99}}, //12 up 1
	{BFN_ArcMain_Hit1,  {122,   0, 122, 122}, { 34,  99}}, //13 up 2
	{BFN_ArcMain_Miss1, {  0,   0, 122, 122}, { 34,  99}}, //14 up miss 1
	{BFN_ArcMain_Miss1, {122,   0, 122, 122}, { 34,  99}}, //15 up miss 2
	
	{BFN_ArcMain_Hit1,  {  0, 124, 114, 122}, { 43,  95}}, //16 right 1
	{BFN_ArcMain_Hit1,  {114, 124, 124, 122}, { 44,  94}}, //17 right 2
	{BFN_ArcMain_Miss1, {  0, 124, 114, 122}, { 42,  94}}, //18 right miss 1
	{BFN_ArcMain_Miss1, {114, 124, 124, 122}, { 42,  94}}, //19 right miss 2
	
	{BFN_ArcMain_Peace, {  0,   0, 122, 124}, { 41,  98}}, //20 peace 1
	{BFN_ArcMain_Peace, {122,   0, 122, 124}, { 41,  97}}, //21 peace 2
	{BFN_ArcMain_Peace, {  0, 124, 122, 124}, { 42,  97}}, //22 peace 3
	
	{BFN_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //23 dead0 0
	{BFN_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //24 dead0 1
	{BFN_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //25 dead0 2
	{BFN_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //26 dead0 3
	
	{BFN_ArcDead_Dead1, {  0,   0, 128, 127}, { 53,  98}}, //27 dead1 0
	{BFN_ArcDead_Dead1, {128,   0, 128, 127}, { 53,  98}}, //28 dead1 1
	{BFN_ArcDead_Dead1, {  0, 128, 128, 128}, { 58,  96}}, //29 dead1 2
	{BFN_ArcDead_Dead1, {128, 128, 128, 128}, { 56,  96}}, //30 dead1 3
	
	{BFN_ArcDead_Dead2, {  0,   0, 128, 128}, { 58,  98}}, //31 dead2 body twitch 0
	{BFN_ArcDead_Dead2, {128,   0, 128, 128}, { 56,  98}}, //32 dead2 body twitch 1
	{BFN_ArcDead_Dead2, {  0, 128, 128, 128}, { 58,  98}}, //33 dead2 balls twitch 0
	{BFN_ArcDead_Dead2, {128, 128, 128, 128}, { 56, 100}}, //34 dead2 balls twitch 1

	{BFN_ArcMain_Shoot0, {  0,  0, 125, 102}, { 42,  84}}, //18 right miss 1
	{BFN_ArcMain_Shoot0, {125,  0, 124, 102}, { 42,  84}}, //19 right miss 2
	
	{BFN_ArcMain_Shoot0, {  0, 128, 125, 118}, { 52, 100}}, //18 right miss 1
	{BFN_ArcMain_Shoot0, {125, 128, 125, 118}, { 42, 100}}, //19 right miss 2

	{BFN_ArcMain_Shoot1, {  0,  0, 136, 116}, { 42,  100}}, //18 right miss 1
	{BFN_ArcMain_Shoot2, {  0,  0, 136, 116}, { 42,  100}}, //19 right miss 2

	{BFN_ArcMain_Shoot3, {  0,  0, 128, 100}, { 42,  84}}, //18 right miss 1
	{BFN_ArcMain_Shoot3, {128,  0, 128, 100}, { 42,  84}}, //19 right miss 2
};

static const Animation char_bfn_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  2,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},             //CharAnim_Left
	{1, (const u8[]){35, 36, ASCR_BACK, 1}},     //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},             //CharAnim_Down
	{1, (const u8[]){37, 38, ASCR_BACK, 1}},     //CharAnim_DownAlt
	{2, (const u8[]){12, 13, ASCR_BACK, 1}},             //CharAnim_Up
	{1, (const u8[]){39, 40, ASCR_BACK, 1}},     //CharAnim_UpAlt
	{2, (const u8[]){16, 17, ASCR_BACK, 1}},             //CharAnim_Right
	{1, (const u8[]){41, 42, ASCR_BACK, 1}},     //CharAnim_RightAlt

	{1, (const u8[]){ 4,  6,  6,  7, ASCR_BACK, 1}},     //CharAnim_LeftAlt
	{1, (const u8[]){ 8, 10, 10, 11, ASCR_BACK, 1}},     //CharAnim_DownAlt
	{1, (const u8[]){12, 14, 14, 15, ASCR_BACK, 1}},     //CharAnim_UpAlt
	{1, (const u8[]){16, 18, 18, 19, ASCR_BACK, 1}},     //CharAnim_RightAlt

	{2, (const u8[]){20, 21, 22, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //PlayerAnim_Sweat
	
	{5, (const u8[]){23, 24, 25, 26, 26, 26, 26, 26, 26, 26, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){26, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){27, 28, 29, 30, 30, 30, 30, 30, 30, 30, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){30, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){31, 32, 30, 30, 30, 30, 30, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){33, 34, 30, 30, 30, 30, 30, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){30, 30, 30, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){33, 34, 30, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_BFN_SetFrame(void *user, u8 frame)
{
	Char_BFN *this = (Char_BFN*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_BFN_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFN_Tick(Character *character)
{
	Char_BFN *this = (Char_BFN*)character;

		
	//Camera stuff
	if ((stage.stage_id == StageId_1_3) && stage.song_step >= 0)
	{
	 this->character.focus_x = FIXED_DEC(-47,1);
	 this->character.focus_y = FIXED_DEC(-65,1);
	 this->character.focus_zoom = FIXED_DEC(1,1);
	}

	
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
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFN, skull); i++, frag++)
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
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(53,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
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
	Animatable_Animate(&character->animatable, (void*)this, Char_BFN_SetFrame);
	Character_Draw(character, &this->tex, &char_BFN_frame[this->frame]);
}

void Char_BFN_SetAnim(Character *character, u8 anim)
{
	Char_BFN *this = (Char_BFN*)character;
	
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
				"dead1.tim", //BFN_ArcDead_Dead1
				"dead2.tim", //BFN_ArcDead_Dead2
				"retry.tim", //BFN_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BFN_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFN_Free(Character *character)
{
	Char_BFN *this = (Char_BFN*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BFN_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BFN *this = Mem_Alloc(sizeof(Char_BFN));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFN_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFN_Tick;
	this->character.set_anim = Char_BFN_SetAnim;
	this->character.free = Char_BFN_Free;
	
	Animatable_Init(&this->character.animatable, char_bfn_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;

	 if (stage.stage_id == StageId_1_3)
	    {
		    this->character.focus_x = FIXED_DEC(-40, 1);
		    this->character.focus_y = FIXED_DEC(-140, 1);
		    this->character.focus_zoom = FIXED_DEC(22, 10);
	    }

	  else if (stage.stage_id >= StageId_3_1 && stage.stage_id <= StageId_3_3 )
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

	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFN.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFNDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim",  //BF_ArcMain_Idle
		"hit0.tim",  //BF_ArcMain_Hit0
		"miss0.tim", //BF_ArcMain_Miss0
		"hit1.tim",  //BF_ArcMain_Hit1
		"miss1.tim", //BF_ArcMain_Miss1
		"peace.tim", //BF_ArcMain_Peace
		"dead0.tim", //BF_ArcMain_Dead0
		"shoot0.tim", //BF_ArcMain_Shoot0
		"shoot1.tim", //BF_ArcMain_Shoot1
		"shoot2.tim", //BF_ArcMain_Shoot0
		"shoot3.tim", //BF_ArcMain_Shoot1
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
	memcpy(this->skull, char_BFN_skull, sizeof(char_BFN_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFN, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
