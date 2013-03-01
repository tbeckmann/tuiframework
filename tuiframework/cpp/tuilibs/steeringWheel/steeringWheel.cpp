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

#include "steeringWheel.h"

namespace tuidevices
{

SteeringWheel::SteeringWheel(const DeviceConfig & deviceConfig)
{
    this->deviceDescriptor.setEntityID(deviceConfig.getEntityID());
    this->entityID = this->deviceDescriptor.getEntityID();
    printf("SteeringWheel - constructor started\n");
    // initialize steering Angle
    this->actSteerAngleDeg = 0.0;

    map<string, Port> portMap;
    // following the values for the xml connection with the complete racing unit
    // seperate them for single steering device integration (into PadelAndShift)
    portMap["accelPos"]     = Port("accelerationPos", "PedalSignalChannel", Port::Source);
    portMap["brakePos"]     = Port("brakePos", "PedalSignalChannel", Port::Source);
    portMap["clutchPos"]    = Port("clutchPos", "PedalSignalChannel", Port::Source);
    portMap["actGear"]      = Port("actGear", "UserInSignalChannel", Port::Source);
    portMap["button1"]      = Port("userButton_1", "UserInSignalChannel", Port::Source);
    portMap["steerAngle"]   = Port("steerAngle", "SteerSignalChannel", Port::Source);
    portMap["steerTorque"]  = Port("steerTorque", "SteerSignalChannel",Port::Sink);

    DeviceType deviceType;
    deviceType.setPortMap(portMap);
    deviceType.setDeviceTypeName(getDeviceName());
    deviceType.setDescription("Digital and Analog IO");
    this->deviceDescriptor.setDeviceType(deviceType);

    std::map<std::string, int> nameChannelNrMap;
    int nr = 1;
    map<string, Port>::iterator i = portMap.begin();
    map<string, Port>::iterator e = portMap.end();
    while (i != e) 
    {
        nameChannelNrMap[(*i).second.getName()] = nr;
        ++nr;
        ++i;
    }

    this->deviceDescriptor.setNameChannelNrMap(nameChannelNrMap);
}

SteeringWheel::~SteeringWheel(void)
{
}

// change this name for other steering device. e.g. "ZFRealSteeringWheel"
std::string SteeringWheel::getDeviceName() 
{
    return "LogitechG27";   //SteeringDevice
}

IDevice * SteeringWheel::createFunction(void * arg) 
{
    DeviceConfig * deviceConfig = static_cast<DeviceConfig *>(arg);
    return new SteeringWheel(*deviceConfig);
}

void SteeringWheel::deviceConnect(tuiframework::ITUIServer & tuiServer) 
{
    tuiServer.tuiServerRegisterDevice(this->entityID, *this);
    tuiServer.tuiServerRegisterEventSink(this->entityID, *this);
    this->tuiServer = &tuiServer;
}

void SteeringWheel::deviceDisconnect() 
{
    if (tuiServer) 
    {
        this->tuiServer->tuiServerDeregisterDevice(this->entityID);
        this->tuiServer->tuiServerDeregisterEventSink(this->entityID);
        this->tuiServer = 0;
    }
}

bool SteeringWheel::deviceExecute() 
{
    {
        int rc = pthread_create(&this->outputLoopThread, NULL, SteeringWheel::outputLoopThread_run, this);
        if (rc) 
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }

    {
        int rc = pthread_create(&this->inputLoopThread, NULL, SteeringWheel::inputLoopThread_run, this);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }
    return true;
}

void SteeringWheel::deviceStop() 
{
}

void SteeringWheel::push(tuiframework::IEvent * event) 
{
    this->outputEventQueue.push(event);
}

void SteeringWheel::deviceSetEventSink(tuiframework::IEventSink * eventSink) {
    this->eventSink = eventSink;
}

const DeviceDescriptor & SteeringWheel::getDeviceDescriptor() const {
    return this->deviceDescriptor;
}

void * SteeringWheel::outputLoopThread_run(void * arg) {
    printf("SteeringWheel - output loop thread started\n");
    SteeringWheel * myWh = static_cast<SteeringWheel *>(arg);
    myWh->executeOutputLoop();
    return 0;
}

void SteeringWheel::executeOutputLoop() {
    this->outputLoopRunning = true;
    while (outputLoopRunning) 
    {
    // write your code for the output loop here
    }
}

void * SteeringWheel::inputLoopThread_run(void * arg) 
{
    printf("SteeringWheel - input loop thread started\n");
    SteeringWheel * steerDevice = static_cast<SteeringWheel *>(arg);
    steerDevice->executeInputLoop();
    return 0;
}

void SteeringWheel::executeInputLoop() 
{
    this->inputLoopRunning = true;
    while (inputLoopRunning) 
    {
        steerAngleQueue.waitForData();
        //New steerAngle arrived
        double *incommingData = steerAngleQueue.pop();
        this->actSteerAngleDeg = *incommingData;
        delete incommingData;
        createDoubleChangedEvent(this->actSteerAngleDeg,"steerAngle");
    }
}

void SteeringWheel::setSteerAngle(double newAngleDeg)
{
    double *manualAngle = new double(newAngleDeg);
    steerAngleQueue.push(manualAngle);
}

void SteeringWheel::createDoubleChangedEvent(double value, std::string name)
{
    DoubleChangedEvent * event = new DoubleChangedEvent();
    event->setAddress(EPAddress(this->tuidevices::SteeringWheel::entityID, this->tuidevices::SteeringWheel::deviceDescriptor.getNameChannelNrMap()["steerAngle"]));
    event->setPayload(static_cast<double>(value));
    //push event
    SteeringWheel::eventSink->push(event);
}

} //namespace tuidevices 