//
//  FreeverbEffect.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/19/14.
//
//

#ifndef __Bespoke__FreeverbEffect__
#define __Bespoke__FreeverbEffect__

#include <iostream>
#include "IAudioProcessor.h"
#include "Slider.h"
#include "Checkbox.h"
#include "freeverb/revmodel.hpp"

class FreeverbEffect : public IAudioProcessor, public IFloatSliderListener
{
public:
   FreeverbEffect();
   ~FreeverbEffect();
   
   static IAudioProcessor* Create() { return new FreeverbEffect(); }
   
   string GetTitleLabel() override { return "freeverb"; }
   void CreateUIControls() override;
   
   //IAudioProcessor
   void ProcessAudio(double time, float* audio, int bufferSize) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   float GetEffectAmount() override;
   string GetType() override { return "freeverb"; }
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& x, int& y) override;
   bool Enabled() const override { return mEnabled; }
   
   revmodel mFreeverb;
   float* mOutputLeftBuffer;
   float* mOutputRightBuffer;
   bool mNeedUpdate;
   bool mFreeze;
   float mRoomSize;
   float mDamp;
   float mWet;
   float mDry;
   float mVerbWidth;
   FloatSlider* mRoomSizeSlider;
   FloatSlider* mDampSlider;
   FloatSlider* mWetSlider;
   FloatSlider* mDrySlider;
   FloatSlider* mWidthSlider;
};

#endif /* defined(__Bespoke__FreeverbEffect__) */
