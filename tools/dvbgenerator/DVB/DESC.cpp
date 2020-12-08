/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.00 - 10/2011 - Add Service List Descriptor (tag
                                          0x41) and the specific Nagra
                                          Channel Descriptor (tag 0x82)
   - LIPPA - SmarDTV - v 2.10 - 01/2012 - Add Usage ID in PMT in Protocol ID 3
   ************************************************************************ */

#include <Tools/Xml.h>
#include <Tools/Conversion.h>
#include <Tools/Validator.h>

#include "DESC.h"


/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsNameRank DVB_DescriptorNames[] =
{
    {0x15, (char *)"DeferredAssociationTag"},
    {0x4A, (char *)"Linkage"},
    {0x41, (char *)"ServiceListDescriptor"},
    {0x43, (char *)"SatelliteDeliverySystem"},
    {0x44, (char *)"CableDeliverySystem"},
    {0x48, (char *)"Service"},
    {0x52, (char *)"StreamIdentifier"},
    {0x5A, (char *)"TerrestrialDeliverySystem"},
    {0x62, (char *)"FrequencyList"},
    {0x66, (char *)"DataBroadcastId"},
    {0x82, (char *)"NagraChannelDescriptor"},
    {0,NULL}
};

dmsDVB_DescriptorList::dmsDVB_DescriptorList() : dmsMPEG_DescriptorList()
{
    m_poNameTab = DVB_DescriptorNames;
}

dmsData* dmsDVB_DescriptorList::CreateData(dmsMPEG_Descriptor *desc, u8 descTag)
{
    switch (descTag)
    {
    case 0x15: return new dmsDVB_DescDeferredAssociationTag(desc);
    case 0x4A: return new dmsDVB_DescLinkage(desc);
    case 0x41: return new dmsDVB_ServiceListDescriptor(desc);
    case 0x43: return new dmsDVB_DescSatelliteDeliverySystem(desc);
    case 0x44: return new dmsDVB_DescCableDeliverySystem(desc);
    case 0x48: return new dmsDVB_DescService(desc);
    case 0x52: return new dmsDVB_DescStreamIdentifier(desc);
    case 0x5A: return new dmsDVB_DescTerrestrialDeliverySystem(desc);
    case 0x62: return new dmsDVB_DescFrequencyList(desc);
    case 0x66: return new dmsDVB_DescDataBroadcastId(desc);
    case 0x82: return new dmsDVB_NagraChannelDescriptor(desc);
    default:
        return NULL;
    }
}

/* ########################################################################

   ######################################################################## */


/* ========================================================================
   0x15 : Deferred Association Tag
   ======================================================================== */

dmsDVB_DescDeferredAssociationTag::dmsDVB_DescDeferredAssociationTag(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(AssociationTagsLoopLength, 8);
    HDR_INIT(AssociationTagsLoop,       0);
    HDR_INIT(TransportStreamId,         16);
    HDR_INIT(ProgramNumber,             16);
    HDR_INIT(PrivateData,               0);

    AssociationTagsLoop = new dmsDVB_DescAssociationTagList();
    PrivateData         = new dmsDataHexa();

    SetLenLimit(&AssociationTagsLoopLength, &AssociationTagsLoop);

    SetLoad(&AssociationTagsLoop, false);
    SetLoad(&TransportStreamId, false);
    SetLoad(&ProgramNumber,     false);
    SetLoad(&PrivateData,       true);
}


dmsDVB_DescAssociationTag::dmsDVB_DescAssociationTag() : dmsData()
{
    HDR_INIT(AssociationTag, 16);

    SetLoad(&AssociationTag, "", false);
}

/* ========================================================================
   0x4A : Linkage
   ======================================================================== */


dmsDVB_DescLinkage::dmsDVB_DescLinkage(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(TransportStreamId, 16);
    HDR_INIT(OriginalNetworkId,  16);
    HDR_INIT(ServiceId,         16);
    HDR_INIT(LinkageType,        8);
    HDR_INIT(Data,               0);

    SetLoad(&TransportStreamId, false);
    SetLoad(&OriginalNetworkId, false);
    SetLoad(&ServiceId,         false);
    SetLoad(&LinkageType,       false);
}

bool dmsDVB_DescLinkage::Load(wxXmlNode *node)
{
    dmsData::Load(node);

    wxXmlNode *subnode;

    switch (LinkageType)
    {
    case 0x09:
        SetData(&Data, new dmsDVB_DescSystemSoftwareUpdateLinkStructure());
        subnode = node->Find("SystemSoftwareUpdateLink", true);
        break;
    case 0x0A:
        SetData(&Data, new dmsDVB_DescTableType());
        subnode = node->Find("TableType", true);
        break;
    case 0x81:
        SetData(&Data, new dmsDVB_DescDataManufacturerVersionIdList());
        subnode = node->Find("Loop", true);
        break;
    default:
        SetData(&Data, new dmsDataHexa());
        subnode = node->Find("PrivateData");
        //LOGE("Unimplemented Linkage Type 0x%02x", LinkageType);
        break;
    }

    if (subnode) return Data->Load(subnode);

    return true;
}



/* ------------------------------------------------------------------------
   Type 0x09
   ------------------------------------------------------------------------ */

dmsDVB_DescSystemSoftwareUpdateLinkStructure::dmsDVB_DescSystemSoftwareUpdateLinkStructure() : dmsData()
{
    HDR_INIT(OUI_DataLength, 8);
    HDR_INIT(OUI_Loop,       0);
    HDR_INIT(PrivateData,    0);

    OUI_Loop    = new dmsDVB_DescLinkageOuiList();
    PrivateData = new dmsDataHexa();

    SetLenLimit(&OUI_DataLength, &OUI_Loop);

    SetLoad(&OUI_Loop,    false);
    SetLoad(&PrivateData, true);
}


dmsDVB_DescLinkageOui::dmsDVB_DescLinkageOui() : dmsData()
{
    HDR_INIT(OUI,            24);
    HDR_INIT(SelectorLength, 8);
    HDR_INIT(SelectorData,   0);

    SelectorData = new dmsDataHexa();

    SetLoad(&OUI, "");
    SetLoad(&SelectorData, true);

    SetLenLimit(&SelectorLength, &SelectorData);

}

/* ------------------------------------------------------------------------
   Type 0x0A
   ------------------------------------------------------------------------ */


dmsNameRank TableTypeNames[] =
{
    {0, (char *)"Not defined"},
    {1, (char *)"NIT"},
    {2, (char *)"BAT"},
    {0, NULL}
};

dmsDVB_DescTableType::dmsDVB_DescTableType() : dmsData()
{
    HDR_INIT(TableType, 8);

    SetLoad(&TableType, "", false);

    SetValues(&TableType, TableTypeNames);
}


/* ------------------------------------------------------------------------
   Type 0x81
   ------------------------------------------------------------------------ */

dmsDVB_DescDataManufacturerVersionId::dmsDVB_DescDataManufacturerVersionId() : dmsData()
{
    HDR_INIT(ManufacturerId, 24);
    HDR_INIT(VersionId,      32);

    SetLoad(&ManufacturerId, false);
    SetLoad(&VersionId,      false);
}

/* ========================================================================
   0x41 : Service List Descriptor
   ======================================================================== */

dmsDVB_ServiceListDescItem::dmsDVB_ServiceListDescItem() : dmsData()
{
    HDR_INIT(ServiceId,  16);
    HDR_INIT(ServiceType, 8);

    SetLoad(&ServiceId,  "ServiceId",false);
    SetLoad(&ServiceType,"ServiceType",false);
}

dmsDVB_ServiceListDescriptor::dmsDVB_ServiceListDescriptor(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(Loop,0);
    Loop = new dmsDVB_ServiceListDescItemList();
    SetLoad(&Loop,"Loop",false);
}

/* ========================================================================
   0x41 : Service List Descriptor
   ======================================================================== */

dmsDVB_NagraChannelDescItem::dmsDVB_NagraChannelDescItem() : dmsData()
{
    HDR_INIT(ServiceId,    16);
    HDR_INIT(ChannelNumber,16);

    SetLoad(&ServiceId,    "ServiceId",    false);
    SetLoad(&ChannelNumber,"ChannelNumber",false);
}

dmsDVB_NagraChannelDescriptor::dmsDVB_NagraChannelDescriptor(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(Loop,0);
    Loop = new dmsDVB_NagraChannelDescItemList();
    SetLoad(&Loop,"Loop",false);
}

/* ========================================================================
   0x82 : Nagra Channel Descriptor (specific Nagra)
   ======================================================================== */

dmsNameRank SDSDN_WestEastFlag[] =
{
    {0, (char *)"Western"},
    {1, (char *)"Eastern"},
    {0, NULL}
};

dmsNameRank SDSDN_Polarization[] =
{
    {0, (char *)"Linear - horizontal"},
    {1, (char *)"Linear - vertical"},
    {2, (char *)"Circular - left"},
    {3, (char *)"Circular - right"},
    {0, NULL}
};

dmsNameRank SDSDN_Modulation[] =
{
    {0, (char *)"Not defined"},
    {1, (char *)"QPSK"},
    {2, (char *)"8PSK"},
    {3, (char *)"16-QAM"},
    {0, NULL}
};


dmsNameRank SDSDN_FEC_Inner[] =
{
    {0, (char *)"Not defined"},
    {1, (char *)"1/2"},
    {2, (char *)"2/3"},
    {3, (char *)"3/4"},
    {4, (char *)"5/6"},
    {5, (char *)"7/8"},
    {6, (char *)"8/9"},
    {7, (char *)"No conv coding"},
    {0, NULL}
};

dmsDVB_DescSatelliteDeliverySystem::dmsDVB_DescSatelliteDeliverySystem(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(Frequency,       32);
    HDR_INIT(OrbitalPosition, 16);
    HDR_INIT(WestEastFlag,    1);
    HDR_INIT(Polarization,    2);
    HDR_INIT(Modulation,      5);
    HDR_INIT(SymbolRate,      28);
    HDR_INIT(FEC_Inner,       4);

    SetLoad(&Frequency,       false);
    SetLoad(&OrbitalPosition, false);
    SetLoad(&WestEastFlag,    false);
    SetLoad(&Polarization,    false);
    SetLoad(&Modulation,      false);
    SetLoad(&SymbolRate,      false);
    SetLoad(&FEC_Inner,       false);

    SetMapping(&Frequency, new dmsvNumeric(":base:bcd"));
    SetMapping(&OrbitalPosition, new dmsvNumeric(":base:bcd"));
    SetValues(&WestEastFlag, SDSDN_WestEastFlag);
    SetValues(&Polarization, SDSDN_Polarization);
    SetValues(&Modulation,   SDSDN_Modulation);
    SetMapping(&SymbolRate, new dmsvNumeric(":base:bcd"));
    SetValues(&FEC_Inner,   SDSDN_FEC_Inner);
}

/* ========================================================================
   0x44 : Cable Delivery System
   ======================================================================== */

dmsNameRank CDSDN_FEC_Outer[] =
{
    {0, (char *)"Not defined"},
    {1, (char *)"No FEC Coding"},
    {2, (char *)"RS(204/188)"},
    {0, NULL}
};


dmsNameRank CDSDN_Modulation[] =
{
    {0, (char *)"Not defined"},
    {1, (char *)"16-QAM"},
    {2, (char *)"32-QAM"},
    {3, (char *)"64-QAM"},
    {4, (char *)"128-QAM"},
    {5, (char *)"256-QAM"},
    {0, NULL}
};

dmsNameRank CDSDN_FEC_Inner[] =
{
    {0, (char *)"Not defined"},
    {1, (char *)"1/2"},
    {2, (char *)"2/3"},
    {3, (char *)"3/4"},
    {4, (char *)"5/6"},
    {5, (char *)"7/8"},
    {6, (char *)"8/9"},
    {7, (char *)"3/5"},
    {8, (char *)"4/5"},
    {9, (char *)"9/10"},
    {15, (char *)"No conv coding"},
    {0, NULL}
};

dmsDVB_DescCableDeliverySystem::dmsDVB_DescCableDeliverySystem(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(Frequency,  32);
    HDR_INIT(Reserved,   12);
    HDR_INIT(FEC_Outer,  4);
    HDR_INIT(Modulation, 8);
    HDR_INIT(SymbolRate, 28);
    HDR_INIT(FEC_Inner,  4);

    SetLoad(&Frequency,  false);
    SetLoad(&FEC_Outer,  false);
    SetLoad(&Modulation, false);
    SetLoad(&SymbolRate, false);
    SetLoad(&FEC_Inner,  false);

    SetMapping(&Frequency, new dmsvNumeric(":base:bcd"));
    SetValues(&FEC_Outer,  CDSDN_FEC_Outer);
    SetValues(&Modulation, CDSDN_Modulation);
    SetMapping(&SymbolRate, new dmsvNumeric(":base:bcd"));
    SetValues(&FEC_Inner,   CDSDN_FEC_Inner);
}


/* ========================================================================
   0x48 : Service
   ======================================================================== */

dmsDVB_DescService::dmsDVB_DescService(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(ServiceType,               8);
    HDR_INIT(ServiceProviderNameLength, 8);
    HDR_INIT(ServiceProviderName,       0);
    HDR_INIT(ServiceNameLength,         8);
    HDR_INIT(ServiceName,               0);

    ServiceProviderName = new dmsDataAscii(false);
    ServiceName         = new dmsDataAscii(false);

    SetLoad(&ServiceType,         false);
    SetLoad(&ServiceProviderName, false);
    SetLoad(&ServiceName,         false);

    SetLenLimit(&ServiceProviderNameLength, &ServiceProviderName);
    SetLenLimit(&ServiceNameLength,         &ServiceName);
}



/* ========================================================================
   0x52 : Stream Identifier
   ======================================================================== */

dmsDVB_DescStreamIdentifier::dmsDVB_DescStreamIdentifier(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(ComponentTag, 8);

    SetLoad(&ComponentTag, false);
}


/* ========================================================================
   0x5A : Terrestrial Delivery System
   ======================================================================== */

dmsNameRank TDSDN_Bandwidth[] =
{
    {0, (char *)"8 MHz"},
    {1, (char *)"7 MHz"},
    {2, (char *)"6 MHz"},
    {0, NULL}
};

dmsNameRank TDSDN_Priority[] =
{
    {0, (char *)"Low"},
    {1, (char *)"High"},
    {0, NULL}
};

dmsNameRank TDSDN_Indicator[] =
{
    {0, (char *)"Used"},
    {1, (char *)"Not Used"},
    {0, NULL}
};

dmsNameRank TDSDN_Constellation[] =
{
    {0, (char *)"QPSK"},
    {1, (char *)"16-QAM"},
    {2, (char *)"64-QAM"},
    {0, NULL}
};

dmsNameRank TDSDN_HierarchyInformation[] =
{
    {0, (char *)"non-hierarchical"},
    {1, (char *)"alpha-1"},
    {2, (char *)"alpha-2"},
    {3, (char *)"alpha-3"},
    {0, NULL}
};

dmsNameRank TDSDN_CodeRate[] =
{
    {0, (char *)"1/2"},
    {1, (char *)"2/3"},
    {2, (char *)"3/4"},
    {3, (char *)"5/6"},
    {4, (char *)"7/8"},
    {0, NULL}
};

dmsNameRank TDSDN_GuardInterval[] =
{
    {0, (char *)"1/32"},
    {1, (char *)"1/16"},
    {2, (char *)"1/8"},
    {3, (char *)"1/4"},
    {0, NULL}
};

dmsNameRank TDSDN_TransmissionMode[] =
{
    {0, (char *)"2k"},
    {1, (char *)"8k"},
    {0, NULL}
};

dmsNameRank TDSDN_OtherFrequencyFlag[] =
{
    {0, (char *)"no other used"},
    {1, (char *)"other used"},
    {0, NULL}
};

dmsDVB_DescTerrestrialDeliverySystem::dmsDVB_DescTerrestrialDeliverySystem(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(CentreFrequency,      32);
    HDR_INIT(Bandwidth,            3);
    HDR_INIT(Priority,             1);
    HDR_INIT(TimeSlicingIndicator, 1);
    HDR_INIT(MPE_FEC_Indicator,    1);
    HDR_INIT(Reserved1,            2);
    HDR_INIT(Constellation,        2);
    HDR_INIT(HierarchyInformation, 3);
    HDR_INIT(CodeRateHP_Stream,    3);
    HDR_INIT(CodeRateLP_Stream,    3);
    HDR_INIT(GuardInterval,        2);
    HDR_INIT(TransmissionMode,     2);
    HDR_INIT(OtherFrequencyFlag,   1);
    HDR_INIT(Reserved2,            32);

    Reserved1 = 31;
    Reserved2 = 0xFFFFFFFF;

    SetLoad(&CentreFrequency,      false);
    SetLoad(&Bandwidth,            false);
    SetLoad(&Priority,             false);
    SetLoad(&TimeSlicingIndicator, false);
    SetLoad(&MPE_FEC_Indicator,    false);
    SetLoad(&Constellation,        false);
    SetLoad(&HierarchyInformation, false);
    SetLoad(&CodeRateHP_Stream,    false);
    SetLoad(&CodeRateLP_Stream,    false);
    SetLoad(&GuardInterval,        false);
    SetLoad(&TransmissionMode,     false);
    SetLoad(&OtherFrequencyFlag,   false);

    SetValues(&Bandwidth,            TDSDN_Bandwidth);
    SetValues(&Priority,             TDSDN_Priority);
    SetValues(&TimeSlicingIndicator, TDSDN_Indicator);
    SetValues(&MPE_FEC_Indicator,    TDSDN_Indicator);
    SetValues(&Constellation,        TDSDN_Constellation);
    SetValues(&HierarchyInformation, TDSDN_HierarchyInformation);
    SetValues(&CodeRateHP_Stream,    TDSDN_CodeRate);
    SetValues(&CodeRateLP_Stream,    TDSDN_CodeRate);
    SetValues(&GuardInterval,        TDSDN_GuardInterval);
    SetValues(&TransmissionMode,     TDSDN_TransmissionMode);
    SetValues(&OtherFrequencyFlag,   TDSDN_OtherFrequencyFlag);
}


/* ========================================================================
   Frequency List
   ======================================================================== */

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

dmsDVB_DescFrequencyList::dmsDVB_DescFrequencyList(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(Reserved,   6);
    HDR_INIT(CodingType, 2);
    HDR_INIT(Loop,       0);

    Reserved = 0x63;

    Loop = new dmsDVB_DescFrequencyListLoop();

    SetLoad(&CodingType, false);
    SetLoad(&Loop, "Loop");
}

dmsDVB_DescFrequencyListItem::dmsDVB_DescFrequencyListItem() : dmsData()
{
    HDR_INIT(CentreFrequency,   32);

    SetLoad(&CentreFrequency, "", false);
}

dmsData *dmsDVB_DescFrequencyListLoop::Create(void *pt, const char *tag)
{
    return new dmsDVB_DescFrequencyListItem();
}

/* ========================================================================
   0x66 : Data Broadcast ID
   ======================================================================== */

dmsDVB_DescDataBroadcastId::dmsDVB_DescDataBroadcastId(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(DataBroadcastId, 16);
    HDR_INIT(IdSelector,      0);

    SetLoad(&DataBroadcastId, "", false);
}


bool dmsDVB_DescDataBroadcastId::Load(wxXmlNode *node)
{
    dmsData::Load(node);

    wxXmlNode *subnode;

    switch (DataBroadcastId)
    {
    case 0x000A:
        SetData(&IdSelector, new dmsDVB_SystemSoftwareUpdateInfo());
        subnode = node->Find("SystemSoftwareUpdateInfo", true);
        break;

    default:
        SetData(&IdSelector, new dmsDataHexa());
        subnode = node->Find("IdSelector");
        break;
        //LOGE("Unimplemented DataBroadcast Id 0x%04x", DataBroadcastId);
    }

    if (subnode) return IdSelector->Load(subnode);

    return true;
}


dmsDVB_SystemSoftwareUpdateInfo::dmsDVB_SystemSoftwareUpdateInfo() : dmsData()
{
    HDR_INIT(OUI_DataLength, 8);
    HDR_INIT(OUI_Loop,       0);
    HDR_INIT(PrivateData,    0);

    OUI_Loop = new dmsDVB_SystemSoftwareUpdateInfoItemList();

    SetLenLimit(&OUI_DataLength, &OUI_Loop);
    SetLoad(&OUI_Loop, "");
    SetUnimplemented(&PrivateData);
}


dmsDVB_SystemSoftwareUpdateInfoItem::dmsDVB_SystemSoftwareUpdateInfoItem() : dmsData()
{
    HDR_INIT(OUI,                  24);
    HDR_INIT(Reserved1,            4);
    HDR_INIT(UpdateType,           4);
    HDR_INIT(Reserved2,            2);
    HDR_INIT(UpdateVersioningFlag, 1);
    HDR_INIT(UpdateVersion,        5);
    HDR_INIT(SelectorLength,       8);
    HDR_INIT(Selector,             0);

    Reserved1  = 15;
    Reserved2  = 3;

    SetLoad(&OUI,                  false);
    SetLoad(&UpdateVersioningFlag, true);
    SetLoad(&UpdateVersion,        true);
    //SetLoad(&Selector,             false);
    SetLoad(&UpdateType,           true);

    SetLenLimit(&SelectorLength, &Selector);
}

bool dmsDVB_SystemSoftwareUpdateInfoItem::Load(wxXmlNode *node)
{
    dmsData::Load(node);

    wxXmlNode *subnode = node->Find("Selector", true);

    if (subnode==NULL) return false;

   if (subnode->Find("UsageIdList"))
        SetData(&Selector, new dmsSSU_SSUI_Proto3_Selector());
    else if (subnode->Find("ProtocolId"))
        SetData(&Selector, new dmsSSU_SSUI_Selector());
    else
        SetData(&Selector, new dmsDataHexa());

    return Selector->Load(subnode);
}

int i = 0;
dmsSSU_SSUI_Selector::dmsSSU_SSUI_Selector() : dmsData()
{
    HDR_INIT(ProtocolId,        16);
    HDR_INIT(HW_ProfileModel,   32);
    HDR_INIT(HW_ProfileRelease, 16);
    HDR_INIT(HW_ProfileVersion, 16);
    HDR_INIT(SW_Version,        32);
    HDR_INIT(GroupIdLoop,       0);

   i=0;
    SetLoad(&ProtocolId,        false);
    SetLoad(&HW_ProfileModel,   false);
    SetLoad(&HW_ProfileRelease, false);
    SetLoad(&HW_ProfileVersion, false);
    SetLoad(&SW_Version,        false);

    GroupIdLoop = new  dmsDVB_GroupIdList();
    SetLenLimit(&GroupId_Count, &GroupIdLoop);
    SetLoad(&GroupIdLoop, false);
}

bool dmsSSU_SSUI_Selector::Load(wxXmlNode *node)
{
    dmsData::Load(node);


    switch (ProtocolId)
    {
    case 0x0001:
            GroupIdLoop = new  dmsDVB_GroupIdList();
            node->Find("GroupIdLoop", true);
        break;
   case 0x0002:
            node->Find("GroupIdLoop", true);
        break;
    default:
            GroupIdLoop = new  dmsDVB_GroupIdList();
            node->Find("GroupIdLoop", true);
        break;
    }
    return true;
}

dmsSSU_SSUI_Proto3_Selector::dmsSSU_SSUI_Proto3_Selector() : dmsData()
{
    HDR_INIT(ProtocolId,       16);
    HDR_INIT(HW_ProfileModel,  32);
    HDR_INIT(HW_ProfileRelease,16);
    HDR_INIT(HW_ProfileVersion,16);
    HDR_INIT(UsageIdCount,     8);
    HDR_INIT(UsageIdList,      0);

   i=0;
    SetLoad(&ProtocolId,        false);
    SetLoad(&HW_ProfileModel,   false);
    SetLoad(&HW_ProfileRelease, false);
    SetLoad(&HW_ProfileVersion, false);

   UsageIdList = new dmsDVB_UsageIdList();
   SetLoad(&UsageIdList, true);
    SetListCount(&UsageIdCount, UsageIdList);
}

/*
bool dmsSSU_SSUI_Proto3_Selector::Load(wxXmlNode *node)
{
    dmsData::Load(node);
   return true;
}
*/
dmsDVB_GroupId::dmsDVB_GroupId() : dmsData()
{

    HDR_INIT(GroupId,            8);

    if (i<100)
    {
        SetLoad(&GroupId, "");
        i++;
    }
    else
    {
        SetLoad(&GroupId, "ERROR Too much GroupID: 100 max");
    }
}

dmsData *dmsDVB_UsageIdList::Create(void *pt, const char *tag)
{
   return new dmsDVB_SubDescUsageID();
}

dmsDVB_SubDescUsageID::dmsDVB_SubDescUsageID() : dmsData()
{
    HDR_INIT(usage_id,        8);
    SetLoad(&usage_id,false);
}
