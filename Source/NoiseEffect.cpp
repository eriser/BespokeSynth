//
//  NoiseEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#include "NoiseEffect.h"
#include "SynthGlobals.h"
#include "Profiler.h"

NoiseEffect::NoiseEffect()
: mAmount(0)
, mWidth(10)
, mSampleCounter(0)
, mRandom(0)
, mAmountSlider(NULL)
, mWidthSlider(NULL)
{
}

void NoiseEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mAmountSlider = new FloatSlider(this,"amount",5,20,110,15,&mAmount,0,1);
   mWidthSlider = new IntSlider(this,"width",5,37,110,15,&mWidth,1,100);
}

void NoiseEffect::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("NoiseEffect");

   if (!mEnabled)
      return;

   ComputeSliders(0);

   for (int i=0; i<bufferSize; ++i)
   {
      if (mSampleCounter < mWidth - 1)
      {
         ++mSampleCounter;
      }
      else
      {
         mRandom = ofRandom(mAmount) + (1-mAmount);
         mSampleCounter = 0;
      }
      audio[i] = audio[i] * mRandom;
   }
}

void NoiseEffect::DrawModule()
{
   
   mWidthSlider->Draw();
   mAmountSlider->Draw();
}

float NoiseEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return mAmount;
}

void NoiseEffect::CheckboxUpdated(Checkbox *checkbox)
{
}

void NoiseEffect::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void NoiseEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

