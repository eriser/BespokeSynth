//
//  NoteSequencerColumn.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/12/16.
//
//

#include "NoteSequencerColumn.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Grid.h"
#include "NoteStepSequencer.h"

NoteSequencerColumn::NoteSequencerColumn()
: mGrid(NULL)
, mColumnSlider(NULL)
, mRowSlider(NULL)
, mGridCable(NULL)
, mRow(0)
, mColumn(0)
, mSequencer(NULL)
{
}

NoteSequencerColumn::~NoteSequencerColumn()
{
}

void NoteSequencerColumn::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mColumnSlider = new IntSlider(this,"column",2,2,80,15,&mColumn,0,15);
   mRowSlider = new IntSlider(this,"row",2,20,80,15,&mRow,0,15);
   
   mGridCable = new PatchCableSource(this, kConnectionType_Special);
   mGridCable->AddTypeFilter("notesequencer");
   AddPatchCableSource(mGridCable);
}

void NoteSequencerColumn::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mColumnSlider->Draw();
   mRowSlider->Draw();
}

void NoteSequencerColumn::PostRepatch(PatchCableSource* cableSource)
{
   if (mGridCable == cableSource)
      SyncWithCable();
}

void NoteSequencerColumn::SyncWithCable()
{
   mSequencer = dynamic_cast<NoteStepSequencer*>(mGridCable->GetTarget());
   if (mSequencer)
   {
      mGrid = mSequencer->GetGrid();
      mColumnSlider->SetExtents(0, mGrid->GetCols()-1);
      mRowSlider->SetExtents(0, mGrid->GetRows()-1);
   }
}

void NoteSequencerColumn::IntSliderUpdated(IntSlider* slider, int oldVal)
{
   if (slider == mRowSlider)
   {
      if (mGrid)
         mGrid->SetVal(mColumn, mRow, 1);
   }
}

void NoteSequencerColumn::GetModuleDimensions(int &width, int &height)
{
   width = 100;
   height = 38;
}

void NoteSequencerColumn::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["target"] = mSequencer ? mSequencer->Path() : "";
}

void NoteSequencerColumn::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteSequencerColumn::SetUpFromSaveData()
{
   mGridCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SyncWithCable();
}
