
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

#ifndef _PHYSXCONNECTION_H_
#define _PHYSXCONNECTION_H_

#include <stdlib.h>
#include <iostream>
#include <pthread.h>

#include <tuiframework/core/HostMsgQueue.h>
#include <tuiframework/core/HostMsg.h>
#include <tuiframework/core/HostAddress.h>

#include <tuitypes/common/CommonTypeReg.h>

#ifdef _WIN32
#include <tuiframework/sockets/UDPReceiverWinSock2.h>
#include <tuiframework/sockets/UDPSenderWinSock2.h>
#else
#include <tuiframework/sockets/UDPReceiverSocket.h>
#include <tuiframework/sockets/UDPSenderSocket.h">
#endif

using namespace tuiframework;
using namespace std;



#ifdef LOCAL_DEBUG
    #define DST_PHY_ADR "127.0.0.1:60005" //Where is your physX-application running?
#else
    #define DST_PHY_ADR "192.168.0.2:60005" //Where is your physX-application running?
#endif
#define RECEIVE_PORT_DSPACE_COSIMDATA 51004

#define RECEIVE_PORT_PHYSX 60004


  /**
    * @class PhysXConnection
    * @brief Help to handle the connection with the co-simulation data
    *
    * Operates with the dSPACE unit in combination with a nVIDIA physX instance. 
    * Using one input and one output thread, to handel incoming and outgoing 
    * co-simulation to dSPACE simulation unit.
    *
    * @author Tommy Beckmann
    */

class PhysXConnection
{
public:
    PhysXConnection();
    ~PhysXConnection();
    bool startConnection();

    void setWheelParametersAndSend(double *drvTrq, double *brkTrq, double *strAngl, int wheelNums);
private:

    volatile bool outputLoopRunning;
    pthread_t outputLoopThread;
    volatile bool inputLoopRunning;
    pthread_t inputLoopThread;

    static void * inputLoopThread_run(void * arg);
    void executeInputLoop();

    static void * outputLoopThread_run(void * arg);
    void executeOutputLoop();

    dataPackageRecCosSim *send2physX;
    HostMsgQueue outHostMsgQueue;
    HostAddress *receiverHostAddress;

    dataSendCoSim *send2dSPACE;
    HostMsgQueue outDSPACEMsgQueue;
    HostAddress *DSPACEHostAddress;

    //receiver socket for dspace data
    SOCKET dataFromdSPACESocket;

    //receiver socket for data from physX
    SOCKET dataFromPhysX;

    void sendDataToPhysX();

    void initDSPACEsocket();
    void initPhysXSocket();

    void sendDataTodSPACE();

#ifdef _WIN32
    UDPSenderWinSock2 udpPhysXSenderSocket;
    UDPSenderWinSock2 udpdSPACESenderSocket;
#else
    UDPSenderSocket udpPhysXSenderSocket;
    UDPSenderSocket udpdSPACESenderSocket;
#endif

};

#endif //_PHYSXCONNECTION_H_
