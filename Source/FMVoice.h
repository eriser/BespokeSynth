//
//  FMVoice.h
//  modularSynth
//
//  Created by Ryan Challinor on 1/6/13.
//
//

#ifndef __modularSynth__FMVoice__
#define __modularSynth__FMVoice__

#include <iostream>
#include "OpenFrameworksPort.h"
#include "IMidiVoice.h"
#include "IVoiceParams.h"
#include "ADSR.h"
#include "EnvOscillator.h"

class IDrawableModule;

class FMVoiceParams : public IVoiceParams
{
public:
   ADSR mOscADSRParams;
   ADSR mModIdxADSRParams;
   ADSR mHarmRatioADSRParams;
   float mModIdx;
   float mHarmRatio;
   float mVol;
};

class FMVoice : public IMidiVoice
{
public:
   FMVoice(IDrawableModule* owner = NULL);
   ~FMVoice();

   // IMidiVoice
   void Start(double time, float amount);
   void Stop(double time);
   void ClearVoice();
   void Process(double time, float* out, int bufferSize);
   void SetVoiceParams(IVoiceParams* params);
private:
   float mOscPhase;
   float mHarmPhase;
   EnvOscillator mOsc;
   EnvOscillator mHarm;
   ADSR mModIdx;
   FMVoiceParams* mVoiceParams;
   IDrawableModule* mOwner;
};

#endif /* defined(__modularSynth__FMVoice__) */
