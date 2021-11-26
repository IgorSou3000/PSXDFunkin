/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week3a2.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Week 3 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
    IO_Data arc_buddy, arc_buddy_ptr[5];
	IO_Data arc_buddy2, arc_buddy2_ptr[5];

	Gfx_Tex tex_back0; //Buildings
	Gfx_Tex tex_back1; //Lights
	Gfx_Tex tex_back2; //Rooftop
	Gfx_Tex tex_back3; //Background train arc
	Gfx_Tex tex_back4; //Sky
	
	//Window state
	u8 win_r, win_g, win_b;
	fixed_t win_time;
	
	//Train state
	fixed_t train_x;
	u8 train_timer;

	//Buddy state
	Gfx_Tex tex_buddy;
	u8 buddy_frame, buddy_tex_id;
	
	Animatable buddy_animatable;

	//Buddy2 state
	Gfx_Tex tex_buddy2;
	u8 buddy2_frame, buddy2_tex_id;
	
	Animatable buddy2_animatable;
} Back_Week3a2;

//Buddy animation and rects
static const CharFrame buddy_frame[] = {
	{0, {  0,   0,  180,  176}, { 48, 114}}, //0 left 1
	{1, {  0,   0,  180,  176}, { 48, 115}}, //3 left 4
	{2, {  0,   0,  180,  176}, { 48, 116}}, //4 left 5
	{3, {  0,   0,  180,  176}, { 48, 116}}, //4 left 5
	{4, {  0,   0,  180,  176}, { 48, 116}}, //4 left 5
	
	
};

static const Animation buddy_anim[] = {
	{2, (const u8[]){0,  1,  2,  3,  4, ASCR_BACK, 1}}, //Left
};

//Buddy2 animation and rects
static const CharFrame buddy2_frame[] = {
	{0, {  0,   0,  168,  132}, { 50, 119}}, //0 left 1
	{1, {  0,   0,  168,  132}, { 50, 119}}, //1 left 2
	{2, {  0,   0,  168,  132}, { 50, 119}}, //2 left 3
	{3, {  0,   0,  168,  132}, { 50, 119}}, //3 left 4
	{4, {  0,   0,  168,  132}, { 50, 119}}, //4 left 5
	
};

static const Animation buddy2_anim[] = {
	{2, (const u8[]){0,  1,  2,  3,  4, ASCR_BACK, 1}}, //Left
};

//Buddy functions
void Week3a2_Buddy_SetFrame(void *user, u8 frame)
{
	Back_Week3a2 *this = (Back_Week3a2*)user;
	
	//Check if this is a new frame
	if (frame != this->buddy_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &buddy_frame[this->buddy_frame = frame];
		if (cframe->tex != this->buddy_tex_id)
			Gfx_LoadTex(&this->tex_buddy, this->arc_buddy_ptr[this->buddy_tex_id = cframe->tex], 0);
	}
}

void Week3a2_Buddy_Draw(Back_Week3a2 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &buddy_frame[this->buddy_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_buddy, &src, &dst, stage.camera.bzoom);
}

//Buddy functions
void Week3a2_Buddy2_SetFrame(void *user, u8 frame)
{
	Back_Week3a2 *this = (Back_Week3a2*)user;
	
	//Check if this is a new frame
	if (frame != this->buddy2_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &buddy2_frame[this->buddy2_frame = frame];
		if (cframe->tex != this->buddy2_tex_id)
			Gfx_LoadTex(&this->tex_buddy2, this->arc_buddy2_ptr[this->buddy2_tex_id = cframe->tex], 0);
	}
}

void Week3a2_Buddy2_Draw(Back_Week3a2 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &buddy2_frame[this->buddy2_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_buddy2, &src, &dst, stage.camera.bzoom);
}

//Week 3 background functions
static const u8 win_cols[][3] = {
	{ 49, 162, 253},
	{ 49, 253, 140},
	{251,  51, 245},
	{253,  69,  49},
	{251, 166,  51},
};

#define TRAIN_START_X FIXED_DEC(500,1)
#define TRAIN_END_X    FIXED_DEC(-8000,1)
#define TRAIN_TIME_A 6
#define TRAIN_TIME_B 14

void Back_Week3a2_Window(Back_Week3a2 *this)
{
	const u8 *win_col = win_cols[RandomRange(0, COUNT_OF(win_cols) - 1)];
	this->win_r = win_col[0];
	this->win_g = win_col[1];
	this->win_b = win_col[2];
	this->win_time = FIXED_DEC(3,1);
}

void Back_Week3a2_DrawBG(StageBack *back)
{
	Back_Week3a2 *this = (Back_Week3a2*)back;
	
	fixed_t fx, fy;

	//Animate and draw Buddy
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 3)
		{
			case 1:
				Animatable_SetAnim(&this->buddy_animatable, 0);
				break;
		}
	}
	Animatable_Animate(&this->buddy_animatable, (void*)this, Week3a2_Buddy_SetFrame);

	Week3a2_Buddy_Draw(this, FIXED_DEC(-180,1) - fx, FIXED_DEC (60,1) - fy);

		//Animate and draw Buddy 2
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 3)
		{
			case 1:
				Animatable_SetAnim(&this->buddy2_animatable, 0);
				break;
		}
	}
	Animatable_Animate(&this->buddy2_animatable, (void*)this, Week3a2_Buddy2_SetFrame);

	Week3a2_Buddy2_Draw(this, FIXED_DEC(110,1) - fx, FIXED_DEC(90,1) - fy);

	
	//Update window
	if (this->win_time > 0)
	{
		this->win_time -= timer_dt;
		if (this->win_time < 0)
			this->win_time = 0;
	}
	if (stage.note_scroll >= 0 && (stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0xF) == 0)
		Back_Week3a2_Window(this);
	
	//Draw rooftop
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	static const struct Back_Week3a2_RoofPiece
	{
		RECT src;
		fixed_t scale;
	} roof_piece[] = {
		{{  0, 0,  16, 256}, FIXED_MUL(FIXED_DEC(3,1) * 7 / 10, FIXED_UNIT + FIXED_DEC(SCREEN_WIDEOADD,2) * 10 / 336)},
		{{ 16, 0,  55, 256}, FIXED_DEC(1,1) * 9 / 10},
		{{55, 0,  128, 256}, FIXED_DEC(295,100) * 7 / 10},
		{{199, 0,  55, 256}, FIXED_DEC(1,1) * 9 / 10},
		{{255, 0,   0, 256}, FIXED_DEC(16,1) + FIXED_DEC(SCREEN_WIDEOADD2,1)}
	};
	
	RECT_FIXED roof_dst = {
		FIXED_DEC(-215,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-104,1) - fy,
		0,
		FIXED_DEC(250,1)
	};
	
	const struct Back_Week3a2_RoofPiece *roof_p = roof_piece;
	for (size_t i = 0; i < COUNT_OF(roof_piece); i++, roof_p++)
	{
		roof_dst.w = roof_p->src.w ? (roof_p->src.w * roof_p->scale) : roof_p->scale;
		Stage_DrawTex(&this->tex_back2, &roof_p->src, &roof_dst, stage.camera.bzoom);
		roof_dst.x += roof_dst.w;
	}
	
	
	//Draw arc
	RECT arcl_src = {0, 0, 256, 105};
	RECT_FIXED arcl_dst = {
		FIXED_DEC(-230,1) - fx,
		FIXED_DEC(-136,1) - fy,
		FIXED_DEC(500,1),
		FIXED_DEC(221,1)
	};

	
	Stage_DrawTex(&this->tex_back3, &arcl_src, &arcl_dst, stage.camera.bzoom);
	
	
	//Draw lights
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	if (this->win_time >= 0)
	{
		RECT lightl_src = {0, 0, 256, 132};
		RECT_FIXED lightl_dst = {
			FIXED_DEC(-175,1) - fx,
			FIXED_DEC(-80,1) - fy,
			FIXED_DEC(155,1),
			FIXED_DEC(103,1)
		};
		
		RECT lightr_src = {0, 132, 256, 124};
		RECT_FIXED lightr_dst = {
			FIXED_DEC(58,1) - fx,
			FIXED_DEC(-64,1) - fy,
			FIXED_DEC(158,1),
			FIXED_DEC(95,1)
		};
		
		u8 win_r = (((fixed_t)this->win_r * this->win_time) >> FIXED_SHIFT) / 6;
		u8 win_g = (((fixed_t)this->win_g * this->win_time) >> FIXED_SHIFT) / 6;
		u8 win_b = (((fixed_t)this->win_b * this->win_time) >> FIXED_SHIFT) / 6;
		
		Stage_DrawTexCol(&this->tex_back1, &lightl_src, &lightl_dst, stage.camera.bzoom, win_r, win_g, win_b);
		Stage_DrawTexCol(&this->tex_back1, &lightr_src, &lightr_dst, stage.camera.bzoom, win_r, win_g, win_b);
	}
	
	RECT building_src = {0, 0, 255, 128};
	RECT_FIXED building_dst = {
		FIXED_DEC(-204,1) - fx,
		FIXED_DEC(-120,1) - fy,
		FIXED_DEC(200,1),
		FIXED_DEC(120,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &building_src, &building_dst, stage.camera.bzoom);
	building_dst.x += building_dst.w;
	building_src.y += building_src.h;
	Stage_DrawTex(&this->tex_back0, &building_src, &building_dst, stage.camera.bzoom);
	
	RECT building_fillsrc = {174, 255, 1, 0};
	RECT building_fill = {0, SCREEN_HEIGHT * 3 / 7, SCREEN_WIDTH, SCREEN_HEIGHT * 4 / 7};
	Gfx_DrawTex(&this->tex_back0, &building_fillsrc, &building_fill);
	
	//Draw sky
	fx = stage.camera.x >> 3;
	fy = stage.camera.y >> 3;
	
	RECT sky_src = {0, 0, 255, 128};
	RECT_FIXED sky_dst = {
		FIXED_DEC(-170,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-119,1) - FIXED_DEC(SCREEN_WIDEOADD,4) - fy,
		FIXED_DEC(400,1) + FIXED_DEC(SCREEN_WIDEOADD,1),
		FIXED_DEC(110,1) + FIXED_DEC(SCREEN_WIDEOADD,2)
	};
	
	Stage_DrawTex(&this->tex_back4, &sky_src, &sky_dst, stage.camera.bzoom);
}

void Back_Week3a2_Free(StageBack *back)
{
	Back_Week3a2 *this = (Back_Week3a2*)back;

	Mem_Free(this->arc_buddy);
	Mem_Free(this->arc_buddy2);
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week3a2_New(void)
{
	//Allocate background structure
	Back_Week3a2 *this = (Back_Week3a2*)Mem_Alloc(sizeof(Back_Week3a2));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week3a2_DrawBG;
	this->back.free = Back_Week3a2_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK3A2\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Gfx_LoadTex(&this->tex_back4, Archive_Find(arc_back, "back4.tim"), 0);
	Mem_Free(arc_back);

	//Load buddy textures
	this->arc_buddy = IO_Read("\\WEEK3A2\\BUDDY.ARC;1");
	this->arc_buddy_ptr[0] = Archive_Find(this->arc_buddy, "nene0.tim");
	this->arc_buddy_ptr[1] = Archive_Find(this->arc_buddy, "nene1.tim");
	this->arc_buddy_ptr[2] = Archive_Find(this->arc_buddy, "nene2.tim");
	this->arc_buddy_ptr[3] = Archive_Find(this->arc_buddy, "nene3.tim");
	this->arc_buddy_ptr[4] = Archive_Find(this->arc_buddy, "nene4.tim");

	//Load buddy textures
	this->arc_buddy2 = IO_Read("\\WEEK3A2\\BUDDY2.ARC;1");
	this->arc_buddy2_ptr[0] = Archive_Find(this->arc_buddy2, "darne0.tim");
	this->arc_buddy2_ptr[1] = Archive_Find(this->arc_buddy2, "darne1.tim");
	this->arc_buddy2_ptr[2] = Archive_Find(this->arc_buddy2, "darne2.tim");
	this->arc_buddy2_ptr[3] = Archive_Find(this->arc_buddy2, "darne3.tim");
	this->arc_buddy2_ptr[4] = Archive_Find(this->arc_buddy2, "darne4.tim");
	
	//Initialize window state
	this->win_time = -1;
	
	//Initialize train state
	this->train_x = TRAIN_END_X;
	this->train_timer = RandomRange(TRAIN_TIME_A, TRAIN_TIME_B);

		//Initialize buddy state
	Animatable_Init(&this->buddy_animatable, buddy_anim);
	Animatable_SetAnim(&this->buddy_animatable, 0);
	this->buddy_frame = this->buddy_tex_id = 0xFF; //Force art load

		//Initialize buddy 2 state
	Animatable_Init(&this->buddy2_animatable, buddy2_anim);
	Animatable_SetAnim(&this->buddy2_animatable, 0);
	this->buddy2_frame = this->buddy2_tex_id = 0xFF; //Force art load

	
	return (StageBack*)this;
}
