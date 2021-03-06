//
//  PatchCable.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/12/15.
//
//

#ifndef __Bespoke__PatchCable__
#define __Bespoke__PatchCable__

#include "IClickable.h"

class RollingBuffer;
class PatchCableSource;

struct PatchCablePos
{
   ofVec2f start;
   ofVec2f end;
   ofVec2f plug;
};

enum ConnectionType
{
   kConnectionType_Note,
   kConnectionType_Audio,
   kConnectionType_UIControl,
   kConnectionType_Grid,
   kConnectionType_Special
};

class PatchCable : public IClickable
{
   friend class PatchCableSource;
public:
   PatchCable(PatchCableSource* owner);
   virtual ~PatchCable();
   
   void Render() override;
   bool TestClick(int x, int y, bool right, bool testOnly = false) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   void GetDimensions(int& width, int& height) override { width = 10; height = 10; }
   
   IDrawableModule* GetOwningModule() const;
   IClickable* GetTarget() const { return mTarget; }
   ConnectionType GetConnectionType() const;
   
   void Grab();
   bool IsValidTarget(IClickable* target) const;
   void Destroy();
   
   static PatchCable* sActivePatchCable;
   
protected:
   void OnClicked(int x, int y, bool right) override;
private:
   void SetTarget(IClickable* target) { mTarget = target; }
   PatchCablePos GetPatchCablePos();
   bool IsOverStart(int x, int y);
   bool IsOverEnd(int x, int y);
   
   PatchCableSource* mOwner;
   IClickable* mTarget;

   bool mHovered;
   bool mDragging;
};

#endif /* defined(__Bespoke__PatchCable__) */
