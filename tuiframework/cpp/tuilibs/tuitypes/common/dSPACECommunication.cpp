#include "dSPACECommunication.h"

MotionData::MotionData()
{
    myDataField = new dataPackageRecFesto;
    //warning be carful with following pointermagic! Only do this if you are 100% familiar with this!
    char *motionFieldPtr;
    motionFieldPtr = (char*) myDataField;
    for(int i=0; i<PACKAGE_SIZE_FESTO; i++)
    {
        *(motionFieldPtr+i) = 48;
    }
}

MotionData::~MotionData()
{
}

std::ostream & MotionData::serialize(std::ostream & os) const 
{
    for(int i=0; i < CHAR_PER_VAL; ++i)
    {
        os << (char)(this->myDataField->pitch[i]) << (char)(this->myDataField->roll[i]) << (char)(this->myDataField->heave[i]) 
        << (char)(this->myDataField->yaw[i]) <<  (char)(this->myDataField->lateral[i]) << (char)(this->myDataField->longitudinal[i]);
    }
    return os;
}

std::istream & MotionData::deSerialize(std::istream & is) 
{
    for(int i=0; i < CHAR_PER_VAL; ++i)
    {
        is >> (char)(this->myDataField->pitch[i]) >> (char)(this->myDataField->roll[i]) >> (char)(this->myDataField->heave[i]) 
        >> (char)(this->myDataField->yaw[i]) >>  (char)(this->myDataField->lateral[i]) >> (char)(this->myDataField->longitudinal[i]);
    }
    return is;
}
