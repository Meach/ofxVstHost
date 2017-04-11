//
//  ofxVstHost.cpp
//  
//
//  Created by Mathieu Hingant on 10/04/17.
//
//

#include "ofxVstHost.h"

//--------------------------------------------------------------
ofxVstHost::ofxVstHost()
{
    _plugins.clear();
    _effects.clear();
    _names.clear();
    
    _blockSize = 512;
    _sampleRate = 44100.f;
    _numProcessCycles = 5;
}


//--------------------------------------------------------------
ofxVstHost::~ofxVstHost()
{
    // Close all effects
    for(int i = 0; i < _effects.size(); i++) {
        closeEffect(i);
    }
    
    // Close all plugins
    for(int i = 0; i < _plugins.size(); i++) {
        closePlugin(i);
    }
    
    // Free our audio variables
    delete [] _audio;
    
    _rBuffer.clear();
    _lBuffer.clear();
}


//--------------------------------------------------------------
void ofxVstHost::setup(float sampleRate, int blockSize)
{
    // Initialise our variables
    _sampleRate = sampleRate;
    _blockSize = blockSize;
    _numProcessCycles = 5;
    
    // Allocate our audio arrays
    _audio = new (float *[2]);
    
    // Allocate our sound buffers
    _rBuffer.allocate(_blockSize, 1);
    _lBuffer.allocate(_blockSize, 1);
    
    // Reset the sound buffers
    _rBuffer.set(0.0f);
    _lBuffer.set(0.0f);
}


//--------------------------------------------------------------
bool ofxVstHost::load(string path, string nameInfo)
{
    ofLogVerbose("ofxVstHost") << "load(" << path << ")";
    
    if (!checkPlatform ())
    {
        ofLogVerbose("ofxVstHost") << "Platform verification failed! Please check your Compiler Settings!";
        return false;
    }
    
    // Check that this module has not been already loaded
    int indexExisting = -1;
    for(int i = 0; i < _plugins.size(); i++) {
        // If we find the same name in our vector means we already loaded this library
        if(_plugins[i].path == path) {
            indexExisting = i;
            ofLogVerbose("ofxVstHost") << "Library " << path << " already exist at index " << i << ". Not loading it again.";
        }
    }
    
    
    // Load new library if it is not in our list already
    if(indexExisting == -1){
        PluginLoader loader;
        if (!loader.loadLibrary (path.c_str()))
        {
            ofLogError("ofxVstHost") << "Failed to load VST Plugin library!";
            return false;
        }
        
        // Add our plugin loader to our vector
        _plugins.push_back(loader);
        
        // Overwrite index as it will be used below to access proper plugin
        indexExisting = _plugins.size() - 1;
    }
    
    
    // Get main entry from our plugin vector
    PluginEntryProc mainEntry = _plugins[indexExisting].getMainEntry ();
    if (!mainEntry)
    {
        ofLogError("ofxVstHost") << "VST Plugin main entry not found!";
        return false;
    }

    
    // Create the effect from this main entry
    AEffect* effect = mainEntry (HostCallback);
    
    if (!effect)
    {
        ofLogError("ofxVstHost") << "Failed to create effect instance!";
        return false;
    }
    
    // Add this effect to our vector
    _effects.push_back(effect);
    _names.push_back(nameInfo);
    
    // Get index of the effect in our vector
    int indexEffect = _effects.size() - 1;
    
    // Initialize the effect
    dispatcher(_effects[indexEffect], effOpen);
    dispatcher(_effects[indexEffect], effSetSampleRate, 0, 0, 0, getSampleRate());
    dispatcher(_effects[indexEffect], effSetBlockSize, 0, getBlockSize());
    dispatcher(_effects[indexEffect], effSetProcessPrecision, 0, kVstProcessPrecision32);
    dispatcher(_effects[indexEffect], effMainsChanged, 0, 1);
    dispatcher(_effects[indexEffect], effStartProcess);
}


//--------------------------------------------------------------
void ofxVstHost::closeEffect(int index)
{
    // Make sure index does not go out of bound
    if(index < _effects.size()) {
        // Close effect
        dispatcher(_effects[index], effStopProcess);
        dispatcher(_effects[index], effMainsChanged, 0, 0);
        dispatcher(_effects[index], effClose);
        
        // Remove this effect from the vector
        _effects.erase(_effects.begin() + index);
        _names.erase(_names.begin() + index);
    }
    else {
        ofLogError("ofxVstHost") << "closeEffect(...), index out of bound. Expected " << _effects.size() - 1 << " max, got " << index;
    }
}

//TODO: Make sure the plugin is not used in other currently open effects
//--------------------------------------------------------------
void ofxVstHost::closePlugin(int index)
{
    // Make sure index does not go out of bound
    if(index < _plugins.size()) {
        // Close library
        _plugins[index].closeLibrary();
        
        // Remove this effect from the vector
        _plugins.erase(_plugins.begin() + index);
    }
    else {
        ofLogError("ofxVstHost") << "closePlugin(...), index out of bound. Expected " << _plugins.size() - 1 << " max, got " << index;
    }
}


//--------------------------------------------------------------
void ofxVstHost::process(int indexEffect, ofSoundBuffer &buffer)
{
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        // Get the soundbuffer in 2 channels
        buffer.getChannel(_rBuffer, 0);
        buffer.getChannel(_lBuffer, 1);
        
        // Tie the pointer array to our sound buffer
        _audio[0] = &_rBuffer[0];
        _audio[1] = &_lBuffer[0];
        
        // Process the effect
        _effects[indexEffect]->processReplacing(_effects[indexEffect], _audio, _audio, buffer.getNumFrames());
        
        // Copy back the different channels to our output buffer
        buffer.setChannel(_rBuffer, 0);
        buffer.setChannel(_lBuffer, 1);
    }
    else {
        ofLogError("ofxVstHost") << "process(...), indexEffect out of bound. Expected " << _effects.size() - 1 << "max, got " << indexEffect;
    }
}

// Return the number of plugins (libraries) loaded currently
//--------------------------------------------------------------
int ofxVstHost::getNumPlugins(void) const
{
    return _plugins.size();
}

// Return the number of effects loaded currently
//--------------------------------------------------------------
int ofxVstHost::getNumEffects(void) const
{
    return _effects.size();
}

//--------------------------------------------------------------
float ofxVstHost::getSampleRate() const
{
    return _sampleRate;
}

//--------------------------------------------------------------
VstInt32 ofxVstHost::getBlockSize() const
{
    return _blockSize;
}


//--------------------------------------------------------------
int ofxVstHost::getNumInputs(int indexEffect) const {
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        return _effects[indexEffect]->numInputs;
    }
    else {
        ofLogError("ofxVstHost") << "getNumInputs(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
        return -1;
    }
}
//--------------------------------------------------------------
int ofxVstHost::getNumOutputs(int indexEffect) const {
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        return _effects[indexEffect]->numOutputs;
    }
    else {
        ofLogError("ofxVstHost") << "getNumOutputs(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
        return -1;
    }
}

//--------------------------------------------------------------
int ofxVstHost::getNumParameters(int indexEffect) const {
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        return _effects[indexEffect]->numParams;
    }
    else {
        ofLogError("ofxVstHost") << "getNumParameters(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
        return -1;
    }
}

//--------------------------------------------------------------
int ofxVstHost::getNumPrograms(int indexEffect) const {
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        return _effects[indexEffect]->numPrograms;
    }
    else {
        ofLogError("ofxVstHost") << "getNumPrograms(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
        return -1;
    }
}

//--------------------------------------------------------------
string ofxVstHost::getVendorString(int indexEffect) const
{
    char vendorString[256] = {0};
    
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        dispatcher (_effects[indexEffect], effGetVendorString, 0, 0, vendorString);
    }
    else {
        ofLogError("ofxVstHost") << "getVendorString(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }
    
    return ofToString(vendorString);
}

//--------------------------------------------------------------
string ofxVstHost::getProductString(int indexEffect) const
{
    char productString[256] = {0};
    
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        dispatcher (_effects[indexEffect], effGetProductString, 0, 0, productString);
    }
    else {
        ofLogError("ofxVstHost") << "getProductString(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }
    
    return ofToString(productString);
}

//--------------------------------------------------------------
string ofxVstHost::getEffectName(int indexEffect) const
{
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _names.size()) {
        return _names[indexEffect];
    }
    else {
        ofLogError("ofxVstHost") << "getEffectName(...), indexEffect out of bound. Expected " << _names.size() - 1 << " max, got " << indexEffect;
    }
}

//--------------------------------------------------------------
string ofxVstHost::listParameterValues(int indexEffect)
{
    string str = "";
    
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        // Iterate parameters...
        for (VstInt32 paramIndex = 0; paramIndex < _effects[indexEffect]->numParams; paramIndex++)
        {
            char paramName[256] = {0};
            char paramLabel[256] = {0};
            char paramDisplay[256] = {0};
            
            dispatcher (_effects[indexEffect], effGetParamName, paramIndex, 0, paramName, 0);
            dispatcher (_effects[indexEffect], effGetParamLabel, paramIndex, 0, paramLabel, 0);
            dispatcher (_effects[indexEffect], effGetParamDisplay, paramIndex, 0, paramDisplay, 0);
            float value = _effects[indexEffect]->getParameter (_effects[indexEffect], paramIndex);
            
            //ofLogVerbose("ofxVSTHost") << "Param " << paramIndex << ": " << paramName << " [" << paramDisplay << " " << paramLabel << "] (normalized = " << value << ")";
            str += "Param " + ofToString(paramIndex) + ": " + ofToString(paramName) + " [" + ofToString(paramDisplay) + " " + ofToString(paramLabel) + "]  (normalized = " + ofToString(value) + ")\n";
        }
    }
    else {
        ofLogError("ofxVstHost") << "listParameterValues(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }
    
    return str;
}


//--------------------------------------------------------------
float ofxVstHost::getParameterValue(int indexEffect, int indexParam) const {
    
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        return _effects[indexEffect]->getParameter (_effects[indexEffect], indexParam);
    }
    else {
        ofLogError("ofxVstHost") << "getParameterValue(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }

}

//--------------------------------------------------------------
void ofxVstHost::setParameterValue(int indexEffect, int indexParam, float value)
{
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        _effects[indexEffect]->setParameter (_effects[indexEffect], indexParam, value);
    }
    else {
        ofLogError("ofxVstHost") << "setParameterValue(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }
}

//--------------------------------------------------------------
string ofxVstHost::getParameterName(int indexEffect, int indexParam) const
{
    char paramName[256] = {0};
    
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        dispatcher (_effects[indexEffect], effGetParamName, indexParam, 0, paramName);
    }
    else {
        ofLogError("ofxVstHost") << "getParameterName(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }
    
    return ofToString(paramName);
}

//--------------------------------------------------------------
string ofxVstHost::getParameterLabel(int indexEffect, int indexParam) const
{
    char paramLabel[256] = {0};
    
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        dispatcher (_effects[indexEffect], effGetParamLabel, indexParam, 0, paramLabel);
    }
    else {
        ofLogError("ofxVstHost") << "getParameterLabel(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }
    
    return ofToString(paramLabel);
}


// Make sure the platform architecture match the VST library one
//--------------------------------------------------------------
bool ofxVstHost::checkPlatform ()
{
#if VST_64BIT_PLATFORM
    ofLogVerbose("ofxVstHost") << "This is a 64 Bit Build!";
#else
    ofLogVerbose("ofxVstHost") << "This is a 32 Bit Build!";
#endif
    
    int sizeOfVstIntPtr = sizeof (VstIntPtr);
    int sizeOfVstInt32 = sizeof (VstInt32);
    int sizeOfPointer = sizeof (void*);
    int sizeOfAEffect = sizeof (AEffect);
    
    ofLogVerbose("ofxVstHost") << "VstIntPtr = " << sizeOfVstIntPtr << " Bytes, VstInt32 = " << sizeOfVstInt32 << " Bytes, Pointer = " << sizeOfPointer << " Bytes, AEffect = " << sizeOfAEffect << " Bytes";
    
    return sizeOfVstIntPtr == sizeOfPointer;
}


// Helper function to make calls to dispacher a bit cleaner
//--------------------------------------------------------------
intptr_t ofxVstHost::dispatcher(AEffect* effect, int32_t opcode, int32_t index, intptr_t value, void *ptr, float opt) const
{
    return effect->dispatcher(effect, opcode, index, value, ptr, opt);
}


//--------------------------------------------------------------
VstIntPtr VSTCALLBACK ofxVstHost::HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
    VstIntPtr result = 0;
    
    // Filter idle calls...
    bool filtered = false;
    if (opcode == audioMasterIdle)
    {
        static bool wasIdle = false;
        if (wasIdle)
            filtered = true;
        else
        {
            ofLogVerbose("ofxVstHost") << "(Future idle calls will not be displayed!)";
            wasIdle = true;
        }
    }
    
    if (!filtered) {
        ofLogVerbose("ofxVstHost") << "PLUG> HostCallback (opcode " << opcode << ")";
        ofLogVerbose("ofxVstHost") << "index = " << index << ", value = " << FromVstPtr<void> (value) << ", ptr = " << ptr << ", opt = " << opt;
    }
    
    switch (opcode)
    {
        case audioMasterVersion :
            result = kVstVersion;
            break;
            
        case audioMasterCurrentId:
            result = effect->uniqueID;
            break;
            
        case audioMasterGetSampleRate:
            //result = getSampleRate();
            break;
            
        case audioMasterGetBlockSize:
            //result = getBlockSize();
            break;
            
        case audioMasterGetCurrentProcessLevel:
            result = kVstProcessLevelUnknown;
            break;
            
        case audioMasterGetAutomationState:
            result = kVstAutomationOff;
            break;
            
        case audioMasterGetLanguage:
            result = kVstLangEnglish;
            break;
            
        case audioMasterGetVendorVersion:
            //result = getVendorVersion();
            break;
            
        case audioMasterGetVendorString:
            //strcpy_s(static_cast<char*>(ptr), kVstMaxVendorStrLen, getVendorString());
            result = 1;
            break;
            
        case audioMasterGetProductString:
            //strcpy_s(static_cast<char*>(ptr), kVstMaxProductStrLen, getProductString());
            result = 1;
            break;
            
        /*case audioMasterGetTime:
            timeinfo.flags      = 0;
            timeinfo.samplePos  = getSamplePos();
            timeinfo.sampleRate = getSampleRate();
            result = reinterpret_cast<VstIntPtr>(&timeinfo);
            break;
            
        case audioMasterGetDirectory:
            result = reinterpret_cast<VstIntPtr>(directoryMultiByte.c_str());
            break;
            
        case audioMasterIdle:
            if(editorHwnd) {
                dispatcher(effEditIdle);
            }
            break;
            
        case audioMasterSizeWindow:
            if(editorHwnd) {
                RECT rc {};
                GetWindowRect(editorHwnd, &rc);
                rc.right = rc.left + static_cast<int>(index);
                rc.bottom = rc.top + static_cast<int>(value);
                resizeEditor(rc);
            }
            break;
            
        case audioMasterCanDo:
            for(const char** pp = getCapabilities(); *pp; ++pp) {
                if(strcmp(*pp, static_cast<const char*>(ptr)) == 0) {
                    result = 1;
                }
            }
            break;*/
    }
    
    return result;
}
