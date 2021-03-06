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


#ifndef _tuiframework_EventQueue_h_
#define _tuiframework_EventQueue_h_

#include "IEventSink.h"
#include "IEventSource.h"


#include "Queue.h"

namespace tuiframework {

class IEvent;

/**
 *  A synchronized FIFO event queue.
 *
 *  EventQueue is designed for multithreaded environments and
 *  offers a conditional wait for data. 
 *
 *  \author Oliver Belaifa
 *  \date 2010-2012
 */
class EventQueue : public Queue<IEvent *>, public IEventSink, public IEventSource {
public:
        /// Initializes the Queue of IEvent * with the initial value 0.
    EventQueue() :
        Queue<IEvent *>(0) {
    }

    virtual ~EventQueue() {
    }

        /// pushes an IEvent in the event queue.
    virtual void push(IEvent * event) {
        Queue<IEvent *>::push(event);
    }


    virtual void waitForData() {
        Queue<IEvent *>::waitForData();
    }


    virtual void cleanupAfterWaitCanceled() {
        Queue<IEvent *>::cleanupAfterWaitCanceled();
    }


    virtual IEvent * pop() {
        return Queue<IEvent *>::pop();
    }

};

}

#endif

