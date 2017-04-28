//
//  ofxVstEffect.h
//
//
//  Created by Mathieu Hingant on 28/04/17.
//
//

#pragma once

#include "ofMain.h"
#include "aeffectx.h"


// WIP - Attempt at having the effects separated and would access them through pointers instead of index

class ofxVstEffect
{
public:
    ofxVstEffect();
    ~ofxVstEffect();
    
    
//private:
    AEffect* aeffect;
    string label;
};
