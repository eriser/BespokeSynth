//
//  VSTPlugin.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/18/16.
//
//

#ifndef __Bespoke__VSTPlugin__
#define __Bespoke__VSTPlugin__

#include "IAudioSource.h"
#include "PolyphonyMgr.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "EnvOscillator.h"
#include "Ramp.h"
#include "ClickButton.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "VSTPlayhead.h"
#include "VSTWindow.h"
#include "IAudioReceiver.h"

class ofxJSONElement;
class NSWindowOverlay;

class VSTPlugin : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IButtonListener, public IAudioReceiver
{
public:
   VSTPlugin();
   ~VSTPlugin();
   static IDrawableModule* Create() { return new VSTPlugin(); }
   
   string GetTitleLabel() override { return "vst"; }
   void CreateUIControls() override;
   
   void SetVol(float vol) { mVol = vol; }
   
   void Poll() override;
   void Exit() override;
   
   juce::AudioProcessor* GetAudioProcessor() { return mPlugin; }
   
   void OnVSTWindowClosed();
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void PreDrawModule() override;
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   bool Enabled() const override { return mEnabled; }
   
   void SetVST(string path);
   void CreateParameterSliders();
   
   float mVol;
   FloatSlider* mVolSlider;
   int mProgramChange;
   DropdownList* mProgramChangeSelector;
   ClickButton* mOpenEditorButton;
   int mOverlayWidth;
   int mOverlayHeight;
   
   float* mWriteBuffer;
   
   juce::AudioPluginFormatManager mFormatManager;
   juce::ScopedPointer<juce::AudioPluginInstance> mPlugin;
   juce::ScopedPointer<VSTWindow> mWindow;
   juce::MidiBuffer mMidiBuffer;
   juce::CriticalSection mMidiInputLock;
   int mNumInputs;
   int mNumOutputs;
   
   struct ParameterSlider
   {
      float mValue;
      FloatSlider* mSlider;
      int mParameterIndex;
   };
   
   vector<ParameterSlider> mParameterSliders;
   
   int mChannel;
   bool mUseVoiceAsChannel;
   float mPitchBendRange;
   int mModwheelCC;
   
   struct ChannelModulations
   {
      ModulationChain* mPitchBend;
      ModulationChain* mModWheel;
      ModulationChain* mPressure;
      float mLastPitchBend;
      float mLastModWheel;
      float mLastPressure;
   };
   
   vector<ChannelModulations> mChannelModulations;
   
   ofMutex mVSTMutex;
   VSTPlayhead mPlayhead;
   
   int mInputBufferSize;
   float* mInputBuffer;
   
   NSWindowOverlay* mWindowOverlay;
   
   enum DisplayMode
   {
      kDisplayMode_Sliders,
      kDisplayMode_PluginOverlay
   };
   
   DisplayMode mDisplayMode;
};

#endif /* defined(__Bespoke__VSTPlugin__) */
