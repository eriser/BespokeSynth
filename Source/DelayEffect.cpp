//
//  DelayEffect.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#include "DelayEffect.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "Profiler.h"

DelayEffect::DelayEffect()
: mDelay(500)
, mFeedback(0)
, mEcho(true)
, mBuffer(DELAY_BUFFER_SIZE)
, mDelaySlider(NULL)
, mFeedbackSlider(NULL)
, mEchoCheckbox(NULL)
, mInterval(kInterval_8nd)
, mIntervalSelector(NULL)
, mAcceptInput(true)
, mShortTime(false)
, mShortTimeCheckbox(NULL)
, mDry(true)
, mDryCheckbox(NULL)
, mFeedbackModuleMode(false)
, mAcceptInputCheckbox(NULL)
{
}

void DelayEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mDelaySlider = new FloatSlider(this,"delay",5,2,90,15,&mDelay,GetMinDelayMs(),1000);
   mFeedbackSlider = new FloatSlider(this,"amount",5,18,90,15,&mFeedback,0,1);
   mEchoCheckbox = new Checkbox(this,"echo",59,49,&mEcho);
   mIntervalSelector = new DropdownList(this,"interval", 5, 34, (int*)(&mInterval));
   mShortTimeCheckbox = new Checkbox(this,"short",55,34,&mShortTime);
   mDryCheckbox = new Checkbox(this,"dry",28,49,&mDry);
   mAcceptInputCheckbox = new Checkbox(this,"in",3,49,&mAcceptInput);
   
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("8nd", kInterval_8nd);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("16nd", kInterval_16nd);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
}

void DelayEffect::ProcessAudio(double time, float* audio, int bufferSize)
{
   Profiler profiler("DelayEffect");

   if (!mEnabled)
      return;

   if (mInterval != kInterval_None)
   {
      mDelay = TheTransport->GetDuration(mInterval) + .1f; //+1 to avoid perfect sample collision
      mDelayRamp.Start(mDelay, 10);
   }

   for (int i=0; i<bufferSize; ++i)
   {
      ComputeSliders(i);

      mAmountRamp.Start(mFeedback,3);
      float delay = mDelayRamp.Value(time);

      float delaySamps = delay / gInvSampleRateMs;
      if (mFeedbackModuleMode)
         delaySamps -= gBufferSize;
      delaySamps = ofClamp(delaySamps, 0.1f, DELAY_BUFFER_SIZE-2);

      int sampsAgoA = int(delaySamps);
      int sampsAgoB = sampsAgoA+1;

      float sample = mBuffer.GetSample(sampsAgoA);
      float nextSample = mBuffer.GetSample(sampsAgoB);
      float a = delaySamps - sampsAgoA;
      float delayedSample = (1-a)*sample + a*nextSample; //interpolate
      
      float in = audio[i];

      if (!mEcho && mAcceptInput) //single delay, no continuous feedback so do it pre
         mBuffer.Write(audio[i]);

      float delayInput = delayedSample * mAmountRamp.Value(time);
      FIX_DENORMAL(delayInput);
      if (delayInput == delayInput) //filter NaNs
         audio[i] += delayInput;

      if (mEcho && mAcceptInput) //continuous feedback so do it post
         mBuffer.Write(audio[i]);
      
      if (!mAcceptInput)
         mBuffer.Write(delayInput);
      
      if (!mDry)
         audio[i] -= in;

      time += gInvSampleRateMs;
   }
}

void DelayEffect::DrawModule()
{
   if (!mEnabled)
      return;
   
   mDelaySlider->Draw();
   mFeedbackSlider->Draw();
   mIntervalSelector->Draw();
   mShortTimeCheckbox->Draw();
   mDryCheckbox->Draw();
   mEchoCheckbox->Draw();
   mAcceptInputCheckbox->Draw();
}

void DelayEffect::GetModuleDimensions(int& width, int& height)
{
   if (mEnabled)
   {
      width = 100;
      height = 64;
   }
   else
   {
      width = 90;
      height = 0;
   }
}

float DelayEffect::GetEffectAmount()
{
   if (!mEnabled || !mAcceptInput)
      return 0;
   return mFeedback;
}

void DelayEffect::SetDelay(float delay)
{
   mDelay = delay;
   mDelayRamp.Start(mDelay, 10);
   mInterval = kInterval_None;
}

void DelayEffect::SetShortMode(bool on)
{
   mShortTime = on;
   mDelaySlider->SetExtents(GetMinDelayMs(),mShortTime?20:1000);
}

void DelayEffect::SetFeedbackModuleMode(bool feedbackMode)
{
   mFeedbackModuleMode = feedbackMode;
   if (mFeedbackModuleMode)
   {
      mDry = false;
      mEcho = false;
      mDelaySlider->SetExtents(GetMinDelayMs(),1000);
   }
   mDryCheckbox->SetShowing(!mFeedbackModuleMode);
   mEchoCheckbox->SetShowing(!mFeedbackModuleMode);
}

float DelayEffect::GetMinDelayMs() const
{
   if (mFeedbackModuleMode)
      return (gBufferSize + 1) * gInvSampleRateMs;
   return .1f;
}

void DelayEffect::SetEnabled(bool enabled)
{
   mEnabled = enabled;
   if (!enabled)
      mBuffer.ClearBuffer();
}

void DelayEffect::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mShortTimeCheckbox)
      SetShortMode(mShortTime);
   if (checkbox == mEnabledCheckbox)
   {
      if (!mEnabled)
         mBuffer.ClearBuffer();
   }
}

void DelayEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mDelaySlider)
   {
      mInterval = kInterval_None;
      mDelayRamp.Start(mDelay, 30);
   }
}

void DelayEffect::DropdownUpdated(DropdownList* list, int oldVal)
{
}

namespace
{
   const int kSaveStateRev = 0;
}

void DelayEffect::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mBuffer.SaveState(out);
}

void DelayEffect::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mBuffer.LoadState(in);
}

