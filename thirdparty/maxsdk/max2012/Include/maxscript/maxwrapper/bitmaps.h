/*	
 *		BitMaps.h - MAX bitmap access classes
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "..\..\bmmlib.h"
// forward declarations
class MAXBitMapWindow;
class MotionTracker;
class MAXBitMap;

// entry in MAXBitMap window table
struct mbm_window				
{
	HWND		window;
	MAXBitMap*	mbm;
};

/* ------------------------ MAXBitMap ------------------------------ */

applyable_class (MAXBitMap)

class MAXBitMap : public Value
{
public:
	BitmapInfo			  bi;					// our BitMapInfo 
	Bitmap*				  bm;					// the actual bitmap
	Tab<MotionTracker*>	  trackers;				// any motion trackers
	MotionTracker*		  dragger;				// tracker currently under drag
	WNDPROC				  main_window_proc;		// original display window proc if ours installed
	GBuffer*			  gb;					// GBuffer if non-NULL
	GBufReader*			  gbr;					// current GBuffer reader if non-NULL
	short				  flags;	

	static Tab<mbm_window> windows;				// table of MAXBitMap windows currently open

	ScripterExport MAXBitMap();
	ScripterExport MAXBitMap(BitmapInfo bi, Bitmap* bm);
			   ~MAXBitMap();

	static void setup();
	static MAXBitMap* find_window_mbm(HWND hwnd);

				classof_methods (MAXBitMap, Value);
#	define		is_bitmap(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MAXBitMap))
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void sprin1(CharStream* s);

	Value*		new_motionTracker();
	void		install_window_proc();

#include "..\macros\define_implementations.h"
	def_visible_generic ( display,			"display"); 
	def_visible_generic ( unDisplay,		"unDisplay" ); 
	def_visible_generic ( save,				"save" ); 
	def_visible_generic ( gotoFrame,		"gotoFrame"); 
	def_visible_generic ( close,			"close");
	def_visible_generic ( getTracker,		"getTracker" ); 
	def_visible_generic ( deleteTracker,	"deleteTracker" ); 
	use_generic			( copy,				"copy" ); 
	def_visible_generic ( zoom,				"zoom" ); 
	def_visible_generic ( crop,				"crop" ); 
	def_visible_generic ( setAsBackground,	"setAsBackground" ); 
	def_visible_generic ( getPixels,		"getPixels" ); 
	def_visible_generic ( setPixels,		"setPixels" ); 
	def_visible_generic ( getIndexedPixels, "getIndexedPixels" ); 
	def_visible_generic ( setIndexedPixels, "setIndexedPixels" ); 
	def_visible_generic ( getChannel,		"getChannel" ); 
	def_visible_generic ( getChannelAsMask, "getChannelAsMask" ); 
	use_generic			( free,				"free");

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	ScripterExport void		to_fpvalue(FPValue& v);
};

#define BM_SAVED		0x0001			// bitmap has been written to and output steam is open
#define BM_READONLY	0x0002			// existing bitmap opened (and so readonly).
#define BM_FILEBACKED	0x0004			// bitmap is backed by a file

/* -------------------- MotionTracker -------------------------- */

applyable_class (MotionTracker)

class MotionTracker : public Value
{
public:
	MAXBitMap*	mbm;			// the bitmap I'm tracking
	int			index;			// which tracker in that bitmap
	int			cur_frame;		// frame I last tracked
	POINT		center;			// current feature center
	RECT		bounds;			// feature bounds relative to center
	RECT		motion_bounds;	// maximum frame-to-frame motion relative to feature center
	POINT		mouse_down_at;	// mouse pos at mousedown
	int			handle_x;		// handle pos at mouse_down..
	int			handle_y;		// handle pos at mouse_down..
	int			handle;			// which handle is dragging
	BYTE*		target;			// current target image as 3 BYTE RGB per pixel
	POINT*		track_cache;	// keeps a cache of tracking coords, one per frame (inval if change gizmo)
	short		compare_mode;	// feature matching space: rgb color, luminence, edge-filtered, etc.
	float		match_distance;	// last tracking match 'distance'
	HBITMAP		id_bitmap;		// unbelievable - I need to use a bitmap copy to do XOR text drawing
	short		flags;

				MotionTracker(MAXBitMap* imbm, int iindex);
			   ~MotionTracker();

				classof_methods(MotionTracker, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	void		gc_trace();

	void		track();
	void		clear_track_cache();
	void		set_center(int x, int y);
	void		set_index(int i);
	void		copy_target();
	void		draw(HWND hWnd);
	void		draw_gizmo(HDC hdc);
	void		inval_gizmo();
	BOOL		start_drag(HWND hwnd, int wParam, long lParam);
	void		drag(HWND hwnd, int wParam, long lParam);
	void		end_drag(HWND hwnd);
	void		move(HWND hwnd, int dx, int dy);
	void		deselect(HWND hwnd);

	def_visible_generic ( resample,    "resample");
	def_visible_generic ( reset,	   "reset");

	def_property ( center );
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
};

#define MT_GIZMO_SELECTED		0x0001
#define MT_GIZMO_MOVED			0x0002
#define MT_ENABLED				0x0004

#define MT_MATCH_RGB			0
#define MT_MATCH_GRAY			1
#define MT_MATCH_EDGE			2
#define MT_MATCH_RANK			3

#define MT_NO_HANDLE			0		// handle codes...
#define MT_CENTER				1		
#define MT_TOPLEFT_BOUNDS		2
#define MT_BOTLEFT_BOUNDS		3
#define MT_TOPRIGHT_BOUNDS		4
#define MT_BOTRIGHT_BOUNDS		5
#define MT_TOPLEFT_MBOUNDS		6
#define MT_BOTLEFT_MBOUNDS		7
#define MT_TOPRIGHT_MBOUNDS		8
#define MT_BOTRIGHT_MBOUNDS		9


