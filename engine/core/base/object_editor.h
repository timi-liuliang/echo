#pragma once

#include "engine/core/base/EchoDef.h"

#ifdef ECHO_EDITOR_MODE
// define node edit interface for echo editor
#define ECHO_EDIT_INTERFACE									\
public:														\
	/* get node icon*/										\
	virtual const char* getEditorIcon() const { return ""; }\

#define ECHO_EDITOR											\
public:														\
	/* get node icon*/										\
	virtual const char* getEditorIcon() const;				\

#else
#define ECHO_EDIT_INTERFACE
#define ECHO_EDITOR
#endif