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

#ifndef _tuidevices_FestoAirmotionRide_h_
#define _tuidevices_FestoAirmotionRide_h_

//core includes
#include <tuiframework/core/EventQueue.h>
#include <tuiframework/core/IDevice.h>
#include <tuiframework/core/IEventSink.h>
#include <tuiframework/core/HostMsgQueue.h>
#include <tuiframework/core/HostMsg.h>
#include <tuiframework/core/ITUIServer.h>
#include <tuiframework/core/IEvent.h>
#include <tuiframework/core/SerializedDataQueue.h>
#include <tuiframework/core/HostMsgDispatcher.h>
#include <tuiframework/core/EventSerializer.h>

//Tommy, todo declare a new event in here:
#include <tuiframework/core/TypeRegistration.h>


#include <tuiframework/server/DeviceDescriptor.h>
#include <tuiframework/server/DeviceConfig.h>
#include <tuitypes/common/CommonTypeReg.h>

#ifdef _WIN32
#include <tuiframework/sockets/UDPReceiverWinSock2.h>
#include <tuiframework/sockets/UDPSenderWinSock2.h>
#else
#include <tuiframework/sockets/UDPReceiverSocket.h>
#include <tuiframework/sockets/UDPSenderSocket.h">
#endif


#include <pthread.h>

using namespace tuiframework;
using namespace std;

#define USELABVIEW_HEXAPOD     // undefine this flag when the FestoKinematicChannel is ready to use

#ifdef USELABVIEW_HEXAPOD
#define SRC_PORT 30000  //sourcerPort from dSPACE
#define DST_PORT 59999  //destination port for labView app
//#define DST_ADR "127.0.0.1:59999"    //LabView app
#define DST_ADR "192.168.0.2:59999"    //LabView app
#define SRC_ADR  "127.0.0.1:995"  //test
#define PACKAGE_SIZE_LABVIEW 32
#else
#define SRC_PORT  994  //src port needed to communicate with festos SPS
#define DST_PORT 995  //destination port used by festos SPS
#define BUF_SIZE 32    //PackageSize for raw data
#define DST_ADR "192.168.0.1:994"    //FESTO IP
#define SRC_ADR  "192.168.0.2:995"  //PC IP to send (Set also to global system settings!)
#endif

// uncomment the following flag to recieve your motion data direct from physX
//#define RECEIVE_DATA_PHYSX 

#ifdef RECEIVE_DATA_PHYSX
    #define RECEIVER_PORT   60006
    #include <winsock2.h>
#endif


namespace tuidevices
{

  /**
    * @class FestoAirmotionRide
    * @brief TUI-Object for the festo motion platform
    *
    *   Befor you start with this files, make sure that you are
    *   familiar with the design of the tuiframework.
    *
    * @author Tommy Beckmann
    */
class FestoAirmotionRide : public tuiframework::IDevice, public tuiframework::IEventSink {

public:
    FestoAirmotionRide(const tuiframework::DeviceConfig & deviceConfig);
    virtual ~FestoAirmotionRide(void);
    //Beware: name == DeviceInstance typename in xml-config!
    static std::string getDeviceName();

    static IDevice * createFunction(void * arg);

    // IDevice
    virtual void deviceConnect(tuiframework::ITUIServer & tuiServer);
    virtual void deviceDisconnect();
    virtual bool deviceExecute();
    virtual void deviceStop();
    virtual void deviceSetEventSink(tuiframework::IEventSink * eventSink);
    virtual const tuiframework::DeviceDescriptor & getDeviceDescriptor() const;
    // IEventSink
    virtual void push(tuiframework::IEvent * event);

protected:
    tuiframework::DeviceDescriptor deviceDescriptor;
    int entityID;
    tuiframework::ITUIServer * tuiServer;
    tuiframework::IEventSink * eventSink;
    
    volatile bool outputLoopRunning;
    pthread_t outputLoopThread;
    volatile bool inputLoopRunning;
    pthread_t inputLoopThread;

    static void * inputLoopThread_run(void * arg);
    void executeInputLoop();

    static void * outputLoopThread_run(void * arg);
    void executeOutputLoop();

    tuiframework::EventQueue outputEventQueue;

    HostMsgQueue outHostMsgQueue;
    HostAddress *receiverHostAddress;

    MotionData *motionData;
    void packLabViewPackage(char *toSend);

    //UDP sender socket for sps
#ifdef _WIN32
    UDPSenderWinSock2 udpSPSSenderSocket;
#else
    UDPSenderSocket udpSPSSenderSocket;
#endif

#ifdef RECEIVE_DATA_PHYSX
    SOCKET inputSocket;
    void convertBufferToValidMotionData(char *buf, int size);
#endif

};

}   //namespace tuidevices

#endif  // _tuidevices_FestoAirmotionRide_h_