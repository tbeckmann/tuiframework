/*
    Copyright (C) 2010, 2011, 2012 The Fraunhofer Institute for Production Systems and
    Design Technology IPK. All rights reserved.

    This file is part of the TUIFramework library.
    It includes a software framework which contains common code
    providing generic functionality for developing applications
    with a tangible user interface (TUI).
    
    The TUIFramework library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    The TUIFramework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TUIFramework.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _tuidevices_PedalAndShift_h_
#define _tuidevices_PedalAndShift_h_

#include <tuiframework/core/EventQueue.h>
#include <tuiframework/core/IDevice.h>
#include <tuiframework/core/IEventSink.h>
#include <tuiframework/server/DeviceDescriptor.h>
#include <tuitypes/common/Gesture.h>
#include <tuiframework/core/ITUIServer.h>
#include <tuiframework/core/IEvent.h>
#include <tuiframework/server/DeviceConfig.h>
#include <tuitypes/common/CommonTypeReg.h>

#include <tuiframework/core/SerializedDataQueue.h>
#ifdef _WIN32
#include <tuiframework/sockets/UDPReceiverWinSock2.h>
#else
#include <tuiframework/sockets/UDPReceiverSocket.h>
#endif

#include <pthread.h>


using namespace tuiframework;
using namespace std;
 
namespace tuidevices
{
  /**
    * @class PedalAndShift
    * @brief Represents the user input for pedal and shfit unit
    *
    * Contains pedal positions, desired gear and userButtons. Creates an event
    * if an value changed.
    * For further details refer to the tuiframework documentation.
    *
    *   @author Tommy Beckmann
    */
class PedalAndShift : public tuiframework::IEventSink    //TODO: public tuiframework::IDevice
{
public:
    PedalAndShift(const tuiframework::DeviceConfig & deviceConfig);
    virtual ~PedalAndShift(void);
    //declare the following methode in your CHILD-class! Beware: name == DeviceInstance typename in xml-config!
   // static std::string getDeviceName();

    //static IDevice * createFunction(void * arg);

    // IDevice TODO uncomment if multiple inheritance from logitech g27 is no longer needed!
/*
    virtual void deviceConnect(tuiframework::ITUIServer & tuiServer);
    virtual void deviceDisconnect();
    virtual bool deviceExecute();
    virtual void deviceStop();
    virtual void deviceSetEventSink(tuiframework::IEventSink * eventSink);
    virtual const tuiframework::DeviceDescriptor & getDeviceDescriptor() const;
*/
    // IEventSink
    virtual void push(tuiframework::IEvent * event);

protected:
    tuiframework::DeviceDescriptor deviceDescriptor;
    int entityID;
    tuiframework::ITUIServer * tuiServer;
    tuiframework::IEventSink * eventSink;
    
    // TODO uncomment if multiple inheritance from logitech g27 is no longer needed!
    /*
    volatile bool outputLoopRunning;
    pthread_t outputLoopThread;
    volatile bool inputLoopRunning;
    pthread_t inputLoopThread;

    static void * inputLoopThread_run(void * arg);
    void executeInputLoop();

    static void * outputLoopThread_run(void * arg);
    void executeOutputLoop();
    */

    tuiframework::EventQueue outputEventQueue;

    Percent accelPercent;
    Percent brakePercent;
    Percent clutchPercent;
    int actGear;

private:
    virtual void createPercentChangedEvent(Percent value, std::string name);

};
} // namspace tuidevices

#endif //_tuidevices_PedalAndShift_h_