/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include "Date.h"

dmsDVB_UTC_Time::dmsDVB_UTC_Time() : dmsData()
{
    HDR_INIT(MJD,  16);
    HDR_INIT(Hour, 8);
    HDR_INIT(Minute, 8);
    HDR_INIT(Second, 8);

    SetLoad(&MJD,  false);
    SetLoad(&Hour, false);
    SetLoad(&Minute, false);
    SetLoad(&Second, false);
}
