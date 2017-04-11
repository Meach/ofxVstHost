#pragma once

#include "ofMain.h"
#include "ofxVSTHost.h"


class ofApp : public ofBaseApp{
    
public:
    void exit();
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    
    // Audio
    //------
    void audioOut(ofSoundBuffer &buffer);
    
    void play(float &lAudioOut, float &rAudioOut);
    bool isNoise;
    
    double wavePhase;
    double pulsePhase;
    double sampleRate;
    double bufferSize;
    
    float frequency;
    float wavePhaseStep;
    float pulsePhaseStep;
    
    // VST
    //----
    ofxVstHost vstHost;
    void loadVst(void);
    
    bool isVstActive;
    void setVstParameter(int index, int x, int y);
};
