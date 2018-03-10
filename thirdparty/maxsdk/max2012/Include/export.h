#ifdef DllExport
#undef DllExport
#endif

#ifdef IMPORTING
#define DllExport	__declspec( dllimport )
#else
#define DllExport	__declspec( dllexport )
#endif

