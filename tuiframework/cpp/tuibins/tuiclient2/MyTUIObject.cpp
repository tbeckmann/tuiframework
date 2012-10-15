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


#include "MyTUIObject.h"

using namespace tuiframework;
using namespace std;

MyTUIObject::MyTUIObject() :
    button(false),
    pressure(0) {
}


MyTUIObject::~MyTUIObject() {
}


void MyTUIObject::connect() {
    try {
        MCONNECT(Matrix4x4ChangedEvent, "MyTUIObjectInstance", "Transf",
                MyTUIObject, this, &MyTUIObject::transfChanged, 1);
        MCONNECT(DigitalChangedEvent, "MyTUIObjectInstance", "Button",
                MyTUIObject, this, &MyTUIObject::buttonChanged, 2);
        MCONNECT(AnalogChangedEvent, "MyTUIObjectInstance", "Pressure",
                MyTUIObject, this, &MyTUIObject::pressureChanged, 2);

        this->ledChannel = getSinkChannel("MyTUIObjectInstance", "LED", 2);
        if (this->ledChannel == 0) {
            cout << "this->ledChannel == 0" << endl;
        }

    } catch (const Exception & e) {
        cerr << "Exception" << endl;
        cerr << e.getFormattedString() << endl;
    }
}


void MyTUIObject::disconnect() {
    try {
        MDISCONNECT(Matrix4x4ChangedEvent, "MyTUIObjectInstance", "Transf",
                MyTUIObject, this, &MyTUIObject::transfChanged, 1);
        MDISCONNECT(DigitalChangedEvent, "MyTUIObjectInstance", "Button",
                MyTUIObject, this, &MyTUIObject::buttonChanged, 2);
        MDISCONNECT(AnalogChangedEvent, "MyTUIObjectInstance", "Pressure",
                MyTUIObject, this, &MyTUIObject::pressureChanged, 2);
    } catch (const Exception & e) {
        cerr << "Exception" << endl;
        cerr << e.getFormattedString() << endl;
    }
}


void MyTUIObject::transfChanged(const Matrix4x4ChangedEvent * e) {
    this->transf = e->getPayload();
    cout << "(1)MyTUIObject: transformation changed: " << this->transf << endl;
}


void MyTUIObject::buttonChanged(const DigitalChangedEvent * e) {
    this->button = e->getPayload();
    cout << "(2)MyTUIObject: state of button changed: " << (this->button ? "True" : "False") << endl;

    this->ledChannel->push(new DigitalChangedEvent(-1, -1, this->button));
}


void MyTUIObject::pressureChanged(const AnalogChangedEvent * e) {
    this->pressure = e->getPayload();
    cout << "(2)MyTUIObject: state of button changed: " << this->pressure << endl;
}

