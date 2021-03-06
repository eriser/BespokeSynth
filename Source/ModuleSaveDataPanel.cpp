//
//  ModuleSaveDataPanel.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 1/28/14.
//
//

#include "ModuleSaveDataPanel.h"
#include "IAudioSource.h"
#include "INoteSource.h"
#include "IAudioReceiver.h"
#include "INoteReceiver.h"

ModuleSaveDataPanel* TheSaveDataPanel = NULL;

const float itemSpacing = 20;

ModuleSaveDataPanel::ModuleSaveDataPanel()
: mSaveModule(NULL)
, mHeight(100)
, mAlignmentX(100)
, mApplyButton(NULL)
{
   assert(TheSaveDataPanel == NULL);
   TheSaveDataPanel = this;
}

ModuleSaveDataPanel::~ModuleSaveDataPanel()
{
   assert(TheSaveDataPanel == this);
   TheSaveDataPanel = NULL;
}

void ModuleSaveDataPanel::SetModule(IDrawableModule* module)
{
   if (mSaveModule == module)
      return;
   
   if (mSaveModule)
      assert(mSaveModule->IsSaveable());
   
   mSaveModule = module;
   mAppearAmount = 0;
   
   ReloadSaveData();
}

void ModuleSaveDataPanel::ReloadSaveData()
{
   for (auto iter = mSaveDataControls.begin(); iter != mSaveDataControls.end(); ++iter)
   {
      RemoveUIControl(*iter);
      (*iter)->Delete();
   }
   mSaveDataControls.clear();
   mLabels.clear();
   mStringDropdowns.clear();
   
   if (mSaveModule == NULL || mSaveModule->IsSaveable() == false)
      return;
   
   UpdateTarget(mSaveModule);
   
   int maxWidth = 40;
   list<ModuleSaveData::SaveVal*> values = mSaveModule->GetSaveData().GetSavedValues();
   for (auto iter = values.begin(); iter != values.end(); ++iter)
      mLabels.push_back((*iter)->mProperty);
   for (int i=0; i<mSaveModule->GetChildren().size(); ++i)
   {
      IDrawableModule* child = mSaveModule->GetChildren()[i];
      list<ModuleSaveData::SaveVal*>& childValues = child->GetSaveData().GetSavedValues();
      for (auto iter = childValues.begin(); iter != childValues.end(); ++iter)
      {
         values.push_back(*iter);
         mLabels.push_back(string(child->Name()) + "." + (*iter)->mProperty);
      }
   }
   for (auto iter = mLabels.begin(); iter != mLabels.end(); ++iter)
      maxWidth = MAX(maxWidth, GetStringWidth(*iter));
   
   mAlignmentX = 10 + maxWidth;
   int x = mAlignmentX;
   int y = 5;
   
   TextEntry* nameEntry = new TextEntry(this,"",x,y,27,mSaveModule->NameMutable());
   mSaveDataControls.push_back(nameEntry);
   y += itemSpacing;
   
   TextEntry* prevTextEntry = nameEntry;
   
   for (auto iter = values.begin(); iter != values.end(); ++iter)
   {
      ModuleSaveData::SaveVal* save = *iter;
      
      IUIControl* control = NULL;
      switch (save->mType)
      {
         case ModuleSaveData::kInt:
            if (save->mEnumValues.empty() == false)
            {
               DropdownList* dropdown = new DropdownList(this, "", x, y, &(save->mInt));
               control = dropdown;
               for (auto iter = save->mEnumValues.begin(); iter != save->mEnumValues.end(); ++iter)
                  dropdown->AddLabel(iter->first.c_str(), iter->second);
            }
            else if (save->mIsTextField)
            {
               control = new TextEntry(this, "",x, y, 7, &(save->mInt), save->mMin, save->mMax);
            }
            else
            {
               control = new IntSlider(this, "", x, y, 150, 15, &(save->mInt), save->mMin, save->mMax);
            }
            break;
         case ModuleSaveData::kFloat:
            if (save->mIsTextField)
               control = new TextEntry(this, "",x, y, 7, &(save->mFloat), save->mMin, save->mMax);
            else
               control = new FloatSlider(this, "", x, y, 150, 15, &(save->mFloat), save->mMin, save->mMax);
            break;
         case ModuleSaveData::kBool:
            control = new Checkbox(this, "", x, y, &(save->mBool));
            break;
         case ModuleSaveData::kString:
            if (save->mFillDropdownFn)
            {
               DropdownList* dropdown = new DropdownList(this, "", x, y, &(save->mInt));
               mStringDropdowns[dropdown] = save;
               control = dropdown;
               FillDropdownList(dropdown, save);
            }
            else
            {
               control = new TextEntry(this, "", x, y, 100, save->mString);
               dynamic_cast<TextEntry*>(control)->SetFlexibleWidth(true);
            }
            break;
      }
      
      TextEntry* textEntry = dynamic_cast<TextEntry*>(control);
      if (textEntry)
      {
         if (prevTextEntry)
            prevTextEntry->SetNextTextEntry(textEntry);
         prevTextEntry = textEntry;
      }
      
      control->SetNoHover(true);
      mSaveDataControls.push_back(control);
      y += itemSpacing;
   }
   
   y += 6;
   mApplyButton = new ClickButton(this,"apply",x,y);
   y += itemSpacing;
   mSaveDataControls.push_back(mApplyButton);
   
   mHeight = y+5;
}

void ModuleSaveDataPanel::DrawModule()
{
   if (Minimized() || mAppearAmount < 1)
      return;
   
   int x = mAlignmentX-5;
   int y = 5;
   
   DrawTextLeftJustify("name", x, y+12);
   y += itemSpacing;
   
   for (auto iter = mLabels.begin(); iter != mLabels.end(); ++iter)
   {
      DrawTextLeftJustify(*iter, x, y+12);
      y += itemSpacing;
   }
   
   for (auto iter = mSaveDataControls.begin(); iter != mSaveDataControls.end(); ++iter)
      (*iter)->Draw();
}

void ModuleSaveDataPanel::UpdatePosition()
{
   if (mSaveModule)
   {
      int moduleX,moduleY,moduleW,moduleH;
      mSaveModule->GetPosition(moduleX, moduleY);
      mSaveModule->GetDimensions(moduleW, moduleH);
      
      SetPosition(moduleX + moduleW, moduleY);
   }
   
   if (mShowing)
      mAppearAmount = ofClamp(mAppearAmount + ofGetLastFrameTime() * 5, 0, 1);
   else
      mAppearAmount = ofClamp(mAppearAmount - ofGetLastFrameTime() * 5, 0, 1);
}

void ModuleSaveDataPanel::ApplyChanges()
{
   if (mSaveModule)
      mSaveModule->SetUpFromSaveData();
   TheSaveDataPanel->SetModule(NULL);
}

void ModuleSaveDataPanel::FillDropdownList(DropdownList* list, ModuleSaveData::SaveVal* save)
{
   list->Clear();
   FillDropdownFn fillFn = save->mFillDropdownFn;
   assert(fillFn);
   fillFn(list);
   
   save->mInt = -1;
   for (int i=0; i<list->GetNumValues(); ++i)
   {
      if (list->GetLabel(i) == save->mString)
      {
         save->mInt = i;
         break;
      }
   }
   
   if (strlen(save->mString) > 0)
      list->SetUnknownItemString(save->mString);
}

void ModuleSaveDataPanel::ButtonClicked(ClickButton* button)
{
   if (button == mApplyButton)
      ApplyChanges();
}

void ModuleSaveDataPanel::CheckboxUpdated(Checkbox* checkbox)
{
}

void ModuleSaveDataPanel::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void ModuleSaveDataPanel::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void ModuleSaveDataPanel::TextEntryComplete(TextEntry* entry)
{
}

void ModuleSaveDataPanel::DropdownClicked(DropdownList* list)
{
   for (auto iter = mStringDropdowns.begin(); iter != mStringDropdowns.end(); ++iter)
   {
      if (list == iter->first)
      {
         FillDropdownList(list, iter->second);
      }
   }
}

void ModuleSaveDataPanel::DropdownUpdated(DropdownList* list, int oldVal)
{
   for (auto iter = mStringDropdowns.begin(); iter != mStringDropdowns.end(); ++iter)
   {
      if (list == iter->first)
      {
         ModuleSaveData::SaveVal* save = iter->second;
         StringCopy(save->mString, list->GetLabel(save->mInt).c_str(), MAX_TEXTENTRY_LENGTH);
      }
   }
}

void ModuleSaveDataPanel::GetModuleDimensions(int& width, int& height)
{
   if (mShowing)
   {
      width = EaseOut(0, mAlignmentX + 250, mAppearAmount);
      height = EaseIn(0, mHeight, mAppearAmount);
   }
   else
   {
      width = 0;
      height = 0;
   }
}