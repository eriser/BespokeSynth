//
//  Neighborhooder.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/10/13.
//
//

#ifndef __modularSynth__Neighborhooder__
#define __modularSynth__Neighborhooder__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"

class Neighborhooder : public NoteEffectBase, public IDrawableModule, public IIntSliderListener
{
public:
   Neighborhooder();
   static IDrawableModule* Create() { return new Neighborhooder(); }
   
   string GetTitleLabel() override { return "neighborhooder"; }
   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;

   void CheckboxUpdated(Checkbox* checkbox) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 120; height = 20; }
   bool Enabled() const override { return mEnabled; }

   int mOctave;
   IntSlider* mOctaveSlider;
};

#endif /* defined(__modularSynth__Neighborhooder__) */

