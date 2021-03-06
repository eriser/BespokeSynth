//
//  NoteEffectBase.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/17/15.
//
//

#ifndef Bespoke_NoteEffectBase_h
#define Bespoke_NoteEffectBase_h

#include "INoteReceiver.h"
#include "INoteSource.h"

class NoteEffectBase : public INoteReceiver, public INoteSource
{
public:
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) override
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
   }
};

#endif
