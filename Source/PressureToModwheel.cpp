//
//  PressureToModwheel.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "PressureToModwheel.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

PressureToModwheel::PressureToModwheel()
{
}

PressureToModwheel::~PressureToModwheel()
{
}

void PressureToModwheel::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void PressureToModwheel::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, pressure, NULL);
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   }
}

void PressureToModwheel::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void PressureToModwheel::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

