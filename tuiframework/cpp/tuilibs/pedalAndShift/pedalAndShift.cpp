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

#include "pedalAndShift.h"

namespace tuidevices
{

PedalAndShift::PedalAndShift(const DeviceConfig & deviceConfig)
{
}

PedalAndShift::~PedalAndShift(void)
{
}

/* uncomment following section for pedal and shift device integration:
std::string PedalAndShift::getDeviceName() 
{
    return "PedalAndShiftDevice";
}
*/
/*
IDevice * PedalAndShift::createFunction(void * arg) 
{
    DeviceConfig * deviceConfig = static_cast<DeviceConfig *>(arg);
    return new PedalAndShift(*deviceConfig);
}
*/

//uncomment if multiple inheritance from logitech g27 is no longer needed!
/*
void PedalAndShift::deviceConnect(tuiframework::ITUIServer & tuiServer) 
{
    tuiServer.tuiServerRegisterDevice(this->entityID, *this);
    tuiServer.tuiServerRegisterEventSink(this->entityID, *this);
    this->tuiServer = &tuiServer;
}

void PedalAndShift::deviceDisconnect() 
{
    if (tuiServer) 
    {
        this->tuiServer->tuiServerDeregisterDevice(this->entityID);
        this->tuiServer->tuiServerDeregisterEventSink(this->entityID);
        this->tuiServer = 0;
    }
}

bool PedalAndShift::deviceExecute() 
{
    {
        int rc = pthread_create(&this->outputLoopThread, NULL, PedalAndShift::outputLoopThread_run, this);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }

    {
        int rc = pthread_create(&this->inputLoopThread, NULL, PedalAndShift::inputLoopThread_run, this);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }
    return true;
}

void PedalAndShift::deviceStop() 
{
}

void PedalAndShift::deviceSetEventSink(tuiframework::IEventSink * eventSink)
{
    this->eventSink = eventSink;
}

const DeviceDescriptor & PedalAndShift::getDeviceDescriptor() const
{
    return this->deviceDescriptor;
}

void * PedalAndShift::outputLoopThread_run(void * arg)
{
    printf("PedalAndShift - output loop thread started\n");
    PedalAndShift * myWh = static_cast<PedalAndShift *>(arg);
    myWh->executeOutputLoop();
    return 0;
}
*/

void PedalAndShift::createPercentChangedEvent(Percent value, std::string name)
{
    //ToDo: Create your change event here
    //push event
    //tuidevices::eventSink->push(event);
}

void PedalAndShift::push(tuiframework::IEvent * event) {
    this->outputEventQueue.push(event);
}

} //namespace tuidevices