#include "phsXConnection.h"


PhysXConnection::PhysXConnection():
udpPhysXSenderSocket(outHostMsgQueue),
udpdSPACESenderSocket(outDSPACEMsgQueue)
{
    send2physX = new dataPackageRecCosSim;
    receiverHostAddress = new HostAddress(DST_PHY_ADR);

    send2dSPACE = new dataSendCoSim;
    DSPACEHostAddress = new HostAddress(DST_ADR_COSIM);
}

PhysXConnection::~PhysXConnection()
{

}

bool PhysXConnection::startConnection() 
{
    {
        int rc = pthread_create(&this->outputLoopThread, NULL, PhysXConnection::outputLoopThread_run, this);
        if (rc) 
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }

    {
        int rc = pthread_create(&this->inputLoopThread, NULL, PhysXConnection::inputLoopThread_run, this);
        if (rc) 
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
        }
    }

    return true;
}

void * PhysXConnection::outputLoopThread_run(void * arg) 
{
    printf("physXConnection - output loop thread started\n");
    PhysXConnection * myConnection = static_cast<PhysXConnection *>(arg);
    myConnection->executeOutputLoop();
    return 0;
}

void PhysXConnection::executeOutputLoop() 
{
    this->udpPhysXSenderSocket.create();
    this->initDSPACEsocket();

    int receivedByteCount = 0;

    this->outputLoopRunning = true;
    while(outputLoopRunning)
    {
        //receive co-simulation data from dSPACE
        struct sockaddr_in other_sin;
        int other_sin_size = (int)sizeof(other_sin);

        receivedByteCount = recvfrom(this->dataFromdSPACESocket, (char *)send2physX, PACKAGE_SIZE_COSIM_OUT, 0, (struct sockaddr *)&other_sin, &other_sin_size);
       // cout << "Received package with size: " << receivedByteCount << " Data:" << send2physX->steerAngle[0] << endl;

        //send message to physX
        HostMsg *motionMsg;
        motionMsg = new HostMsg(*receiverHostAddress, (char *) send2physX, PACKAGE_SIZE_COSIM_OUT);
        motionMsg->setHostAddress(*receiverHostAddress);
        this->outHostMsgQueue.push(motionMsg);
    }
}

void * PhysXConnection::inputLoopThread_run(void * arg) 
{
    printf("physXConnection - output loop thread started\n");
    PhysXConnection * myConnection = static_cast<PhysXConnection *>(arg);
    myConnection->executeInputLoop();
    return 0;
}

void PhysXConnection::executeInputLoop()
{
    this->udpdSPACESenderSocket.create();
    this->initPhysXSocket();

    int receivedByteCount = 0;
    this->inputLoopRunning = true;

    while(this->inputLoopRunning)
    {
        //receive data from nvidia
        struct sockaddr_in other_sin;
        int other_sin_size = (int)sizeof(other_sin);
        receivedByteCount = recvfrom(this->dataFromPhysX, (char *)send2dSPACE, PACKAGE_SIZE_COSIM, 0, (struct sockaddr *)&other_sin, &other_sin_size);
        //std::cout << "received Data for dspace dx: " << send2dSPACE->collisionFlag << std::endl;
        //send message to dSPACE
        HostMsg *coSimMsg;
        coSimMsg = new HostMsg(*DSPACEHostAddress, (char *)send2dSPACE ,PACKAGE_SIZE_COSIM);
        coSimMsg->setHostAddress(*DSPACEHostAddress);
        this->outDSPACEMsgQueue.push(coSimMsg);
    }
}


void PhysXConnection::initDSPACEsocket()
{
    // Start the UDP receiver socket
    WSAData wsaData;

    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        std::cout<< "WSAStartup failed:" << res << std::endl;
    }

    this->dataFromdSPACESocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (this->dataFromdSPACESocket == INVALID_SOCKET) {
        std::cout << "socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) failed" << std::endl;
        WSACleanup();
        pthread_exit(0);
    }

    SOCKADDR_IN my_sin;
    memset(&my_sin, 0, sizeof(my_sin));
    my_sin.sin_family = AF_INET;
    my_sin.sin_port = htons(RECEIVE_PORT_DSPACE_COSIMDATA);
    my_sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(this->dataFromdSPACESocket , (struct sockaddr *)&my_sin, sizeof(my_sin)) == -1) 
    {
        std::cout << "bind(sfd, (struct sockaddr *)&my_sin, sizeof(my_sin)) failed" << std::endl;
        pthread_exit(0);
    }
}

void PhysXConnection::setWheelParametersAndSend(double *drvTrq, double *brkTrq, double *strAngl, int wheelNums)
{
    //set new values
    for(int i=0; i < wheelNums; i++)
    {
        send2physX->wheelRotation[i] = drvTrq[i];
        send2physX->brakeTorque[i] = brkTrq[i];
        send2physX->steerAngle[i] = strAngl[i];
    }
    //call send method
    sendDataToPhysX();
    return;
}

void PhysXConnection::sendDataToPhysX()
{
    //send data
    char *toSend;
    toSend = (char*)&send2physX; 
    HostMsg *motionMsg;
    motionMsg = new HostMsg(*receiverHostAddress, toSend, PACKAGE_SIZE_COSIM_OUT);
    motionMsg->setHostAddress(*receiverHostAddress);
    this->outHostMsgQueue.push(motionMsg);
    return;
}

void PhysXConnection::initPhysXSocket()
{
    // Start the UDP receiver socket
    WSAData wsaData;
    
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0)
    {
        std::cout<< "WSAStartup failed:" << res << std::endl;
    }

    this->dataFromPhysX = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (this->dataFromPhysX == INVALID_SOCKET)
    {
        std::cout << "socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) failed" << std::endl;
        WSACleanup();
        pthread_exit(0);
    }

    SOCKADDR_IN my_sin;
    memset(&my_sin, 0, sizeof(my_sin));
    my_sin.sin_family = AF_INET;
    my_sin.sin_port = htons(RECEIVE_PORT_PHYSX);
    my_sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(this->dataFromPhysX , (struct sockaddr *)&my_sin, sizeof(my_sin)) == -1)
    {
        std::cout << "bind(sfd, (struct sockaddr *)&my_sin, sizeof(my_sin)) failed ---dSPACE socket" << std::endl;
        pthread_exit(0);
    }
}
