//
//  PressureToVibrato.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#ifndef __Bespoke__PressureToVibrato__
#define __Bespoke__PressureToVibrato__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "Checkbox.h"
#include "ModulationChain.h"
#include "DropdownList.h"

class PressureToVibrato : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IDropdownListener
{
public:
   PressureToVibrato();
   virtual ~PressureToVibrato();
   static IDrawableModule* Create() { return new PressureToVibrato(); }
   
   string GetTitleLabel() override { return "pressure to vibrato"; }
   void CreateUIControls() override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 138; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   NoteInterval mVibratoInterval;
   DropdownList* mIntervalSelector;
   float mVibratoAmount;
   FloatSlider* mVibratoSlider;
   
   Modulations mModulation;
};

#endif /* defined(__Bespoke__PressureToVibrato__) */
