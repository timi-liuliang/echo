#pragma once

#include "buildnumber.h" // defines VERSION_INT

// VERSION_INT is defined in buildnumber.h and is written to by the builder, inserting the current build number.
// VERSION_INT is used for the fourth least siginifact version component eg. 4.1.1.[36]

// The product and file version could be different.
// For example, VIZ 4.0 works with max files version 4.2

// MAX File version:
#ifndef MAX_VERSION_MAJOR
#define MAX_VERSION_MAJOR 14
#endif

#ifndef MAX_VERSION_MINOR
#define MAX_VERSION_MINOR 0
#endif

#ifndef MAX_VERSION_POINT
#define MAX_VERSION_POINT 0
#endif

// MAX Product version
#ifndef MAX_PRODUCT_VERSION_MAJOR
#define MAX_PRODUCT_VERSION_MAJOR 14
#endif

#ifndef MAX_PRODUCT_VERSION_MINOR
#define MAX_PRODUCT_VERSION_MINOR 0
#endif

#ifndef MAX_PRODUCT_VERSION_POINT
#define MAX_PRODUCT_VERSION_POINT 0
#endif

#ifndef MAX_PRODUCT_YEAR_NUMBER
#define MAX_PRODUCT_YEAR_NUMBER 2012
#endif

// MAX_RELEASE_EXTERNAL is an alternative for MAX_RELEASE (plugapi.h)
// Define it when you want the "branded" (UI) version number to be different 
// from the internal one.
//#define MAX_RELEASE_EXTERNAL	10900

// This should be blank for a non-service-pack release
#ifndef MAX_PRODUCT_VERSION_SP
#define MAX_PRODUCT_VERSION_SP "\0"
#endif


#define _MAX_VERSION(a, b, c,d) a##b##c##d
#define MAX_VERSION _MAX_VERSION(MAX_VERSION_MAJOR, MAX_VERSION_MINOR, MAX_VERSION_POINT,VERSION_INT)


