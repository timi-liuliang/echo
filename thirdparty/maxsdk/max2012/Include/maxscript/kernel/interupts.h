#pragma once

#include "..\ScripterExport.h"
#include "exceptions.h"

extern ScripterExport BOOL		check_maxscript_interrupt;
extern ScripterExport int		MAXScript_signals;
extern ScripterExport void		escape_checker();

#define check_interrupts()	if (check_maxscript_interrupt) escape_checker(); if (MAXScript_signals) throw SignalException()
