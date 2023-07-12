#pragma once
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define BUILD_VERSION 0

#define MAJOR_VERSION_STRING "1"
#define MINOR_VERSION_STRING "0"
#define PATCH_VERSION_STRING "0"
#define BUILD_VERSION_STRING "0"

#define LIB_VERSION_STRING MAJOR_VERSION_STRING "." \
	MINOR_VERSION_STRING "." \
	PATCH_VERSION_STRING "." \
	BUILD_VERSION_STRING "\0"

#define COPYRIGHT_YEAR 2023
#define COPYRIGHT_STRING "Dark Assassins Inc. 2023"

#ifdef DEBUG
#define WIN_FILENAME "libfunclogd.dll"
#else
#define WIN_FILENAME "libfunclog.dll"
#endif
