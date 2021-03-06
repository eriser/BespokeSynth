//
//  NoteSustain.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 6/19/15.
//
//

#include "NoteSustain.h"
#include "SynthGlobals.h"

NoteSustain::NoteSustain()
: mSustain(100)
, mSustainSlider(NULL)
{
   TheTransport->AddAudioPoller(this);
}

void NoteSustain::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mSustainSlider = new FloatSlider(this,"sustain",5,2,80,15,&mSustain,1,999);
   mSustainSlider->SetMode(FloatSlider::kSquare);
}

NoteSustain::~NoteSustain()
{
   TheTransport->RemoveAudioPoller(this);
}

void NoteSustain::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mSustainSlider->Draw();
}

void NoteSustain::OnTransportAdvanced(float amount)
{
   for (auto iter = mNoteOffs.begin(); iter != mNoteOffs.end();)
   {
      if (iter->mTime < gTime)
      {
         PlayNoteOutput(gTime, iter->mPitch, 0, iter->mVoiceIdx);
         iter = mNoteOffs.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
}

void NoteSustain::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
      return;
   }
   
   if (velocity > 0)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   
   if (velocity > 0)
   {
      ComputeSliders(0);
      
      bool found = false;
      for (auto& queued : mNoteOffs)
      {
         if (queued.mPitch == pitch)
         {
            queued.mTime = time + mSustain;
            queued.mVoiceIdx = voiceIdx;
            found = true;
            break;
         }
      }
      
      if (!found)
         mNoteOffs.push_back(QueuedNoteOff(time+mSustain, pitch, voiceIdx));
   }
}

void NoteSustain::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void NoteSustain::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadFloat("sustain", moduleInfo, 100, 1, 1000, K(isTextField));
   mModuleSaveData.LoadBool("enabled", moduleInfo, false);
   
   SetUpFromSaveData();
}

void NoteSustain::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mSustain = mModuleSaveData.GetFloat("sustain");
   mEnabled = mModuleSaveData.GetBool("enabled");
}
