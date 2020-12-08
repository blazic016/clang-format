/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <Tools/Tools.h>
#include <Tools/Xml.h>

#include "TDT.h"
#include "DESC.h"
#include "Date.h"

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsDVB_TDT_Data::dmsDVB_TDT_Data(dmsMPEG_SectionSSI0* section) : dmsData()
{
    HDR_INIT(UTC_Time, 0);

    UTC_Time = new dmsDVB_UTC_Time();

    section->m_iPID  = 0x0014;
    section->TableId = 0x70;

    section->SetName("TDT_Section");

    SetName("TDT_Data");

    SetLoad(&UTC_Time, false);
}

