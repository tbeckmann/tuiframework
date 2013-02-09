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



#include "Percent.h"
 
Percent::Percent(){
	this->myActNumType = NONE;
	this->value = -1.0;
	this->isLimited = false;
	this->overflow = false;
}


Percent::Percent(double startVal, bool isLimit)
{
	this->myActNumType = DOUBLE;
	this->value = startVal;
	this->isLimited = isLimit;
}

Percent::Percent(int startVal, bool isLimit)
{
	this->myActNumType = INTEGER;
	this->value = startVal;
	this->isLimited = isLimit;
}

Percent::~Percent()
{
}

double Percent::getDoubleValue() const
{
	return this->value;
}
int Percent::getIntegerValue () const
{
	return static_cast<int>(this->value);
}
	

bool  Percent::getOverflow() const
{
	return this->overflow;
}
	
bool  Percent::checkIfLimited() const
{
	return this->isLimited;
}

void Percent::setDoubleValue(double toSet)
{
	this->value = toSet;
}

void Percent::setIntegerValue(int toSet)
{
	this->value = (double)toSet;
}

std::ostream & Percent::serialize(std::ostream & os) const {
    os << (int)(this->myActNumType) << " " << this->value << " " << this->isLimited << " " << this->overflow;
    return os;
}

std::istream & Percent::deSerialize(std::istream & is) {
    long valType;

    is >> valType >> this->value >> this->isLimited >> this->overflow;
    this->myActNumType = (ValueType) valType;
    return is;
}