//
//  MultibandCompressor.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/27/14.
//
//

#ifndef __Bespoke__MultibandCompressor__
#define __Bespoke__MultibandCompressor__

#include <iostream>
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "RollingBuffer.h"
#include "LinkwitzRileyFilter.h"
#include "PeakTracker.h"

#define COMPRESSOR_MAX_BANDS 10

class MultibandCompressor : public IAudioReceiver, public IAudioSource, public IDrawableModule, public IFloatSliderListener, public IIntSliderListener
{
public:
   MultibandCompressor();
   virtual ~MultibandCompressor();
   static IDrawableModule* Create() { return new MultibandCompressor(); }
   
   string GetTitleLabel() override { return "multiband compressor"; }
   void CreateUIControls() override;
   
   //IAudioReceiver
   float* GetBuffer(int& bufferSize) override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& w, int&h) override { w=210; h=150; }
   bool Enabled() const override { return mEnabled; }
   
   void CalcFilters();
   
   
   int mInputBufferSize;
   float* mInputBuffer;
   float* mWorkBuffer;
   float* mOutBuffer;
   
   float mDryWet;
   FloatSlider* mDryWetSlider;
   IntSlider* mNumBandsSlider;
   int mNumBands;
   float mFreqMin;
   float mFreqMax;
   FloatSlider* mFMinSlider;
   FloatSlider* mFMaxSlider;
   float mRingTime;
   FloatSlider* mRingTimeSlider;
   float mMaxBand;
   FloatSlider* mMaxBandSlider;
   
   CLinkwitzRiley_4thOrder mFilters[COMPRESSOR_MAX_BANDS];
   PeakTracker mPeaks[COMPRESSOR_MAX_BANDS];
};

#endif /* defined(__Bespoke__MultibandCompressor__) */

