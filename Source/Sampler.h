//
//  Sampler.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/5/14.
//
//

#ifndef __modularSynth__Sampler__
#define __modularSynth__Sampler__

#include <iostream>
#include "IAudioSource.h"
#include "PolyphonyMgr.h"
#include "SampleVoice.h"
#include "ADSR.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ADSRDisplay.h"
#include "Checkbox.h"
#include "IAudioReceiver.h"
#include "PitchDetector.h"

class ofxJSONElement;

#define MAX_SAMPLER_LENGTH 2*gSampleRate

class Sampler : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IAudioReceiver
{
public:
   Sampler();
   ~Sampler();
   static IDrawableModule* Create() { return new Sampler(); }
   
   string GetTitleLabel() override { return "sampler"; }
   void CreateUIControls() override;
   
   void SetVol(float vol) { mVoiceParams.mVol = vol; mADSRDisplay->SetVol(vol); }
   void Poll() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override { bufferSize = gBufferSize; return mRecordBuffer; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   void StopRecording();
   float DetectSampleFrequency();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   bool Enabled() const override { return mEnabled; }
   
   
   PolyphonyMgr mPolyMgr;
   SampleVoiceParams mVoiceParams;
   FloatSlider* mVolSlider;
   ADSRDisplay* mADSRDisplay;
   float mThresh;
   FloatSlider* mThreshSlider;

   float* mSampleData;
   int mRecordPos;
   bool mRecording;
   Checkbox* mRecordCheckbox;
   bool mPitchCorrect;
   Checkbox* mPitchCorrectCheckbox;
   bool mPassthrough;
   Checkbox* mPassthroughCheckbox;
   
   float* mWriteBuffer;
   float* mRecordBuffer;
   
   PitchDetector mPitchDetector;
   bool mWantDetectPitch;
};


#endif /* defined(__modularSynth__Sampler__) */

