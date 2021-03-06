//
//  FreqDelay.h
//  modularSynth
//
//  Created by Ryan Challinor on 5/10/13.
//
//

#ifndef __modularSynth__FreqDelay__
#define __modularSynth__FreqDelay__

#include <iostream>
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "DelayEffect.h"
#include "Slider.h"

class FreqDelay : public IAudioReceiver, public IAudioSource, public IDrawableModule, public INoteReceiver, public IFloatSliderListener
{
public:
   FreqDelay();
   virtual ~FreqDelay();
   static IDrawableModule* Create() { return new FreqDelay(); }
   
   string GetTitleLabel() override { return "freq delay"; }
   void CreateUIControls() override;

   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;

   //IAudioSource
   void Process(double time) override;

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& x, int& y) override { x = 130; y = 110; }
   bool Enabled() const override { return true; }

   int mInputBufferSize;
   float* mInputBuffer;
   float* mDryBuffer;
   float mDryWet;
   FloatSlider* mDryWetSlider;

   DelayEffect mDelayEffect;
};

#endif /* defined(__modularSynth__FreqDelay__) */

