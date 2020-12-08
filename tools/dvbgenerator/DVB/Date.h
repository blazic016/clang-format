/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _LIB_DVB_DATE_H_
#define _LIB_DVB_DATE_H_


#include <Tools/Header.h>


class dmsDVB_UTC_Time : public dmsData
{
public:
    u16 MJD;
    u8  Hour;
    u8  Minute;
    u8  Second;

    dmsDVB_UTC_Time();
};


#endif /* _LIB_DVB_DATE_H_ */
