/*
    Copyright (C) 2010, 2011, 2012 The Fraunhofer Institute for Production Systems and
    Design Technology IPK. All rights reserved.

    This file is part of the TUIFramework library.
    It includes a software framework which contains common code
    providing generic functionality for developing applications
    with a tangible user interface (TUI).
    designed for creating applications with a tangible user interface (TUI).
    
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

#ifndef _ModelRepresentationObject_h_
#define _ModelRepresentationObject_h_

#include <tuiframework/client/client.h>
#include <tuitypes/common/CommonTypeReg.h>

//core includes
#include <tuiframework/core/HostMsgQueue.h>
#include <tuiframework/core/HostMsg.h>
#include <tuiframework/core/Queue.h>
#include <tuiframework/core/SerializedDataQueue.h>

//data structures for dSPACE udp communication
#include <tuitypes/common/dSPACECommunication.h>

//connection to physX
#include "phsXConnection.h"


#ifdef _WIN32
#include <tuiframework/sockets/UDPReceiverWinSock2.h>
#include <tuiframework/sockets/UDPSenderWinSock2.h>
#else
#include <tuiframework/sockets/UDPReceiverSocket.h>
#include <tuiframework/sockets/UDPSenderSocket.h">
#endif

using namespace tuiframework;
using namespace std;

#define TUI_PACKAGE_OVERHEAD 22 // [Byte], overhead IP,Port and Event from recieved packages

#define DEBUG_OUTPUT

  /**
    *   @class ModelRepresentationObject
    *   @brief Represents the interfact to your model device
    *
    *   Operates with the dSPACE unit. Using one input and one output thread,
    *   to handel incoming and outgoing data to dSPACE simulation unit.
    *
    * @author Tommy Beckmann
    */
class ModelRepresentationObject {
public:
    ModelRepresentationObject();
    virtual ~ModelRepresentationObject(void);
    //tui client methods:
    void connect();
    void disconnect();

    void setCenterOfGravity(double xVal, double yVal, double zVal);
    bool executeThreads();

protected:
    //methods for dSPACE connection:
  /** @fn void executeOutputLoop()
    * @brief establish a thread for data from client to dSPACE
    */
    void executeOutputLoop(); 
    static void * outputThread_run(void * arg);
    volatile bool outputLoopRunning;
    pthread_t outputLoopThread;

  /** @fn void executeOutputLoop()
    * @brief establish a thread for data from dSPACE to client
    */
    void executeInputLoop(); 
    static void * inputThread_run(void * arg);
    volatile bool inputLoopRunning;
    pthread_t inputLoopThread; 

    //event reaction:
    void steerAngleChanged(const DoubleChangedEvent * e);
    void accelPosChanged(const PercentChangedEvent * e);
    void brakePosChanged(const PercentChangedEvent * e);
    void clutchPosChanged(const PercentChangedEvent * e);
    void gearChanged(const IntegerChangedEvent * e);
    void buttonChanged(const IntegerChangedEvent * e);
    // MsgQueue for UDP sender socket
    HostMsgQueue outHostMsgQueue;
    SerializedDataQueue inSerializedDataQueue;

    //UDP sender socket for sps
#ifdef _WIN32
    UDPSenderWinSock2 udpSenderSocket;
    UDPReceiverWinSock2 udpFestoReceiverSocket;
#else
    UDPSenderSocket udpSenderSocket;
    UDPReceiverSocket udpFestoReceiverSocket;
#endif

private:
    double *CoGPosWoldXYZ;   // center of gravity vehicle in world coordinates
    HostAddress dSPACEHostAddress;
    HostMsg *dataPackage;
    char *dataFieldToSend;
    tuiframework::IEventChannel *festoChannel;  //recognize motion data change

    //Data structs to send
    dataPackageSend *valuesTodSPACE;
    dataSendCoSim *coSimVal;

    //Data structs to receive
    MotionData *motionData;

    Queue<dataPackageSend> dSPACESendQueue;
    //private functions
    void sendDataTodSPACE();

    //physX connection representation
    PhysXConnection *connect2PhysX;

};

#endif //_MySteeringObject_h_
