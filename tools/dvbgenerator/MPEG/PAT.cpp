/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <Tools/Tools.h>
#include <Tools/Xml.h>

#include "PAT.h"


/* ########################################################################

   ######################################################################## */


dmsMPEG_DescProgramNumberPID::dmsMPEG_DescProgramNumberPID() : dmsData()
{
    HDR_INIT(ProgramNumber, 16);
    HDR_INIT(Reserved,      3);
    HDR_INIT(PID,           13);

    Reserved = 7;

    SetLoad(&ProgramNumber, true);
    SetLoad(&PID,           true);
}


dmsData *dmsMPEG_DescProgramNumberPIDList::Create(void *pt, const char *tag)
{
    return new dmsMPEG_DescProgramNumberPID();
}



dmsMPEG_PAT_Data::dmsMPEG_PAT_Data(dmsMPEG_Section* section) : dmsData()
{
    HDR_INIT(Loop, 0);

    section->TableId = 0x0;
    section->m_iPID  = 0x0000;
    section->SetName(&section->TableIdExtension, "TransportStreamId");

    section->SetName("PAT_Section");

    Loop = new dmsMPEG_DescProgramNumberPIDList();

    SetName("PAT_Data");

    SetLoad(&Loop, "Loop");
}
