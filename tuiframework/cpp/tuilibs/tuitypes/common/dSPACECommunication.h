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

#ifndef _dSPACECOMMUNICATION_H
#define _dSPACECOMMUNICATION_H

#include <tuiframework/core/ISerializable.h>
#include <iostream>
#include <cstdio>

using namespace std;

#define DIMENSIONS 3    //!< calculation dimensions (dimensions are usually not > 3)
#define COLUMDATA 6     //!< defined by dSPACE, refer to MDL/VehicleDynamics/ExternIn
//#define USE_STABI_VALS  //!< define this flag to use Stabilization_F_External and Stabilization_Trq_External

#define SRC_PORT_DSPACE  51002  //!< source port

#define DST_ADR_USER "192.168.0.4:51001"  //!< dSPACE_IP:port user data
#define DST_ADR_COSIM "192.168.0.4:51002" //!< dSPACE_IP:port for CoSimulation data

#define SRC_FESTO_PORT 51003

#define LOCAL_DEBUG  //!< decide where your PhysX application is running, local or on computer refered in phsXConnection.h

#define BYTES_OF_A_DOUBLE 8  //How many bytes has one double on your machine?
#define BYTES_OF_A_INT    4

#define BUT_NUM 7

#define PACKAGE_SIZE_IN 36 + (BUT_NUM * BYTES_OF_A_INT) // !<[Byte] following sturct size for send routine
  /**
    * @struct userTodSPACE
    * @brief Represents all data send to dSAPCE
    * @note Do not change the PACKAGE_SIZE_IN unless you make changes in the sturct
    * @author Tommy Beckmann
    */
typedef struct userTodSPACE
{
    double accelPos;   //!< 8 [Byte] accel Pedal Pos 0-1
    double brakePos;   //!< 8 [Byte] brake Pedal Pos 0-1
    double clutchPos;  //!< 8 [Byte] clutch Pedal Pos 0-1
    double steerAngle; //!< 8 [Byte] steerAngle in [Deg]
    int actGear;        //!< 4 [Byte] desired gear
    int userButton[BUT_NUM];    //!< 4 [Byte] user button
}dataPackageSend;

#define WHEEL_NUMBER 4  //!< Wheel number of the simulation, increase for e.g. use a trailor or truck

#ifdef USE_STABI_VALS
#define PACKAGE_SIZE_COSIM  (2 * DIMENSIONS * 8) + (2*6*3*8) + BYTES_OF_A_INT // !<[Byte] following sturct size for send routine
#else
#define PACKAGE_SIZE_COSIM  (3 * DIMENSIONS * 8 + 8 * WHEEL_NUMBER) + BYTES_OF_A_INT // !<[Byte] following sturct size for send routine
#endif
  /**
    * @struct coSimTodSPACE
    * @brief Represents data for coSimulation on seperate port
    * @note Do not change the PACKAGE_SIZE_COSIM  unless you make changes in the sturct
    * @author Tommy Beckmann
    */
typedef struct coSimTodSPACE
{
    double extAccel_xyz[DIMENSIONS];  //!< (DIMENSIONS * 8) [Byte] external linear accelerations
    double extAccel_abg[DIMENSIONS]; //!< (DIMENSIONS * 8) [Byte] external anuglar accelerations
    double physXVelocities[DIMENSIONS]; //!< (DIMENSIONS *8) [Byte] act PxVehileNoDrive Velocities [m/s]
    double frictionPerWheel[WHEEL_NUMBER]; //!< (WHEEL_NUMBER *8)[Byte] friction factor 0 - 1
    int collisionFlag; //!< detect a collision and turn of the virtual controller
#ifdef USE_STABI_VALS
    double extStabiF[DIMENSIONS, COLUMDATA]; //!< stabilization data, TODO: find out for what ? 
    double extStabiTrq[DIMENSIONS, COLUMDATA];
#endif
}dataSendCoSim;

#define PACKAGE_SIZE_COSIM_OUT  (WHEEL_NUMBER*BYTES_OF_A_DOUBLE*3) //!< [Byte] to receive from the ds1006
  /**
    * @struct coSimfromdSPACE
    * @brief Represents all data received from ds1006
    * @note Do not change the PACKAGE_SIZE_OUT unless you make changes in the sturct
    * @author Tommy Beckmann
    */
typedef struct coSimfromdSPACE
{
    double wheelRotation[WHEEL_NUMBER]; //!< (WHEEL_NUMBER * 8) [Byte] wheelRotation [rad/s]
    double brakeTorque[WHEEL_NUMBER]; //!< (WHEEL_NUMBER * 8) [Byte] actual brake torque per wheel [Nm]
    double steerAngle[WHEEL_NUMBER];  //!< (WHEEL_NUMBER * 8) [Byte] act steer angle per wheel [Deg]
}dataPackageRecCosSim;

#define PACKAGE_SIZE_FESTO 24 //!< [Byte] to receive from the ds1006 for festos motion data
#define CHAR_PER_VAL 4
  /**
    * @struct festoFromdSPACE
    * @brief Represents motion data received from ds1006. Works with the LabView Kinematic on tui-serverside.
    *
    *   For valid dataformat refer to festos airmotion ride users guide.
    *
    * @note Do not change the PACKAGE_SIZE_FESTO unless you make changes in the sturct
    * @author Tommy Beckmann
    */
typedef struct festoFromdSPACE
{
    char pitch[CHAR_PER_VAL];
    char roll[CHAR_PER_VAL];
    char heave[CHAR_PER_VAL];
    char yaw[CHAR_PER_VAL];
    char lateral[CHAR_PER_VAL];
    char longitudinal[CHAR_PER_VAL];
}dataPackageRecFesto;

class  MotionData: public tuiframework::ISerializable
{
public:
    MotionData();
    virtual ~MotionData();

    virtual const char *getStartPtr() const {return (const char*)myDataField;}
    virtual char *getStartPtr(){return (char*)myDataField;}

    virtual std::ostream & serialize(std::ostream & os) const;
    virtual std::istream & deSerialize(std::istream & is);

private:
    dataPackageRecFesto *myDataField;

};

#endif //_dSPACECOMMUNICATION_H
