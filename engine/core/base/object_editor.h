#pragma once

#include "engine/core/base/EchoDef.h"

#ifdef ECHO_EDITOR_MODE
// define node edit interface for echo editor
#define ECHO_EDITOR_INTERFACE								\
public:														\
	/* editor update self*/									\
	virtual void editor_update_self() {}					\
															\
	/* get node icon*/										\
	virtual const char* getEditorIcon() const { return ""; }\
															\
	/* on editor select this node*/							\
	virtual void onEditorSelectThisNode() {}				\
															\
protected:													\
	any				m_editorData;							\

#define ECHO_EDITOR											\
public:														\
	/* editor update self*/									\
	virtual void editor_update_self();						\
															\
	/* get node icon*/										\
	virtual const char* getEditorIcon() const;				\
															\
	/* on editor select this node*/							\
	virtual void onEditorSelectThisNode();					\

#else
#define ECHO_EDITOR_INTERFACE
#define ECHO_EDITOR
#endif