//
//  ofxVstHostPluginLoader.cpp
//
//
//  Created by Mathieu Hingant on 28/04/17.
//
//

#include "ofxVstHostPluginLoader.h"

//--------------------------------------------------------------
ofxVstHostPluginLoader::ofxVstHostPluginLoader()
{
    _module = 0;
    _mainProc = 0;
    _path = "";
}

//--------------------------------------------------------------
ofxVstHostPluginLoader::~ofxVstHostPluginLoader ()
{
    //closeLibrary();
    //ofLogWarning("ofxVstHostPluginLoader") << "CLOSING";
}

//--------------------------------------------------------------
bool ofxVstHostPluginLoader::loadLibrary (const char* fileName)
{
    // Save path
    _path = fileName;
    
#if _WIN32
    _module = LoadLibrary (fileName);
    
#elif TARGET_API_MAC_CARBON
    CFStringRef fileNameString = CFStringCreateWithCString (NULL, fileName, kCFStringEncodingUTF8);
    if (fileNameString == 0)
        return false;
    CFURLRef url = CFURLCreateWithFileSystemPath (NULL, fileNameString, kCFURLPOSIXPathStyle, false);
    CFRelease (fileNameString);
    if (url == 0)
        return false;
    _module = CFBundleCreate (NULL, url);
    CFRelease (url);
    if (_module && CFBundleLoadExecutable ((CFBundleRef)_module) == false)
        return false;
    
#elif __linux__
    _module = dlopen (fileName, RTLD_LAZY);
    if (!_module) {
        fprintf (stderr, "%s\n", dlerror());
        return false;
    }
#endif
    return _module != 0;
}

//--------------------------------------------------------------
void ofxVstHostPluginLoader::closeLibrary(void) {
    if (_module)
    {
#if _WIN32
        FreeLibrary ((HMODULE)_module);
        
#elif TARGET_API_MAC_CARBON
        CFBundleUnloadExecutable ((CFBundleRef)_module);
        CFRelease ((CFBundleRef)_module);
        
#elif __linux__
        dlclose(_module);
#endif
        
        _module = nullptr;
        _path = "";
    }
}


//--------------------------------------------------------------
PluginEntryProc ofxVstHostPluginLoader::getMainEntry ()
{
    //PluginEntryProc mainProc = 0;
    if(_mainProc == 0) {
#if _WIN32
        _mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)_module, "VSTPluginMain");
        if (!_mainProc) {
            _mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)_module, "main");
        }
        
#elif TARGET_API_MAC_CARBON
        _mainProc = (PluginEntryProc)CFBundleGetFunctionPointerForName ((CFBundleRef)_module, CFSTR("VSTPluginMain"));
        if (!_mainProc) {
            _mainProc = (PluginEntryProc)CFBundleGetFunctionPointerForName ((CFBundleRef)_module, CFSTR("main_macho"));
        }
        
#elif __linux__
        _mainProc = (PluginEntryProc)dlsym(_module, "VSTPluginMain");
        if (!_mainProc) {
            _mainProc = (PluginEntryProc)dlsym(_module, "main");
        }
        
#endif
    }
    
    return _mainProc;
}


//--------------------------------------------------------------
string ofxVstHostPluginLoader::getPath() const
{
    return _path;
}
