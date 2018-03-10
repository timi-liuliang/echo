#pragma once

// The Biped Tracks
#define KEY_NOTHING    -1
#define KEY_LARM		0
#define KEY_RARM		1
#define KEY_LHAND		2
#define KEY_RHAND		3
#define KEY_LLEG		4
#define KEY_RLEG		5
#define KEY_LFOOT		6
#define KEY_RFOOT		7
#define KEY_SPINE		8
#define KEY_TAIL		9 
#define KEY_HEAD		10					
#define KEY_PELVIS		11		
#define KEY_VERTICAL	12
#define KEY_HORIZONTAL	13
#define KEY_TURN		14
#define KEY_FOOTPRINTS	15
#define KEY_NECK	    16
#define KEY_PONY1	    17
#define KEY_PONY2	    18
#define KEY_PROP1	    19
#define KEY_PROP2	    20
#define KEY_PROP3	    21
// these two are no longer active, but are still defined for old files
#define KEY_LFARM		22
#define KEY_RFARM		23
#define NKEYTRACKS      22

// nubs
#define NUB_START    NKEYTRACKS
#define NUB_RHAND   (NKEYTRACKS+0)
#define NUB_LHAND   (NKEYTRACKS+1) 
#define NUB_LFOOT   (NKEYTRACKS+2) 
#define NUB_RFOOT   (NKEYTRACKS+3)
#define NUB_TAIL    (NKEYTRACKS+4) 
#define NUB_HEAD    (NKEYTRACKS+5) 
#define NUB_PONY1   (NKEYTRACKS+6) 
#define NUB_PONY2   (NKEYTRACKS+7) 
#define NNUBTRACKS      8

// twist tracks, no keys
#define TWIST_START		100
#define TWS_LFARM		100
#define TWS_RFARM		101
#define TWS_LUPARM		102
#define TWS_RUPARM		103
#define TWS_LTHIGH		104
#define TWS_RTHIGH		105
#define TWS_LCALF		106
#define TWS_RCALF		107
#define TWS_LHORSE		108
#define TWS_RHORSE		109
#define NTWISTTRACKS	10

// maximum 200 Xtra tracks
// Xtra ids: 200-399, Xtra nub ids: 400-599
#define XTRA_START		200
#define XTRA_NUB_START	400
#define MAXXTRAS		200
#define XTRA_NUBOFFSET	200 // add this to an Xtra track id to get its nub id

// RK: 10/12/99 defines for adapt locks
#define ADAPT_LOCKHOR	0	
#define ADAPT_LOCKVER	1
#define ADAPT_LOCKTURN	2
#define ADAPT_LOCKLLEG	3
#define ADAPT_LOCKRLEG	4
#define ADAPT_LOCKFFRM	5
#define ADAPT_LOCKTIME	6

// Track Selection used in IBipMaster::GetTrackSelection.. SetTrackSelection.
//bodytracks
#define NO_TRACK        0
#define HOR_TRACK       1
#define VER_TRACK       2
#define ROT_TRACK       3
#define HORVER_TRACK    4
#define HORROT_TRACK	5
#define VERROT_TRACK	6
#define HORVERROT_TRACK	7
#define DEFAULT_TRACK	8 // the remembered track if the com is locked, otherwise HOR_TRACK

#define MAXFINGERS      5
#define MAXFINGERLINKS  4
#define MAXLEGJNTS		4

