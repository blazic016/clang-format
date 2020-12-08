/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DMS_HEADERS_CORBA_H_
#define _DMS_HEADERS_CORBA_H_

#include <Tools/Header.h>

class dmsCorbaIOR : public dmsData
{
public:
    u32          TypeIdLength;
    dmsData*     TypeId;
    u32          TaggedProfilesCount;
    dmsDataList* TaggedProfileList;

public:
    dmsCorbaIOR();

    void SetTypeId(const char *id);
    void SetTypeId(dmsData *data);
};



#endif /* _DMS_HEADERS_CORBA_H_ */
