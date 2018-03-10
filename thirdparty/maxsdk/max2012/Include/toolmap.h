/**********************************************************************
 *<
	FILE: toolmap.h

	DESCRIPTION:  defines a mapping between control identifiers and osnaps 

	CREATED BY: John Hutchinson

	HISTORY: January 15, 1996
 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "tab.h"


class msgdata: public MaxHeapOperators {
public:
	int recipient;
	int subindex;
	int msg;
	msgdata(){};
	msgdata(int r, int s, int m):recipient(r), subindex(s), msg(m){};
};



class tooltabentry: public MaxHeapOperators {

private:

	int indexer;
	msgdata data;

public:

	tooltabentry(){};
	tooltabentry(int i, int r, int s, int m);
	msgdata *Query(int i);

};


class ToolTab : public Tab<tooltabentry *> {
	public:         
		msgdata *Lookup(int toolid); 
	};




