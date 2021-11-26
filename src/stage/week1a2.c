/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week1a2.h"

#include "../archive.h"
#include "../mem.h"

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Stage
	Gfx_Tex tex_back1; //curtain
	Gfx_Tex tex_back2; //back
    Gfx_Tex tex_back3; //plate
} Back_Week1a2;

//Week 1 background functions
void Back_Week1a2_DrawBG(StageBack *back)
{
	Back_Week1a2 *this = (Back_Week1a2*)back;
	
	fixed_t fx, fy;

	//Draw Plate
	fx = stage.camera.x;
	fy = stage.camera.y;
	
		static const struct Back_Week1_PlatePiece
	{
		RECT src;
		fixed_t scale;
	} plate_piece[] = {
		{{  0, 0, 0, 0}, FIXED_DEC(1,1)},
	
	};
	
	RECT_FIXED plate_dst = {
		FIXED_DEC(0,1) - fx,
		FIXED_DEC(0,1) - fy,
		0,
		FIXED_DEC(0,1)
	};
	
	RECT plate_src = {0, 0,188,21};
	RECT_FIXED plate_fill;
    plate_fill.x = FIXED_DEC(-165,1) - fx,
	plate_fill.y = FIXED_DEC(64,1) - fy,
	plate_fill.w = FIXED_DEC(340,1);
	plate_fill.h = FIXED_DEC(45,1);
	Stage_DrawTex(&this->tex_back3, &plate_src, &plate_fill, stage.camera.bzoom);
	
	const struct Back_Week1_PlatePiece *plate_p = plate_piece;
	for (size_t i = 0; i < COUNT_OF(plate_piece); i++, plate_p++)
	{
		plate_dst.w = plate_p->src.w ? (plate_p->src.w * plate_p->scale) : plate_p->scale;
		Stage_DrawTex(&this->tex_back3, &plate_p->src, &plate_dst, stage.camera.bzoom);
		plate_dst.x += plate_dst.w;
	}

	//Draw Stage
    fx = stage.camera.x;
	fy = stage.camera.y;
	
	static const struct Back_Week1a2_BackPiece
	{
		RECT src;
		fixed_t scale;
	} back_piece[] = {
		{{  0, 0, 0, 0}, FIXED_DEC(1,1)},
	
	};
	
	RECT_FIXED back_dst = {
		FIXED_DEC(0,1) - fx,
		FIXED_DEC(0,1) - fy,
		0,
		FIXED_DEC(0,1)
	};
	
	RECT back_src = {0, 0,256, 59};
	RECT_FIXED back_fill;
    back_fill.x = FIXED_DEC(-213,1) - fx,
	back_fill.y = FIXED_DEC(0,1) - fy,
	back_fill.w = FIXED_DEC(384,1);
	back_fill.h = FIXED_DEC(144,1);
	Stage_DrawTex(&this->tex_back0, &back_src, &back_fill, stage.camera.bzoom);
	
	const struct Back_Week1a2_BackPiece *back_p = back_piece;
	for (size_t i = 0; i < COUNT_OF(back_piece); i++, back_p++)
	{
		back_dst.w = back_p->src.w ? (back_p->src.w * back_p->scale) : back_p->scale;
		Stage_DrawTex(&this->tex_back0, &back_p->src, &back_dst, stage.camera.bzoom);
		back_dst.x += back_dst.w;
	}

	//Draw stages
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	static const struct Back_Week1a2_CurtainPiece
	{
		RECT src;
		fixed_t scale;
	} curtain_piece[] = {
		{{  0, 0, 0, 0}, FIXED_DEC(1,1)},
	
	};
	
	RECT_FIXED curtain_dst = {
		FIXED_DEC(0,1) - fx,
		FIXED_DEC(0,1) - fy,
		0,
		FIXED_DEC(0,1)
	};
	
	RECT curtain_src = {0, 0,256,74};
	RECT_FIXED curtain_fill;
    curtain_fill.x = FIXED_DEC(-420,1) - fx,
	curtain_fill.y = FIXED_DEC(-190,1) - fy,
	curtain_fill.w = FIXED_DEC(410,1);
	curtain_fill.h = FIXED_DEC(106,1);
	Stage_DrawTex(&this->tex_back1, &curtain_src, &curtain_fill, stage.camera.bzoom);
	
	const struct Back_Week1a2_CurtainPiece *curtain_p = curtain_piece;
	for (size_t i = 0; i < COUNT_OF(curtain_piece); i++, curtain_p++)
	{
		curtain_dst.w = curtain_p->src.w ? (curtain_p->src.w * curtain_p->scale) : curtain_p->scale;
		Stage_DrawTex(&this->tex_back1, &curtain_p->src, &curtain_dst, stage.camera.bzoom);
		curtain_dst.x += curtain_dst.w;
	}

	//Draw Back
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	static const struct Back_Week1a2_StagePiece
	{
		RECT src;
		fixed_t scale;
	} stage_piece[] = {
		{{  0, 0, 0, 0}, FIXED_DEC(1,1)},
	
	};
	
	RECT_FIXED stage_dst = {
		FIXED_DEC(0,1) - fx,
		FIXED_DEC(0,1) - fy,
		0,
		FIXED_DEC(0,1)
	};
	
	RECT stage_src = {0, 0,256,120};
	RECT_FIXED stage_fill;
    stage_fill.x = FIXED_DEC(-215,1) - fx,
	stage_fill.y = FIXED_DEC(-190,1) - fy,
	stage_fill.w = FIXED_DEC(392,1);
	stage_fill.h = FIXED_DEC(205,1);
	Stage_DrawTex(&this->tex_back2, &stage_src, &stage_fill, stage.camera.bzoom);
	
	const struct Back_Week1a2_StagePiece *stage_p = stage_piece;
	for (size_t i = 0; i < COUNT_OF(stage_piece); i++, stage_p++)
	{
		stage_dst.w = stage_p->src.w ? (stage_p->src.w * stage_p->scale) : stage_p->scale;
		Stage_DrawTex(&this->tex_back2, &stage_p->src, &stage_dst, stage.camera.bzoom);
		stage_dst.x += stage_dst.w;
	}
}

	

void Back_Week1a2_Free(StageBack *back)
{
	Back_Week1a2 *this = (Back_Week1a2*)back;
	
	//Free structure
	Mem_Free(this);
}


StageBack *Back_Week1a2_New(void)
{
	//Allocate background structure
	Back_Week1a2 *this = (Back_Week1a2*)Mem_Alloc(sizeof(Back_Week1a2));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week1a2_DrawBG;
	this->back.free = Back_Week1a2_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK1A2\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
