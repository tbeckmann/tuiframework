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


#ifndef _PERCENT_H_
#define _PERCENT_H_

#include <tuiframework/core/ISerializable.h>
#include <iostream>
#include <cstdio>
#include <cmath>

using namespace std; 

/**
*	@class Percent
*	@brief Class percent representation
*
*	Use this for example for festos muscle contraction
*
*	@author Tommy Beckmann
*/
class Percent : public tuiframework::ISerializable {
private:
	/** ValueType enum, decide which precession is needed*/
	enum ValueType{
		INTEGER = 1,  /*!< max. 100 valid values */
		DOUBLE  = 2,  /*!< the vlaue is represented in float */
		NONE    = 3   /*!< initial state, vlaue invalid */
	};
	
	double value;	//> Actual value, if ValueType = NONE -> value = -1.0 (for invalid)
	bool isLimited; //> If this flag is true, only values from 0 to 1.0 are returned
	bool overflow;  //> True if !isLimited && (value < 0 || value > 100)

public:

	Percent();

	Percent(double startVal, bool isLimit);
	Percent(int startVal, bool isLimit);

    virtual ~Percent();

	/** @fn float getDoubleValue() const 
	*   @brief Get the actual percent value in floating point
	*   @return value in range of 0.0 - 1.0 if isLimited = true, all other floating values else
	*/
	double getDoubleValue() const;
	
	/** @fn int getIntegerValue () const 
	*   @brief Get the actual percent value in floating point
	*   @return value in range of 0 - 100 if isLimited = true, all other integer values else
	*/
	int	  getIntegerValue () const;
	
	/** @fn bool getOverflow() const;
	*   @brief Check if an overflow (value out of range 0 - 100) happened
	*   @return true if value out of range, false else
	*/
	bool  getOverflow() const;
	
	/** @fn bool checkIfLimited() const;
	*   @brief Check if the limited Flag is set
	*   @return true if value is limited, false else
	*/
	bool  checkIfLimited() const;

	void setDoubleValue(double toSet);
	void setIntegerValue(int toSet);

	virtual std::ostream & serialize(std::ostream & os) const;
    virtual std::istream & deSerialize(std::istream & is);

protected:
	ValueType myActNumType;

};


#endif //_PERCENT_H_