//
//  NoteStepSequencer.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/3/13.
//
//

#include "NoteStepSequencer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "NoteStepSequencer.h"
#include "LaunchpadInterpreter.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"

NoteStepSequencer::NoteStepSequencer()
: mInterval(kInterval_8n)
, mArpIndex(-1)
, mIntervalSelector(NULL)
, mNumSteps(8)
, mLength(8)
, mLengthSlider(NULL)
, mArpStep(1)
, mArpPingPongDirection(1)
, mArpStepSlider(NULL)
, mTimeMode(kTimeMode_Synced)
, mTimeModeSelector(NULL)
, mGrid(NULL)
, mLastPitch(-1)
, mLastVel(0)
, mOctave(3)
, mOctaveSlider(NULL)
, mHold(false)
, mHoldCheckbox(NULL)
, mNoteMode(kNoteMode_Scale)
, mNoteModeSelector(NULL)
, mWaitingForDownbeat(false)
, mNoteRange(15)
, mFreeTimeStep(30)
, mFreeTimeSlider(NULL)
, mFreeTimeCounter(0)
, mSetLength(false)
, mController(NULL)
, mShiftBackButton(NULL)
, mShiftForwardButton(NULL)
, mOffset(0)
, mOffsetSlider(NULL)
, mLastNoteLength(1)
, mLastNoteStartTime(0)
, mAlreadyDidNoteOff(false)
, mRandomizePitchButton(NULL)
, mRandomizeLengthButton(NULL)
, mRandomizeVelocityButton(NULL)
{
   SetEnabled(false);
   
   TheTransport->AddListener(this, mInterval);
   TheTransport->AddAudioPoller(this);
   
   for (int i=0;i<NSS_MAX_STEPS;++i)
   {
      mTones[i] = 7;
      mVels[i] = 127;
      mNoteLengths[i] = 1;
   }
   
   SetIsNoteOrigin(true);
}

void NoteStepSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",75,2,(int*)(&mInterval));
   mLengthSlider = new IntSlider(this,"length",77,20,98,15,&mLength,1,mNumSteps);
   mArpStepSlider = new IntSlider(this,"step",121,2,40,15,&mArpStep,-3,3);
   mTimeModeSelector = new DropdownList(this,"timemode",5,2,(int*)(&mTimeMode));
   mGrid = new Grid(5,55,200,80,8,24);
   mVelocityGrid = new Grid(5,117,200,15,8,1);
   mOctaveSlider = new IntSlider(this,"octave",166,2,53,15,&mOctave,0,6);
   mHoldCheckbox = new Checkbox(this,"hold",180,20,&mHold);
   mNoteModeSelector = new DropdownList(this,"notemode",5,20,(int*)(&mNoteMode));
   mFreeTimeSlider = new FloatSlider(this,"t",75,2,44,15,&mFreeTimeStep,0,1000,0);
   mShiftBackButton = new ClickButton(this,"<",130,2);
   mShiftForwardButton = new ClickButton(this,">",145,2);
   mOffsetSlider = new FloatSlider(this,"offset",5,38,80,15,&mOffset,-1,1);
   mRandomizePitchButton = new ClickButton(this,"pitch",140,38);
   mRandomizeLengthButton = new ClickButton(this,"len",-1,-1);
   mRandomizeVelocityButton = new ClickButton(this,"vel",-1,-1);
   
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mIntervalSelector->AddLabel("kick", kInterval_Kick);
   mIntervalSelector->AddLabel("snare", kInterval_Snare);
   mIntervalSelector->AddLabel("hat", kInterval_Hat);
   
   mNoteModeSelector->AddLabel("scale", kNoteMode_Scale);
   mNoteModeSelector->AddLabel("chromatic", kNoteMode_Chromatic);
   mNoteModeSelector->AddLabel("5ths", kNoteMode_Fifths);
   
   mGrid->SetSingleColumnMode(true);
   mGrid->SetFlip(true);
   mGrid->SetListener(this);
   mGrid->SetGridMode(Grid::kHorislider);
   mGrid->SetClickClearsToZero(false);
   mVelocityGrid->SetGridMode(Grid::kMultislider);
   mVelocityGrid->SetClickClearsToZero(false);
   mVelocityGrid->SetListener(this);
   
   mTimeModeSelector->AddLabel("synced", kTimeMode_Synced);
   mTimeModeSelector->AddLabel("downbeat", kTimeMode_Downbeat);
   mTimeModeSelector->AddLabel("step", kTimeMode_Step);
   mTimeModeSelector->AddLabel("free", kTimeMode_Free);
   
   mFreeTimeSlider->SetMode(FloatSlider::kSquare);
   
   mArpStepSlider->SetShowing(mTimeMode != kTimeMode_Synced);
   mFreeTimeSlider->SetShowing(mTimeMode == kTimeMode_Free);
   
   mRandomizeLengthButton->PositionTo(mRandomizePitchButton, kAnchorDirection_Right);
   mRandomizeVelocityButton->PositionTo(mRandomizeLengthButton, kAnchorDirection_Right);
}

NoteStepSequencer::~NoteStepSequencer()
{
   TheTransport->RemoveListener(this);
   TheTransport->RemoveAudioPoller(this);
}

void NoteStepSequencer::SetMidiController(string name)
{
   if (mController)
      mController->RemoveListener(this);
   
   mController = TheSynth->FindMidiController(name);
   if (mController)
      mController->AddListener(this, 0);
   
   UpdateLights();
}

void NoteStepSequencer::Init()
{
   IDrawableModule::Init();
   
   SyncGridToSeq();
}

void NoteStepSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   ofSetColor(255,255,255,gModuleDrawAlpha);
   
   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mArpStepSlider->Draw();
   mTimeModeSelector->Draw();
   mOctaveSlider->Draw();
   mHoldCheckbox->Draw();
   mNoteModeSelector->Draw();
   mFreeTimeSlider->Draw();
   mShiftBackButton->Draw();
   mShiftForwardButton->Draw();
   mOffsetSlider->Draw();
   mRandomizePitchButton->Draw();
   mRandomizeLengthButton->Draw();
   mRandomizeVelocityButton->Draw();
   
   mGrid->Draw();
   mVelocityGrid->Draw();
   
   DrawTextLeftJustify("random:", 138, 50);
   
   ofPushStyle();
   ofFill();
   int gridX, gridY;
   mGrid->GetPosition(gridX, gridY);
   for (int i=0;i<mNoteRange;++i)
   {
      if (GetPitch(i)%TheScale->GetTet() == TheScale->ScaleRoot()%TheScale->GetTet())
         ofSetColor(0,255,0,80);
      else if (GetPitch(i)%TheScale->GetTet() == (TheScale->ScaleRoot()+7)%TheScale->GetTet())
         ofSetColor(200,150,0,80);
      else if (mNoteMode == kNoteMode_Chromatic && TheScale->IsInScale(GetPitch(i)))
         ofSetColor(100,75,0,80);
      else
         continue;
      
      float boxHeight = (float(mGrid->GetHeight())/mNoteRange);
      float y = gridY + mGrid->GetHeight() - i*boxHeight;
      ofRect(gridX,y-boxHeight,mGrid->GetWidth(),boxHeight);
   }
   
   for (int i=0; i<mGrid->GetCols(); ++i)
   {
      if (mVels[i] == 0 || i >= mLength)
      {
         ofSetColor(0,0,0,100);
         ofFill();
         float boxWidth = (float(mGrid->GetWidth())/mGrid->GetCols());
         ofRect(gridX + boxWidth * i, gridY, boxWidth, mGrid->GetHeight());
      }
   }
   
   ofPopStyle();
}

void NoteStepSequencer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   mGrid->TestClick(x,y,right);
   mVelocityGrid->TestClick(x, y, right);
}

void NoteStepSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
   mVelocityGrid->MouseReleased();
}

bool NoteStepSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mGrid->NotifyMouseMoved(x,y);
   mVelocityGrid->NotifyMouseMoved(x, y);
   return false;
}

bool NoteStepSequencer::MouseScrolled(int x, int y, float scrollX, float scrollY)
{
   mGrid->NotifyMouseScrolled(x,y,scrollX,scrollY);
   mVelocityGrid->NotifyMouseScrolled(x,y,scrollX,scrollY);
   return false;
}

void NoteStepSequencer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
   {
      mNoteOutput.Flush();
      
      if (mEnabled && mTimeMode == kTimeMode_Downbeat)
         mWaitingForDownbeat = true;
   }
}

void NoteStepSequencer::GridUpdated(Grid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mGrid)
   {
      for (int i=0; i<mLength; ++i)
      {
         for (int j=0; j<mNoteRange; ++j)
         {
            float val = mGrid->GetVal(i,j);
            if (val > 0)
            {
               mTones[i] = j;
               mNoteLengths[i] = val;
               break;
            }
         }
      }
   }
   if (grid == mVelocityGrid)
   {
      for (int i=0; i<mLength; ++i)
         mVels[i] = mVelocityGrid->GetVal(i,0) * 127;
   }
}

int NoteStepSequencer::GetPitch(int tone)
{
   int numPitchesInScale = TheScale->NumPitchesInScale();
   switch (mNoteMode)
   {
      case kNoteMode_Scale:
         return TheScale->GetPitchFromTone(tone+mOctave*numPitchesInScale+TheScale->GetScaleDegree());
      case kNoteMode_Chromatic:
         return tone + mOctave * TheScale->GetTet() + TheScale->ScaleRoot();
      case kNoteMode_Fifths:
      {
         int oct = (tone/2)*numPitchesInScale;
         bool isFifth = tone%2 == 1;
         int fifths = oct;
         if (isFifth)
            fifths += 4;
         return TheScale->GetPitchFromTone(fifths+mOctave*numPitchesInScale+TheScale->GetScaleDegree());

      }
   }
   return tone;
}

void NoteStepSequencer::OnTransportAdvanced(float amount)
{
   Profiler profiler("NoteStepSequencer");
   
   ComputeSliders(0);
   
   if (mLastNoteLength < 1 && !mAlreadyDidNoteOff)
   {
      if (gTime > mLastNoteStartTime + TheTransport->GetDuration(mInterval) * mLastNoteLength)
      {
         PlayNoteOutput(gTime, mLastPitch, 0);
         mAlreadyDidNoteOff = true;
      }
   }
   
   if (mTimeMode == kTimeMode_Free)
   {
      float ms = amount * TheTransport->MsPerBar();
      mFreeTimeCounter += ms;
      if (mFreeTimeCounter > mFreeTimeStep)
      {
         mFreeTimeCounter -= mFreeTimeStep;
         OnTimeEvent(0);
      }
   }
}

void NoteStepSequencer::OnTimeEvent(int samplesTo)
{
   if (!mEnabled || mHold)
      return;
   
   if (mArpStep != 0)
   {
      mArpIndex += mArpStep;
      while (mArpIndex >= mLength)
         mArpIndex -= mLength;
      while (mArpIndex <0)
         mArpIndex += mLength;
   }
   else //pingpong
   {
      assert (mArpPingPongDirection == 1 || mArpPingPongDirection == -1);
      mArpIndex += mArpPingPongDirection;
      if (mLength >= 2)
      {
         if (mArpIndex < 0)
         {
            mArpIndex = 1;
            mArpPingPongDirection = 1;
         }
         if (mArpIndex > mLength - 1)
         {
            mArpIndex = mLength - 2;
            mArpPingPongDirection = -1;
         }
      }
      else
      {
         mArpIndex = ofClamp(mArpIndex,0,mLength-1);
      }
   }
   
   float offsetMs = (-mOffset/TheTransport->CountInStandardMeasure(mInterval))*TheTransport->MsPerBar();
   
   bool isDownbeat = TheTransport->GetQuantized(offsetMs, mInterval) == 0;
   
   if (mTimeMode == kTimeMode_Downbeat && isDownbeat)
      mArpIndex = 0;
   
   if (mWaitingForDownbeat && isDownbeat)
      mWaitingForDownbeat = false;
   
   if (mWaitingForDownbeat && mTimeMode == kTimeMode_Downbeat)
      return;
   
   if (mTimeMode == kTimeMode_Synced)
   {
      int stepsPerMeasure = TheTransport->CountInStandardMeasure(mInterval) * TheTransport->GetTimeSigTop()/TheTransport->GetTimeSigBottom();
      int numMeasures = ceil(float(mLength) / stepsPerMeasure);
      int measure = TheTransport->GetMeasure() % numMeasures;
      int step = (TheTransport->GetQuantized(offsetMs, mInterval) + measure * stepsPerMeasure) % mLength;
      mArpIndex = step;
   }
   
   int offPitch = -1;
   if (mLastPitch >= 0 && !mAlreadyDidNoteOff)
   {
      offPitch = mLastPitch;
   }
   
   int current = mTones[mArpIndex];
   if (mVels[mArpIndex] <= 1)
   {
      mLastPitch = -1;
      mLastVel = 0;
   }
   else
   {
      int outPitch = GetPitch(current);
      
      if (mLastPitch == outPitch && mVels[mArpIndex] > 1 && mVels[mArpIndex] <= mLastVel && !mAlreadyDidNoteOff)
      {
         //if it's the same note and repeats are treated as holds, clear note off
         offPitch = -1;
         mLastVel = mVels[mArpIndex];
         mLastNoteLength = mNoteLengths[mArpIndex];
         mLastNoteStartTime = gTime;
         mAlreadyDidNoteOff = false;
      }
      else
      {
         if (mLastPitch == outPitch && !mAlreadyDidNoteOff)   //same note, play noteoff first
         {
            PlayNoteOutput(gTime, mLastPitch, 0, -1);
            offPitch = -1;
         }
         if (mVels[mArpIndex] > 1)
         {
            PlayNoteOutput(gTime, outPitch, mVels[mArpIndex], -1);
            mLastPitch = outPitch;
            mLastVel = mVels[mArpIndex];
            mLastNoteLength = mNoteLengths[mArpIndex];
            mLastNoteStartTime = gTime;
            mAlreadyDidNoteOff = false;
         }
      }
   }
   
   if (offPitch != -1)
   {
      PlayNoteOutput(gTime, offPitch, 0, -1);
      if (offPitch == mLastPitch)
      {
         mLastPitch = -1;
         mLastVel = 0;
      }
   }
   
   mGrid->SetHighlightCol(mArpIndex);
   mVelocityGrid->SetHighlightCol(mArpIndex);
   
   UpdateLights();
}

void NoteStepSequencer::UpdateLights()
{
   if (mController)
   {
      for (int i=0;i<NSS_MAX_STEPS;++i)
      {
         int button = StepToButton(i);
         int color = 0;
         if (i<mLength)
         {
            if (i == mArpIndex)
            {
               color = LaunchpadInterpreter::LaunchpadColor(0,3);
            }
            else if (mVels[i] > 1)
            {
               int level = (mVels[i] / 50) + 1;
               color = LaunchpadInterpreter::LaunchpadColor(level,level);
            }
            else
            {
               color = LaunchpadInterpreter::LaunchpadColor(1,0);
            }
         }
         mController->SendNote(0, button, color);
      }
   }
}

int NoteStepSequencer::ButtonToStep(int button)
{
   if ((button >= 9 && button <= 12) ||
       (button >= 25 && button <= 28))
   {
      return button <= 12 ? (button - 9) : (button - 21);
   }
   return -1;
}

int NoteStepSequencer::StepToButton(int step)
{
   return step < 4 ? (step + 9) : (step + 21);
}

namespace
{
   const float extraW = 10;
   const float extraH = 75;
}

void NoteStepSequencer::GetModuleDimensions(int& width, int& height)
{
   width = mGrid->GetWidth() + extraW;
   height = mGrid->GetHeight() + extraH;
}

void NoteStepSequencer::Resize(float w, float h)
{
   mGrid->SetDimensions(MAX(w - extraW, 210), MAX(h - extraH, 80));
   UpdateVelocityGridPos();
}

void NoteStepSequencer::UpdateVelocityGridPos()
{
   mVelocityGrid->SetDimensions(mGrid->GetWidth(), 15);
   int gridX,gridY;
   mGrid->GetPosition(gridX, gridY);
   mVelocityGrid->SetPosition(gridX, gridY + mGrid->GetHeight());
}

void NoteStepSequencer::OnMidiNote(MidiNote& note)
{
   int step = ButtonToStep(note.mPitch);
   if (step != -1 && note.mVelocity > 0)
   {
      if (mSetLength)
      {
         mLength = step+1;
         UpdateLights();
      }
      else
      {
         mArpIndex = step-1;
      }
   }
}

void NoteStepSequencer::OnMidiControl(MidiControl& control)
{
   if (control.mControl >= 21 && control.mControl <= 28)
   {
      int step = control.mControl - 21;
      mTones[step] = MIN(control.mValue/127.0f * mNoteRange, mNoteRange-1);
      SyncGridToSeq();
   }
   
   if (control.mControl >= 41 && control.mControl <= 48)
   {
      int step = control.mControl - 41;
      if (control.mValue >= 1)
         mVels[step] = control.mValue;
      SyncGridToSeq();
      UpdateLights();
   }
   
   if (control.mControl == 115)
   {
      mSetLength = control.mValue > 0;
      if (mController)
         mController->SendCC(0, 116, control.mValue);
   }
}

void NoteStepSequencer::ShiftSteps(int amount)
{
   int newTones[NSS_MAX_STEPS];
   int newVels[NSS_MAX_STEPS];
   float newLengths[NSS_MAX_STEPS];
   memcpy(newTones, mTones, NSS_MAX_STEPS*sizeof(int));
   memcpy(newVels, mVels, NSS_MAX_STEPS*sizeof(int));
   memcpy(newLengths, mNoteLengths, NSS_MAX_STEPS*sizeof(float));
   for (int i=0; i<mLength; ++i)
   {
      int dest = (i + mLength + amount) % mLength;
      newTones[dest] = mTones[i];
      newVels[dest] = mVels[i];
      newLengths[dest] = mNoteLengths[i];
   }
   memcpy(mTones, newTones, NSS_MAX_STEPS*sizeof(int));
   memcpy(mVels, newVels, NSS_MAX_STEPS*sizeof(int));
   memcpy(mNoteLengths, newLengths, NSS_MAX_STEPS*sizeof(float));
   SyncGridToSeq();
}

void NoteStepSequencer::ButtonClicked(ClickButton* button)
{
   if (button == mShiftBackButton)
      ShiftSteps(-1);
   if (button == mShiftForwardButton)
      ShiftSteps(1);
   if (button == mRandomizePitchButton)
   {
      for (int i=0; i<mNumSteps; ++i)
         mTones[i] = rand() % mNoteRange;
      SyncGridToSeq();
   }
   if (button == mRandomizeLengthButton)
   {
      for (int i=0; i<mNumSteps; ++i)
         mNoteLengths[i] = ofClamp(ofRandom(1.2f), FLT_EPSILON, 1);
      SyncGridToSeq();
   }
   if (button == mRandomizeVelocityButton)
   {
      for (int i=0; i<mNumSteps; ++i)
         mVels[i] = rand() % 128;
      SyncGridToSeq();
   }
}

void NoteStepSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      TheTransport->UpdateListener(this, mInterval, (-mOffset/TheTransport->CountInStandardMeasure(mInterval)), false);
   if (list == mTimeModeSelector)
   {
      mArpStepSlider->SetShowing(mTimeMode != kTimeMode_Synced);
      mIntervalSelector->SetShowing(mTimeMode != kTimeMode_Free);
      mFreeTimeSlider->SetShowing(mTimeMode == kTimeMode_Free);
      mShiftBackButton->SetShowing(mTimeMode == kTimeMode_Synced);
      mShiftForwardButton->SetShowing(mTimeMode == kTimeMode_Synced);
      
      if (mTimeMode == kTimeMode_Free && mInterval < kInterval_Kick)
      {
         mFreeTimeStep = TheTransport->GetDuration(mInterval);
         TheTransport->UpdateListener(this, kInterval_None);
      }
      else if (oldVal == kTimeMode_Free)
      {
         TheTransport->UpdateListener(this, mInterval, (-mOffset/TheTransport->CountInStandardMeasure(mInterval)), false);
      }
   }
}

void NoteStepSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mOffsetSlider)
   {
      if (mTimeMode != kTimeMode_Free)
      {
         TheTransport->UpdateListener(this, mInterval, (-mOffset/TheTransport->CountInStandardMeasure(mInterval)), false);
      }
   }
}

void NoteStepSequencer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mArpStepSlider)
   {
      if (oldVal > 0)
         mArpPingPongDirection = 1;
      else if (oldVal < 0)
         mArpPingPongDirection = -1;
   }
}

void NoteStepSequencer::SyncGridToSeq()
{
   mGrid->Clear();
   mVelocityGrid->Clear();
   for (int i=0; i<NSS_MAX_STEPS; ++i)
   {
      if (mTones[i] < 0)
         continue;
      
      mGrid->SetVal(i,mTones[i],mNoteLengths[i],false);
      mVelocityGrid->SetVal(i, 0, mVels[i]/127.0f, false);
   }
   mGrid->SetGrid(mNumSteps, mNoteRange);
   mVelocityGrid->SetGrid(mNumSteps, 1);
}

void NoteStepSequencer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["gridwidth"] = mGrid->GetWidth();
   moduleInfo["gridheight"] = mGrid->GetHeight();
}

void NoteStepSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   mModuleSaveData.LoadString("controller", moduleInfo, "", FillDropdown<MidiController*>);
   mModuleSaveData.LoadInt("gridwidth", moduleInfo, 400, 210, 2000, true);
   mModuleSaveData.LoadInt("gridheight", moduleInfo, 200, 80, 2000, true);
   mModuleSaveData.LoadInt("gridrows", moduleInfo, 15, 1, 127);
   mModuleSaveData.LoadInt("gridsteps", moduleInfo, 8, 1, NSS_MAX_STEPS);

   SetUpFromSaveData();
}

void NoteStepSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetMidiController(mModuleSaveData.GetString("controller"));
   mGrid->SetDimensions(mModuleSaveData.GetInt("gridwidth"), mModuleSaveData.GetInt("gridheight"));
   mNoteRange = mModuleSaveData.GetInt("gridrows");
   mNumSteps = mModuleSaveData.GetInt("gridsteps");
   mLength = MIN(mLength, mNumSteps);
   mGrid->SetGrid(mNumSteps, mNoteRange);
   mVelocityGrid->SetGrid(mNumSteps, 1);
   mLengthSlider->SetExtents(1, mNumSteps);
   UpdateVelocityGridPos();
}

namespace
{
   const int kSaveStateRev = 1;
}

void NoteStepSequencer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mGrid->SaveState(out);
   mVelocityGrid->SaveState(out);
}

void NoteStepSequencer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mGrid->LoadState(in);
   mVelocityGrid->LoadState(in);
   GridUpdated(mGrid, 0, 0, 0, 0);
   GridUpdated(mVelocityGrid, 0, 0, 0, 0);
}
