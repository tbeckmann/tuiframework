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


#include "MulticastListener.h"

#include "../core/ISerializedDataSink.h"
#include "../logging/Logger.h"

#include "../core/IThreadMessageSink.h"


#include "../core/HostAddress.h"
#include "../core/TypeRegistration.h"

#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <cstring> // GCC 4.3 related build problem

#define DEFAULT_PORT    9163

using namespace std;

namespace tuiframework {

MulticastListener::MulticastListener(ISerializedDataSink & serializedDataSink) :
    serializedDataSink(serializedDataSink),
    port(DEFAULT_PORT),
    threadMessageSink(0) {
}


MulticastListener::~MulticastListener() {
}



void MulticastListener::setMCGroupIPAddress(unsigned int mcGroupIPAddress) {
    this->mcGroupIPAddress = mcGroupIPAddress;
}


unsigned int MulticastListener::getMCGroupIPAddress() const {
    return this->mcGroupIPAddress;
}


void MulticastListener::setPort(unsigned short int port) {
    this->port = port;
}


unsigned short int MulticastListener::getPort() {
    return this->port;
}


void MulticastListener::setThreadMessageSink(IThreadMessageSink * threadMessageSink) {
	this->threadMessageSink = threadMessageSink;
}


void MulticastListener::setHostAddressSink(IHostAddressSink * hostAddressSink) {
    this->hostAddressSink = hostAddressSink;
}


void MulticastListener::run() {
    // The default cancelability state of pthread's is deferred.

    pthread_cleanup_push(notifyCancellation, this);
    if (this->threadMessageSink) {
        this->threadMessageSink->threadCreated(this->tid);
    }

    this->sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sfd == -1) {
        TFFATAL("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) failed");
        pthread_exit(0);
    }

    pthread_cleanup_push(closeSocket, this);

    const int one = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
        TFFATAL("setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) failed");
        pthread_exit(0);
    }

    struct sockaddr_in my_sin;
    memset(&my_sin, 0, sizeof(my_sin));
    my_sin.sin_family = AF_INET;
    my_sin.sin_port = htons(this->port);
    my_sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(this->sfd, (struct sockaddr *)&my_sin, sizeof(my_sin)) == -1) {
        TFFATAL("bind(sfd, (struct sockaddr *)&my_sin, sizeof(my_sin)) failed");
        pthread_exit(0);
    }
    
         /* use setsockopt() to request that the kernel join a multicast group */
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = htonl(this->mcGroupIPAddress);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(this->sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        TFFATAL("setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) failed");
        pthread_exit(0);
    }
    
    fd_set rfds, fds;

    FD_ZERO(&fds);
    FD_SET(0, &fds);
    FD_SET(sfd, &fds);

    while (true) {
        rfds = fds;
        int retVal = select(sfd + 1, &rfds, NULL, NULL, 0);
        if (retVal == -1) {
            TFFATAL("select failed.");
        }

        if (FD_ISSET(sfd, &rfds)) {
            char buffer[4096];
            struct sockaddr_in other_sin;
            socklen_t other_sin_size = (socklen_t)sizeof(other_sin);

            int receivedByteCount = recvfrom(sfd, (void *)buffer, 4096, 0, (struct sockaddr *)&other_sin, &other_sin_size);

/*
            if (this->hostAddressSink) {
                HostAddress hostAddress(ntohl(other_sin.sin_addr.s_addr), ntohs(other_sin.sin_port));
                this->hostAddressSink->push(hostAddress);
            }
*/

            HostAddress ipAddress(ntohl(other_sin.sin_addr.s_addr), ntohs(other_sin.sin_port));

            //std::cout << "receiver ipAddress: " << ipAddress.toString() << std::endl;

                //@@TODO: split in a seperate class
            stringstream ss;
            ss << HostEvent::EventTypeID() << " ";
            HostEvent::serializeHeader(ipAddress, true, ss);
            ss << " " << buffer;

            string str = ss.str();

            char * incomingSerializedData = new char[str.size() + 1];
            memcpy(incomingSerializedData, str.c_str(), str.size() + 1);

            this->serializedDataSink.push(pair<char *, int>(incomingSerializedData, receivedByteCount));
        }
    }

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
}


static void * runfunc(void * arg) {
    printf("MulticastListener thread started\n");
    MulticastListener * mcListener = static_cast<MulticastListener *>(arg);
    mcListener->run();
    return 0;
}


void MulticastListener::create() {
    int rc = pthread_create(&this->tid, NULL, runfunc, this);
    if (rc) {
        TFFATAL("ERROR in thread creation");
    }
}


void MulticastListener::cancel() {
    pthread_cancel(this->tid);
}


void MulticastListener::notifyCancellation(void * arg) {
    MulticastListener * mcListener = static_cast<MulticastListener *>(arg);
    if (mcListener->threadMessageSink) {
        mcListener->threadMessageSink->threadCanceled(mcListener->tid);
    }
}


void MulticastListener::closeSocket(void * arg) {
    MulticastListener * mcListener = static_cast<MulticastListener *>(arg);
    close(mcListener->sfd);
}


void MulticastListener::dropMembership(void * arg) {
    MulticastListener * mcListener = static_cast<MulticastListener *>(arg);
    
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = htonl(mcListener->mcGroupIPAddress);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(mcListener->sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        TFFATAL("setsockopt(sfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) failed");
    }
}


}
