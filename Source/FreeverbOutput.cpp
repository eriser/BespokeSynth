//
//  FreeverbOutput.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/27/14.
//
//

#include "FreeverbOutput.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include "ModularSynth.h"

FreeverbOutput* TheFreeverbOutput = NULL;

FreeverbOutput::FreeverbOutput()
: mRoomSizeSlider(NULL)
, mDampSlider(NULL)
, mWetSlider(NULL)
, mDrySlider(NULL)
, mWidthSlider(NULL)
, mNeedUpdate(false)
{
   assert(TheFreeverbOutput == NULL);
   TheFreeverbOutput = this;
   
   //mFreeverb.setmode(GetParameter(KMode));
   //mFreeverb.setroomsize(GetParameter(KRoomSize));
   mFreeverb.setdamp(50);
   mFreeverb.setwet(.5f);
   mFreeverb.setdry(1);
   //mFreeverb.setwidth(GetParameter(KWidth));
   mFreeverb.update();
   
   mFreeze = false;
   mRoomSize = mFreeverb.getroomsize();
   mDamp = mFreeverb.getdamp();
   mWet = mFreeverb.getwet();
   mDry = mFreeverb.getdry();
   mVerbWidth = mFreeverb.getwidth();
}

FreeverbOutput::~FreeverbOutput()
{
   assert(TheFreeverbOutput == this || TheFreeverbOutput == nullptr);
   TheFreeverbOutput = nullptr;
}

void FreeverbOutput::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRoomSizeSlider = new FloatSlider(this,"room size",5,4,100,15,&mRoomSize,.1f,.99f);
   mDampSlider = new FloatSlider(this,"damp",5,20,100,15,&mDamp,0,100);
   mWetSlider = new FloatSlider(this,"wet",5,36,100,15,&mWet,0,1);
   mDrySlider = new FloatSlider(this,"dry",5,52,100,15,&mDry,0,1);
   mWidthSlider = new FloatSlider(this,"width",5,68,100,15,&mVerbWidth,0,1);
}

void FreeverbOutput::ProcessAudio(float* left, float* right, int bufferSize)
{
   Profiler profiler("FreeverbOutput");
   
   assert(bufferSize == gBufferSize);
   
   if (!mEnabled || gTime < 100)
      return;
   
   ComputeSliders(0);
   
   if (mNeedUpdate)
   {
      mFreeverb.update();
      mNeedUpdate = false;
   }
   
   mFreeverb.processreplace(left, right, left, right, bufferSize, 1);
}

void FreeverbOutput::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mRoomSizeSlider->Draw();
   mDampSlider->Draw();
   mWetSlider->Draw();
   mDrySlider->Draw();
   mWidthSlider->Draw();
}

void FreeverbOutput::GetModuleDimensions(int& width, int& height)
{
   width = 95;
   height = 84;
}

void FreeverbOutput::CheckboxUpdated(Checkbox* checkbox)
{
}

void FreeverbOutput::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mRoomSizeSlider)
   {
      mFreeverb.setroomsize(mRoomSize);
      mNeedUpdate = true;
   }
   if (slider == mDampSlider)
   {
      mFreeverb.setdamp(mDamp);
      mNeedUpdate = true;
   }
   if (slider == mWetSlider)
   {
      mFreeverb.setwet(mWet);
      mNeedUpdate = true;
   }
   if (slider == mDrySlider)
   {
      mFreeverb.setdry(mDry);
      mNeedUpdate = true;
   }
   if (slider == mWidthSlider)
   {
      mFreeverb.setwidth(mVerbWidth);
      mNeedUpdate = true;
   }
}

void FreeverbOutput::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("enabled", moduleInfo, true);
   mModuleSaveData.LoadInt("leftchannel", moduleInfo, 1, 1, MAX_OUTPUT_CHANNELS);
   mModuleSaveData.LoadInt("rightchannel", moduleInfo, 2, 1, MAX_OUTPUT_CHANNELS);
   
   SetUpFromSaveData();
}

void FreeverbOutput::SetUpFromSaveData()
{
   SetEnabled(mModuleSaveData.GetBool("enabled"));
   mLeftChannel = mModuleSaveData.GetInt("leftchannel");
   mRightChannel = mModuleSaveData.GetInt("rightchannel");
}
