//
//  ofxVstHostPluginLoader.h
//  
//
//  Created by Mathieu Hingant on 11/04/17.
//
//

#pragma once

#include "ofMain.h"
#include "aeffectx.h"

#if _WIN32
#include <windows.h>
#elif TARGET_API_MAC_CARBON
#include <CoreFoundation/CoreFoundation.h>
#elif __linux__
#include <dlfcn.h>
#endif

typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);


class ofxVstHostPluginLoader
{
public:
    ofxVstHostPluginLoader();
    ~ofxVstHostPluginLoader();    

    bool loadLibrary(const char* fileName);
    void closeLibrary(void);
    PluginEntryProc getMainEntry(void);
    
    string getPath() const;
    
private:
    // Pointer to our library
    void* _module;
    
    PluginEntryProc _mainProc;
    
    // Path to this library not to load it twice
    string _path;
};
