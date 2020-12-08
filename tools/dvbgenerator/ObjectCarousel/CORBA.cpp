/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Tools/Header.h>

#include "CORBA.h"

#include "BIOP.h"

dmsCorbaIOR::dmsCorbaIOR() : dmsData()
{
    HDR_INIT(TypeIdLength,        32);
    HDR_INIT(TypeId,              0);
    HDR_INIT(TaggedProfilesCount, 32);
    HDR_INIT(TaggedProfileList,    0);

    TypeId            = new dmsData();
    TaggedProfileList = new dmsDataList();

    SetLenLimit(&TypeIdLength, &TypeId);
    SetListCount(&TaggedProfilesCount, TaggedProfileList);
}

void dmsCorbaIOR::SetTypeId(const char *id)
{
    TypeId->m_oBuffer.Set(id, true);
}


void dmsCorbaIOR::SetTypeId(dmsData *data)
{
    TypeId->m_oBuffer.Set(data->m_oBuffer);
}
