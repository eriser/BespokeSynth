//
//  AudioMeter.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/18/15.
//
//

#ifndef __Bespoke__AudioMeter__
#define __Bespoke__AudioMeter__

#include <iostream>
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "PeakTracker.h"

class AudioMeter : public IAudioReceiver, public IAudioSource, public IDrawableModule, public IFloatSliderListener
{
public:
   AudioMeter();
   virtual ~AudioMeter();
   static IDrawableModule* Create() { return new AudioMeter(); }
   
   string GetTitleLabel() override { return "audiometer"; }
   void CreateUIControls() override;
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=120; h=22; }
   bool Enabled() const override { return mEnabled; }
   
   int mInputBufferSize;
   float* mInputBuffer;
   float mLevel;
   float mMaxLevel;
   FloatSlider* mLevelSlider;
   PeakTracker mPeakTracker;
};

#endif /* defined(__Bespoke__AudioMeter__) */
