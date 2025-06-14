// Matrix Screensaver
// Copyright (c) J Brown 2003 (catch22.net)
// Copyright (c) 2011-2025 Henry++

#pragma once

#include "routine.h"

#include "resource.h"
#include "app.h"

// config
#define UID 0xDEADBEEF

#define CLASS_FULLSCREEN APP_NAME_SHORT L"_Fullscreen"
#define CLASS_PREVIEW APP_NAME_SHORT L"_Preview"

#define GLYPH_REDRAW 0x8000
#define GLYPH_BLANK 0x4000
#define RND_MASK 0xB400

#define AMOUNT_MIN 1
#define AMOUNT_MAX 26
#define AMOUNT_DEFAULT 26

#define DENSITY_MIN 5
#define DENSITY_MAX 50
#define DENSITY_DEFAULT 30

#define SPEED_MIN 1
#define SPEED_MAX 10
#define SPEED_DEFAULT 6

#define HUE_MIN 1
#define HUE_MAX 255
#define HUE_DEFAULT 85

#define HUE_RANDOM FALSE
#define HUE_RANDOM_SMOOTHTRANSITION TRUE

// constants inferred from matrix.bmp
#define MAX_INTENSITY 5 // number of intensity levels
#define GLYPH_WIDTH 14 // width of each glyph (pixels)
#define GLYPH_HEIGHT 14 // height of each glyph (pixels)

typedef struct _STATIC_DATA
{
	LONG amount;
	LONG density;
	LONG speed;
	LONG hue;
	BOOLEAN is_esc_only;
	BOOLEAN is_random;
	BOOLEAN is_smooth;
	BOOLEAN is_preview;
} STATIC_DATA, *PSTATIC_DATA;

typedef ULONG GLYPH;
typedef PULONG PGLYPH;

//
//	The "matrix" is basically an array of these
//  column structures, positioned side-by-side
//
typedef struct _MATRIX_COLUMN
{
	PGLYPH glyph;

	LONG state;
	LONG countdown;

	ULONG blip_pos;
	ULONG blip_length;

	ULONG length;
	ULONG run_length;

	BOOLEAN is_started;
} MATRIX_COLUMN, *PMATRIX_COLUMN;

typedef struct _MATRIX
{
        // bitmap containing glyphs.
        HDC hdc;
        HBITMAP hbitmap;

        // cached window dc and back-buffer for drawing
        HDC hdc_window;
        HDC hdc_back;
        HBITMAP hbitmap_back;

        INT current_hue;

	ULONG width;
	ULONG height;
	ULONG numcols;
	ULONG numrows;

	MATRIX_COLUMN column[1];
} MATRIX, *PMATRIX;
