#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::exit() {
    /*if(vstHost){
        delete vstHost;
    }*/
    
    /*if(vstHost.size() > 0){
        for (int i = 0; i < vstHost.size(); i++) {
            delete vstHost[i];
        }
        vstHost.clear();
    }*/
}


//--------------------------------------------------------------
void ofApp::setup(){
    sampleRate = 44100;
    wavePhase = 0;
    pulsePhase = 0;
    bufferSize = 1024;
    
    // base frequency of the lowest sine wave in cycles per second (hertz)
    frequency = 172.5;
    
    // mapping frequencies from Hz into full oscillations of sin() (two pi)
    wavePhaseStep = (frequency / sampleRate) * TWO_PI;
    pulsePhaseStep = (0.5 / sampleRate) * TWO_PI;
    
    
    vstHost.setup(sampleRate, bufferSize);
    
    
    //loadVst();
    isVstActive = false;
    
    
    ofSoundStreamSetup(2, 0, this, sampleRate, bufferSize, 4);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    string str = "";
    
    for(int i = 0; i < vstHost.getNumEffects(); i++) {
        str += vstHost.listParameterValues(i) + "\n";
    }
    
    ofSetColor(255);
    ofDrawBitmapStringHighlight(str, 20, 20);
}


//--------------------------------------------------------------
void ofApp::loadVst(void)
{
    string vstPath = ofToDataPath("vsts/MVerb");
    //string vstPath = ofToDataPath("vsts/mda-Delay");
    
#if defined __APPLE__
    vstPath += ".vst";
#elif defined _WIN32
    vstPath += ".dll";
#elif defined __linux__
    vstPath += ".so";
#endif
    //vstPath = "/home/nuc/Documents/of_v0.8.4_linux_release/apps/mashmachineWorkspace/mashmachine/bin/data/vst/linux/HiLoFilter.so";
    
    //ofSetLogLevel("ofxVSTHost", OF_LOG_VERBOSE);
    
    
    /*if(vstHost){
        delete vstHost;
    }
    
    //TODO: should we put the setup inside load()? Otherwise we will forget it!
    //vstHost.setup(2, sampleRate, bufferSize);
    //vstHost.load(vstPath);
    
    vstHost = new ofxVstHost();
    vstHost->setup(vstPath);*/
    
    vstHost.load(vstPath);
    
    
    /*if(vstHost.size() > 0){
        for (int i = 0; i < vstHost.size(); i++) {
            
            if(i == vstHost.size() - 1) {
                vstHost[i]->close(true);
            }
            else {
                vstHost[i]->close();
            }
            delete vstHost[i];
            //vstHost.erase(vstHost.begin());
        }
        vstHost.clear();
    }
    
    vstHost.resize(40);
    
    for(int i = 0; i < vstHost.size(); i++) {
        vstHost[i] = new ofxVstHost();
        vstHost[i]->setup(vstPath);
    }*/
    
    
    //isVstActive = false;
    
    //vstParametersList = vstHost.listParameterValues();
}


//--------------------------------------------------------------
void ofApp::setVstParameter(int index, int x, int y)
{
    vstHost.setParameterValue(index, 1, ofMap(x, 0, ofGetWindowWidth(), 0.0, 1.0));
    vstHost.setParameterValue(index, 7, ofMap(y, 0, ofGetWindowHeight(), 0.0, 1.0));
}


//--------------------------------------------------------------
void ofApp::play(float &lAudioOut, float &rAudioOut)
{
    // Noise
    if (isNoise){
        lAudioOut = ofRandom(0, 1);
        rAudioOut = ofRandom(0, 1);
    }
    else {
        // build up a chord out of sine waves at 3 different frequencies
        float sampleLow = sin(wavePhase);
        float sampleMid = sin(wavePhase * 1.5);
        float sampleHi = sin(wavePhase * 2.0);
        
        // pulse each sample's volume
        sampleLow *= sin(pulsePhase);
        sampleMid *= sin(pulsePhase * 1.04);
        sampleHi *= sin(pulsePhase * 1.09);
        
        float fullSample = (sampleLow + sampleMid + sampleHi);
        
        // reduce the full sample's volume so it doesn't exceed 1
        fullSample *= 0.3;
        
        // write the computed sample to the left and right channels
        lAudioOut = fullSample;
        rAudioOut = fullSample;
        
        // get the two phase variables ready for the next sample
        wavePhase += wavePhaseStep;
        pulsePhase += pulsePhaseStep;
    }
}



//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer &buffer)
{
    // Fill the audio buffer
    for(int i = 0; i < buffer.getNumFrames(); i++) {
        // Return the sample value for this loop
        play(buffer.getSample(i, 0), buffer.getSample(i, 1));
    }
    
    // Apply effect
    if(isVstActive) {
        // Send the samples to the VST
        /*if (vstHost.isInitialised()) {
            vstHost.process(buffer);
        }*/
        
        for(int i = 0; i < vstHost.getNumEffects(); i ++) {
            vstHost.process(i, buffer);
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // Load
    if (key == 'l' || key == 'L' ){
        loadVst();
    }
    
    // Noise
    if (key == 'n' || key == 'N' ){
        isNoise = !isNoise;
    }
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    setVstParameter(button, x, y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    isVstActive = true;
    setVstParameter(button, x, y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    isVstActive = false;
    setVstParameter(button, x, y);
}
