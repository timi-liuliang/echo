#include "buildver.h"        // For product-specific stuff like MAXVER_PRODUCTNAME
#include "maxversion.h"     // For build number, etc.
 
// 
// Should be overridden per-project/product 
// either use these defaults or define in verroverrides.h and include it in the resource file before
// including this maxversion.r file
// and also make sure the version information has been removed from your resource file
// also see maxversion.h
 
#ifndef MAXVER_PRODUCTNAME
#define MAXVER_PRODUCTNAME "3ds Max\0"
#endif
 
#ifndef MAXVER_INTERNALNAME
#define MAXVER_INTERNALNAME "\0"
#endif
 
#ifndef MAXVER_ORIGINALFILENAME
#define MAXVER_ORIGINALFILENAME "\0"
#endif

#ifndef MAXVER_FILEDESCRIPTION
#define MAXVER_FILEDESCRIPTION "\0"
#endif
 
#ifndef MAXVER_COMMENTS
#define MAXVER_COMMENTS "\0"
#endif
 
#ifndef MAXVER_COPYRIGHT
#define MAXVER_COPYRIGHT		"© 2011 Autodesk, Inc. All rights reserved.\0"
#endif

#ifndef MAXVER_COMPANYNAME
#define MAXVER_COMPANYNAME "Autodesk, Inc.\0"
#endif

//
// Everything else is fixed...
//
 
#define _MAXVER_TOSTRING(x)					#x
#define _MAXVER_RCVERSION(a, b, c, d)		a,b,c,d
#define _MAXVER_RCVERSION_STR(a, b, c, d)	_MAXVER_TOSTRING(a##.b##.c##.##d)
#define MAXVER_FILEVERSION					_MAXVER_RCVERSION(MAX_VERSION_MAJOR, MAX_VERSION_MINOR, MAX_VERSION_POINT, VERSION_INT)
#define MAXVER_FILEVERSION_STR			_MAXVER_RCVERSION_STR(MAX_VERSION_MAJOR, MAX_VERSION_MINOR, MAX_VERSION_POINT, VERSION_INT)
#define MAXVER_PRODUCTVERSION				_MAXVER_RCVERSION(MAX_PRODUCT_VERSION_MAJOR, MAX_PRODUCT_VERSION_MINOR, MAX_PRODUCT_VERSION_POINT, VERSION_INT)
#define MAXVER_PRODUCTVERSION_STR		_MAXVER_RCVERSION_STR(MAX_PRODUCT_VERSION_MAJOR, MAX_PRODUCT_VERSION_MINOR, MAX_PRODUCT_VERSION_POINT, VERSION_INT)
#define MAXVER_PRIVATE_BUILD			VERSION_STRING

//
// Version resource
//
// why not allow this to remain in each .rc file - that way this method can be overridden with per-dll changes as necessary?
//q:would be nice if the below stuff could be kept in the .rc file ...
//A:when msdev writes out an .rc file it does so destructively... 
 
VS_VERSION_INFO VERSIONINFO
 FILEVERSION MAXVER_FILEVERSION
 PRODUCTVERSION MAXVER_PRODUCTVERSION 
 FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x40004L
 FILETYPE 0x1L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", MAXVER_COMPANYNAME
            VALUE "FileVersion", MAXVER_FILEVERSION_STR
            VALUE "InternalName", MAXVER_INTERNALNAME
            VALUE "LegalCopyright", MAXVER_COPYRIGHT
            VALUE "OriginalFilename", MAXVER_ORIGINALFILENAME
            VALUE "Private Build Data", MAXVER_PRIVATE_BUILD
            VALUE "ProductName", MAXVER_PRODUCTNAME
            VALUE "ProductVersion", MAXVER_PRODUCTVERSION_STR
            VALUE "FileDescription", MAXVER_FILEDESCRIPTION
            VALUE "Comments", MAXVER_COMMENTS
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
