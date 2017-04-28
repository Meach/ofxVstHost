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
    //_names.clear();
    
    _blockSize = 512;
    _sampleRate = 44100.f;
    _numProcessCycles = 5;
}


//--------------------------------------------------------------
ofxVstHost::~ofxVstHost()
{
    //ofLogNotice("ofxVstHost") << "CLOSING";
    //closeAll();
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


// Load the VST effect and return the index in our vector if it's a success. -1 if not
//--------------------------------------------------------------
int ofxVstHost::load(string path, string label)
{
    ofLogVerbose("ofxVstHost") << "load(" << path << ")";
    
    if (!checkPlatform ())
    {
        ofLogVerbose("ofxVstHost") << "Platform verification failed! Please check your Compiler Settings!";
        return -1;
    }
    
    // Check that this module has not been already loaded
    int indexExisting = -1;
    for(int i = 0; i < _plugins.size(); i++) {
        // If we find the same name in our vector means we already loaded this library
        if(_plugins[i].getPath() == path) {
            indexExisting = i;
            ofLogVerbose("ofxVstHost") << "Library " << path << " already exist at index " << i << ". Not loading it again.";
        }
    }
    
    
    // Load new library if it is not in our list already
    if(indexExisting == -1){
        // Make room in our vector for a new plugin
        _plugins.resize(_plugins.size() + 1);
        
        // Overwrite index as it will be used below to access proper plugin
        indexExisting = _plugins.size() - 1;
        
        // Load the plugin
        if (!_plugins[indexExisting].loadLibrary (path.c_str()))
        {
            ofLogError("ofxVstHost") << "Failed to load VST Plugin library!";
            return -1;
        }
    }
    
    
    // Get main entry from our plugin vector
    PluginEntryProc mainEntry = _plugins[indexExisting].getMainEntry ();
    if (!mainEntry)
    {
        ofLogError("ofxVstHost") << "VST Plugin main entry not found for " << label;
        return -1;
    }
    
    
    // Create the effect from this main entry
    ofxVstEffect effect;
    effect.aeffect = mainEntry (HostCallback);
    
    if (!effect.aeffect)
    {
        ofLogError("ofxVstHost") << "Failed to create effect instance!";
        return -1;
    }
    
    effect.label = label;
    
    // Add this effect to our vector
    _effects.push_back(effect);
    
    // Get index of the effect in our vector
    int indexEffect = _effects.size() - 1;
    
    // Initialize the effect
    dispatcher(_effects[indexEffect].aeffect, effOpen);
    dispatcher(_effects[indexEffect].aeffect, effSetSampleRate, 0, 0, 0, getSampleRate());
    dispatcher(_effects[indexEffect].aeffect, effSetBlockSize, 0, getBlockSize());
    dispatcher(_effects[indexEffect].aeffect, effSetProcessPrecision, 0, kVstProcessPrecision32);
    dispatcher(_effects[indexEffect].aeffect, effMainsChanged, 0, 1);
    dispatcher(_effects[indexEffect].aeffect, effStartProcess);
    
    
    ofLogVerbose("ofxVstHost") << "Created effect successfully with label " << effect.label << ", at index " << indexEffect;
    
    return indexEffect;
}


//--------------------------------------------------------------
void ofxVstHost::closeEffect(int index)
{
    //ofLogVerbose("ofxVstHost") << "closeEffect(" <<  index << ")";
    
    // Make sure index does not go out of bound
    if(index < _effects.size()) {
        // Close effect
        dispatcher(_effects[index].aeffect, effStopProcess);
        dispatcher(_effects[index].aeffect, effMainsChanged, 0, 0);
        dispatcher(_effects[index].aeffect, effClose);
        
        // Remove this effect from the vector
        _effects.erase(_effects.begin() + index);
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
void ofxVstHost::closeAllEffects(void)
{
    if(_effects.size() > 0) {
        do{
            closeEffect(0);
        }while(_effects.size() > 0);
    }
}


//--------------------------------------------------------------
void ofxVstHost::closeAllPlugins(void)
{
    if(_plugins.size() > 0) {
        do{
            closePlugin(0);
        }while(_plugins.size() > 0);
    }
}

//--------------------------------------------------------------
void ofxVstHost::closeAll(void)
{
    // Close all effects
    closeAllEffects();
    
    // Close all plugins
    closeAllPlugins();
    
    // Free our audio variables
    delete [] _audio;
    
    _rBuffer.clear();
    _lBuffer.clear();
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
        _effects[indexEffect].aeffect->processReplacing(_effects[indexEffect].aeffect, _audio, _audio, buffer.getNumFrames());
        
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
        return _effects[indexEffect].aeffect->numInputs;
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
        return _effects[indexEffect].aeffect->numOutputs;
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
        return _effects[indexEffect].aeffect->numParams;
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
        return _effects[indexEffect].aeffect->numPrograms;
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
        dispatcher (_effects[indexEffect].aeffect, effGetVendorString, 0, 0, vendorString);
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
        dispatcher (_effects[indexEffect].aeffect, effGetProductString, 0, 0, productString);
    }
    else {
        ofLogError("ofxVstHost") << "getProductString(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }
    
    return ofToString(productString);
}

//--------------------------------------------------------------
string ofxVstHost::getEffectLabel(int indexEffect) const
{
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        return _effects[indexEffect].label;
    }
    else {
        ofLogError("ofxVstHost") << "getEffectName(...), indexEffect out of bound. Expected " << _effects.size() - 1 << " max, got " << indexEffect;
    }
    
    return "";
}

//--------------------------------------------------------------
string ofxVstHost::listParameterValues(int indexEffect)
{
    string str = "";
    
    // Make sure indexEffect does not go out of bound
    if(indexEffect < _effects.size()) {
        // Iterate parameters...
        for (VstInt32 paramIndex = 0; paramIndex < _effects[indexEffect].aeffect->numParams; paramIndex++)
        {
            char paramName[256] = {0};
            char paramLabel[256] = {0};
            char paramDisplay[256] = {0};
            
            dispatcher (_effects[indexEffect].aeffect, effGetParamName, paramIndex, 0, paramName, 0);
            dispatcher (_effects[indexEffect].aeffect, effGetParamLabel, paramIndex, 0, paramLabel, 0);
            dispatcher (_effects[indexEffect].aeffect, effGetParamDisplay, paramIndex, 0, paramDisplay, 0);
            float value = _effects[indexEffect].aeffect->getParameter (_effects[indexEffect].aeffect, paramIndex);
            
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
        return _effects[indexEffect].aeffect->getParameter (_effects[indexEffect].aeffect, indexParam);
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
        _effects[indexEffect].aeffect->setParameter (_effects[indexEffect].aeffect, indexParam, value);
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
        dispatcher (_effects[indexEffect].aeffect, effGetParamName, indexParam, 0, paramName);
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
        dispatcher (_effects[indexEffect].aeffect, effGetParamLabel, indexParam, 0, paramLabel);
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
