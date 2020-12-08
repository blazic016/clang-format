/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "DSI.h"
#include <ObjectCarousel/BIOP.h>

#include "DSMCC.h"
#include "Group.h"

dmsDSI_Section::dmsDSI_Section()
{
    MPEG  = new dmsMPEG_Section();
    DSMCC = new dmsDSMCC_Data(MPEG);
    DSI   = new dmsDSI_Data(DSMCC);
}

dmsDSI_Section::~dmsDSI_Section()
{
    DELNUL(MPEG)
}

dmsDSI_Data::dmsDSI_Data(dmsDSMCC_Data *Header) : dmsData()
{
    HDR_INIT(ServerId,                      0);
    HDR_INIT(CompatibilityDescriptorLength, 16);
    HDR_INIT(PrivateDataLength,             16);
    HDR_INIT(PrivateData,                   0);

    Header->m_poSection->SetName("DSI_Section");
    Header->m_poSection->TableId = 0x3B;

    Header->SetName("DSI_Message");
    Header->ProtocolDiscriminator = 0x11;
    Header->DsmccType             = 0x03;
    Header->MessageId             = 0x1006;
    Header->Reserved              = 0xFF;
    Header->AdaptationLength      = 0x00;
    Header->Data                  = this;

    SetName("DSI_Data");

    ServerId = new dmsData();
    ServerId->SetName("ServerId");
    ServerId->m_oBuffer.Set(0xFF, 20);

    SetLenLimit(&PrivateDataLength, &PrivateData);

    m_poHeader = Header;


}

bool dmsDSI_Data::Update()
{
    m_poHeader->m_poSection->TableIdExtension = m_poHeader->TransactionId & 0xFFFF;

    return true;
}


dmsDSI_GroupInfoIndication::dmsDSI_GroupInfoIndication() : dmsData()
{
    HDR_INIT(NumberOfGroups, 16);
    HDR_INIT(GroupInfoList,  0);

    GroupInfoList = new dmsDSI_GroupInfoIndicationItemList();

    SetListCount(&NumberOfGroups, GroupInfoList);
    SetLoad(&GroupInfoList, "");
}


dmsDSI_GroupInfoIndicationItem::dmsDSI_GroupInfoIndicationItem() : dmsData()
{
    HDR_INIT(GroupId,            32);
    HDR_INIT(GroupSize,          32);
    HDR_INIT(GroupCompatibility, 0);
    HDR_INIT(GroupInfoLength,    16);
    HDR_INIT(GroupInfo,          0);
    HDR_INIT(PrivateDataLength,  16);
    HDR_INIT(Private,            0);

    GroupCompatibility = new dmsDSM_CompatibilityDescriptor();
    GroupInfo          = new dmsDSMCC_DescriptorList(NULL);

    SetLenLimit(&GroupInfoLength, &GroupInfo);
    SetLenLimit(&PrivateDataLength, &Private);

    SetLoad(&GroupCompatibility, false);

    SetName("GroupInfo");

    m_poGroup = NULL;
}

bool dmsDSI_GroupInfoIndicationItem::Update()
{
    GroupId   = m_poGroup->m_poSection->DSMCC->TransactionId;
    GroupSize = m_poGroup->GetCumulativeSize();

    return true;
}
