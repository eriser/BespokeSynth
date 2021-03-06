//
//  ControlSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 8/27/15.
//
//

#include "ControlSequencer.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

list<ControlSequencer*> ControlSequencer::sControlSequencers;

ControlSequencer::ControlSequencer()
: mGrid(NULL)
, mUIControl(NULL)
, mInterval(kInterval_16n)
, mIntervalSelector(NULL)
, mLength(4)
, mLengthSelector(NULL)
, mControlCable(NULL)
{
   TheTransport->AddListener(this, mInterval);
   
   sControlSequencers.push_back(this);
}

ControlSequencer::~ControlSequencer()
{
   TheTransport->RemoveListener(this);
   
   sControlSequencers.remove(this);
}

void ControlSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new Grid(5,23,130,40,16,1);
   mIntervalSelector = new DropdownList(this,"interval",5,3,(int*)(&mInterval));
   mLengthSelector = new DropdownList(this,"length",45,3,(int*)(&mLength));
   
   mControlCable = new PatchCableSource(this, kConnectionType_UIControl);
   //mControlCable->SetManualPosition(86, 10);
   AddPatchCableSource(mControlCable);
   
   mGrid->SetGridMode(Grid::kMultislider);
   mGrid->SetHighlightCol(-1);
   mGrid->SetClickClearsToZero(false);
   mGrid->SetMajorColSize(4);
   mGrid->SetListener(this);
   
   /*mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("2", kInterval_2);*/
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
   
   mLengthSelector->AddLabel("4n", 1);
   mLengthSelector->AddLabel("2n", 2);
   mLengthSelector->AddLabel("1", 4);
   mLengthSelector->AddLabel("2", 8);
   mLengthSelector->AddLabel("3", 12);
   mLengthSelector->AddLabel("4", 16);
   mLengthSelector->AddLabel("6", 24);
   mLengthSelector->AddLabel("8", 32);
   mLengthSelector->AddLabel("16", 64);
   mLengthSelector->AddLabel("32", 128);
   mLengthSelector->AddLabel("64", 256);
   mLengthSelector->AddLabel("128", 512);
}

void ControlSequencer::Init()
{
   IDrawableModule::Init();
}

void ControlSequencer::Poll()
{
}

void ControlSequencer::OnTimeEvent(int samplesTo)
{
   int stepsPerMeasure = TheTransport->CountInStandardMeasure(mInterval) * TheTransport->GetTimeSigTop()/TheTransport->GetTimeSigBottom();
   int numMeasures = MAX(1,ceil(float(mGrid->GetCols()) / stepsPerMeasure));
   int measure = TheTransport->GetMeasure() % numMeasures;
   int step = (TheTransport->GetQuantized(0, mInterval) + measure * stepsPerMeasure) % mGrid->GetCols();
   
   mGrid->SetHighlightCol(step);
   
   if (mUIControl && mEnabled)
      mUIControl->SetFromMidiCC(mGrid->GetVal(step, 0));
}

void ControlSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mGrid->Draw();
   mIntervalSelector->Draw();
   mLengthSelector->Draw();
   
   int currentHover = mGrid->CurrentHover();
   if (currentHover != -1 && mUIControl)
   {
      ofPushStyle();
      ofSetColor(ofColor::grey);
      float val = mGrid->GetVal(currentHover % mGrid->GetCols(), currentHover / mGrid->GetCols());
      DrawText(mUIControl->GetDisplayValue(mUIControl->GetValueForMidiCC(val)), mGrid->GetPosition().x, mGrid->GetPosition().y+12);
      ofPopStyle();
   }
}

void ControlSequencer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   mGrid->TestClick(x, y, right);
}

void ControlSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mGrid->MouseReleased();
}

bool ControlSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x,y);
   mGrid->NotifyMouseMoved(x,y);
   return false;
}

void ControlSequencer::SetNumSteps(int numSteps, bool stretch)
{
   int oldNumSteps = mGrid->GetCols();
   assert(numSteps != 0);
   assert(oldNumSteps != 0);
   if (stretch)   //updated interval, stretch old pattern out to make identical pattern at higher res
   {              // abcd becomes aabbccdd
      vector<float> pattern;
      pattern.resize(oldNumSteps);
      for (int i=0; i<oldNumSteps; ++i)
         pattern[i] = mGrid->GetVal(i,0);
      float ratio = float(numSteps)/oldNumSteps;
      for (int i=0; i<numSteps; ++i)
         mGrid->SetVal(i, 0, pattern[int(i/ratio)]);
   }
   else           //updated length, copy old pattern out to make identical pattern over longer time
   {              // abcd becomes abcdabcd
      int numCopies = numSteps / oldNumSteps;
      for (int i=1; i<numCopies; ++i)
      {
         for (int j=0; j<oldNumSteps; ++j)
            mGrid->SetVal(i*oldNumSteps + j, 0, mGrid->GetVal(j, 0));
      }
   }
   mGrid->SetGrid(numSteps,1);
}

void ControlSequencer::GridUpdated(Grid* grid, int col, int row, float value, float oldValue)
{
   if (grid == mGrid)
   {
      int numValues = mUIControl ? mUIControl->GetNumValues() : 0;
      if (numValues > 1)
      {
         for (int i=0; i<mGrid->GetCols(); ++i)
         {
            float val = mGrid->GetVal(i, 0);
            val = int((val * (numValues-1)) + .5f) / float(numValues-1);   //quantize to match the number of allowed values
            mGrid->SetVal(i, 0, val);
         }
      }
   }
}

void ControlSequencer::PostRepatch(PatchCableSource* cableSource)
{
   if (mControlCable->GetPatchCables().empty() == false)
      mUIControl = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[0]->GetTarget());
   else
      mUIControl = NULL;
   if (mUIControl)
   {
      for (int i=0; i<mGrid->GetCols(); ++i)
         mGrid->SetVal(i, 0, mUIControl->GetMidiValue());
   }
}

void ControlSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
   int newSteps = int(mLength/4.0f * TheTransport->CountInStandardMeasure(mInterval));
   if (list == mIntervalSelector)
   {
      if (newSteps > 0)
      {
         TheTransport->UpdateListener(this, mInterval);
         SetNumSteps(newSteps, true);
      }
      else
      {
         mInterval = (NoteInterval)oldVal;
      }
   }
   if (list == mLengthSelector)
   {
      if (newSteps > 0)
         SetNumSteps(newSteps, false);
      else
         mLength = oldVal;
   }
}

namespace
{
   const float extraW = 10;
   const float extraH = 30;
}

void ControlSequencer::GetModuleDimensions(int &width, int &height)
{
   width = mGrid->GetWidth() + extraW;
   height = mGrid->GetHeight() + extraH;
}

void ControlSequencer::Resize(float w, float h)
{
   w = MAX(w - extraW, 130);
   h = MAX(h - extraH, 40);
   SetGridSize(w,h);
}

void ControlSequencer::SetGridSize(float w, float h)
{
   mGrid->SetDimensions(w, h);
}

void ControlSequencer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["uicontrol"] = mUIControl ? mUIControl->Path() : "";
}

void ControlSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("uicontrol", moduleInfo);
   mModuleSaveData.LoadFloat("gridwidth", moduleInfo, 130, 120, 1000);
   mModuleSaveData.LoadFloat("gridheight", moduleInfo, 40, 15, 1000);
   
   SetUpFromSaveData();
}

void ControlSequencer::SetUpFromSaveData()
{
   string controlPath = mModuleSaveData.GetString("uicontrol");
   if (!controlPath.empty())
   {
      mUIControl = TheSynth->FindUIControl(controlPath);
      if (mUIControl)
         mControlCable->SetTarget(mUIControl);
   }
   else
   {
      mUIControl = NULL;
   }
   SetGridSize(mModuleSaveData.GetFloat("gridwidth"), mModuleSaveData.GetFloat("gridheight"));
}

namespace
{
   const int kSaveStateRev = 0;
}

void ControlSequencer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   mGrid->SaveState(out);
}

void ControlSequencer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   mGrid->LoadState(in);
}
