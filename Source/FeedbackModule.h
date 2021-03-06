//
//  FeedbackModule.h
//  Bespoke
//
//  Created by Ryan Challinor on 2/1/16.
//
//

#ifndef __Bespoke__FeedbackModule__
#define __Bespoke__FeedbackModule__

#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DelayEffect.h"

class PatchCableSource;

class FeedbackModule : public IAudioReceiver, public IAudioSource, public IDrawableModule, public IFloatSliderListener
{
public:
   FeedbackModule();
   virtual ~FeedbackModule();
   static IDrawableModule* Create() { return new FeedbackModule(); }
   
   string GetTitleLabel() override { return "feedback"; }
   void CreateUIControls() override;
   
   void PostRepatch(PatchCableSource* cable) override;
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=120; h=80; }
   bool Enabled() const override { return mEnabled; }
   
   int mInputBufferSize;
   float* mInputBuffer;
   DelayEffect mDelay;
   
   IAudioReceiver* mFeedbackTarget;
   PatchCableSource* mFeedbackTargetCable;
   RollingBuffer mFeedbackVizBuffer;
};

#endif /* defined(__Bespoke__FeedbackModule__) */
