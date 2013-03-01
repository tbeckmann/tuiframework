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

#include "LogitechG27Dev.h"

namespace tuidevices {

LogitecG27RacingUnit::LogitecG27RacingUnit(const tuiframework::DeviceConfig & deviceConfig)
:SteeringWheel(deviceConfig),
PedalAndShift(deviceConfig)
#ifdef SEND_DATA_TOPHYSX_SAMPLE
    ,udpPhysXSenderSocket(outHostMsgQueue)
#endif 
{
    //init value before connection the unit
    this->actSteerAngleDeg  = -1000;
    this->steerRawValue = 0;
    for(int i=0; i < PEDAL_NUM; i++)
    {
        this->pedalRawValue[i]=0;
        this->pedalPercentVal[i]=-1;
    }
    for(int i=0; i < BUTTON_NUM; i++)
    {
        this->buttonValues[i] = false;
    }

    //Create a dummy window (necessary for logitech driver SDK)
    HWND dummyHWND =    ::CreateWindowA("STATIC","dummy",WS_DISABLED,0,0,100,100,NULL,NULL,NULL,NULL);
                        ::SetWindowTextA(dummyHWND,"Logitech Wheel Dummy");

    this->in_controllerInput = new ControllerInput(dummyHWND, TRUE); // TRUE means that XInput devices will be ignored
    this->in_wheel = new Wheel(in_controllerInput);
    //Check connection and inform the user:
    if(in_wheel->IsConnected(WHEEL_INDEX))
    {
        initWheel();
        cout << "Wheel successfully initialized" << endl;
    }
    else
    {
        cout << "Warning, wheel G27 not found -> output loop will not be started!\nPress enter Key to continue..." << endl;
        getchar();
    }

#ifdef SEND_DATA_TOPHYSX_SAMPLE
    receiverHostAddress = new HostAddress(DSTI_ADR);
    this->udpPhysXSenderSocket.create();
    send2physX.acceleration = 0.0f;
    send2physX.brake = 0.0f;
    send2physX.steer = 0.0f;
    send2physX.handBrake = false;
    send2physX.gearUp = false;
    send2physX.gearDown = false;
#endif

}

LogitecG27RacingUnit::~LogitecG27RacingUnit()
{
}

/* refer to basic class in steeringWheel.cpp 
std::string LogitecG27RacingUnit::getDeviceName() {
    return "LogitechG27";
}
*/


IDevice * LogitecG27RacingUnit::createFunction(void * arg) 
{
    DeviceConfig * deviceConfig = static_cast<DeviceConfig *>(arg);
    return new LogitecG27RacingUnit(*deviceConfig);
}

void LogitecG27RacingUnit::initWheel()
{
    setWheelStdPreferredProperties();
}

void LogitecG27RacingUnit::setWheelStdPreferredProperties()
{
    ControllerPropertiesData propertiesData_;
    ZeroMemory(&propertiesData_, sizeof(propertiesData_));

    propertiesData_.allowGameSettings = TRUE;
    propertiesData_.combinePedals = FALSE;
    propertiesData_.damperGain = 100;
    propertiesData_.defaultSpringEnabled = TRUE;
    propertiesData_.defaultSpringGain = 100;
    propertiesData_.forceEnable = FORCE_ENABLE;
    propertiesData_.gameSettingsEnabled = FALSE;
    propertiesData_.overallGain = 100;
    propertiesData_.springGain = 100;
    propertiesData_.wheelRange = WHEEL_RANGE;

    in_wheel->SetPreferredControllerProperties(propertiesData_);
}

bool LogitecG27RacingUnit::deviceExecute() 
{
    {
        int rc = pthread_create(&this->SteeringWheel::outputLoopThread, NULL, LogitecG27RacingUnit::outputLoopThread_run, this);
        if (rc) 
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }

    {
        int rc = pthread_create(&this->SteeringWheel::inputLoopThread, NULL, LogitecG27RacingUnit::inputLoopThread_run, this);
        if (rc) 
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }
return true;
}

void * LogitecG27RacingUnit::outputLoopThread_run(void * arg) 
{
    printf("RacingWheel - output loop thread started\n");
    LogitecG27RacingUnit * steerDevice = static_cast<LogitecG27RacingUnit *>(arg);
    //Only start the loop if wheel is connected
    if(steerDevice->in_wheel->IsConnected(WHEEL_INDEX))
    {
        steerDevice->LogitecG27RacingUnit::executeOutputLoop();
    }
    else // wheel not connected
    {
        return 0;
    }
    return 0;
}

void LogitecG27RacingUnit::executeOutputLoop() 
{
    this->outputLoopRunning = true;
    int oldSteerRawValue=-1; 
    int oldPedalRawValue[3];
    for(int i=0; i < PEDAL_NUM; i++)
    {//initialize temp values
        oldPedalRawValue[i]=-1;
    }

    BYTE oldButtonVals[BUTTON_NUM];
    for(int i=0; i < BUTTON_NUM; i++)
    {//initialize temp values
        oldButtonVals[i] = 254;
    }
    const char *pedEventNames[] = {"accelerationPos", "brakePos", "clutchPos"};

    while(this->outputLoopRunning)
    {
        in_controllerInput->Update();
        in_wheel->Update();
        // get linear values for steer angle
        steerRawValue = in_wheel->GetState(WHEEL_INDEX)->lX; 
        //get pedal values
        this->pedalRawValue[0] = in_wheel->GetState(WHEEL_INDEX)->lY;
        this->pedalRawValue[1] = in_wheel->GetState(WHEEL_INDEX)->lRz;
        this->pedalRawValue[2] = in_wheel->GetState(WHEEL_INDEX)->rglSlider[1];
        //get button Values
        for(int i=0; i < BUTTON_NUM; i++)
        {
            this->buttonValues[i] = in_wheel->GetState(WHEEL_INDEX)->rgbButtons[i];
        }

        if( this->steerRawValue != oldSteerRawValue)  //send event only on change
        {
            //convert raw value in degree
            this->actSteerAngleDeg = - static_cast<double>(this->steerRawValue) * static_cast<double>((WHEEL_RANGE / 2)) / (RAW_VAL_MAX / 2);
            createDoubleChangedEvent(this->actSteerAngleDeg, "steerAngle");
        #ifdef SEND_DATA_TOPHYSX_SAMPLE
            setPhysXData(this->pedalPercentVal[0], this->pedalPercentVal[1], this->actSteerAngleDeg, true);
        #endif
        #ifdef DEBUG_OUTPUT
            cout << "steering Wheel x-Koor:" << this->actSteerAngleDeg << "\t raw: " << steerRawValue << endl;
        #endif
        }
        else
        {
            Sleep(1);
        }

        //check the pedals for new values
        for(int i=0; i < PEDAL_NUM; i++)
        {
            if( this->pedalRawValue[i] != oldPedalRawValue[i] )
            {
                this->pedalPercentVal[i] = -(100 * this->pedalRawValue[i]) / (RAW_VAL_MAX) + 50.0;
                    if(this->pedalPercentVal[i] < 1.1) //set a hysteresis
                    {
                        this->pedalPercentVal[i] = 0.0;
                    }
                Percent tempVal(this->pedalPercentVal[i], false);
                createPercentChangedEvent(tempVal, pedEventNames[i]);
            #ifdef SEND_DATA_TOPHYSX_SAMPLE
                setPhysXData(this->pedalPercentVal[0], this->pedalPercentVal[1], this->actSteerAngleDeg, false);
            #endif
            #ifdef DEBUG_OUTPUT
                cout << "steering Wheel Pedl.No: " << i << " " << "\t raw: " << this->pedalRawValue[i] << "\t percent: " << this->pedalPercentVal[i] << endl;
            #endif
            }
            else
            {
                Sleep(1);
            }
        }

        //check the buttons for new values
        for(int i=0; i < BUTTON_NUM; i++)
        {
            if(this->buttonValues[i] != oldButtonVals[i])
            { //Button Value changed
                cout << "Button pressed " << this->buttonValues[i] << " " << i << endl;
#ifdef SEND_DATA_TOPHYSX_SAMPLE
                //send button values to PhysXSample
                if(i == BUTTON8 && this->buttonValues[i])
                {
                    setPhysXButton(true, false, false);
                }
                else if(i == BUTTON8 && !this->buttonValues[i])
                {
                    setPhysXButton(false, false, false);
                }
                if(i == BUTTON0 && this->buttonValues[i])
                {
                    setPhysXButton(true, false, true);
                }
                else if(i == BUTTON0 && !this->buttonValues[i])
                {
                    setPhysXButton(false, false, false);
                }
                if(i == BUTTON1 && this->buttonValues[i])
                {
                    setPhysXButton(false, true, true);
                }
                else if(i == BUTTON1 && !this->buttonValues[i])
                {
                    setPhysXButton(false, false, false);
                }
#endif
            }
        }

        //update values
        oldSteerRawValue = this->steerRawValue;
        for(int i=0; i < PEDAL_NUM; i++)
        {
            oldPedalRawValue[i] = this->pedalRawValue[i];
        }
        for(int i=0; i < BUTTON_NUM; i++)
        {
            oldButtonVals[i] = this->buttonValues[i];
        }

        Sleep(CYC_THREADT_MS);

    }
}

void LogitecG27RacingUnit::createDoubleChangedEvent(double value, std::string name)
{
    DoubleChangedEvent * event = new DoubleChangedEvent();
    event->setAddress(EPAddress(this->tuidevices::SteeringWheel::entityID, this->tuidevices::SteeringWheel::deviceDescriptor.getNameChannelNrMap()["steerAngle"]));
    event->setPayload(static_cast<double>(value));
#ifdef DEBUG_OUTPUT
    cout << "This is OutputThread - DoubleChangedEvent dropped! MyID: " << this->tuidevices::SteeringWheel::entityID << "\t" << this->tuidevices::SteeringWheel::deviceDescriptor.getNameChannelNrMap()["steerAngle"] << endl;
#endif
    //push event
    SteeringWheel::eventSink->push(event);
}

void LogitecG27RacingUnit::createPercentChangedEvent(Percent value, std::string name)
{
    PercentChangedEvent *event = new PercentChangedEvent();
    event->setAddress(EPAddress(this->tuidevices::SteeringWheel::entityID, this->tuidevices::SteeringWheel::deviceDescriptor.getNameChannelNrMap()[name]));
    event->setPayload(static_cast<Percent>(value) );
    //push event
    tuidevices::SteeringWheel::eventSink->push(event);
}

void * LogitecG27RacingUnit::inputLoopThread_run(void * arg)
{
    printf("RacingWheel- input loop thread started\n");
    LogitecG27RacingUnit * steerDevice = static_cast<LogitecG27RacingUnit *>(arg);
    //Only start the loop if wheel is connected
    if(steerDevice->in_wheel->IsConnected(WHEEL_INDEX))
    {
        steerDevice->LogitecG27RacingUnit::executeInputLoop();
    }
    else // wheel not connected
    {
        return 0;
    }
    return 0;
}

void LogitecG27RacingUnit::executeInputLoop()
{
    this->inputLoopRunning = true;
    while(this->inputLoopRunning)
    {
        //Write your input loop in here
        //TODO: force goes in!
    }
}

void LogitecG27RacingUnit::playLEDdemo()
{
    in_wheel->PlayLeds(WHEEL_INDEX,1000,100,900);
    in_controllerInput->Update();
    in_wheel->Update();
}

#ifdef SEND_DATA_TOPHYSX_SAMPLE
void LogitecG27RacingUnit::setPhysXData(float accel, float brake, float steer, bool steerChange)
{
    //make sure the values are between 0.0f --- 1.0f respective -1.0f --- 1.0f
    if(steerChange)
    {
        send2physX.steer = steer/480;
    }
    else
    {
        send2physX.acceleration = accel/100;
        send2physX.brake = brake/100;
    }
    sendDataToPhysX();
}

void LogitecG27RacingUnit::setPhysXButton(bool handB, bool gUp, bool gDown)
{
    send2physX.handBrake = handB;
    send2physX.gearUp = gUp;
    send2physX.gearDown = gDown;
    sendDataToPhysX();
}

void LogitecG27RacingUnit::sendDataToPhysX()
{
    //send data
    HostMsg *motionMsg;
    motionMsg = new HostMsg(*receiverHostAddress, (char*) &send2physX, PACKAGE_SIZE_PHYSX);
    motionMsg->setHostAddress(*receiverHostAddress);
    this->outHostMsgQueue.push(motionMsg);
#ifdef DEBUG_OUTPUT
    cout << "sended to physx" << endl;
#endif
}
#endif

} //namespace tuidevices
