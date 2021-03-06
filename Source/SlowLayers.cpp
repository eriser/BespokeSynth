//
//  SlowLayers.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/13/15.
//
//

#include "SlowLayers.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "Profiler.h"

SlowLayers::SlowLayers()
: mBuffer(NULL)
, mLoopPos(0)
, mNumBars(1)
, mVol(1)
, mSmoothedVol(1)
, mVolSlider(NULL)
, mNumBarsSelector(NULL)
, mFeedInSlider(NULL)
, mFeedIn(1)
{
   //TODO(Ryan) buffer sizes
   mBuffer = new float[MAX_BUFFER_SIZE];
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   ::Clear(mInputBuffer, mInputBufferSize);
   Clear();
}

void SlowLayers::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mClearButton = new ClickButton(this,"clear", 147, 102);
   mVolSlider = new FloatSlider(this,"volume", 4, 102, 110, 15, &mVol, 0, 2);
   mNumBarsSelector = new DropdownList(this,"num bars",165, 4, &mNumBars);
   mFeedInSlider = new FloatSlider(this,"feed in", 4, 120, 110, 15, &mFeedIn, 0, 1);
   
   mNumBarsSelector->AddLabel(" 1 ",1);
   mNumBarsSelector->AddLabel(" 2 ",2);
   mNumBarsSelector->AddLabel(" 3 ",3);
   mNumBarsSelector->AddLabel(" 4 ",4);
   mNumBarsSelector->AddLabel(" 6 ",6);
   mNumBarsSelector->AddLabel(" 8 ",8);
   mNumBarsSelector->AddLabel("12 ",12);
}

SlowLayers::~SlowLayers()
{
   delete[] mInputBuffer;
   delete[] mBuffer;
}

float* SlowLayers::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void SlowLayers::Process(double time)
{
   Profiler profiler("SlowLayers");
   
   if (!mEnabled || GetTarget() == NULL)
      return;
   
   ComputeSliders(0);
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   int loopLengthInSamples = LoopLength();
   
   int layers = 4;
   for (int i=0; i<bufferSize; ++i)
   {
      float smooth = .001f;
      mSmoothedVol = mSmoothedVol * (1-smooth) + mVol * smooth;
      float volSq = mSmoothedVol * mSmoothedVol;
      
      float measurePos = TheTransport->GetMeasurePos(i) + TheTransport->GetMeasure();
      FloatWrap(measurePos, 1 << layers * mNumBars);
      int offset = measurePos * loopLengthInSamples;
      
      mBuffer[offset % loopLengthInSamples] += mInputBuffer[i] * mFeedIn;
      
      float output = (1-mFeedIn)*mInputBuffer[i];
      for (int i=0; i<layers; ++i)
         output += GetInterpolatedSample(offset/float(1<<i), mBuffer, loopLengthInSamples);
      
      output *= volSq;
      
      out[i] += output;
   }
   
   Add(out, mInputBuffer, bufferSize);
   
   GetVizBuffer()->WriteChunk(mInputBuffer, bufferSize);
   
   ::Clear(mInputBuffer, mInputBufferSize);
}

int SlowLayers::LoopLength() const
{
   return TheTransport->GetDuration(kInterval_1n) * mNumBars * gSampleRate / 1000;
}

void SlowLayers::DrawModule()
{

   
   if (Minimized() || IsVisible() == false)
      return;
   
   ofPushMatrix();
   
   ofTranslate(BUFFER_X,BUFFER_Y);
   
   DrawAudioBuffer(BUFFER_W, BUFFER_H, mBuffer, 0, LoopLength(), TheTransport->GetMeasurePos()*LoopLength(), mVol);
   ofSetColor(255,255,0,gModuleDrawAlpha);
   for (int i=1; i<mNumBars; ++i)
   {
      float x = BUFFER_W/mNumBars * i;
      ofLine(x,BUFFER_H/2-5,x,BUFFER_H/2+5);
   }
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   ofPopMatrix();
   
   mClearButton->Draw();
   mNumBarsSelector->Draw();
   mVolSlider->Draw();
   mFeedInSlider->Draw();
}

void SlowLayers::Clear()
{
   bzero(mBuffer, sizeof(float) * MAX_BUFFER_SIZE);
}

void SlowLayers::SetNumBars(int numBars)
{
   mNumBars = numBars;
}

void SlowLayers::GetModuleDimensions(int& width, int& height)
{
   width = 197;
   height = 155;
}

void SlowLayers::ButtonClicked(ClickButton* button)
{
   if (button == mClearButton)
      ::Clear(mBuffer, MAX_BUFFER_SIZE);
}

void SlowLayers::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SlowLayers::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void SlowLayers::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void SlowLayers::CheckboxUpdated(Checkbox* checkbox)
{
}

void SlowLayers::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void SlowLayers::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

