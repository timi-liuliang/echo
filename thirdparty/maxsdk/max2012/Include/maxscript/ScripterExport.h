
#pragma once

#ifdef BLD_MAXSCRIPT
#	define ScripterExport __declspec( dllexport )
#else
#	define ScripterExport __declspec( dllimport )
#endif