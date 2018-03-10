#pragma once
#include "maxheap.h"
#include <WTypes.h>

// Moved from VEDGE.CPP -TH
#define UNDEF 0xFFFFFFFF
#define UNDEF_FACE 0x3FFFFFFF

class VEdge: public MaxHeapOperators {
	DWORD f[2];
	public:
		void SetFace(int i, DWORD n) { f[i] =(f[i]&0xc0000000)|n;}
		void SetWhichSide(int i, int s) { f[i] =(f[i]&0x3FFFFFFF)|((s&3)<<30); }
		DWORD GetFace(int i){ return f[i]&0x3fffffff; }
		int GetWhichSide(int i) { return (f[i]>>30)&3; }
	};

struct Edge: public MaxHeapOperators {
	Edge(unsigned short fl = 0, DWORD v0 = UNDEF, DWORD v1 = UNDEF, DWORD f0 = UNDEF, DWORD f1 = UNDEF)
	{ flags = fl; v[0] = v0; v[1] = v1; f[0] = f0; f[1]= f1;}
	unsigned short flags;
	DWORD v[2];  /* indices of two vertices defining edge */
	DWORD f[2];  /* indices of two neighboring faces  */
};




