//
//  ModwheelToPressure.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/4/16.
//
//

#ifndef __Bespoke__ModwheelToPressure__
#define __Bespoke__ModwheelToPressure__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "ModulationChain.h"

class ModwheelToPressure : public NoteEffectBase, public IDrawableModule
{
public:
   ModwheelToPressure();
   virtual ~ModwheelToPressure();
   static IDrawableModule* Create() { return new ModwheelToPressure(); }
   
   string GetTitleLabel() override { return "modwheel to pressure"; }
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 120; height = 0; }
   bool Enabled() const override { return mEnabled; }
};

#endif /* defined(__Bespoke__ModwheelToPressure__) */
