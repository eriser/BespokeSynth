//
//  SlowLayers.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/13/15.
//
//

#ifndef __Bespoke__SlowLayers__
#define __Bespoke__SlowLayers__

#include <iostream>
#include "IAudioSource.h"
#include "IAudioReceiver.h"
#include "IDrawableModule.h"
#include "RollingBuffer.h"
#include "ClickButton.h"
#include "RadioButton.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"

class SlowLayers : public IAudioSource, public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener, public IRadioButtonListener, public IAudioReceiver
{
public:
   SlowLayers();
   ~SlowLayers();
   static IDrawableModule* Create() { return new SlowLayers(); }
   
   string GetTitleLabel() override { return "slow layers"; }
   void CreateUIControls() override;
   
   void Clear();
   int NumBars() { return mNumBars; }
   void SetNumBars(int numBars);
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   bool CheckNeedsDraw() override { return true; }

   void ButtonClicked(ClickButton* button) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override;
   bool Enabled() const override { return mEnabled; }

   int LoopLength() const;

   static const int BUFFER_X = 4;
   static const int BUFFER_Y = 4;
   static const int BUFFER_W = 155;
   static const int BUFFER_H = 93;

   int mInputBufferSize;
   float* mInputBuffer;
   float* mBuffer;
   float mLoopPos;
   int mNumBars;
   float mVol;
   float mFeedIn;
   float mSmoothedVol;
   FloatSlider* mVolSlider;
   ClickButton* mClearButton;
   DropdownList* mNumBarsSelector;
   FloatSlider* mFeedInSlider;
};

#endif /* defined(__Bespoke__SlowLayers__) */
