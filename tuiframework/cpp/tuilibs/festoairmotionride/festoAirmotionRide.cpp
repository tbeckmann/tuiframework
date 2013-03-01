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

#include "festoAirmotionRide.h"

namespace tuidevices
{

FestoAirmotionRide::FestoAirmotionRide(const DeviceConfig & deviceConfig):
udpSPSSenderSocket(outHostMsgQueue)
{
    this->deviceDescriptor.setEntityID(deviceConfig.getEntityID());
    this->entityID = this->deviceDescriptor.getEntityID();
    printf("festoAirmotionRide - constructor started\n");

    map<string, Port> portMap;

#ifdef USELABVIEW_HEXAPOD
    // following the values for the xml connection with the LabView vi
    portMap["alpha"] = Port("alpha", "FestoLabViewChannel", Port::Sink);
    portMap["beta"] = Port("beta", "FestoLabViewChannel", Port::Sink);
    portMap["gamma"] = Port("gamma", "FestoLabViewChannel", Port::Sink);
    portMap["heave"] = Port("heave", "FestoLabViewChannel", Port::Sink);
    portMap["lateral"] = Port("lateral", "FestoLabViewChannel", Port::Sink);
    portMap["longitudinal"] = Port("longitudinal", "FestoLabViewChannel", Port::Sink);
#else
    //  use this for your own kinematic, running on your tui server
    portMap["actAngleDeg"] = Port("actAngleDeg[abg]", "FestoKinematicChannel", Port::Sink);
    portMap["actXposEuler"] = Port("actXposEuler[xyz]", "FestoKinematicChannel", Port::Sink);

    /* use this for seperate muscle contraction, e.g. kinematic running on dSPACE
    portMap["m1"] = Port("muscle1", "FestoContractionChannel", Port::Sink);
    portMap["m2"] = Port("muscle2", "FestoContractionChannel", Port::Sink);
    portMap["m3"] = Port("muscle3", "FestoContractionChannel", Port::Sink);
    portMap["m4"] = Port("muscle4", "FestoContractionChannel", Port::Sink);
    portMap["m5"] = Port("muscle5", "FestoContractionChannel", Port::Sink);
    portMap["m6"] = Port("muscle6", "FestoContractionChannel", Port::Sink);
    */
#endif

    motionData = new MotionData;
    receiverHostAddress = new HostAddress(DST_ADR);

    DeviceType deviceType;
    deviceType.setPortMap(portMap);
    deviceType.setDeviceTypeName(getDeviceName());
    deviceType.setDescription("Festos airmotion ride device abstraction");
    this->deviceDescriptor.setDeviceType(deviceType);

    std::map<std::string, int> nameChannelNrMap;
    int nr = 1;
    map<string, Port>::iterator i = portMap.begin();
    map<string, Port>::iterator e = portMap.end();
    while (i != e) {
        nameChannelNrMap[(*i).second.getName()] = nr;
        ++nr;
        ++i;
    }

    this->deviceDescriptor.setNameChannelNrMap(nameChannelNrMap); 
}

FestoAirmotionRide::~FestoAirmotionRide(void)
{
}

std::string FestoAirmotionRide::getDeviceName() 
{
    return "FESTOAirmotionRide";
}

IDevice * FestoAirmotionRide::createFunction(void * arg) 
{
    DeviceConfig * deviceConfig = static_cast<DeviceConfig *>(arg);
    return new FestoAirmotionRide(*deviceConfig);
}

void FestoAirmotionRide::deviceConnect(tuiframework::ITUIServer & tuiServer) 
{
    tuiServer.tuiServerRegisterDevice(this->entityID, *this);
    tuiServer.tuiServerRegisterEventSink(this->entityID, *this);
    this->tuiServer = &tuiServer;
}

void FestoAirmotionRide::deviceDisconnect() 
{
    if (tuiServer) 
    {
        this->tuiServer->tuiServerDeregisterDevice(this->entityID);
        this->tuiServer->tuiServerDeregisterEventSink(this->entityID);
        this->tuiServer = 0;
    }
}

bool FestoAirmotionRide::deviceExecute() 
{
    {
        int rc = pthread_create(&this->outputLoopThread, NULL, FestoAirmotionRide::outputLoopThread_run, this);
        if (rc) 
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }

    {
        int rc = pthread_create(&this->inputLoopThread, NULL, FestoAirmotionRide::inputLoopThread_run, this);
        if (rc) 
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }
    return true;
}


void FestoAirmotionRide::deviceStop() 
{
}


void FestoAirmotionRide::push(tuiframework::IEvent * event) 
{
    this->outputEventQueue.push(event);
}

void FestoAirmotionRide::deviceSetEventSink(tuiframework::IEventSink * eventSink) 
{
    this->eventSink = eventSink;
}


const DeviceDescriptor & FestoAirmotionRide::getDeviceDescriptor() const 
{
    return this->deviceDescriptor;
}

void * FestoAirmotionRide::outputLoopThread_run(void * arg) 
{
    printf("FestoAirmotionRide - output loop thread started\n");
    FestoAirmotionRide * myFeschto = static_cast<FestoAirmotionRide *>(arg);
    myFeschto->executeOutputLoop();
    return 0;
}

void FestoAirmotionRide::executeOutputLoop() 
{
    HostAddress senderHostAddress(SRC_ADR);
    this->udpSPSSenderSocket.create();

    this->outputLoopRunning = true;
    while (outputLoopRunning) 
    {
        this->outputEventQueue.waitForData();
        IEvent * myEvent = this->outputEventQueue.pop();

        if (myEvent) 
        {
            MotionLabViewChangedEvent *inMsg;
            inMsg = new MotionLabViewChangedEvent;

            inMsg = static_cast<MotionLabViewChangedEvent*>(myEvent);

            const MotionData & mData  = inMsg->getPayload();
            //copy the payload to the class motion field
            char *motionFieldPtr;
            motionFieldPtr = (char*) (this->motionData->getStartPtr());
            for(int i=0; i < PACKAGE_SIZE_FESTO; i++)
            {
                *(motionFieldPtr+i) = mData.getStartPtr()[i];
            }
            //send data to labView vi
            char *toSend;
            toSend = new char[PACKAGE_SIZE_LABVIEW]; 
            packLabViewPackage(toSend);   //package packed, send it!

            HostMsg *motionMsg;
            motionMsg = new HostMsg(*receiverHostAddress, toSend, PACKAGE_SIZE_LABVIEW);
            motionMsg->setHostAddress(*receiverHostAddress);
            this->outHostMsgQueue.push(motionMsg);
            delete[] toSend;
            delete inMsg;
        }
    }
}

void FestoAirmotionRide::packLabViewPackage(char *toSend)
{
    int count=0;         //send field counter
    toSend[count] = 'd'; //start sequenz
    ++count;

    int motionCount = 0;  //motion counter
    for(int j=0; j < 6; ++j)
    {
        for( int i=0 ; i < CHAR_PER_VAL; i++, count++, motionCount++) toSend[count] = this->motionData->getStartPtr()[motionCount];
        if(j != 5)
        {//last value got no separate char
            toSend[count] = ':';
            ++count;
        }
    }
    toSend[count] = 10; //CR
    ++count;
    toSend[count] = 13; //LF
    return;
}

void * FestoAirmotionRide::inputLoopThread_run(void * arg) 
{
    printf("FestoAirmotionRide - input loop thread started\n");
    FestoAirmotionRide * myFeschto = static_cast<FestoAirmotionRide *>(arg);
    myFeschto->executeInputLoop();
    return 0;
}


#ifdef RECEIVE_DATA_PHYSX 
//For now lets use the input loop for physX example
void FestoAirmotionRide::executeInputLoop() {

    cout << "Airmotion inputLoop running" << endl;

    this->udpSPSSenderSocket.create();

    this->inputLoopRunning = true;

    // Start the UDP receiver socket
    WSAData wsaData;
    
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        std::cout<< "WSAStartup failed:" << res << std::endl;
    }

    this->inputSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (this->inputSocket == INVALID_SOCKET) {
        std::cout << "socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) failed" << std::endl;
        WSACleanup();
        pthread_exit(0);
    }

    SOCKADDR_IN my_sin;
    memset(&my_sin, 0, sizeof(my_sin));
    my_sin.sin_family = AF_INET;
    my_sin.sin_port = htons(RECEIVER_PORT);
    my_sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(this->inputSocket , (struct sockaddr *)&my_sin, sizeof(my_sin)) == -1) 
    {
        std::cout << "bind(sfd, (struct sockaddr *)&my_sin, sizeof(my_sin)) failed" << std::endl;
        pthread_exit(0);
    }

    while (inputLoopRunning) 
    {
        char buffer[24];
        struct sockaddr_in other_sin;
        int other_sin_size = (int)sizeof(other_sin);
        //std::cout << "Ready for receiving data" << std::endl;
        int receivedByteCount = recvfrom(this->inputSocket, (char *)buffer, 24, 0, (struct sockaddr *)&other_sin, &other_sin_size);
        //std::cout << "Lin x" << (int) buffer[0] << " Lin y " << (int) buffer[4] << std::endl;
        convertBufferToValidMotionData(buffer, 24);
        //send data to labView vi
        char *toSend;
        toSend = new char[PACKAGE_SIZE_LABVIEW]; 
        packLabViewPackage(toSend);   //package packed, send it!

        HostMsg *motionMsg;
        motionMsg = new HostMsg(*receiverHostAddress, toSend, PACKAGE_SIZE_LABVIEW);
        motionMsg->setHostAddress(*receiverHostAddress);
        this->outHostMsgQueue.push(motionMsg);
        delete [] toSend;
    }

}

void FestoAirmotionRide::convertBufferToValidMotionData(char *buf, int size)
{
    //reset motion field to send
    char *motionPtr = motionData->getStartPtr();
    //set field to 0 (= 48 ASCII)
    for(int i=0; i < PACKAGE_SIZE_FESTO; i++)
    {
        motionPtr[i] = '0';
    }

#define INT_OFFSET 4
#define END_VAL_FESTO 600
    int actIntPos = 0; //start point for next int val 0, 4, 8, 6, ...

    for(int j=0; j < 6; j++)
    {
        int *intVal = (int*) &buf[actIntPos];
        char tempField[4] = {'0','0','0','0'}; // [0] sign bit [1]MSB [3] LSB	
        //set sign bit
        if(*intVal < 0)
        {
            tempField[0] = '1';
        }
        else
        {
            tempField[0] = '0';
        }
    unsigned int absVal = abs(*intVal);
    char tempValbuffer[32];
    itoa(absVal, tempValbuffer, 10);
    if(absVal < 10) //0 - 9
    {// buffer[0] equals LSB
        tempField[3] = tempValbuffer[0];
        tempField[2] = '0';
        tempField[1] = '0';
    }
    else if(absVal < 100) // 10 - 99
    {// buffer[1] equals LSB
        tempField[3] = tempValbuffer[1];
        tempField[2] = tempValbuffer[0];
        tempField[1] = '0';
    }
    else // 100 - ....
    {
        if(absVal > END_VAL_FESTO)
        {// staurate for FESCHTO
            tempField[3] = '0';
            tempField[2] = '0';
            tempField[1] = '6';
        }
        else
        {
            tempField[3] = tempValbuffer[2];
            tempField[2] = tempValbuffer[1];
            tempField[1] = tempValbuffer[0];
        }
    }
    memcpy(&motionPtr[actIntPos], tempField, INT_OFFSET);
    actIntPos += INT_OFFSET;
    }// for(...)
 
}
#else

void FestoAirmotionRide::executeInputLoop() {
 
    this->inputLoopRunning = true;
    while (inputLoopRunning) 
    {
        //nothing to do in here, yet
        Sleep(1000);
    }
}

#endif


}  //namespace tuidevices