//
//  TextEntry.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/5/12.
//
//

#ifndef __modularSynth__TextEntry__
#define __modularSynth__TextEntry__

#include <iostream>
#include "IUIControl.h"
#include "SynthGlobals.h"

class TextEntry;

class ITextEntryListener
{
public:
   virtual ~ITextEntryListener() {}
   virtual void TextEntryComplete(TextEntry* entry) = 0;
   virtual void TextEntryActivated(TextEntry* entry) {}
};

enum TextEntryType
{
   kTextEntry_Text,
   kTextEntry_Int,
   kTextEntry_Float
};

class TextEntry : public IUIControl
{
public:
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, char* var);
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, int* var, int min, int max);
   TextEntry(ITextEntryListener* owner, const char* name, int x, int y, int charWidth, float* var, float min, float max);
   void OnKeyPressed(int key);
   void Render() override;
   
   void MakeActiveTextEntry();
   static void ClearActiveTextEntry(bool acceptEntry);
   static TextEntry* GetActiveTextEntry() { return sCurrentTextEntry; }
   
   void SetNextTextEntry(TextEntry* entry);
   void UpdateDisplayString();
   void SetInErrorMode(bool error) { mInErrorMode = error; }
   void SetDescription(string desc) { mDescription = desc; }
   void SetFlexibleWidth(bool flex) { mFlexibleWidth = flex; }
   void ClearInput() { bzero(mString, MAX_TEXTENTRY_LENGTH); mCaretPosition = 0; }
   const char* GetText() const { return mString; }
   
   void GetDimensions(int& width, int& height) override;

   //IUIControl
   void SetFromMidiCC(float slider) override {}
   void SetValue(float value) override {}
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, bool shouldSetValue = true) override;
   
protected:
   ~TextEntry();   //protected so that it can't be created on the stack
   
private:
   void Construct(ITextEntryListener* owner, const char* name, int x, int y, int charWidth);  //shared constructor
   
   bool AllowCharacter(char c);
   void AcceptEntry();
   
   void OnClicked(int x, int y, bool right) override;
   
   int mCharWidth;
   ITextEntryListener* mListener;
   char mString[MAX_TEXTENTRY_LENGTH];
   char* mVarString;
   int* mVarInt;
   float* mVarFloat;
   int mIntMin;
   int mIntMax;
   float mFloatMin;
   float mFloatMax;
   int mCaretPosition;
   float mCaretBlinkTimer;
   bool mCaretBlink;
   TextEntryType mType;
   TextEntry* mNextTextEntry;
   TextEntry* mPreviousTextEntry;
   bool mInErrorMode;
   string mDescription;
   bool mFlexibleWidth;
   
   static TextEntry* sCurrentTextEntry;
};

#endif /* defined(__modularSynth__TextEntry__) */
