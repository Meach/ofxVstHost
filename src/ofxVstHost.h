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


//TODO:
// - Close library function


//TODO: try to use this to know which is which!
struct effect_data {
    AEffect* effect;
    string label;
};


class ofxVstHost
{
public:
    ofxVstHost();
    ~ofxVstHost();
    
    
    // Main controls
    void setup(float sampleRate = 48000.f, int blockSize = 512);
    bool load(string path, string nameInfo = "");
    void closeEffect(int index);
    void closePlugin(int index);
    
    void process(int indexEffect, ofSoundBuffer &buffer);
    
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
    string getEffectName(int indexEffect) const;
    
    // Effect parameters
    string listParameterValues(int indexEffect);
    float getParameterValue(int indexEffect, int indexParam) const;
    void setParameterValue(int indexEffect, int indexParam, float value);
    string getParameterName(int indexEffect, int indexParam) const;
    string getParameterLabel(int indexEffect, int indexParam) const;
    
    
private:
    // Module is where to load the library
    vector<PluginLoader> _plugins;
    // Effect is one instance of this loaded library
    vector<AEffect* > _effects;
    // Each effect can be given a name to know which effect is which
    vector<string> _names;
    
    
    //--------------------------------------------------------------
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
