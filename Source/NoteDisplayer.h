//
//  NoteDisplayer.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/17/15.
//
//

#ifndef __Bespoke__NoteDisplayer__
#define __Bespoke__NoteDisplayer__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class NoteDisplayer : public NoteEffectBase, public IDrawableModule
{
public:
   NoteDisplayer();
   static IDrawableModule* Create() { return new NoteDisplayer(); }
   
   string GetTitleLabel() override { return "notedisplayer"; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 110; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   int mNote;
   int mVelocity;
};

#endif /* defined(__Bespoke__NoteDisplayer__) */
