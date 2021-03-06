//
//  LFO.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/27/12.
//
//

#ifndef __modularSynth__LFO__
#define __modularSynth__LFO__

#include <iostream>
#include "SynthGlobals.h"
#include "Transport.h"
#include "Oscillator.h"

enum LFOMode
{
   kLFOMode_Envelope,
   kLFOMode_Oscillator
};

class LFO : public ITimeListener, public IAudioPoller
{
public:
   LFO();
   ~LFO();
   float Value(int samplesIn = 0) const;
   void SetOffset(float offset) { mPhaseOffset = offset; }
   void SetPeriod(NoteInterval interval);
   void SetType(OscillatorType type);
   void SetPulseWidth(float width) { mOsc.SetPulseWidth(width); }
   void SetMode(LFOMode mode) { mMode = mode; }

   //ITimeListener
   void OnTimeEvent(int samplesTo) override;
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
private:
   NoteInterval mPeriod;
   float mPhaseOffset;
   Oscillator mOsc;
   float mRandom;
   LFOMode mMode;
};

#endif /* defined(__modularSynth__LFO__) */
