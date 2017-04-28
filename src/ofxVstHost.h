//
//  ofxVstHost.h
//
//
//  Created by Mathieu Hingant on 10/04/17.
//
//

#pragma once

#include "ofMain.h"
#include "ofxVstHostPluginLoader.h"
#include "ofxVstEffect.h"


//TODO: use ofxVstEffect class!!

class ofxVstHost
{
public:
    ofxVstHost();
    ~ofxVstHost();
    
    
    // Main controls
    void setup(float sampleRate = 48000.f, int blockSize = 512);
    int load(string path, string label = "");
    void closeEffect(int index);
    void closePlugin(int index);
    void closeAllEffects(void);
    void closeAllPlugins(void);
    void closeAll(void);
    
    void process(int indexEffect, ofSoundBuffer &buffer);
    
    // Plugins info
    int getNumPlugins(void) const;
    int getNumEffects(void) const;
    float getSampleRate(void) const;
    VstInt32 getBlockSize(void) const;
    
    
    // Effect info
    int getNumInputs(int indexEffect) const;
    int getNumOutputs(int indexEffect) const;
    int getNumParameters(int indexEffect) const;
    int getNumPrograms(int indexEffect) const;
    string getVendorString(int indexEffect) const;
    string getProductString(int indexEffect) const;
    string getEffectLabel(int indexEffect) const;
    
    
    // Effect parameters
    string listParameterValues(int indexEffect);
    float getParameterValue(int indexEffect, int indexParam) const;
    void setParameterValue(int indexEffect, int indexParam, float value);
    string getParameterName(int indexEffect, int indexParam) const;
    string getParameterLabel(int indexEffect, int indexParam) const;
    
    
private:
    
    // Module is where to load the library
    vector<ofxVstHostPluginLoader> _plugins;
    // Effect is one instance of this loaded library
    vector<ofxVstEffect> _effects;
    
    VstInt32 _blockSize;
    float _sampleRate;
    VstInt32 _numProcessCycles;

    
    // Out audio array used to process the audio
    float** _audio;
    
    // Our sound buffer when processing the audio
    ofSoundBuffer _rBuffer;
    ofSoundBuffer _lBuffer;
    
    static bool checkPlatform();
    
    
    //--------------------------------------------------------------
    intptr_t dispatcher(AEffect* effect, int32_t opcode, int32_t index = 0, intptr_t value = 0, void *ptr = nullptr, float opt = 0.0f) const;
    
    static VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
};
