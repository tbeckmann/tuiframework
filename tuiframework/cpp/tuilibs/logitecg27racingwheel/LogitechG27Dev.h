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

#ifndef _tuiframework_LogitecG27RacingUnit_h_
#define _tuiframework_LogitecG27RacingUnit_h_

#include <iostream>
#include <steeringWheel/steeringWheel.h>
#include <pedalAndShift/pedalAndShift.h>
#include <stdlib.h>

#include <tuiframework/core/HostMsgQueue.h>
#include <tuiframework/core/HostMsg.h>

#ifdef _WIN32
#include <tuiframework/sockets/UDPReceiverWinSock2.h>
#include <tuiframework/sockets/UDPSenderWinSock2.h>
#else
#include <tuiframework/sockets/UDPReceiverSocket.h>
#include <tuiframework/sockets/UDPSenderSocket.h">
#endif

#include "..\..\tuilibs\logitecg27racingwheel\LogitechG27Dev.h"
#include <logitecg27racingwheel\SteeringWheelSDKFiles\inc\LogiWheel.h>
#include  <logitecg27racingwheel\SteeringWheelSDKFiles\inc\LogiControllerInput.h>
#include "..\..\tuilibs\pedalAndShift\pedalAndShift.h"

#include <cstdio> // GCC 4.3 related build problem

using namespace LogitechSteeringWheel;
using namespace LogitechControllerInput;
using namespace std;

#define WHEEL_INDEX     0       // logitech internal device ID, constant for now
#define FORCE_ENABLE    TRUE    // turn force feedback on / off
#define WHEEL_RANGE     900     // [deg]
#define RAW_VAL_MAX     65536   // given by logitec SDK

#define CYC_THREADT_MS  10  // give the ms to repeate your output und input loop thread
#define PEDAL_NUM       3   // how many pedals are needed? 
#define BUTTON_NUM      23  // how many buttons are checked

#define BUTTON0 4    // gear down button
#define BUTTON1 3    // gear up button
#define BUTTON8 7    //first red user button on steering wheel

#define DEBUG_OUTPUT //decide if you like to see debug information on your std::out

//define this flag to send data to our physX sample with the car driving parcour
//#define SEND_DATA_TOPHYSX_SAMPLE

#ifdef SEND_DATA_TOPHYSX_SAMPLE

#define DSTI_ADR "127.0.0.1:60005"
#define PACKAGE_SIZE_PHYSX    96 + 8*3
    typedef struct physX {
        float acceleration;
        float brake;
        float steer;
        bool handBrake;
        bool gearUp;
        bool gearDown;
    }sendDataToPhysX;

#endif

namespace tuidevices {
  /**
    * @class LogitecG27RacingUnit
    * @brief Represents the interfact to gaming devices from logitech
    *
    * Operates with the logitech "SteeringWheelSDK" files and a working directX
    * installation. Befor you start with this files, make sure that you are
    * familiar with the design of the tuiframework.
    *  For further details refer to the logitech SDK documentation.
    *
    * @author Tommy Beckmann
    */
    class LogitecG27RacingUnit : public SteeringWheel, public PedalAndShift{
public:
    // functions for tui integration:
    LogitecG27RacingUnit(const tuiframework::DeviceConfig & deviceConfig);
    virtual ~LogitecG27RacingUnit();
    static IDevice * createFunction(void * arg);
    bool deviceExecute();
    // The getDeviceName() function is implemented in the steeringWheel.h
    // Change the static name there for new steering device integration!
    // static std::string getDeviceName();

  /**
    * @fn void playLEDdemo()
    * @brief Flash the drive LED´s on your wheel for demonstration.
    */
    void playLEDdemo();

protected:
    // functions for tui integration:
    void executeOutputLoop(); 
    static void * outputLoopThread_run(void * arg);

    void executeInputLoop(); 
    static void * inputLoopThread_run(void * arg);


  /**
    * @fn void initWheel()
    * @brief Set preferred wheel settings.
    */
    void initWheel();

private:
    Wheel *in_wheel;
    ControllerInput *in_controllerInput;
    void setWheelStdPreferredProperties();
    int steerRawValue;  //raw input from steering Wheel 
    int pedalRawValue[PEDAL_NUM];
    double pedalPercentVal[PEDAL_NUM];
    BYTE buttonValues[BUTTON_NUM];

  /**
    * @fn void createDoubleChangedEvent()
    * @brief creates an change event with the new value for responding name.
    */
    void createDoubleChangedEvent(double value, std::string name);
    void createPercentChangedEvent(Percent value, std::string name);

#ifdef SEND_DATA_TOPHYSX_SAMPLE
    sendDataToPhysX send2physX;
    HostMsgQueue outHostMsgQueue;
    HostAddress *receiverHostAddress;

    void setPhysXData(float accel, float brake, float steer, bool steerChange);
    void setPhysXButton(bool handB, bool gUp, bool gDown);

    void sendDataToPhysX();

    #ifdef _WIN32
        UDPSenderWinSock2 udpPhysXSenderSocket;
    #else
        UDPSenderSocket udpPhysXSenderSocket;
    #endif
#endif

};

} //namespace tuidevices

#endif   //_tuiframework_LogitecG27RacingUnit_h_