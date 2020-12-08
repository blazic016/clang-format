/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <Tools/Tools.h>
#include <Tools/Xml.h>

#include "BAT.h"
#include "DESC.h"

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsDVB_BAT_Data::dmsDVB_BAT_Data(dmsMPEG_Section* section) : dmsData()
{
    HDR_INIT(Reserved1,                 4);
    HDR_INIT(NetworkDescriptorsLength,  12);
    HDR_INIT(NetworkDescriptors,        0);
    HDR_INIT(Reserved2,                 4);
    HDR_INIT(TransportStreamLoopLength, 12);
    HDR_INIT(TransportStreamLoop,       0);

    section->TableId = 0x00;
    section->m_iPID  = 0x0011;
    section->SetName("BAT_Section");
    section->SetName(&section->TableIdExtension, "BouquetId");

    section->SetLoad(&section->TableIdExtension, false);

    SetName("BAT_Data");

    NetworkDescriptors  = new dmsDVB_DescriptorList();
    TransportStreamLoop = new dmsDVB_BAT_TS_List();

    SetLoad(&NetworkDescriptors, true);

    Reserved1 = 15;
    Reserved2 = 15;

    SetLenLimit(&NetworkDescriptorsLength, &NetworkDescriptors);

    SetLenLimit(&TransportStreamLoopLength, &TransportStreamLoop);
    SetLoad(&TransportStreamLoop, "TransportStreamLoop");
}


dmsData *dmsDVB_BAT_TS_List::Create(void *pt, const char *tag)
{
    return new dmsDVB_BAT_TS_Item();
}



dmsDVB_BAT_TS_Item::dmsDVB_BAT_TS_Item() : dmsData()
{
    HDR_INIT(TransportStreamId,          16);
    HDR_INIT(OriginalNetworkId,          16);
    HDR_INIT(Reserved,                   4);
    HDR_INIT(TransportDescriptorsLength, 12);
    HDR_INIT(TransportDescriptors,        0);

    TransportDescriptors = new dmsDVB_DescriptorList();
    Reserved = 15;

    SetLoad(&TransportStreamId, true);
    SetLoad(&OriginalNetworkId, true);

    SetLenLimit(&TransportDescriptorsLength, &TransportDescriptors);
    SetLoad(&TransportDescriptors, "TransportDescriptors");
}
