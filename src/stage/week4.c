/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week4.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Week 4 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_hench, arc_hench_ptr[2];
	
	Gfx_Tex tex_back0; //Foreground limo
	Gfx_Tex tex_back1; //Background limo
	Gfx_Tex tex_back1a2; //Background limo 2
	Gfx_Tex tex_back2; //Sunset
	Gfx_Tex tex_back3; //Car
	
	//Car state
	fixed_t car_x;
	fixed_t car_timer;
	
	//Henchmen state
	Gfx_Tex tex_hench;
	u8 hench_frame, hench_tex_id;
	
	Animatable hench_animatable;
} Back_Week4;

//Henchmen animation and rects
static const CharFrame henchmen_frame[10] = {
	{0, {  0,   0, 103, 111}, { 71,  98}}, //0 left 1
	{0, {103,   0, 108, 111}, { 71,  97}}, //1 left 2
	{0, {  0, 131, 108, 111}, { 69,  95}}, //2 left 3
	{0, {103, 131,  63, 111}, { 42,  95}}, //3 left 4
	{0, {172, 131,  63, 111}, { 42,  95}}, //4 left 5
	
	{1, {  0,   0, 108, 111}, { 35, 101}}, //5 right 1
	{1, {108,   0, 108, 111}, { 33, 100}}, //6 right 2
	{1, {  0, 140, 108, 111}, { 33,  99}}, //7 right 3
	{1, {108, 140,  64, 111}, { 26,  99}}, //8 right 4
	{1, {168, 140,  64, 111}, { 26,  99}}, //9 right 5
};

static const Animation henchmen_anim[2] = {
	{2, (const u8[]){0, 1, 1, 2, 2, ASCR_BACK, 1}}, //Left
	{2, (const u8[]){5, 6, 6, 7, 7, ASCR_BACK, 1}}, //Right
};

//Henchmen functions
void Week4_Henchmen_SetFrame(void *user, u8 frame)
{
	Back_Week4 *this = (Back_Week4*)user;
	
	//Check if this is a new frame
	if (frame != this->hench_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &henchmen_frame[this->hench_frame = frame];
		if (cframe->tex != this->hench_tex_id)
			Gfx_LoadTex(&this->tex_hench, this->arc_hench_ptr[this->hench_tex_id = cframe->tex], 0);
	}
}

void Week4_Henchmen_Draw(Back_Week4 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &henchmen_frame[this->hench_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_hench, &src, &dst, stage.camera.bzoom);
}

//Week 4 background functions
#define CAR_START_X FIXED_DEC(-500,1)
#define CAR_END_X    FIXED_DEC(500,1)
#define CAR_TIME_A FIXED_DEC(5,1)
#define CAR_TIME_B FIXED_DEC(14,1)

void Back_Week4_DrawBG(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	fixed_t fx, fy;

	//Draw foreground limo
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fglimo_src = {0, 0, 256, 71};
	RECT_FIXED fglimo_dst = {
		FIXED_DEC(-290,1) - fx,
		FIXED_DEC(50,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &fglimo_src, &fglimo_dst, stage.camera.bzoom);

	//Draw foreground limo 2
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fglimo2_src = {0, 68, 256, 66};
	RECT_FIXED fglimo2_dst = {
		FIXED_DEC(-40,1) - fx,
		FIXED_DEC(53,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &fglimo2_src, &fglimo2_dst, stage.camera.bzoom);

	//Draw foreground limo 3
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fglimo3_src = {0,133, 256, 67};
	RECT_FIXED fglimo3_dst = {
		FIXED_DEC(150,1) - fx,
		FIXED_DEC(53,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &fglimo3_src, &fglimo3_dst, stage.camera.bzoom);

	//Move car
	this->car_timer -= timer_dt;
	if (this->car_timer <= 0)
	{
		this->car_timer = RandomRange(CAR_TIME_A, CAR_TIME_B);
		this->car_x = CAR_START_X;
	}
	
	if (this->car_x < CAR_END_X)
		this->car_x += timer_dt * 2700;
	
	//Draw car
	fx = stage.camera.x * 4 / 3;
	fy = stage.camera.y * 4 / 3;
	
	RECT car_src = {0, 0, 256, 128};
	RECT_FIXED car_dst = {
		this->car_x - fx,
		FIXED_DEC(-85,1) - fy,
		FIXED_DEC(200,1),
		FIXED_DEC(240,1)
	};
	
	Stage_DrawTex(&this->tex_back3, &car_src, &car_dst, stage.camera.bzoom);
	
	//Animate and draw henchmen
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 7)
		{
			case 0:
				Animatable_SetAnim(&this->hench_animatable, 0);
				break;
			case 4:
				Animatable_SetAnim(&this->hench_animatable, 1);
				break;
		}
	}
	Animatable_Animate(&this->hench_animatable, (void*)this, Week4_Henchmen_SetFrame);
	
	Week4_Henchmen_Draw(this, FIXED_DEC(-50,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw(this,  FIXED_DEC(50,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(150,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(250,1) - fx, FIXED_DEC(30,1) - fy);

	//Draw background limo
	//Use same scroll as henchmen
	RECT bglimo_src = {0, 0, 256, 89};
	RECT_FIXED bglimo_dst = {
		FIXED_DEC(-210,1) - fx,
		FIXED_DEC(15,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &bglimo_src, &bglimo_dst, stage.camera.bzoom);

	//Draw background limo 2
	//Use same scroll as henchmen
	RECT bglimo2_src = {0, 89, 256,126};
	RECT_FIXED bglimo2_dst = {
		FIXED_DEC(-20,1) - fx,
		FIXED_DEC(27,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &bglimo2_src, &bglimo2_dst, stage.camera.bzoom);
	
	//Draw sunset
	fx = stage.camera.x >> 4;
	fy = stage.camera.y >> 4;
	
	RECT sunset_src = {0, 0, 256, 256};
	RECT_FIXED sunset_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(340 + SCREEN_WIDEOADD,1),
		FIXED_DEC(260,1)
	};
	
	Stage_DrawTex(&this->tex_back2, &sunset_src, &sunset_dst, stage.camera.bzoom);
}

void Back_Week4_Free(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	//Free henchmen archive
	Mem_Free(this->arc_hench);
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week4_New(void)
{
	//Allocate background structure
	Back_Week4 *this = (Back_Week4*)Mem_Alloc(sizeof(Back_Week4));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week4_DrawBG;
	this->back.free = Back_Week4_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK4\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back1a2, Archive_Find(arc_back, "back1a2.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Mem_Free(arc_back);
	
	//Load henchmen textures
	this->arc_hench = IO_Read("\\WEEK4\\HENCH.ARC;1");
	this->arc_hench_ptr[0] = Archive_Find(this->arc_hench, "hench0.tim");
	this->arc_hench_ptr[1] = Archive_Find(this->arc_hench, "hench1.tim");
	
	//Initialize car state
	this->car_x = CAR_END_X;
	this->car_timer = RandomRange(CAR_TIME_A, CAR_TIME_B);
	
	//Initialize henchmen state
	Animatable_Init(&this->hench_animatable, henchmen_anim);
	Animatable_SetAnim(&this->hench_animatable, 0);
	this->hench_frame = this->hench_tex_id = 0xFF; //Force art load
	
	return (StageBack*)this;
}
