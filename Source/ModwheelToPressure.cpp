//
//  ModwheelToPressure.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#include "ModwheelToPressure.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

ModwheelToPressure::ModwheelToPressure()
{
}

ModwheelToPressure::~ModwheelToPressure()
{
}

void ModwheelToPressure::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
}

void ModwheelToPressure::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, NULL, modWheel);
   }
   else
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   }
}

void ModwheelToPressure::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void ModwheelToPressure::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
