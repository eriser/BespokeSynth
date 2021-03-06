//
//  LFOController.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 10/22/13.
//
//

#include "LFOController.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"

LFOController* TheLFOController = NULL;

LFOController::LFOController()
: dummy(0)
, dummy2(0)
, mIntervalSelector(NULL)
, mOscSelector(NULL)
, mMinSlider(NULL)
, mMaxSlider(NULL)
, mWantBind(false)
, mBindButton(NULL)
, mLFO(NULL)
, mSlider(NULL)
, mStopBindTime(-1)
{
   assert(TheLFOController == NULL);
   TheLFOController = this;
}

void LFOController::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",5,40,&dummy);
   mOscSelector = new DropdownList(this,"osc",50,40,&dummy);
   mMinSlider = new FloatSlider(this,"min",5,4,120,15,&dummy2,0,1);
   mMaxSlider = new FloatSlider(this,"max",5,22,120,15,&dummy2,0,1);
   mBindButton = new ClickButton(this,"bind",5,60);
   
   mIntervalSelector->AddLabel("64", kInterval_64);
   mIntervalSelector->AddLabel("32", kInterval_32);
   mIntervalSelector->AddLabel("16", kInterval_16);
   mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("2nt", kInterval_2nt);
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
   mOscSelector->AddLabel("rand",kOsc_Random);
}

LFOController::~LFOController()
{
   assert(TheLFOController == this || TheLFOController == nullptr);
   TheLFOController = nullptr;
}

bool LFOController::WantsBinding(FloatSlider* slider)
{
   if (slider == mMinSlider || slider == mMaxSlider)
      return false;
   return mWantBind;
}

void LFOController::SetSlider(FloatSlider* slider)
{
   mSlider = slider;
   mLFO = slider->AcquireLFO();
   
   if (mLFO == NULL)
      return;
   
   LFOSettings* lfoSettings = mLFO->GetLFOSettings();
   assert(lfoSettings);
   
   mMinSlider->MatchExtents(slider);
   mMaxSlider->MatchExtents(slider);
   mMinSlider->SetVar(&lfoSettings->mMin);
   mMaxSlider->SetVar(&lfoSettings->mMax);
   mIntervalSelector->SetVar((int*)(&lfoSettings->mInterval));
   mOscSelector->SetVar((int*)(&lfoSettings->mOscType));
   mLFO->SetEnabled(true);
   
   mStopBindTime = gTime + 1000;
}

void LFOController::Poll()
{
   if (mWantBind && mStopBindTime != -1 && gTime > mStopBindTime)
   {
      mWantBind = false;
      mStopBindTime = -1;
   }
}

void LFOController::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mIntervalSelector->Draw();
   mOscSelector->Draw();
   mMinSlider->Draw();
   mMaxSlider->Draw();
   mBindButton->Draw();
   
   if (mSlider && mLFO && mLFO->IsEnabled())
   {
      DrawText(mSlider->Name(), 50, 70);
      DrawConnection(mSlider);
   }
}

void LFOController::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (mLFO == NULL)
      return;
   
   mLFO->UpdateFromSettings();
}

void LFOController::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (mLFO == NULL)
      return;
   
   LFOSettings* lfoSettings = mLFO->GetLFOSettings();
   assert(lfoSettings);
   
   if (slider == mMinSlider)
      lfoSettings->mMax = MAX(lfoSettings->mMax, lfoSettings->mMin);
   if (slider == mMaxSlider)
      lfoSettings->mMin = MIN(lfoSettings->mMax, lfoSettings->mMin);
}

void LFOController::ButtonClicked(ClickButton* button)
{
   if (button == mBindButton)
      mWantBind = true;
}

