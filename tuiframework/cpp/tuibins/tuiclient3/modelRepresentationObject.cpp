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

#include "ModelRepresentationObject.h"

ModelRepresentationObject::ModelRepresentationObject():
dSPACEHostAddress(DST_ADR_USER),
udpSenderSocket(outHostMsgQueue),
udpFestoReceiverSocket(inSerializedDataQueue)
{
    this->CoGPosWoldXYZ = new double[DIMENSIONS];
    setCenterOfGravity(0,0,0);
    this->valuesTodSPACE = new dataPackageSend;
    //set initial values
    this->valuesTodSPACE->accelPos = 0.0;
    this->valuesTodSPACE->brakePos = 0.0;
    this->valuesTodSPACE->clutchPos = 0.0;
    this->valuesTodSPACE->steerAngle = 0.0;
    this->valuesTodSPACE->actGear = 0;
    for(int i=0; i < BUT_NUM; i++)
        this->valuesTodSPACE->userButton[i] = 0;

    this->coSimVal = new dataSendCoSim;
    for(int i=0; i < DIMENSIONS; i++)
    {
#ifdef USE_STABI_VALS
        for(int j=0; j < COLUMDATA; j++)
        {
            this->coSimVal->extStabiF[i,j] = 0;
            this->coSimVal->extStabiTrq[i,j] = 0;
        }
#endif
        this->coSimVal->extAccel_xyz[i] = 0;
        this->coSimVal->extAccel_abg[i] = 0;
    }
    this->motionData = new MotionData;

    this->connect2PhysX = new PhysXConnection();
}


ModelRepresentationObject::~ModelRepresentationObject()
{
    delete[] this->CoGPosWoldXYZ;
    delete[] this->valuesTodSPACE;
    delete[] this->motionData;
}


void ModelRepresentationObject::connect() 
{
//start threads:
    this->executeThreads();
    try {
        CONNECT(PercentChangedEvent, "ModelRepresentation", "accelPos",
                ModelRepresentationObject, this, &ModelRepresentationObject::accelPosChanged);
        CONNECT(PercentChangedEvent, "ModelRepresentation", "brakePos",
                ModelRepresentationObject, this, &ModelRepresentationObject::brakePosChanged);
        CONNECT(PercentChangedEvent, "ModelRepresentation", "clutchPos",
                ModelRepresentationObject, this, &ModelRepresentationObject::clutchPosChanged);
        CONNECT(IntegerChangedEvent, "ModelRepresentation", "actGear",
                ModelRepresentationObject, this, &ModelRepresentationObject::gearChanged);
        CONNECT(IntegerChangedEvent, "ModelRepresentation", "button1",
                ModelRepresentationObject, this, &ModelRepresentationObject::buttonChanged);
        CONNECT(DoubleChangedEvent, "ModelRepresentation", "steerAngle",
                ModelRepresentationObject, this, &ModelRepresentationObject::steerAngleChanged);
/* 
        CONNECT(AnalogChangedEvent, "MyTUIObjectInstance", "Pressure",
                MyTUIObject, this, &MyTUIObject::pressureChanged); PercentChangedEvent
*/
        this->festoChannel = getSinkChannel("ModelRepresentation", "alpha");
        if (this->festoChannel == 0) {
            cout << "this->festoChannel == 0" << endl;
        }
    } catch (const Exception & e) {
        cerr << "Exception" << endl;
        cerr << e.getFormattedString() << endl;
    }
}

void ModelRepresentationObject::disconnect()
{
//TODO
}

void ModelRepresentationObject::steerAngleChanged(const DoubleChangedEvent * e) 
{
#ifdef DEBUG_OUTPUT
    cout << "Steering Angele Changed: " << this->valuesTodSPACE->steerAngle << endl;
#endif
    this->valuesTodSPACE->steerAngle = e->getPayload();
    sendDataTodSPACE(); 
}

void ModelRepresentationObject::accelPosChanged(const PercentChangedEvent * e)
{
#ifdef DEBUG_OUTPUT
    cout << "Acceleration Ped Pos changed: " << this->valuesTodSPACE->accelPos << endl;
#endif
    Percent newVal;
    newVal = e->getPayload();
    this->valuesTodSPACE->accelPos = newVal.getDoubleValue();
    sendDataTodSPACE();
}

void ModelRepresentationObject::brakePosChanged(const PercentChangedEvent * e)
{
#ifdef DEBUG_OUTPUT
    cout << "Braking Ped Pos changed: " << this->valuesTodSPACE->brakePos << endl;
#endif
    Percent newVal;
    newVal = e->getPayload();
    this->valuesTodSPACE->brakePos = newVal.getDoubleValue();
    sendDataTodSPACE();
}

void ModelRepresentationObject::gearChanged(const IntegerChangedEvent * e)
{
#ifdef DEBUG_OUTPUT
    cout << "Gear changed: " << this->valuesTodSPACE->actGear << endl;
#endif
    this->valuesTodSPACE->actGear = e->getPayload();
    sendDataTodSPACE();
}

void ModelRepresentationObject::buttonChanged(const IntegerChangedEvent * e)
{
#ifdef DEBUG_OUTPUT
    cout << "Button changed: " << this->valuesTodSPACE->userButton[0] << endl;
#endif
    this->valuesTodSPACE->userButton[0] = e->getPayload();
    sendDataTodSPACE();
}

void ModelRepresentationObject::clutchPosChanged(const PercentChangedEvent * e)
{
#ifdef DEBUG_OUTPUT
    cout << "Braking Ped Pos changed: " << this->valuesTodSPACE->clutchPos << endl;
#endif
    Percent newVal;
    newVal = e->getPayload();
    this->valuesTodSPACE->clutchPos = newVal.getDoubleValue();
    sendDataTodSPACE();
}


//dSPACE connection methods (use ethernet / udp):
bool ModelRepresentationObject::executeThreads()
{
    {
       
        int rc = pthread_create(&this->outputLoopThread, NULL, ModelRepresentationObject::outputThread_run, this);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }

    {
        int rc = pthread_create(&this->inputLoopThread, NULL, ModelRepresentationObject::inputThread_run, this);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }
    return true;
}

void *ModelRepresentationObject::inputThread_run(void * arg) 
{
    printf("ModelRepresentationObject - input loop thread started\n");
    ModelRepresentationObject * modelObject = static_cast<ModelRepresentationObject *>(arg);
    modelObject->ModelRepresentationObject::executeInputLoop();
    return 0;
}

void ModelRepresentationObject::executeInputLoop() 
{
    this->udpFestoReceiverSocket.create();
    this->udpFestoReceiverSocket.setMyPort(SRC_FESTO_PORT);

    //start the physX connection
    this->connect2PhysX->startConnection();

    this->inputLoopRunning = true;
    while(this->inputLoopRunning)
    {
        this->inSerializedDataQueue.waitForData();

        Queue<std::pair<char *, int > > *tempQueue;
        tempQueue = new Queue<std::pair<char *, int > >;
        tempQueue->push(this->inSerializedDataQueue.pop());

        std::pair<char*, int> tempPair;
        tempPair.first = new char[PACKAGE_SIZE_FESTO + TUI_PACKAGE_OVERHEAD];
        if(tempQueue->size() != 0)
        {
            tempPair = tempQueue->pop();
            memcpy( (char*) this->motionData->getStartPtr() , &(tempPair.first[TUI_PACKAGE_OVERHEAD]), PACKAGE_SIZE_FESTO );
            this->festoChannel->push(new MotionLabViewChangedEvent(-1, -1, *this->motionData));
        }
    }
}


void *ModelRepresentationObject::outputThread_run(void * arg) 
{
    printf("ModelRepresentationObject - output loop thread started\n");
    ModelRepresentationObject * modelObject = static_cast<ModelRepresentationObject *>(arg);
    modelObject->ModelRepresentationObject::executeOutputLoop();
    return 0;
}

void ModelRepresentationObject::executeOutputLoop() 
{
    this->outputLoopRunning = true;
    this->udpSenderSocket.setMyPort(SRC_PORT_DSPACE);
    this->udpSenderSocket.create();
    HostMsg *udpPackage;
    while(this->outputLoopRunning)
    {
        //wait on a queue
        this->dSPACESendQueue.waitForData();
        dataPackageSend *tempSend; 
        tempSend = &(this->dSPACESendQueue.pop());
        if(tempSend != NULL)
        {
            udpPackage = new HostMsg(dSPACEHostAddress, (char*) tempSend, PACKAGE_SIZE_IN);	
            udpPackage->setHostAddress(dSPACEHostAddress);
            this->outHostMsgQueue.push(udpPackage);
            //cout << "Message sent" << endl;
        }
    }
}

void ModelRepresentationObject::setCenterOfGravity(double xVal, double yVal, double zVal)
{
    if(DIMENSIONS != 3) 
    { //this method is only for 3-D spaces valid!
        return;
    }
    else
    {
        this->CoGPosWoldXYZ[0] = xVal;
        this->CoGPosWoldXYZ[1] = yVal;
        this->CoGPosWoldXYZ[2] = zVal;
    }
}

void ModelRepresentationObject::sendDataTodSPACE()
{
    dataPackageSend *sendObject = new dataPackageSend;
    sendObject->accelPos = this->valuesTodSPACE->accelPos;
    sendObject->brakePos = this->valuesTodSPACE->brakePos;
    sendObject->clutchPos = this->valuesTodSPACE->clutchPos;
    sendObject->steerAngle = this->valuesTodSPACE->steerAngle;
    sendObject->actGear = this->valuesTodSPACE->actGear;
    sendObject->userButton[0] = this->valuesTodSPACE->userButton[0];

    this->dSPACESendQueue.push(*sendObject);
}
