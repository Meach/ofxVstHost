ofxVstHost
=====================================

Introduction
------------
Enable to load VST plugins and use them as audio effects in openFrameworks.

License
-------
See `license.md`.

Installation
------------
1. Go to `https://www.steinberg.net/en/company/developers.html` and download the VST SDK.
1. Extract the VST SDK on your computer and copy the `pluginterfaces` folder inside the addon `lib\VST2_SDK\` folder. 

Dependencies
------------
None.

Compatibility
------------
OF 0.9+ Mac/Win/Linux.


#### Compile on Linux
For the compile to succeed under Linux, a change has to be made to the aeffect.h
file in the VST SDK sources.
This will only affect the compiler behaviour under Linux and won't have any effect for
other targets.

// ORIGINAL @ line 37
```
#elif defined(__GNUC__)
    #pragma pack(push,8)
    #define VSTCALLBACK __cdecl
```

// REPLACE @ line 37
```
#elif defined(__GNUC__)
    #pragma pack(push,8)
    #if defined(__linux__)
        #define VSTCALLBACK
    #else
        #define VSTCALLBACK __cdecl
    #endif
```

#### Architecture
Make sure the VST architecture (32/64-bit) match your project architecture!


Known issues
------------
VST editor not implemented (i.e. VSTi cannot be displayed).
