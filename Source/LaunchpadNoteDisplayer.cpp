//
//  LaunchpadNoteDisplayer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/16/13.
//
//

#include "LaunchpadNoteDisplayer.h"
#include "OpenFrameworksPort.h"
#include "LaunchpadKeyboard.h"
#include "ModularSynth.h"
#include "FillSaveDropdown.h"

LaunchpadNoteDisplayer::LaunchpadNoteDisplayer()
: mLaunchpad(NULL)
{
}

void LaunchpadNoteDisplayer::DrawModule()
{

   DrawConnection(mLaunchpad);
}

void LaunchpadNoteDisplayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);

   if (mLaunchpad)
      mLaunchpad->DisplayNote(pitch, velocity);
}

void LaunchpadNoteDisplayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("launchpad", moduleInfo, "", FillDropdown<LaunchpadKeyboard*>);
   
   SetUpFromSaveData();
}

void LaunchpadNoteDisplayer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mLaunchpad = dynamic_cast<LaunchpadKeyboard*>(TheSynth->FindModule(mModuleSaveData.GetString("launchpad"), false));
   if (mLaunchpad)
      mLaunchpad->SetHasDisplayer();
}


