/**********************************************************************
 *<
	FILE: IGameExport.h

	DESCRIPTION:	Defines preprocessor switch for IGame.dll
				    and client application

	CREATED BY:		Alex Zadorozhny

	HISTORY:		05|12|2005

	IGame Version: 1.122

 *>	Copyright (c) 2005, All Rights Reserved.
 **********************************************************************/
/*!\file IGameExport.h
\brief Defines preprocessor switch for IGame.dll
	   and client application
*/


#pragma once


//! Preprocessor switch for the DLL's interface
#ifdef IMPORTING
#define IGAMEEXPORT	__declspec( dllimport )
#else
#define IGAMEEXPORT	__declspec( dllexport )
#endif



