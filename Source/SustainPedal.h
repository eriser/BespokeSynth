//
//  SustainPedal.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/7/14.
//
//

#ifndef __Bespoke__SustainPedal__
#define __Bespoke__SustainPedal__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "TextEntry.h"

class SustainPedal : public NoteEffectBase, public IDrawableModule
{
public:
   SustainPedal();
   static IDrawableModule* Create() { return new SustainPedal(); }
   
   string GetTitleLabel() override { return "sustain"; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 90; height = 0; }
   bool Enabled() const override { return mEnabled; }
   
   list<int> mSustainedNotes;
   ofMutex mMutex;
};

#endif /* defined(__Bespoke__SustainPedal__) */
