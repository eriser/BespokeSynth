//
//  SampleVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/5/14.
//
//

#ifndef __modularSynth__SampleVoice__
#define __modularSynth__SampleVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"

class IDrawableModule;

class SampleVoiceParams : public IVoiceParams
{
public:
   ADSR mAdsr;
   float mVol;
   float* mSampleData;
   int mSampleLength;
   float mDetectedFreq;
};

class SampleVoice : public IMidiVoice
{
public:
   SampleVoice(IDrawableModule* owner = NULL);
   ~SampleVoice();
   
   // IMidiVoice
   void Start(double time, float amount);
   void Stop(double time);
   void ClearVoice();
   void Process(double time, float* out, int bufferSize);
   void SetVoiceParams(IVoiceParams* params);
private:
   ADSR mAdsr;
   SampleVoiceParams* mVoiceParams;
   float mPos;
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__SampleVoice__) */
