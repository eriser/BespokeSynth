//
//  MidiDevice.h
//  additiveSynth
//
//  Created by Ryan Challinor on 11/19/12.
//
//

#ifndef __additiveSynth__MidiDevice__
#define __additiveSynth__MidiDevice__

#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

struct MidiNote
{
   const char* mDeviceName;
   int mPitch;
   float mVelocity; //0-127
   int mChannel;
};

struct MidiControl
{
   const char* mDeviceName;
   int mControl;
   float mValue;
   int mChannel;
};

struct MidiProgramChange
{
   const char* mDeviceName;
   int mProgram;
   int mChannel;
};

struct MidiPitchBend
{
   const char* mDeviceName;
   float mValue;
   int mChannel;
};

struct MidiPressure
{
   const char* mDeviceName;
   int mPitch;
   float mPressure;
   int mChannel;
};

class MidiDeviceListener
{
public:
   virtual ~MidiDeviceListener() {}
   virtual void ControllerPageSelected() {}
   virtual void OnMidiNote(MidiNote& note) = 0;
   virtual void OnMidiControl(MidiControl& control) = 0;
   virtual void OnMidiProgramChange(MidiProgramChange& program) {}
   virtual void OnMidiPitchBend(MidiPitchBend& pitchBend) {}
   virtual void OnMidiPressure(MidiPressure& pressure) {}
};

class MidiDevice : public MidiInputCallback
{
public:
   MidiDevice(MidiDeviceListener* listener);
   virtual ~MidiDevice();
   
   bool ConnectInput(const char* name);
   void ConnectInput(int index);
   void ConnectOutput(const char* name, int channel = 1);
   void ConnectOutput(int index, int channel = 1);
   bool Reconnect();
   bool IsConnected();

   const char* Name() { return TheSynth->GetGlobalManagers()->mDeviceManager.isMidiInputEnabled(mDeviceNameIn) ? mDeviceNameIn : mDeviceNameOut; }
   
   vector<string> GetPortList();
   
   void SendNote(int pitch, int velocity, bool forceNoteOn = false, int channel = -1);
   void SendCC(int ctl, int value, int channel = -1);
   void SendAftertouch(int pressure, int channel = -1);
   void SendPitchBend(int bend, int channel = -1);
   void SendData(unsigned char a, unsigned char b, unsigned char c);
   
private:
   void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) override;
   
   char mDeviceNameIn[64];
   char mDeviceNameOut[64];
   
   MidiOutput* mMidiOut;
   MidiDeviceListener* mListener;
   int mOutputChannel;
};

#endif /* defined(__additiveSynth__MidiDevice__) */
