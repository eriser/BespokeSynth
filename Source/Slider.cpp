//
//  Slider.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 12/4/12.
//
//

#include "Slider.h"
#include "IDrawableModule.h"
#include "SynthGlobals.h"
#include "FloatSliderLFOControl.h"
#include "LFOController.h"
#include "FileStream.h"
#include "ModularSynth.h"

FloatSlider::FloatSlider(IFloatSliderListener* owner, const char* label, int x, int y, int w, int h, float* var, float min, float max, int digits /* = -1 */)
: mVar(var)
, mWidth(w)
, mHeight(h)
, mMin(min)
, mMax(max)
, mMouseDown(false)
, mFineRefX(-999)
, mRefY(-999)
, mShowDigits(digits)
, mOwner(owner)
, mLFOControl(NULL)
, mRelative(false)
, mTouching(false)
, mRelativeOffset(-999)
, mClamped(true)
, mMode(kNormal)
, mOriginalValue(0)
, mClampIntMin(-999)
, mMinValueDisplay("")
, mMaxValueDisplay("")
, mFloatEntry(NULL)
, mShowName(true)
{
   assert(owner);
   SetLabel(label);
   SetPosition(x,y);
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

FloatSlider::~FloatSlider()
{
}

void FloatSlider::Init()
{
   if (mVar)
      mOriginalValue = *mVar;
}

FloatSliderLFOControl* FloatSlider::AcquireLFO()
{
   if (mLFOControl == NULL)
   {
      if (GetParent() != TheSynth->GetTopModalFocusItem()) //popups don't get these
         mLFOControl = LFOPool::GetLFO(this);
   }
   return mLFOControl;
}

void FloatSlider::SetLabel(const char* label)
{
   SetName(label);
}

void FloatSlider::Render()
{
   mLastDisplayedValue = *mVar;
   
   ofPushStyle();

   ofColor color;
   if (IsPreset())
      color.set(0,255,0,gModuleDrawAlpha);
   else
      color.set(255,255,255,gModuleDrawAlpha);

   ofFill();
   ofSetColor(color.r,color.g,color.b,color.a*.2f);
   ofRect(mX,mY,mWidth,mHeight);
   ofNoFill();

   ofSetColor(color);
   
   float screenPos;
   if (mLFOControl && mLFOControl->Active())
   {
      float val = ofClamp(mLFOControl->Value(),mMin,mMax);
      screenPos = mX+1+(mWidth-2)*ValToPos(val);
      float lfomax = ofClamp(mLFOControl->Max(),mMin,mMax);
      float screenPosMax = mX+1+(mWidth-2)*ValToPos(lfomax);
      float lfomin = ofClamp(mLFOControl->Min(),mMin,mMax);
      float screenPosMin = mX+1+(mWidth-2)*ValToPos(lfomin);
      
      ofPushStyle();
      ofSetColor(0,200,0,gModuleDrawAlpha);
      ofFill();
      ofRect(screenPosMin,mY,screenPos-screenPosMin,mHeight, 1); //lfo bar
      ofPopStyle();

      ofPushStyle();
      ofSetColor(0,255,0,gModuleDrawAlpha);
      ofSetLineWidth(2);
      ofLine(screenPosMin,mY+1,screenPosMin,mY+mHeight-1); //min bar
      ofLine(screenPosMax,mY+1,screenPosMax,mY+mHeight-1); //max bar
      ofPopStyle();
   }
   else
   {
      ofPushStyle();
      if (*mVar >= mMin && *mVar <= mMax)
         ofSetColor(255,0,0,gModuleDrawAlpha);
      else
         ofSetColor(30,30,30,gModuleDrawAlpha);
      float val = ofClamp(*mVar,mMin,mMax);
      screenPos = mX+1+(mWidth-2)*ValToPos(val);
      ofSetLineWidth(2);
      ofLine(screenPos,mY+1,screenPos,mY+mHeight-1);  //value bar
      ofPopStyle();
   }
   
   DrawBeacon(screenPos, mY+mHeight/2);
   
   DrawHover();

   string display;
   if (mShowName)
      display = string(Name());
   if (display.length() > 0) //only show a colon if there's a label
      display += ":";
   if (mFloatEntry)
   {
      ofSetColor(255, 255, 100);
      display += mFloatEntry->GetText();
   }
   else
   {
      display += GetDisplayValue(*mVar);
   }
   DrawText(display, mX+2, mY+5+mHeight/2);

   ofPopStyle();
}

void FloatSlider::DisplayLFOControl()
{
   FloatSliderLFOControl* lfo = AcquireLFO();
   if (lfo)
   {
      int thisx,thisy;
      GetPosition(thisx,thisy);
      
      lfo->SetLFOEnabled(true);
      
      int w,h;
      lfo->GetDimensions(w, h);
      lfo->SetPosition(thisx,thisy+15);
      //lfo->SetParent(this);
      TheSynth->PushModalFocusItem(lfo);
      
      if (TheLFOController)
         TheLFOController->SetSlider(this);
   }
}

void FloatSlider::OnClicked(int x, int y, bool right)
{
   if (right)
   {
      DisplayLFOControl();
      return;
   }

   mFineRefX = ofMap(ValToPos(*GetModifyValue()),0.0f,1.0f,mX+1,mX+mWidth-1,true)-mX;
   mRefY = y;
   SetValueForMouse(x,y);
   mMouseDown = true;
   mTouching = true;
   UpdateTouching();
}

void FloatSlider::MouseReleased()
{
   if (mMouseDown)
      mTouching = false;
   mMouseDown = false;
   mRefY = -999;
   if (mRelative && (mLFOControl == NULL || mLFOControl->Active() == false))
      SetValue(0);
}

bool FloatSlider::MouseMoved(float x, float y)
{
   CheckHover(x,y);
   if (mMouseDown)
      SetValueForMouse(x,y);
   return mMouseDown;
}

void FloatSlider::SetValueForMouse(int x, int y)
{
   float* var = GetModifyValue();
   float fX = x;
   bool clampInt = false;
   if (GetKeyModifiers() & kModifier_Shift)
   {
      if (mFineRefX == -999)
      {
         mFineRefX = x;
      }
      float precision = mShowDigits != -1 ? 100 : 10;
      fX = mFineRefX + (fX-mFineRefX)/precision;
   }
   else if (GetKeyModifiers() & kModifier_Command)
   {
      clampInt = true;
      if (mClampIntMin == -999)
      {
         mClampIntMin = floor(*var);
         mClampIntMax = ceil(*var);
         if (mClampIntMin == mClampIntMax)
            ++mClampIntMax;
      }
   }
   else
   {
      mFineRefX = -999;
      mClampIntMin = -999;
   }
   float oldVal = *var;
   float pos = ofMap(fX+mX,mX+1,mX+mWidth-1,0.0f,1.0f);
   *var = PosToVal(pos);
   if (mRelative && (mLFOControl == NULL || mLFOControl->Active() == false))
   {
      if (!mTouching || mRelativeOffset == -999)
      {
         mRelativeOffset = *var;
         *var = 0;
      }
      else
      {
         *var -= mRelativeOffset;
      }
   }
   *var = ofClamp(*var,mMin,mMax);
   if (clampInt)
      *var = ofClamp(*var,mClampIntMin,mClampIntMax);
   
   if (oldVal != *var)
   {
      mOwner->FloatSliderUpdated(this, oldVal);
   }

   if (mLFOControl && mLFOControl->Active())
   {
      float move = (y - mRefY) * -.003f;
      float change = move * (mMax - mMin);
      *mLFOControl->MinPtr() = ofClamp(mLFOControl->Min() + change, mMin, mLFOControl->Max());
      mRefY = y;
   }
}

void FloatSlider::SetFromMidiCC(float slider)
{
   SetValue(GetValueForMidiCC(slider));
}

float FloatSlider::GetValueForMidiCC(float slider) const
{
   slider = ofClamp(slider,0,1);
   return PosToVal(slider);
}

float FloatSlider::PosToVal(float pos) const
{
   if (pos < 0)
      return mMin;
   if (pos > 1)
      return mMax;
   if (mMode == kNormal)
      return mMin + pos*(mMax-mMin);
   if (mMode == kLogarithmic)
      return mMin * powf(mMax/mMin, pos);
   if (mMode == kSquare)
      return mMin + pos*pos*(mMax-mMin);
   assert(false);
   return 0;
}

float FloatSlider::ValToPos(float val) const
{
   if (mMode == kNormal)
      return (val - mMin) / (mMax-mMin);
   if (mMode == kLogarithmic)
      return log(val/mMin) / log(mMax/mMin);
   if (mMode == kSquare)
      return sqrtf((val - mMin) / (mMax-mMin));
   return 0;
}

void FloatSlider::SetValue(float value)
{
   if (TheLFOController && TheLFOController->WantsBinding(this))
   {
      TheLFOController->SetSlider(this);
      return;
   }
   
   float* var = GetModifyValue();
   float oldVal = *var;
   if (mRelative)
   {
      if (!mTouching || mRelativeOffset == -999)
      {
         mRelativeOffset = value;
         value = 0;
      }
      else
      {
         value -= mRelativeOffset;
      }
   }
   if (mClamped)
      *var = ofClamp(value,mMin,mMax);
   else
      *var = value;
   DisableLFO();
   if (oldVal != *var)
   {
      mOwner->FloatSliderUpdated(this, oldVal);
   }
}

void FloatSlider::UpdateTouching()
{
   if (mRelative && (mLFOControl == NULL || mLFOControl->Active() == false))
   {
      if (!mTouching)
         SetValue(0);
      mRelativeOffset = -999;
   }
}

void FloatSlider::MatchExtents(FloatSlider* slider)
{
   mMax = slider->mMax;
   mMin = slider->mMin;
}

void FloatSlider::DisableLFO()
{
   if (mLFOControl)
      mLFOControl->SetEnabled(false);
}

float FloatSlider::GetValue() const
{
   return *mVar;
}

float FloatSlider::GetMidiValue()
{
   if (mMin == mMax)
      return 0;
   
   return ValToPos(*mVar);
}

string FloatSlider::GetDisplayValue(float val) const
{
   if (val == mMin && mMinValueDisplay != "")
      return mMinValueDisplay;
   if (val == mMax && mMaxValueDisplay != "")
      return mMaxValueDisplay;
   
   int decDigits = 3;
   if (mShowDigits != -1)
      decDigits = mShowDigits;
   else if (mMax-mMin > 1000)
      decDigits = 0;
   else if (mMax-mMin > 100)
      decDigits = 1;
   else if (mMax-mMin > 10)
      decDigits = 2;
   
   float displayVar = val;
   if (decDigits == 0)  //round down if we're showing int value
      displayVar = (int)displayVar;
   return ofToString(displayVar,decDigits);
}

void FloatSlider::Compute(int samplesIn /*= 0*/)
{
   if (mLFOControl && mLFOControl->Active())
   {
      float oldVal = *mVar;
      *mVar = mLFOControl->Value(samplesIn);
      if (oldVal != *mVar)
      {
         mOwner->FloatSliderUpdated(this, oldVal);
      }
   }
}

float* FloatSlider::GetModifyValue()
{
   if (mLFOControl && mLFOControl->Active())
      return mLFOControl->MaxPtr();
   return mVar;
}

void FloatSlider::Double()
{
   float doubl = *GetModifyValue() * 2.0f;
   if (doubl >= mMin && doubl <= mMax)
      SetValue(doubl);
}

void FloatSlider::Halve()
{
   float half = *GetModifyValue() * .5f;
   if (half >= mMin && half <= mMax)
      SetValue(half);
}

void FloatSlider::Increment(float amount)
{
   float val = *GetModifyValue() + amount;
   if (val >= mMin && val <= mMax)
      SetValue(val);
}

void FloatSlider::ResetToOriginal()
{
   SetValue(mOriginalValue);
}

bool FloatSlider::CheckNeedsDraw()
{
   if (IUIControl::CheckNeedsDraw())
      return true;
   
   return *mVar != mLastDisplayedValue;
}

bool FloatSlider::AttemptTextInput()
{
   if (mFloatEntry)
      mFloatEntry->Delete();
   mFloatEntry = new TextEntry(this, "", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 10, &mEntryValue, mMin, mMax);
   mFloatEntry->MakeActiveTextEntry();
   mFloatEntry->ClearInput();
   return true;
}

void FloatSlider::TextEntryComplete(TextEntry* entry)
{
   mFloatEntry->Delete();
   mFloatEntry = NULL;
   SetValue(mEntryValue);
}

namespace
{
   const int kFloatSliderSaveStateRev = 0;
}

void FloatSlider::SaveState(FileStreamOut& out)
{
   out << kFloatSliderSaveStateRev;
   
   out << (float)*mVar;
   
   bool hasLFO = mLFOControl && mLFOControl->Active();
   out << hasLFO;
   if (hasLFO)
   {
      mLFOControl->GetSettings().SaveState(out);
   }
}

void FloatSlider::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kFloatSliderSaveStateRev);
   
   float var;
   in >> var;
   if (shouldSetValue)
      SetValueDirect(var);
   
   bool hasLFO;
   in >> hasLFO;
   if (hasLFO)
   {
      FloatSliderLFOControl* lfo = AcquireLFO();
      if (shouldSetValue)
         lfo->SetLFOEnabled(true);
      mLFOControl->GetLFOSettings()->LoadState(in);
      if (shouldSetValue)
         lfo->UpdateFromSettings();
   }
}

IntSlider::IntSlider(IIntSliderListener* owner, const char* label, int x, int y, int w, int h, int* var, int min, int max)
: mVar(var)
, mWidth(w)
, mHeight(h)
, mMin(min)
, mMax(max)
, mMouseDown(false)
, mOwner(owner)
, mOriginalValue(0)
, mSliderVal(0)
, mIntEntry(NULL)
, mShowName(true)
{
   assert(owner);
   SetLabel(label);
   SetPosition(x,y);
   (dynamic_cast<IDrawableModule*>(owner))->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
   CalcSliderVal();
}

IntSlider::~IntSlider()
{
}

void IntSlider::Init()
{
   if (mVar)
      mOriginalValue = *mVar;
}

void IntSlider::SetLabel(const char* label)
{
   SetName(label);
}

void IntSlider::Poll()
{
   if (*mVar != mLastSetValue)
      CalcSliderVal();
}

void IntSlider::Render()
{
   mLastDisplayedValue = *mVar;
   
   ofPushStyle();

   ofColor color;
   if (IsPreset())
      color.set(0,255,0,gModuleDrawAlpha);
   else
      color.set(255,255,255,gModuleDrawAlpha);

   ofFill();
   ofSetColor(color.r,color.g,color.b,color.a*.2f);
   ofRect(mX,mY,mWidth,mHeight);
   ofNoFill();

   ofSetColor(color);
   if (mWidth / MAX(1, (mMax - mMin)) > 3)   //hash marks
   {
      ofPushStyle();
      ofSetColor(100,100,100,gModuleDrawAlpha);
      for (int i=mMin+1; i<mMax; ++i)
      {
         float x = mX+1+(mWidth-2)*((i-mMin)/float(mMax-mMin));
         ofLine(x,mY+1,x,mY+mHeight-1);
      }
      ofPopStyle();
   }

   int val = ofClamp(*mVar,mMin,mMax);
   ofPushStyle();
   if (*mVar >= mMin && *mVar <= mMax)
      ofSetColor(255,0,0,gModuleDrawAlpha);
   else
      ofSetColor(30,30,30,gModuleDrawAlpha);
   float xpos = mX+1+(mWidth-2)*((val-mMin)/float(mMax-mMin));
   ofSetLineWidth(2);
   ofLine(xpos,mY+1,xpos,mY+mHeight-1);
   ofPopStyle();
   
   DrawBeacon(xpos, mY+mHeight/2);
   
   DrawHover();

   string display;
   if (mShowName)
      display = string(Name());
   if (display.length() > 0) //only show a colon if there's a label
      display += ":";
   if (mIntEntry)
   {
      ofSetColor(255, 255, 100);
      display += mIntEntry->GetText();
   }
   else
   {
      display += GetDisplayValue(*mVar);
   }
   DrawText(display, mX+4, mY+5+mHeight/2);

   ofPopStyle();
}

void IntSlider::CalcSliderVal()
{
   mLastSetValue = *mVar;
   mSliderVal = ofMap(*mVar,mMin,mMax,0.0f,1.0f,K(clamp));
}

void IntSlider::OnClicked(int x, int y, bool right)
{
   if (right)
      return;
   
   SetValueForMouse(x,y);
   mMouseDown = true;
}

bool IntSlider::MouseMoved(float x, float y)
{
   CheckHover(x,y);
   if (mMouseDown)
      SetValueForMouse(x,y);
   return mMouseDown;
}

void IntSlider::SetValueForMouse(int x, int y)
{
   int oldVal = *mVar;
   *mVar = (int)round(ofMap(x+mX,mX+1,mX+mWidth-1,mMin,mMax));
   *mVar = ofClamp(*mVar,mMin,mMax);
   if (oldVal != *mVar)
   {
      CalcSliderVal();
      mOwner->IntSliderUpdated(this, oldVal);
   }
}

void IntSlider::SetFromMidiCC(float slider)
{
   slider = ofClamp(slider,0,1);
   SetValue(GetValueForMidiCC(slider));
   mSliderVal = slider;
   mLastSetValue = *mVar;
}

float IntSlider::GetValueForMidiCC(float slider) const
{
   slider = ofClamp(slider,0,1);
   return int(ofMap(slider,0,1,mMin,mMax));
}

void IntSlider::SetValue(float value)
{
   int oldVal = *mVar;
   *mVar = (int)ofClamp(value,mMin,mMax);
   if (oldVal != *mVar)
   {
      CalcSliderVal();
      gControlTactileFeedback = 1;
      mOwner->IntSliderUpdated(this, oldVal);
   }
}

float IntSlider::GetValue() const
{
   return *mVar;
}

float IntSlider::GetMidiValue()
{
   return mSliderVal;
}

string IntSlider::GetDisplayValue(float val) const
{
   return ofToString(val,0);
}

void IntSlider::Increment(float amount)
{
   int val = *mVar + (int)amount;
   if (val >= mMin && val <= mMax)
      SetValue(val);
}

void IntSlider::ResetToOriginal()
{
   SetValue(mOriginalValue);
}

bool IntSlider::CheckNeedsDraw()
{
   if (IUIControl::CheckNeedsDraw())
      return true;
   
   return *mVar != mLastDisplayedValue;
}

bool IntSlider::AttemptTextInput()
{
   if (mIntEntry)
      mIntEntry->Delete();
   mIntEntry = new TextEntry(this, "", HIDDEN_UICONTROL, HIDDEN_UICONTROL, 10, &mEntryValue, mMin, mMax);
   mIntEntry->MakeActiveTextEntry();
   mIntEntry->ClearInput();
   return true;
}

void IntSlider::TextEntryComplete(TextEntry* entry)
{
   mIntEntry->Delete();
   mIntEntry = NULL;
   SetValue(mEntryValue);
}

namespace
{
   const int kIntSliderSaveStateRev = 0;
}

void IntSlider::SaveState(FileStreamOut& out)
{
   out << kIntSliderSaveStateRev;
   
   out << (float)*mVar;
}

void IntSlider::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev == kIntSliderSaveStateRev);
   
   float var;
   in >> var;
   if (shouldSetValue)
      SetValueDirect(var);
}