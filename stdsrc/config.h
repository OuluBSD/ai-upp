// Configuration header for stdsrc libraries
// Defines platform-specific macros and settings

#ifndef STDSRC_CONFIG_H
#define STDSRC_CONFIG_H

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WIN32 1
    #define PLATFORM_WIN 1
    #ifndef flagGUI
        #define flagGUI 1
    #endif
    #ifndef flagWIN
        #define flagWIN 1
    #endif
#elif defined(__APPLE__)
    #define PLATFORM_OSX 1
    #define PLATFORM_POSIX 1
    #ifndef flagGUI
        #define flagGUI 1
    #endif
    #ifndef flagOSX
        #define flagOSX 1
    #endif
#else
    #define PLATFORM_POSIX 1
    #ifndef flagGUI
        #define flagGUI 1
    #endif
    #ifndef flagPOSIX
        #define flagPOSIX 1
    #endif
#endif

// Feature flags
#define GUIPLATFORM_GTK 1
#define GUIPLATFORM_WIN32 1
#define GUIPLATFORM_OSX 1

// Directory separator
#ifdef _WIN32
    #define DIR_SEP '\\'
    #define DIR_SEPS "\\"
#else
    #define DIR_SEP '/'
    #define DIR_SEPS "/"
#endif

// Common macros
#define UPPSDRAW 1
#define GUI_SIGNED_PIXEL 1

// Define null pointer
#ifndef NULL
    #define NULL 0
#endif

#endif // STDSRC_CONFIG_H