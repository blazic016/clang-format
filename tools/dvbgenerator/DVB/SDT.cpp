/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#include <Tools/Tools.h>
#include <Tools/Xml.h>

#include "SDT.h"
#include "DESC.h"

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsDVB_SDT_Data::dmsDVB_SDT_Data(dmsMPEG_Section* section) : dmsData()
{
    HDR_INIT(OriginalNetworkId, 16);
    HDR_INIT(Reserved,          8);
    HDR_INIT(Loop,              0);

    Loop = new dmsDVB_SDT_TS_ItemList();

    section->m_iPID  = 0x0011;
    section->TableId = 0x42;

    section->SetName("SDT_Section");
    section->SetName(&section->TableIdExtension, "TransportStreamId");

    SetName("SDT_Data");

    SetLoad(&OriginalNetworkId, false);
    SetLoad(&Loop, true);

    Reserved = 255;
}


dmsDVB_SDT_TS_Item::dmsDVB_SDT_TS_Item() : dmsData()
{
    HDR_INIT(ServiceId,                16);
    HDR_INIT(Reserved,                 6);
    HDR_INIT(EIT_ScheduleFlag,         1);
    HDR_INIT(EIT_PresentFollowingFlag, 1);
    HDR_INIT(RunningStatus,            3);
    HDR_INIT(FreeCA_Mode,              1);
    HDR_INIT(DescriptorLoopLength,     12);
    HDR_INIT(DescriptorLoop,           0);

    DescriptorLoop = new dmsDVB_DescriptorList();
    Reserved = 63;

    SetLoad(&ServiceId,                false);
    SetLoad(&EIT_ScheduleFlag,         false);
    SetLoad(&EIT_PresentFollowingFlag, false);
    SetLoad(&RunningStatus,            false);
    SetLoad(&FreeCA_Mode,              false);
    SetLoad(&DescriptorLoop,           true);

    SetLenLimit(&DescriptorLoopLength, &DescriptorLoop);
}
