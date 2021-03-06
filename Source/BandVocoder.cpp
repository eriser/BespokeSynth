//
//  BandVocoder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/1/14.
//
//

#include "BandVocoder.h"
#include "ModularSynth.h"
#include "Profiler.h"

BandVocoder::BandVocoder()
: mInputPreamp(1)
, mCarrierPreamp(1)
, mVolume(1)
, mInputSlider(NULL)
, mCarrierSlider(NULL)
, mVolumeSlider(NULL)
, mDryWet(1)
, mDryWetSlider(NULL)
, mNumBands(64)
, mNumBandsSlider(NULL)
, mFreqMin(50)
, mFMinSlider(NULL)
, mFreqMax(7500)
, mFMaxSlider(NULL)
, mQ(25)
, mQSlider(NULL)
, mRingTime(.01f)
, mRingTimeSlider(NULL)
, mMaxBand(.3f)
, mMaxBandSlider(NULL)
{
   SetEnabled(false);
   
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
   
   mCarrierInputBuffer = new float[mInputBufferSize];
   Clear(mCarrierInputBuffer, mInputBufferSize);
   
   mWorkBuffer = new float[mInputBufferSize];
   Clear(mWorkBuffer, mInputBufferSize);
   
   mOutBuffer = new float[mInputBufferSize];
   Clear(mOutBuffer, mInputBufferSize);
   
   for (int i=0; i<VOCODER_MAX_BANDS; ++i)
   {
      mPeaks[i].SetDecayTime(mRingTime);
      mOutputPeaks[i].SetDecayTime(mRingTime);
      mPeaks[i].SetLimit(mMaxBand);
      mOutputPeaks[i].SetLimit(mMaxBand);
   }
   
   CalcFilters();
}

void BandVocoder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mInputSlider = new FloatSlider(this,"input", 5, 29, 100, 15, &mInputPreamp, 0, 2);
   mCarrierSlider = new FloatSlider(this,"carrier", 5, 47, 100, 15, &mCarrierPreamp, 0, 2);
   mVolumeSlider = new FloatSlider(this,"volume", 5, 65, 100, 15, &mVolume, 0, 2);
   mDryWetSlider = new FloatSlider(this,"dry/wet", 5, 83, 100, 15, &mDryWet, 0, 1);
   mNumBandsSlider = new IntSlider(this,"bands", 110, 29, 100, 15, &mNumBands, 1, VOCODER_MAX_BANDS);
   mFMinSlider = new FloatSlider(this,"fmin", 110, 47, 100, 15, &mFreqMin, 10, 300);
   mFMaxSlider = new FloatSlider(this,"fmax", 110, 65, 100, 15, &mFreqMax, 300, gSampleRate/2-1);
   mQSlider = new FloatSlider(this,"q", 110, 83, 100, 15, &mQ, 0.1f, 50);
   mRingTimeSlider = new FloatSlider(this, "ring", 110, 101, 100, 15, &mRingTime, .0001f, .1f, 4);
   mMaxBandSlider = new FloatSlider(this,"max band", 5, 101, 100, 15, &mMaxBand, 0.001f, 1);
}

BandVocoder::~BandVocoder()
{
   delete[] mInputBuffer;
   delete[] mCarrierInputBuffer;
   delete[] mWorkBuffer;
}

void BandVocoder::SetCarrierBuffer(float *carrier, int bufferSize)
{
   assert(bufferSize == mInputBufferSize);
   memcpy(mCarrierInputBuffer, carrier, bufferSize*sizeof(float));
}

float* BandVocoder::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void BandVocoder::Process(double time)
{
   Profiler profiler("BandVocoder");

   if (GetTarget() == NULL || !mEnabled)
      return;
   
   ComputeSliders(0);
   
   float inputPreampSq = mInputPreamp * mInputPreamp;
   float carrierPreampSq = mCarrierPreamp * mCarrierPreamp;
   float volSq = mVolume * mVolume;
   
   int bufferSize = gBufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   Clear(mOutBuffer, bufferSize);
   
   Mult(mInputBuffer, inputPreampSq, bufferSize);
   Mult(mCarrierInputBuffer, carrierPreampSq, bufferSize);
   
   for (int i=0; i<mNumBands; ++i)
   {
      //get modulator band
      memcpy(mWorkBuffer,mInputBuffer, bufferSize*sizeof(float));
      mBiquadCarrier[i].Filter(mWorkBuffer, bufferSize);
      
      float oldPeak = mPeaks[i].GetPeak();
      
      //calculate modulator band level
      mPeaks[i].Process(mWorkBuffer, bufferSize);
      
      //get carrier band
      memcpy(mWorkBuffer,mCarrierInputBuffer, bufferSize*sizeof(float));
      mBiquadOut[i].Filter(mWorkBuffer, bufferSize);
      
      //multiply carrier band by modulator band level
      //Mult(mWorkBuffer, mPeaks[i].GetPeak(), bufferSize);
      for (int j=0; j<bufferSize; ++j)
         mWorkBuffer[j] *= ofMap(j,0,bufferSize,oldPeak,mPeaks[i].GetPeak());
      
      //don't allow a band to go crazy
      /*mOutputPeaks[i].Process(mWorkBuffer, bufferSize);
      if (mOutputPeaks[i].GetPeak() > mMaxBand)
         Mult(mWorkBuffer, 1/mOutputPeaks[i].GetPeak(), bufferSize);*/
      
      //accumulate output band into total output
      Add(mOutBuffer, mWorkBuffer, bufferSize);
   }

   Mult(mInputBuffer, (1-mDryWet)/inputPreampSq * volSq, bufferSize);
   Mult(mOutBuffer, mDryWet * volSq, bufferSize);
   
   Add(out, mInputBuffer, bufferSize);
   Add(out, mOutBuffer, bufferSize);
   
   GetVizBuffer()->WriteChunk(out,bufferSize);
   
   Clear(mInputBuffer, mInputBufferSize);
}

void BandVocoder::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   
   mInputSlider->Draw();
   mCarrierSlider->Draw();
   mVolumeSlider->Draw();
   mDryWetSlider->Draw();
   mFMinSlider->Draw();
   mFMaxSlider->Draw();
   mQSlider->Draw();
   mNumBandsSlider->Draw();
   mRingTimeSlider->Draw();
   mMaxBandSlider->Draw();
   
   ofSetColor(0,255,0);
   for (int i=0; i<mNumBands; ++i)
   {
      ofLine(i*3,0,i*3,-mPeaks[i].GetPeak()*200);
   }
}

void BandVocoder::CalcFilters()
{
   for (int i=0; i<mNumBands; ++i)
   {
      float a = float(i)/mNumBands;
      //float f = Interp(a, mFreqMin, mFreqMax);
      float f = mFreqMin * powf(mFreqMax/mFreqMin, a);
      
      if (i==0)
      {
         mBiquadCarrier[i].SetFilterType(kFilterType_Lowpass);
         mBiquadOut[i].SetFilterType(kFilterType_Lowpass);
      }
      else if (i == mNumBands-1)
      {
         mBiquadCarrier[i].SetFilterType(kFilterType_Highpass);
         mBiquadOut[i].SetFilterType(kFilterType_Highpass);
      }
      else
      {
         mBiquadCarrier[i].SetFilterType(kFilterType_Bandpass);
         mBiquadOut[i].SetFilterType(kFilterType_Bandpass);
      }
      
      mBiquadCarrier[i].SetFilterParams(f, mQ);
      mBiquadOut[i].SetFilterParams(f, mQ);
   }
}

void BandVocoder::CheckboxUpdated(Checkbox* checkbox)
{
}

void BandVocoder::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mNumBandsSlider)
   {
      CalcFilters();
   }
}

void BandVocoder::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mFMinSlider || slider == mFMaxSlider || slider == mQSlider)
   {
      CalcFilters();
   }
   if (slider == mRingTimeSlider)
   {
      for (int i=0; i<VOCODER_MAX_BANDS; ++i)
      {
         mPeaks[i].SetDecayTime(mRingTime);
         mOutputPeaks[i].SetDecayTime(mRingTime);
      }
   }
   if (slider == mMaxBandSlider)
   {
      for (int i=0; i<VOCODER_MAX_BANDS; ++i)
      {
         mPeaks[i].SetLimit(mMaxBand);
         mOutputPeaks[i].SetLimit(mMaxBand);
      }
   }
}

void BandVocoder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void BandVocoder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}


