//
//  NoteSustain.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/19/15.
//
//

#ifndef __Bespoke__NoteSustain__
#define __Bespoke__NoteSustain__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"
#include "Slider.h"
#include "Transport.h"

class NoteSustain : public NoteEffectBase, public IDrawableModule, public IFloatSliderListener, public IAudioPoller
{
public:
   NoteSustain();
   ~NoteSustain();
   static IDrawableModule* Create() { return new NoteSustain(); }
   
   string GetTitleLabel() override { return "note sustain"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; mNoteOutput.Flush(); }
   
   void OnTransportAdvanced(float amount) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 110; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   struct QueuedNoteOff
   {
      QueuedNoteOff(double time, double pitch, double voiceIdx) : mTime(time), mPitch(pitch), mVoiceIdx(voiceIdx) {}
      double mTime;
      int mPitch;
      int mVoiceIdx;
   };
   
   float mSustain;
   FloatSlider* mSustainSlider;
   list<QueuedNoteOff> mNoteOffs;
};


#endif /* defined(__Bespoke__NoteSustain__) */
