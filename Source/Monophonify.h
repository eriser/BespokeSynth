//
//  Monophonify.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/12/12.
//
//

#ifndef __modularSynth__Monophonify__
#define __modularSynth__Monophonify__

#include <iostream>
#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "ModulationChain.h"

struct HeldNote
{
   HeldNote();
   HeldNote(int p, int v) : mPitch(p), mVelocity(v) {}
   int mPitch;
   int mVelocity;
};

class Monophonify : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener
{
public:
   Monophonify();
   static IDrawableModule* Create() { return new Monophonify(); }
   
   string GetTitleLabel() override { return "monophonify"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 110; height = 20; }
   bool Enabled() const override { return mEnabled; }
   
   list<HeldNote> mHeldNotes;
   ofMutex mHeldNotesMutex;
   
   float mGlideTime;
   FloatSlider* mGlideSlider;
   ModulationChain mPitchBend;
};


#endif /* defined(__modularSynth__Monophonify__) */

