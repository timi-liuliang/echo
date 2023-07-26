#include "CMDLine.h"
#include "EditorMode.h"
#include "EditorOpenMode.h"
#include "GameMode.h"
#include "VsGenMode.h"
#include "RegEditMode.h"

namespace Echo
{
	bool CMDLine::Parser(int argc, char* argv[])
	{
		if ( argc > 1 )
		{	
			StringArray sargv;
			for( int i=1; i<argc; i++)
				sargv.emplace_back( argv[i]);

			if (sargv[0] == "play")
			{
				GameMode gameMode;
				gameMode.exec(argc, argv);
			}
			else if (sargv[0] == "open")
			{
				EditOpenMode openMode;
				openMode.exec(argc, argv);
			}
			else if (sargv[0] == "vs")
			{
				VsGenMode vsGenMode;
				vsGenMode.exec(argc, argv);
			}
			else if (sargv[0] == "regedit")
			{
				RegEditMode regEditMode;
				regEditMode.exec(argc, argv);
			}

			return true;
		}
		else
		{
			EditorMode editorMode;
			editorMode.exec(argc, argv);
		}

		return false;
	}
}
