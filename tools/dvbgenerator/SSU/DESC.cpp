/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <Tools/Xml.h>
#include <Tools/Conversion.h>

#include "DESC.h"


dmsNameRank SSU_UNT_DescriptorNames[] =
{
    {0x01, (char *)"Scheduling"},
    {0x02, (char *)"Update"},
    {0x03, (char *)"SSU_Location"},
    {0x04, (char *)"Message"},
    {0x05, (char *)"SSU_EventName"},
    {0x06, (char *)"TargetSmartCard"},
    {0x07, (char *)"TargetMAC_Address"},
    {0x08, (char *)"TargetSerialNumber"},
    {0x09, (char *)"TargetIP_Address"},
    {0x0A, (char *)"Target_IPv6_Address"},
    {0x0B, (char *)"SSU_SubGroupAssociation"},
    {0x57, (char *)"TelephoneDescriptor"},
    {0x5F, (char *)"PrivateDataSpecifier"},
    {0,NULL}
};

dmsSSU_UNT_DescriptorList::dmsSSU_UNT_DescriptorList() : dmsMPEG_DescriptorList()
{
    m_poNameTab = SSU_UNT_DescriptorNames;
}

dmsData* dmsSSU_UNT_DescriptorList::CreateData(dmsMPEG_Descriptor *desc, u8 descTag)
{
    switch (descTag)
    {
    case 0x01: return new dmsSSU_UNT_DescScheduling(desc);
    case 0x02: return new dmsSSU_UNT_DescUpdate(desc);
    case 0x03: return new dmsSSU_UNT_DescSSU_Location(desc);
    case 0x04: return new dmsSSU_UNT_DescMessage(desc);
    case 0x05: return new dmsSSU_UNT_DescSSU_EventName(desc);
    case 0x06: return new dmsSSU_UNT_DescTargetSmartCard(desc);
    case 0x07: return new dmsSSU_UNT_DescTargetMAC_Address(desc);
    case 0x08: return new dmsSSU_UNT_DescTargetSerialNumber(desc);
    case 0x09: return new dmsSSU_UNT_DescTargetIP_Address(desc);
    case 0x0A: return new dmsSSU_UNT_DescTarget_IPv6_Address(desc);
    case 0x0B: return new dmsSSU_UNT_DescSSU_SubGroupAssociation(desc);
    case 0x57: return new dmsSSU_UNT_DescTelephoneDescriptor(desc);
    case 0x5F: return new dmsSSU_UNT_DescPrivateDataSpecifier(desc);
    default:
        return NULL;
    }
}


/* ========================================================================
   Initialisations
   ======================================================================== */

/* ------------------------------------------------------------------------
   0x01 : Scheduling
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescScheduling::dmsSSU_UNT_DescScheduling(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(StartDateTime,          0);
    HDR_INIT(EndDateTime,            0);
    HDR_INIT(FinalAvailability,      1);
    HDR_INIT(PeriodicityFlag,        1);
    HDR_INIT(PeriodUnit,             2);
    HDR_INIT(DurationUnit,           2);
    HDR_INIT(EstimatedCycleTimeUnit, 2);
    HDR_INIT(Period,                 8);
    HDR_INIT(Duration,               8);
    HDR_INIT(EstimatedCycleTime,     8);
    HDR_INIT(PrivateData,            0);

    StartDateTime = new dmsDVB_UTC_Time();
    EndDateTime   = new dmsDVB_UTC_Time();
    PrivateData   = new dmsDataHexa();

    SetLoad(&StartDateTime,          false);
    SetLoad(&EndDateTime,            false);
    SetLoad(&FinalAvailability,      false);
    SetLoad(&PeriodicityFlag,        false);
    SetLoad(&PeriodUnit,             false);
    SetLoad(&DurationUnit,           false);
    SetLoad(&EstimatedCycleTimeUnit, false);
    SetLoad(&Period,                 false);
    SetLoad(&Duration,               false);
    SetLoad(&EstimatedCycleTime,     false);
    SetLoad(&PrivateData,            true);
}

/* ------------------------------------------------------------------------
   0x02 : Update
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescUpdate::dmsSSU_UNT_DescUpdate(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(UpdateFlag,     2);
    HDR_INIT(UpdateMethod,   4);
    HDR_INIT(UpdatePriority, 2);
    HDR_INIT(PrivateData,    0);

    PrivateData   = new dmsDataHexa();

    SetLoad(&UpdateFlag,     false);
    SetLoad(&UpdateMethod,   false);
    SetLoad(&UpdatePriority, false);
    SetLoad(&PrivateData,    true);
}

/* ------------------------------------------------------------------------
   0x03 : SSU_Location
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescSSU_Location::dmsSSU_UNT_DescSSU_Location(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(DataBroadcastId,        16);
    HDR_INIT(DataBroadcastIdSubData, 0);
    HDR_INIT(PrivateData,            0);

    PrivateData = new dmsDataHexa();

    SetLoad(&DataBroadcastId, false);
    SetLoad(&PrivateData,     true);
}


bool dmsSSU_UNT_DescSSU_Location::Load(wxXmlNode *node)
{
    dmsData::Load(node);

    switch (DataBroadcastId)
    {
    case 0x000A:
        SetData(&DataBroadcastIdSubData, new dmsSSU_UNT_DescSSU_Location0x000A());
        DataBroadcastIdSubData->Load(node->Find(""));
        break;
    default:
        LOGE(L"Unimplemented SSU Location Sub Data [0x%04X]", DataBroadcastId);
        return false;
    }

    return true;
}



dmsSSU_UNT_DescSSU_Location0x000A::dmsSSU_UNT_DescSSU_Location0x000A() : dmsData()
{
    HDR_INIT(AssociationTag, 16);

    SetLoad(&AssociationTag, false);
}


/* ------------------------------------------------------------------------
   0x04 : Message
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescMessage::dmsSSU_UNT_DescMessage(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(DescriptorNumber,     4);
    HDR_INIT(LastDescriptorNumber, 4);
    HDR_INIT(ISO_639_LanguageCode, 0);
    HDR_INIT(Text,                 0);

    ISO_639_LanguageCode = new dmsDataAscii(false);
    Text                 = new dmsDataAscii();

    SetLoad(&DescriptorNumber,     false);
    SetLoad(&LastDescriptorNumber, false);
    SetLoad(&ISO_639_LanguageCode, false);
    SetLoad(&Text,                 false);
}

/* ------------------------------------------------------------------------
   0x05 : SSU_EventName
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescSSU_EventName::dmsSSU_UNT_DescSSU_EventName(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(ISO_639_LanguageCode, 0);
    HDR_INIT(NameLength,           8);
    HDR_INIT(Name,                 0);
    HDR_INIT(TextLength,           8);
    HDR_INIT(Text,                 0);

    ISO_639_LanguageCode = new dmsDataAscii(false);
    Name                 = new dmsDataAscii();
    Text                 = new dmsDataAscii();

    SetLenLimit(&NameLength, &Name);
    SetLenLimit(&TextLength, &Text);

    SetLoad(&ISO_639_LanguageCode, false);
    SetLoad(&Name,                 false);
    SetLoad(&Text,                 false);
}

/* ------------------------------------------------------------------------
   0x06 : TargetSmartCard
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescTargetSmartCard::dmsSSU_UNT_DescTargetSmartCard(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(SuperCA_SystemId, 32);
    HDR_INIT(PrivateData,      0);

    PrivateData = new dmsDataHexa();

    SetLoad(&SuperCA_SystemId, false);
    SetLoad(&PrivateData,      true);
}

/* ------------------------------------------------------------------------
   0x07 : TargetMAC_Address
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescTargetMAC_AddressItem::dmsSSU_UNT_DescTargetMAC_AddressItem() : dmsData()
{
    HDR_INIT(MAC_AddrMatch, 0)

    MAC_AddrMatch = new dmsDataHexa();

    SetLoad((void *)&MAC_AddrMatch, (const char *)"", (bool)false);
}

dmsSSU_UNT_DescTargetMAC_Address::dmsSSU_UNT_DescTargetMAC_Address(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(MAC_AddrMask,      0)
    HDR_INIT(MAC_AddrMatchList, 0)

    MAC_AddrMask      = new dmsDataHexa();
    MAC_AddrMatchList = new dmsSSU_UNT_DescTargetMAC_AddressItemList();

    SetLoad(&MAC_AddrMask,      false);
    SetLoad(&MAC_AddrMatchList, false);
}

/* ------------------------------------------------------------------------
   0x08 : TargetSerialNumber
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescTargetSerialNumber::dmsSSU_UNT_DescTargetSerialNumber(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(SerialData,  0)

    SerialData = new dmsDataHexa();

    SetLoad(&SerialData, false);
}

/* ------------------------------------------------------------------------
   0x09 : TargetIP_Address
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescTargetIP_AddressItem::dmsSSU_UNT_DescTargetIP_AddressItem() : dmsData()
{
    HDR_INIT(IP_AddrMatch, 0)

    IP_AddrMatch = new dmsDataHexa();

    SetLoad((void *)&IP_AddrMatch, (const char *)"", (bool)false);
}

dmsSSU_UNT_DescTargetIP_Address::dmsSSU_UNT_DescTargetIP_Address(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(IP_AddrMask,      0)
    HDR_INIT(IP_AddrMatchList, 0)

    IP_AddrMask      = new dmsDataHexa();
    IP_AddrMatchList = new dmsSSU_UNT_DescTargetIP_AddressItemList();

    SetLoad(&IP_AddrMask,      false);
    SetLoad(&IP_AddrMatchList, false);
}

/* ------------------------------------------------------------------------
   0x0A : Target_IPv6_Address
   ------------------------------------------------------------------------ */


dmsSSU_UNT_DescTargetIPv6_AddressItem::dmsSSU_UNT_DescTargetIPv6_AddressItem() : dmsData()
{
    HDR_INIT(IPv6_AddrMatch, 0)

    IPv6_AddrMatch = new dmsDataHexa();

    SetLoad((void *)&IPv6_AddrMatch, (const char *)"", (bool)false);
}

dmsSSU_UNT_DescTarget_IPv6_Address::dmsSSU_UNT_DescTarget_IPv6_Address(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(IPv6_AddrMask,      0)
    HDR_INIT(IPv6_AddrMatchList, 0)

    IPv6_AddrMask      = new dmsDataHexa();
    IPv6_AddrMatchList = new dmsSSU_UNT_DescTargetIPv6_AddressItemList();

    SetLoad(&IPv6_AddrMask,      false);
    SetLoad(&IPv6_AddrMatchList, false);
}

/* ------------------------------------------------------------------------
   0x0B : SSU_SubGroupAssociation
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescSSU_SubGroupTag::dmsSSU_UNT_DescSSU_SubGroupTag() : dmsData()
{
    HDR_INIT(OUI,                           24);
    HDR_INIT(SubGroupAssociationDescriptor, 16);

    SetLoad(&OUI,                           false);
    SetLoad(&SubGroupAssociationDescriptor, false);
}


dmsSSU_UNT_DescSSU_SubGroupAssociation::dmsSSU_UNT_DescSSU_SubGroupAssociation(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(SubGroupTag, 0);

    SubGroupTag = new dmsSSU_UNT_DescSSU_SubGroupTag();

    SetLoad(&SubGroupTag, false);
}


/* ------------------------------------------------------------------------
   0x57 : TelephoneDescriptor
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescTelephoneDescriptor::dmsSSU_UNT_DescTelephoneDescriptor(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(Reserved1,                   2);
    HDR_INIT(ForeignAvailability,         1);
    HDR_INIT(ConnectionType,              5);
    HDR_INIT(Reserved2,                   1);
    HDR_INIT(CountryPrefixLength,         2);
    HDR_INIT(InternationalAreaCodeLength, 3);
    HDR_INIT(OperatorCodeLength,          2);
    HDR_INIT(Reserved3,                   1);
    HDR_INIT(NationalAreaCodeLength,      3);
    HDR_INIT(CoreNumberLength,            4);
    HDR_INIT(CountryPrefix,               0);
    HDR_INIT(InternationalAreaCode,       0);
    HDR_INIT(OperatorCode,                0);
    HDR_INIT(NationalAreaCode,            0);
    HDR_INIT(CoreNumber,                  0);

    CountryPrefix         = new dmsDataAscii(false);
    InternationalAreaCode = new dmsDataAscii(false);
    OperatorCode          = new dmsDataAscii(false);
    NationalAreaCode      = new dmsDataAscii(false);
    CoreNumber            = new dmsDataAscii(false);

    Reserved1 = 3;
    Reserved2 = 1;
    Reserved3 = 1;

    SetLoad(&ForeignAvailability,         false);
    SetLoad(&ConnectionType,              false);
    SetLoad(&CountryPrefix,               false);
    SetLoad(&InternationalAreaCode,       false);
    SetLoad(&OperatorCode,                false);
    SetLoad(&NationalAreaCode,            false);
    SetLoad(&CoreNumber,                  false);

    SetDataLen(&CountryPrefixLength,         CountryPrefix);
    SetDataLen(&InternationalAreaCodeLength, InternationalAreaCode);
    SetDataLen(&OperatorCodeLength,          OperatorCode);
    SetDataLen(&NationalAreaCodeLength,      NationalAreaCode);
    SetDataLen(&CoreNumberLength,            CoreNumber);
}

/* ------------------------------------------------------------------------
   0x5F : PrivateDataSpecifier
   ------------------------------------------------------------------------ */

dmsSSU_UNT_DescPrivateDataSpecifier::dmsSSU_UNT_DescPrivateDataSpecifier(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(PrivateDataSpecifier, 32);

    SetLoad(&PrivateDataSpecifier, "", false);
}
