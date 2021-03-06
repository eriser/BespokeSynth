//
//  LooperRecorder.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/9/12.
//
//

#include "LooperRecorder.h"
#include "Looper.h"
#include "SynthGlobals.h"
#include "Transport.h"
#include "Scale.h"
#include "Stutter.h"
#include "ModularSynth.h"
#include "ChaosEngine.h"
#include "Profiler.h"
#include "VinylTempoControl.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"

LooperRecorder::LooperRecorder()
: mRecordBuffer(MAX_BUFFER_SIZE)
, mDrawDebug(false)
, mNumBars(1)
, mNumBarsSelector(NULL)
, mDebugCheckbox(NULL)
, mSpeed(1.0f)
, mBaseTempo(120)
, mResampAndSetButton(NULL)
, mResampleButton(NULL)
, mMergeSource(NULL)
, mSwapSource(NULL)
, mCopySource(NULL)
, mCommitCount(0)
, mDoubleTempoButton(NULL)
, mHalfTempoButton(NULL)
, mShiftMeasureButton(NULL)
, mHalfShiftButton(NULL)
, mShiftDownbeatButton(NULL)
, mClearOverdubButton(NULL)
, mUnquietInputTime(-1)
, mOrigSpeedButton(NULL)
, mSnapPitchButton(NULL)
, mHeadphonesTarget(NULL)
, mOutputTarget(NULL)
, mCommitDelay(0)
, mCommitDelaySlider(NULL)
, mFreeRecording(false)
, mFreeRecordingCheckbox(NULL)
, mStartFreeRecordTime(0)
, mCancelFreeRecordButton(NULL)
, mCommitToLooper(NULL)
, mRecorderMode(kRecorderMode_Record)
, mModeSelector(NULL)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
   
   mWriteBuffer = new float[mInputBufferSize];

   mQuietInputRamp.SetValue(1);
}

void LooperRecorder::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mNumBarsSelector = new DropdownList(this,"length",97,3,&mNumBars);
   //mDebugCheckbox = new Checkbox(this,"db",100,168,&mDrawDebug);
   mResampAndSetButton = new ClickButton(this,"resample & set key", -1, -1);
   mResampleButton = new ClickButton(this,"resample", -1, -1);
   mOrigSpeedButton = new ClickButton(this,"orig speed", 3, 57);
   mDoubleTempoButton = new ClickButton(this,"2xtempo",-1,-1);
   mHalfTempoButton = new ClickButton(this,".5tempo",-1,-1);
   mShiftMeasureButton = new ClickButton(this,"shift",-1,-1);
   mHalfShiftButton = new ClickButton(this,"half",-1,-1);
   mShiftDownbeatButton = new ClickButton(this,"downbeat",-1,-1);
   mModeSelector = new RadioButton(this,"mode",162,3,((int*)(&mRecorderMode)));
   mClearOverdubButton = new ClickButton(this,"clear",-1,-1);
   mSnapPitchButton = new ClickButton(this,"snap to pitch",-1,-1);
   mCommitDelaySlider = new FloatSlider(this,"delay",-1,-1,70,15,&mCommitDelay,0,1);
   mFreeRecordingCheckbox = new Checkbox(this,"free rec",97,84,&mFreeRecording);
   mCancelFreeRecordButton = new ClickButton(this,"cancel",-1,-1);
   
   mNumBarsSelector->AddLabel(" 1 ",1);
   mNumBarsSelector->AddLabel(" 2 ",2);
   mNumBarsSelector->AddLabel(" 3 ",3);
   mNumBarsSelector->AddLabel(" 4 ",4);
   mNumBarsSelector->AddLabel(" 6 ",6);
   mNumBarsSelector->AddLabel(" 8 ",8);
   mNumBarsSelector->AddLabel("12 ",12);
   
   mModeSelector->AddLabel("record", kRecorderMode_Record);
   mModeSelector->AddLabel("overdub", kRecorderMode_Overdub);
   mModeSelector->AddLabel("loop", kRecorderMode_Loop);
   
   mDoubleTempoButton->PositionTo(mNumBarsSelector, kAnchorDirection_Below);
   mHalfTempoButton->PositionTo(mDoubleTempoButton, kAnchorDirection_Below);
   mShiftMeasureButton->PositionTo(mHalfTempoButton, kAnchorDirection_Below);
   mHalfShiftButton->PositionTo(mShiftMeasureButton, kAnchorDirection_Right);
   mShiftDownbeatButton->PositionTo(mShiftMeasureButton, kAnchorDirection_Below);
   
   mCommitDelaySlider->PositionTo(mModeSelector, kAnchorDirection_Below);
   mClearOverdubButton->PositionTo(mCommitDelaySlider, kAnchorDirection_Below);
   
   mSnapPitchButton->PositionTo(mOrigSpeedButton, kAnchorDirection_Below);
   mResampleButton->PositionTo(mSnapPitchButton, kAnchorDirection_Below);
   mResampAndSetButton->PositionTo(mResampleButton, kAnchorDirection_Below);
   
   mCancelFreeRecordButton->PositionTo(mFreeRecordingCheckbox, kAnchorDirection_Right);
   
   SyncCablesToLoopers();
}

LooperRecorder::~LooperRecorder()
{
   delete[] mInputBuffer;
   delete[] mWriteBuffer;
}

float* LooperRecorder::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void LooperRecorder::Init()
{
   IDrawableModule::Init();
   mBaseTempo = TheTransport->GetTempo();
}

void LooperRecorder::Process(double time)
{
   Profiler profiler("LooperRecorder");

   if (!mEnabled || GetTarget() == NULL)
      return;

   ComputeSliders(0);
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);

   if (mCommitToLooper)
   {
      SyncLoopLengths();
      mCommitToLooper->SetNumBars(mNumBars);
      mCommitToLooper->Commit();
      
      mRecorderMode = kRecorderMode_Record;
      mQuietInputRamp.Start(0, 10);
      mUnquietInputTime = gTime + 1000; //no input for 1 second
      mCommitToLooper = NULL;
   }
   
   if (mUnquietInputTime != -1 && time >= mUnquietInputTime)
   {
      mQuietInputRamp.Start(time, 1, time+10);
      mUnquietInputTime = -1;
   }
   
   UpdateSpeed();
   
   bool acceptInput = (mRecorderMode == kRecorderMode_Record || mRecorderMode == kRecorderMode_Overdub);
   bool loop = (mRecorderMode == kRecorderMode_Loop || mRecorderMode == kRecorderMode_Overdub);

   
   for (int i=0; i<bufferSize; ++i)
   {
      if (acceptInput)
      {
         mInputBuffer[i] *= mQuietInputRamp.Value(time);
         time += gInvSampleRateMs;
         mWriteBuffer[i] = mInputBuffer[i];
      }
      else
      {
         mWriteBuffer[i] = 0;
      }
   }

   if (loop)
   {
      int delaySamps = TheTransport->GetDuration(kInterval_1n) * NumBars() / gInvSampleRateMs;
      delaySamps = MIN(delaySamps, MAX_BUFFER_SIZE-1);
      for (int i=0; i<bufferSize; ++i)
      {
         float sample = mRecordBuffer.GetSample(delaySamps - i);
         mWriteBuffer[i] += sample;
         mInputBuffer[i] += sample;
      }
   }
   
   mRecordBuffer.WriteChunk(mWriteBuffer, bufferSize);
   
   Add(out, mInputBuffer, bufferSize);
   
   GetVizBuffer()->WriteChunk(mInputBuffer,bufferSize);
   
   Clear(mInputBuffer, mInputBufferSize);
}

void LooperRecorder::DrawCircleHash(ofVec2f center, float progress, float width, float innerRadius, float outerRadius)
{
   ofSetLineWidth(width);
   float sinTheta = sin(progress * TWO_PI);
   float cosTheta = cos(progress * TWO_PI);
   ofLine(innerRadius * sinTheta + center.x, innerRadius * -cosTheta + center.y,
          outerRadius * sinTheta + center.x, outerRadius * -cosTheta + center.y);
}

void LooperRecorder::SyncCablesToLoopers()
{
   int numLoopers = MAX(4, mLoopers.size());
   
   if (!mLoopers.empty() && mLoopers[mLoopers.size()-1] != NULL)
      ++numLoopers; //add an extra cable for an additional looper
   
   numLoopers = MIN(9, numLoopers);
   
   if (numLoopers == mLooperPatchCables.size())
      return;  //nothing to do
   
   if (numLoopers > mLooperPatchCables.size())
   {
      int oldSize = mLooperPatchCables.size();
      mLooperPatchCables.resize(numLoopers);
      for (int i=0; i<oldSize; ++i)
      {
         mLooperPatchCables[i]->SetTarget(mLoopers[i]);
      }
      for (int i=oldSize; i<mLooperPatchCables.size(); ++i)
      {
         mLooperPatchCables[i] = new PatchCableSource(this, kConnectionType_Special);
         mLooperPatchCables[i]->AddTypeFilter("looper");
         Looper* looper = NULL;
         if (i < mLoopers.size())
            looper = mLoopers[i];
         mLooperPatchCables[i]->SetTarget(looper);
         mLooperPatchCables[i]->SetManualPosition(130+i*12, 117);
         AddPatchCableSource(mLooperPatchCables[i]);
      }
   }
   else
   {
      for (int i=numLoopers; i<mLooperPatchCables.size(); ++i)
         RemovePatchCableSource(mLooperPatchCables[i]);
      mLooperPatchCables.resize(numLoopers);
   }
}

void LooperRecorder::PreRepatch(PatchCableSource* cableSource)
{
   for (int i=0; i<mLooperPatchCables.size(); ++i)
   {
      if (cableSource == mLooperPatchCables[i])
      {
         Looper* looper = NULL;
         if (i < mLoopers.size())
            looper = mLoopers[i];
         if (looper)
            looper->SetRecorder(NULL);
      }
   }
}

void LooperRecorder::PostRepatch(PatchCableSource* cableSource)
{
   mLoopers.resize(mLooperPatchCables.size());
   for (int i=0; i<mLooperPatchCables.size(); ++i)
   {
      if (cableSource == mLooperPatchCables[i])
      {
         mLoopers[i] = dynamic_cast<Looper*>(mLooperPatchCables[i]->GetTarget());
         if (mLoopers[i])
            mLoopers[i]->SetRecorder(this);
      }
   }
   
   SyncCablesToLoopers();
}

void LooperRecorder::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mResampleButton->Draw();
   mResampAndSetButton->Draw();
   mDoubleTempoButton->Draw();
   mHalfTempoButton->Draw();
   mShiftMeasureButton->Draw();
   mShiftDownbeatButton->Draw();
   mModeSelector->Draw();
   mClearOverdubButton->Draw();
   mNumBarsSelector->Draw();
   mOrigSpeedButton->Draw();
   mSnapPitchButton->Draw();
   mHalfShiftButton->Draw();
   mCommitDelaySlider->Draw();
   mFreeRecordingCheckbox->Draw();
   mCancelFreeRecordButton->Draw();

   if (mSpeed != 1)
   {
      float rootPitch = AdjustedRootForSpeed();
      int pitch = int(rootPitch + .5f);
      int cents = (rootPitch - pitch) * 100;
      
      string speed = "speed "+ofToString(mSpeed,2)+", ";
      
      string detune = NoteName(pitch);
      if (cents > 0)
         detune += " +" + ofToString(cents) + " cents";
      if (cents < 0)
         detune += " -" + ofToString(-cents) + " cents";
      
      DrawText(speed + detune,5,118);
   }

   mRecordBuffer.Draw(3,3,90,50,gSampleRate);
   ofPushStyle();
   ofVec2f center(48,28);
   float radius = 25;
   ofSetColor(255,255,255,100*gModuleDrawAlpha);
   DrawCircleHash(center, (TheTransport->GetMeasurePos() + TheTransport->GetMeasure() % 8) / 8, 1, radius * .9f, radius);
   DrawCircleHash(center, (TheTransport->GetMeasurePos() + TheTransport->GetMeasure() % mNumBars) / mNumBars, 3, radius * .7f, radius);
   for (int i=0; i<mNumBars; ++i)
      DrawCircleHash(center, float(i)/mNumBars, 1, radius * .8f, radius);
   ofPopStyle();
   
   if (mDrawDebug)
      mRecordBuffer.Draw(0,162,800,100);
   
   DrawText("loopers:",125,109);
}

void LooperRecorder::RemoveLooper(Looper* looper)
{
   for (int i=0; i<mLoopers.size(); ++i)
   {
      if (mLoopers[i] == looper)
         mLoopers[i] = NULL;
   }
}

float LooperRecorder::AdjustedRootForSpeed()
{
   float rootFreq = TheScale->PitchToFreq(TheScale->ScaleRoot()+24);
   rootFreq *= mSpeed;
   return TheScale->FreqToPitch(rootFreq);
}

void LooperRecorder::SnapToClosestPitch()
{
   float currentPitch = AdjustedRootForSpeed();
   float desiredPitch = int(currentPitch + .5f);

   float currentFreq = TheScale->PitchToFreq(currentPitch);
   float desiredFreq = TheScale->PitchToFreq(desiredPitch);

   TheTransport->SetTempo(TheTransport->GetTempo() * desiredFreq/currentFreq);
}

void LooperRecorder::KeyPressed(int key)
{
   IDrawableModule::KeyPressed(key);
   if (GetKeyModifiers() == kModifier_Control)
   {
      if (key >= '1' && key <= '4')
      {
         int looper = key - '1';
         if (looper < mLoopers.size())
            Commit(mLoopers[key - '1']);
      }
      if (key >= '5' && key <= '8')
      {
         int idx = key - '5';
         SetNumBars(powf(2,idx));
      }
   }

   if (key == 'k')   //resample and set key
      Resample(true);
   if (key == 'l')   //resample (without setting key)
      Resample(false);
}

void LooperRecorder::Resample(bool setKey)
{
   if (setKey)
   {
      SnapToClosestPitch();
      TheScale->SetRoot(int(AdjustedRootForSpeed()+.5f));
   }
   
   SyncLoopLengths();
}

void LooperRecorder::UpdateSpeed()
{
   float newSpeed = TheTransport->GetTempo() / mBaseTempo;
   if (mSpeed != newSpeed)
   {
      mSpeed = newSpeed;
      if (mSpeed == 0)
         mSpeed = .001f;
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->SetSpeed(mSpeed);
      }
   }
}

void LooperRecorder::SyncLoopLengths()
{
   if (mSpeed <= 0)  //avoid crashing
      return;

   for (int i=0; i<mLoopers.size(); ++i)
   {
      if (mLoopers[i])
      {
         if (mSpeed != 1)
            mLoopers[i]->ResampleForNewSpeed();
         mLoopers[i]->RecalcLoopLength();
      }
   }

   mBaseTempo = TheTransport->GetTempo();
   
   if (TheVinylTempoControl)
      TheVinylTempoControl->Stop();
}

void LooperRecorder::Commit(Looper* looper)
{
   mCommitToLooper = looper;
}

void LooperRecorder::RequestMerge(Looper* looper)
{
   if (mMergeSource == NULL)
   {
      mMergeSource = looper;
   }
   else if (mMergeSource == looper)
   {
      mMergeSource = NULL;
   }
   else
   {
      looper->MergeIn(mMergeSource);
      mMergeSource = NULL;
   }
}

void LooperRecorder::RequestSwap(Looper *looper)
{
   if (mSwapSource == NULL)
   {
      mSwapSource = looper;
   }
   else if (mSwapSource == looper)
   {
      mSwapSource = NULL;
   }
   else
   {
      looper->SwapBuffers(mSwapSource);
      mSwapSource = NULL;
   }
}

void LooperRecorder::RequestCopy(Looper *looper)
{
   if (mCopySource == NULL)
   {
      mCopySource = looper;
   }
   else if (mCopySource == looper)
   {
      mCopySource = NULL;
   }
   else
   {
      looper->CopyBuffer(mCopySource);
      mCopySource = NULL;
   }
}

void LooperRecorder::ResetSpeed()
{
   mBaseTempo = TheTransport->GetTempo();
   
   if (TheVinylTempoControl)
      TheVinylTempoControl->Stop();
}

void LooperRecorder::StartFreeRecord()
{
   if (mFreeRecording)
      return;
   
   mFreeRecording = true;
   mStartFreeRecordTime = gTime;
}

void LooperRecorder::EndFreeRecord()
{
   if (!mFreeRecording)
      return;
   
   float recordedTime = gTime - mStartFreeRecordTime;
   int beats = mNumBars * TheTransport->GetTimeSigTop();
   float minutes = recordedTime / 1000.0f / 60.0f;
   TheTransport->SetTempo(beats/minutes);
   TheTransport->SetDownbeat();
   mRecorderMode = kRecorderMode_Loop;
   mFreeRecording = false;
}

void LooperRecorder::CancelFreeRecord()
{
   mFreeRecording = false;
   mStartFreeRecordTime = 0;
}

void LooperRecorder::ButtonClicked(ClickButton* button)
{
   if (button == mResampleButton)
      SyncLoopLengths();

   if (button == mResampAndSetButton)
   {
      SnapToClosestPitch();
      TheScale->SetRoot(int(AdjustedRootForSpeed()+.5f));
      SyncLoopLengths();
   }

   if (button == mDoubleTempoButton)
   {
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->DoubleNumBars();
      }
      TheTransport->SetTempo(TheTransport->GetTempo()*2);
      mBaseTempo = TheTransport->GetTempo();
      float pos = TheTransport->GetMeasurePos() + (TheTransport->GetMeasure()%8);
      int count = TheTransport->GetMeasure() - int(pos);
      pos *= 2;
      count += int(pos);
      pos -= int(pos);
      TheTransport->SetMeasurePos(pos);
      TheTransport->SetMeasure(count);
   }

   if (button == mHalfTempoButton)
   {
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->HalveNumBars();
      }
      TheTransport->SetTempo(TheTransport->GetTempo()/2);
      mBaseTempo = TheTransport->GetTempo();
      float pos = TheTransport->GetMeasurePos() + (TheTransport->GetMeasure()%8);
      int count = TheTransport->GetMeasure() - int(pos);
      pos /= 2;
      count += int(pos);
      pos -= int(pos);
      TheTransport->SetMeasurePos(pos);
      TheTransport->SetMeasure(count);
   }

   if (button == mShiftMeasureButton)
   {
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->ShiftMeasure();
      }
      int newMeasure = TheTransport->GetMeasure()-1;
      if (newMeasure < 0)
         newMeasure = 7;
      TheTransport->SetMeasure(newMeasure);
   }

   if (button == mHalfShiftButton)
   {
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->HalfShift();
      }
      int newMeasure = int(TheTransport->GetMeasure()+TheTransport->GetMeasurePos()-.5f);
      if (newMeasure < 0)
         newMeasure = 7;
      TheTransport->SetMeasure(newMeasure);
      float newMeasurePos = TheTransport->GetMeasurePos() - .5f;
      FloatWrap(newMeasurePos,1);
      TheTransport->SetMeasurePos(newMeasurePos);
   }

   if (button == mShiftDownbeatButton)
   {
      TheTransport->SetMeasure(TheTransport->GetMeasure()/8 * 8);   //align to 8 bars
      TheTransport->SetDownbeat();
      for (int i=0; i<mLoopers.size(); ++i)
      {
         if (mLoopers[i])
            mLoopers[i]->ShiftDownbeat();
      }
      if (TheChaosEngine)
         TheChaosEngine->RestartProgression();
   }

   if (button == mClearOverdubButton)
      mRecordBuffer.ClearBuffer();

   if (button == mOrigSpeedButton)
   {
      TheTransport->SetTempo(mBaseTempo);
   }

   if (button == mSnapPitchButton)
      SnapToClosestPitch();
   
   if (button == mCancelFreeRecordButton)
      CancelFreeRecord();
}

void LooperRecorder::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mFreeRecordingCheckbox)
   {
      bool freeRec = mFreeRecording;
      mFreeRecording = !mFreeRecording; //flip back so these methods won't be ignored
      if (freeRec)
         StartFreeRecord();
      else
         EndFreeRecord();
   }
}

void LooperRecorder::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void LooperRecorder::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
}

void LooperRecorder::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mNumBarsSelector)
   {
      mNumBars = MAX(1, mNumBars);
   }
}

void LooperRecorder::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void LooperRecorder::Poll()
{
}

void LooperRecorder::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("headphonestarget", moduleInfo, "", FillDropdown<IAudioReceiver*>);
   mModuleSaveData.LoadString("outputtarget", moduleInfo, "", FillDropdown<IAudioReceiver*>);
   
   if (!moduleInfo["loopers"].isNull())
   {
      mLoopers.resize(moduleInfo["loopers"].size());
      for (int i=0; i<moduleInfo["loopers"].size(); ++i)
         mLoopers[i] = dynamic_cast<Looper*>(TheSynth->FindModule(moduleInfo["loopers"][i].asString()));
      SyncCablesToLoopers();
   }

   SetUpFromSaveData();
}

void LooperRecorder::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["loopers"].resize(mLoopers.size());
   for (int i=0; i<mLoopers.size(); ++i)
   {
      string name;
      if (mLoopers[i])
         name = mLoopers[i]->Name();
      moduleInfo["loopers"][i] = name;
   }
}

void LooperRecorder::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetHeadphonesTarget(TheSynth->FindAudioReceiver(mModuleSaveData.GetString("headphonestarget")));
   SetOutputTarget(TheSynth->FindAudioReceiver(mModuleSaveData.GetString("outputtarget")));
}

namespace
{
   const int kSaveStateRev = 0;
}

void LooperRecorder::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mBaseTempo;
   out << mSpeed;
   mRecordBuffer.SaveState(out);
}

void LooperRecorder::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> mBaseTempo;
   in >> mSpeed;
   mRecordBuffer.LoadState(in);
}


