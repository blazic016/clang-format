/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 11/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <MPEG/MPEG.h>

#include "DSMCC.h"

dmsDSMCC_Data::dmsDSMCC_Data(dmsMPEG_Section* section) : dmsData()
{
    HDR_INIT(ProtocolDiscriminator, 8);
    HDR_INIT(DsmccType,             8);
    HDR_INIT(MessageId,             16);
    HDR_INIT(TransactionId,         32);
    HDR_INIT(Reserved,              8);
    HDR_INIT(AdaptationLength,      8);
    HDR_INIT(MessageLength,         16);
    HDR_INIT(Data,                  0);

    section->TableId = 0x3C;
    section->Data    = this;

    m_poSection = section;

    SetLenLimit(&MessageLength, &Data);

    SetName("DataCarouselMessage");
}
