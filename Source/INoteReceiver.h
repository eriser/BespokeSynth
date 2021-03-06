//
//  INoteReceiver.h
//  modularSynth
//
//  Created by Ryan Challinor on 12/2/12.
//
//

#ifndef modularSynth_INoteReceiver_h
#define modularSynth_INoteReceiver_h

#include "OpenFrameworksPort.h"

class ModulationChain;

class INoteReceiver
{
public:
   virtual ~INoteReceiver() {}
   virtual void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = NULL, ModulationChain* modWheel = NULL, ModulationChain* pressure = NULL) = 0;
   virtual void SendPressure(int pitch, int pressure) {}
};

#endif
