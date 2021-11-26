#include "clucky.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Garcello character structure
enum
{
	Gcl_ArcMain_Idle0,
	Gcl_ArcMain_Idle1,
	Gcl_ArcMain_Left0,
	Gcl_ArcMain_Left1,
	Gcl_ArcMain_Miss0,
	Gcl_ArcMain_Down0,
    Gcl_ArcMain_Down1,
	Gcl_ArcMain_Up0,
	Gcl_ArcMain_Up1,
	Gcl_ArcMain_Right0,
	Gcl_ArcMain_Right1,
	
	Gcl_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Gcl_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Gcl;

//Gcl character definitions
static const CharFrame char_Gcl_frame[] = {
	{Gcl_ArcMain_Idle0, {  0,   0,   80, 162}, { 48, 155}}, //0 idle 1
	{Gcl_ArcMain_Idle0, { 80,   0,   80, 162}, { 46, 155}}, //1 idle 2
	{Gcl_ArcMain_Idle1, {  0,   0,   79, 162}, { 48, 156}}, //2 idle 3
	{Gcl_ArcMain_Idle1, { 79,   0,   80, 162}, { 48, 155}}, //3 idle 4
	
	{Gcl_ArcMain_Left0, {  0,   0,  90,162}, { 54,  155}}, //4 left 1
	{Gcl_ArcMain_Left0, { 90,   0,  90,162}, { 54,  155}}, //5 left 2
	{Gcl_ArcMain_Left1, {  0,   0,  90,162}, { 54,  155}}, //4 left 1
	{Gcl_ArcMain_Left1, { 90,   0,  90,162}, { 54,  155}}, //5 left 2
	{Gcl_ArcMain_Miss0, {  0,   0,  90,162}, { 54,  155}}, //4 left 1
	{Gcl_ArcMain_Miss0, { 90,   0,  90,162}, { 54,  155}}, //5 left 2
	
	{Gcl_ArcMain_Down0, {  0,   0,  90, 158}, { 44, 150}}, //6 down 1
	{Gcl_ArcMain_Down0, { 90,   0,  90, 158}, { 41, 150}}, //7 down 2
	{Gcl_ArcMain_Down1, {  0,   0,  91, 157}, { 44, 150}}, //6 down 1
	{Gcl_ArcMain_Down1, { 91,   0,  89, 157}, { 41, 150}}, //7 down 2
	
	{Gcl_ArcMain_Up0, {  0,   0,  94, 170}, { 46, 161}}, //8 up 1
	{Gcl_ArcMain_Up0, { 94,   0,  94, 170}, { 46, 161}}, //9 up 2
	{Gcl_ArcMain_Up1, {  0,   0,  96, 170}, { 46, 161}}, //8 up 1
	{Gcl_ArcMain_Up1, { 96,   0,  90, 170}, { 46, 161}}, //9 up 2

	{Gcl_ArcMain_Right0, {  0,   0,  96, 164}, { 48, 155}}, //10 right 1
	{Gcl_ArcMain_Right0, { 96,   0,  96, 164}, { 48, 155}}, //11 right 2
	{Gcl_ArcMain_Right1, {  0,   0,  93, 164}, { 48, 155}}, //10 right 1
	{Gcl_ArcMain_Right1, { 93,   0,  93, 164}, { 48, 155}}, //11 right 2
};

static const Animation char_Gcl_anim[CharAnim_Max] = {
	{2, (const u8[]){ 1,  2,  3,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  6,  7, ASCR_BACK, 1}},         //CharAnim_Left
	{2, (const u8[]){ 7,  8,  8,  9, ASCR_BACK, 1}},   //CharAnim_LeftAlt
	{2, (const u8[]){10, 11, 12, 13, ASCR_BACK, 1}},         //CharAnim_Down
	{2, (const u8[]){10, 11, 12, 13, ASCR_BACK, 1}},   //CharAnim_DownAlt
	{2, (const u8[]){14, 15, 16, 17, ASCR_BACK,  1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){18, 19, 20, 21,ASCR_BACK,  1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Gcl character functions
void Char_Gcl_SetFrame(void *user, u8 frame)
{
	Char_Gcl *this = (Char_Gcl*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_Gcl_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Gcl_Tick(Character *character)
{
	Char_Gcl *this = (Char_Gcl*)character;

		if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_Right))
	
	//Perform idle dance
	Character_CheckEndSing(character);
	
	{
		if ((Animatable_Ended(&character->animatable) || character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_RightAlt) &&
		   (character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt) &&
		    (stage.song_step & 0x3) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Gcl_SetFrame);
	Character_Draw(character, &this->tex, &char_Gcl_frame[this->frame]);
}

void Char_Gcl_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Gcl_Free(Character *character)
{
	Char_Gcl *this = (Char_Gcl*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Gcl_New(fixed_t x, fixed_t y)
{
	//Allocate Gcl object
	Char_Gcl *this = Mem_Alloc(sizeof(Char_Gcl));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Gcl_New] Failed to allocate Gcl object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Gcl_Tick;
	this->character.set_anim = Char_Gcl_SetAnim;
	this->character.free = Char_Gcl_Free;
	
	Animatable_Init(&this->character.animatable, char_Gcl_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character stage information
	this->character.spec = CHAR_SPEC_MISSANIM;


	this->character.health_i = 11;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-130,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CLUCKY.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Gcl_ArcMain_Idle0
		"idle1.tim", //Gcl_ArcMain_Idle1
		"left0.tim",  //Gcl_ArcMain_Left
		"left1.tim",  //Gcl_ArcMain_Left
		"miss0.tim",  //Gcl_ArcMain_Left
		"down0.tim",  //Gcl_ArcMain_Down
		"down1.tim",  //Gcl_ArcMain_Down
		"up0.tim",    //Gcl_ArcMain_Up
		"up1.tim",    //Gcl_ArcMain_Up
		"right0.tim", //Gcl_ArcMain_Right
		"right1.tim", //Gcl_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
