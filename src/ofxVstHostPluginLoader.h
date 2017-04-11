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


struct PluginLoader
{
    // Pointer to our library
    void* module;
    
    // Path to this library not to load it twice
    string path;
    
    
    //--------------------------------------------------------------
    PluginLoader ()
    : module (0)
    {}
    
    //--------------------------------------------------------------
    ~PluginLoader ()
    {
        closeLibrary();
    }
    
    //--------------------------------------------------------------
    bool loadLibrary (const char* fileName)
    {
        // Save path
        path = fileName;
        
#if _WIN32
        module = LoadLibrary (fileName);
        
#elif TARGET_API_MAC_CARBON
        CFStringRef fileNameString = CFStringCreateWithCString (NULL, fileName, kCFStringEncodingUTF8);
        if (fileNameString == 0)
            return false;
        CFURLRef url = CFURLCreateWithFileSystemPath (NULL, fileNameString, kCFURLPOSIXPathStyle, false);
        CFRelease (fileNameString);
        if (url == 0)
            return false;
        module = CFBundleCreate (NULL, url);
        CFRelease (url);
        if (module && CFBundleLoadExecutable ((CFBundleRef)module) == false)
            return false;
        
#elif __linux__
        module = dlopen (fileName, RTLD_LAZY);
        if (!module) {
            fprintf (stderr, "%s\n", dlerror());
            return false;
        }
#endif
        return module != 0;
    }
    
    //--------------------------------------------------------------
    void closeLibrary(void) {
        if (module)
        {
#if _WIN32
            FreeLibrary ((HMODULE)module);
            
#elif TARGET_API_MAC_CARBON
            CFBundleUnloadExecutable ((CFBundleRef)module);
            CFRelease ((CFBundleRef)module);
            
#elif __linux__
            dlclose(module);
#endif
            
            module = nullptr;
            path = "";
        }
    }
    
    
    //--------------------------------------------------------------
    PluginEntryProc getMainEntry ()
    {
        PluginEntryProc mainProc = 0;
#if _WIN32
        mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)module, "VSTPluginMain");
        if (!mainProc)
            mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)module, "main");
        
#elif TARGET_API_MAC_CARBON
        mainProc = (PluginEntryProc)CFBundleGetFunctionPointerForName ((CFBundleRef)module, CFSTR("VSTPluginMain"));
        if (!mainProc)
            mainProc = (PluginEntryProc)CFBundleGetFunctionPointerForName ((CFBundleRef)module, CFSTR("main_macho"));
        
#elif __linux__
        mainProc = (PluginEntryProc)dlsym(module, "VSTPluginMain");
        if (!mainProc)
            mainProc = (PluginEntryProc)dlsym(module, "main");
#endif
        return mainProc;
    }
};
