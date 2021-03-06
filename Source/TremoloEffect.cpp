//
//  TremoloEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/27/12.
//
//

#include "TremoloEffect.h"
#include "OpenFrameworksPort.h"
#include "Profiler.h"

TremoloEffect::TremoloEffect()
: mAmount(0)
, mAmountSlider(NULL)
, mOffset(0)
, mOffsetSlider(NULL)
, mInterval(kInterval_16n)
, mIntervalSelector(NULL)
, mOscType(kOsc_Square)
, mOscSelector(NULL)
, mDuty(.5f)
, mDutySlider(NULL)
, mWindowPos(0)
{
   mLFO.SetPeriod(mInterval);
   mLFO.SetType(mOscType);
   Clear(mWindow, kAntiPopWindowSize);
}

void TremoloEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mAmountSlider = new FloatSlider(this,"amount",5,4,85,15,&mAmount,0,1);
   mOffsetSlider = new FloatSlider(this,"offset",5,20,85,15,&mOffset,0,1);
   mIntervalSelector = new DropdownList(this,"interval",5,57,(int*)(&mInterval));
   mOscSelector = new DropdownList(this,"osc",50,57,(int*)(&mOscType));
   mDutySlider = new FloatSlider(this,"duty",5,36,85,15,&mDuty,0,1);
   
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   
   mOscSelector->AddLabel("sin",kOsc_Sin);
   mOscSelector->AddLabel("saw",kOsc_Saw);
   mOscSelector->AddLabel("-saw",kOsc_NegSaw);
   mOscSelector->AddLabel("squ",kOsc_Square);
   mOscSelector->AddLabel("tri",kOsc_Tri);
}

void TremoloEffect::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("TremoloEffect");

   if (!mEnabled)
      return;

   ComputeSliders(0);

   if (mAmount > 0)
   {
      for (int i=0; i<bufferSize; ++i)
      {
         //smooth out LFO a bit to avoid pops with square/saw LFOs
         mWindow[mWindowPos] = mLFO.Value(i+kAntiPopWindowSize/2);
         mWindowPos = (mWindowPos+1) % kAntiPopWindowSize;
         float lfoVal = 0;
         for (int j=0; j<kAntiPopWindowSize; ++j)
            lfoVal += mWindow[j];
         lfoVal /= kAntiPopWindowSize;
         
         audio[i] = audio[i] * (1 - (mAmount * (1-lfoVal)));
      }
   }
}

void TremoloEffect::DrawModule()
{
   if (!mEnabled)
      return;

   ofPushStyle();
   ofSetColor(0,200,0);
   ofFill();
   ofRect(5, 4, mLFO.Value() * 85 * mAmount, 14);
   ofPopStyle();
   
   mAmountSlider->Draw();
   mIntervalSelector->Draw();
   mOffsetSlider->Draw();
   mOscSelector->Draw();
   mDutySlider->Draw();
}

void TremoloEffect::GetModuleDimensions(int& width, int& height)
{
   if (mEnabled)
   {
      width = 95;
      height = 80;
   }
   else
   {
      width = 90;
      height = 0;
   }
}

float TremoloEffect::GetEffectAmount()
{
   if (!mEnabled)
      return 0;
   return mAmount;
}

void TremoloEffect::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      mLFO.SetPeriod(mInterval);
   if (list == mOscSelector)
      mLFO.SetType(mOscType);
}

void TremoloEffect::CheckboxUpdated(Checkbox* checkbox)
{
}

void TremoloEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mOffsetSlider)
      mLFO.SetOffset(mOffset);
   if (slider == mDutySlider)
   {
      mLFO.SetPulseWidth(mDuty);
   }
}

