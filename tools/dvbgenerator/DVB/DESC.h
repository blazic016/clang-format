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

#ifndef _DMS_LIB_DVB_DESCRIPTOR_H_
#define _DMS_LIB_DVB_DESCRIPTOR_H_

#include <MPEG/DESC.h>
#include <Tools/Header.h>


/* ========================================================================
   Creation de descripteurs
   ======================================================================== */

class dmsDVB_DescriptorList : public dmsMPEG_DescriptorList
{
public:
    dmsDVB_DescriptorList();

    dmsData* CreateData(dmsMPEG_Descriptor *desc, u8 descTag);
};

/* ========================================================================
   Entetes
   ======================================================================== */

/* ------------------------------------------------------------------------
   0x15 : Deferred Association Tag
   ------------------------------------------------------------------------ */

class dmsDVB_DescAssociationTag : public dmsData
{
public:
    u16 AssociationTag;

    dmsDVB_DescAssociationTag();
};


HDR_DEFINE_LIST(dmsDVB_DescAssociationTag);


class dmsDVB_DescDeferredAssociationTag : public dmsData
{
public:
    u_8                            AssociationTagsLoopLength;
    dmsDVB_DescAssociationTagList* AssociationTagsLoop;
    u16                            TransportStreamId;
    u16                            ProgramNumber;
    dmsData*                       PrivateData;

    dmsDVB_DescDeferredAssociationTag(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x4A : Linkage
   ------------------------------------------------------------------------ */

class dmsDVB_DescLinkage : public dmsData
{
public:
    u16 TransportStreamId;
    u16 OriginalNetworkId;
    u16 ServiceId;
    u_8 LinkageType;
    dmsData *Data;

    dmsDVB_DescLinkage(dmsMPEG_Descriptor *desc);

    bool Load(wxXmlNode *node);
};



// Data 0x09


class dmsDVB_DescLinkageOui : public dmsData
{
public:
    u24      OUI;
    u8       SelectorLength;
    dmsData* SelectorData;

    dmsDVB_DescLinkageOui();
};

HDR_DEFINE_LIST(dmsDVB_DescLinkageOui);

class dmsDVB_DescSystemSoftwareUpdateLinkStructure : public dmsData
{
public:
    u8                         OUI_DataLength;
    dmsDVB_DescLinkageOuiList* OUI_Loop;
    dmsData*                   PrivateData;

    dmsDVB_DescSystemSoftwareUpdateLinkStructure();
};

// Data 0x0A

class dmsDVB_DescTableType : public dmsData
{
public:
    u8 TableType;

    dmsDVB_DescTableType();
};

// Data 0x81

class dmsDVB_DescDataManufacturerVersionId : public dmsData
{
public:
    u24 ManufacturerId;
    u32 VersionId;

    dmsDVB_DescDataManufacturerVersionId();
};

HDR_DEFINE_LIST(dmsDVB_DescDataManufacturerVersionId);

/* ------------------------------------------------------------------------
   0x41 : Service List Descriptor
   ------------------------------------------------------------------------ */

class dmsDVB_ServiceListDescItem : public dmsData
{
public:
    u16 ServiceId;
    u8  ServiceType;

    dmsDVB_ServiceListDescItem();
};

HDR_DEFINE_LIST(dmsDVB_ServiceListDescItem);

class dmsDVB_ServiceListDescriptor : public dmsData
{
public:
   dmsDVB_ServiceListDescItemList *Loop;

    dmsDVB_ServiceListDescriptor(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x82 : Nagra Channel Descriptor (specific Nagra)
   ------------------------------------------------------------------------ */

class dmsDVB_NagraChannelDescItem : public dmsData
{
public:
    u16 ServiceId;
    u16 ChannelNumber;

    dmsDVB_NagraChannelDescItem();
};

HDR_DEFINE_LIST(dmsDVB_NagraChannelDescItem);

class dmsDVB_NagraChannelDescriptor : public dmsData
{
public:
   dmsDVB_NagraChannelDescItemList *Loop;

    dmsDVB_NagraChannelDescriptor(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x43 : Satellite Delivery System
   ------------------------------------------------------------------------ */

class dmsDVB_DescSatelliteDeliverySystem : public dmsData
{
public:
    u32 Frequency;
    u16 OrbitalPosition;
    u1  WestEastFlag;
    u2  Polarization;
    u5  Modulation;
    u28 SymbolRate;
    u4  FEC_Inner;

    dmsDVB_DescSatelliteDeliverySystem(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x44 : Cable Delivery System
   ------------------------------------------------------------------------ */

class dmsDVB_DescCableDeliverySystem : public dmsData
{
public:
    u32 Frequency;
    u12 Reserved;
    u4  FEC_Outer;
    u8  Modulation;
    u28 SymbolRate;
    u4  FEC_Inner;

    dmsDVB_DescCableDeliverySystem(dmsMPEG_Descriptor *desc);
};


/* ------------------------------------------------------------------------
   0x48 : Service
   ------------------------------------------------------------------------ */

class dmsDVB_DescService : public dmsData
{
public:
    u_8      ServiceType;
    u_8      ServiceProviderNameLength;
    dmsData* ServiceProviderName;
    u_8      ServiceNameLength;
    dmsData* ServiceName;

    dmsDVB_DescService(dmsMPEG_Descriptor *desc);
};


/* ------------------------------------------------------------------------
   0x52 : Stream Identifier
   ------------------------------------------------------------------------ */

class dmsDVB_DescStreamIdentifier : public dmsData
{
public:
    u_8 ComponentTag;

    dmsDVB_DescStreamIdentifier(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x5A : Terrestrial Delivery System
   ------------------------------------------------------------------------ */

class dmsDVB_DescTerrestrialDeliverySystem : public dmsData
{
public:
    u32 CentreFrequency;
    u3  Bandwidth;
    u1  Priority;
    u1  TimeSlicingIndicator;
    u1  MPE_FEC_Indicator;
    u2  Reserved1;
    u2  Constellation;
    u3  HierarchyInformation;
    u3  CodeRateHP_Stream;
    u3  CodeRateLP_Stream;
    u2  GuardInterval;
    u2  TransmissionMode;
    u1  OtherFrequencyFlag;
    u32 Reserved2;

    dmsDVB_DescTerrestrialDeliverySystem(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x62 : Frequency List
   ------------------------------------------------------------------------ */

class dmsDVB_DescFrequencyListItem : public dmsData
{
public:
    u32 CentreFrequency;

    dmsDVB_DescFrequencyListItem();
};

class dmsDVB_DescFrequencyListLoop : public dmsDataList
{
public:
    dmsDVB_DescFrequencyListLoop():dmsDataList(){;}
    dmsData *Create(void *pt, const char *tag);
};

class dmsDVB_DescFrequencyList : public dmsData
{
public:
    u6 Reserved;
    u2 CodingType;
    dmsDVB_DescFrequencyListLoop* Loop;

    dmsDVB_DescFrequencyList(dmsMPEG_Descriptor *desc);
};


/* ------------------------------------------------------------------------
   0x66 : Data Broadcast ID
   ------------------------------------------------------------------------ */

class dmsDVB_GroupId : public dmsData
{
public:
    u8 GroupId;

    dmsDVB_GroupId();
};

HDR_DEFINE_LIST(dmsDVB_GroupId);

class dmsDVB_SubDescUsageID : public dmsData
{
public:
    u8  usage_id;
   dmsDVB_SubDescUsageID();
};

class dmsDVB_UsageIdList : public dmsDataList
{
public:
    dmsDVB_UsageIdList():dmsDataList(){;}
    dmsData *Create(void *pt, const char *tag);
};

class dmsSSU_SSUI_Selector : public dmsData
{
public:
    u16 ProtocolId;
    u32 HW_ProfileModel;
    u16 HW_ProfileRelease;
    u16 HW_ProfileVersion;
    u32 SW_Version;

    u_8             GroupId_Count;
    dmsDVB_GroupIdList* GroupIdLoop;

    dmsSSU_SSUI_Selector();

    bool Load(wxXmlNode *node);
};

/**
 *  Special SSU SSUI Selector when protocol ID is 0x00003
 *  Idem dmsSSU_SSUI_Selector expect:
 *  - No SW_Version
 *  + UsageIdList
 */
class dmsSSU_SSUI_Proto3_Selector : public dmsData
{
public:
   u16 ProtocolId;
   u32 HW_ProfileModel;
   u16 HW_ProfileRelease;
   u16 HW_ProfileVersion;

   u8                  UsageIdCount;
   dmsDVB_UsageIdList *UsageIdList;

   dmsSSU_SSUI_Proto3_Selector();
/*
   bool Load(wxXmlNode *node);
*/
};

class dmsDVB_SystemSoftwareUpdateInfoItem : public dmsData
{
public:
    u24      OUI;
    u4       Reserved1;
    u4       UpdateType;
    u2       Reserved2;
    u1       UpdateVersioningFlag;
    u5       UpdateVersion;
    u8       SelectorLength;
    dmsData* Selector;

    dmsDVB_SystemSoftwareUpdateInfoItem();

    bool Load(wxXmlNode *node);
};

HDR_DEFINE_LIST(dmsDVB_SystemSoftwareUpdateInfoItem);


class dmsDVB_SystemSoftwareUpdateInfo : public dmsData
{
public:
    u8                                       OUI_DataLength;
    dmsDVB_SystemSoftwareUpdateInfoItemList* OUI_Loop;
    dmsData*                                 PrivateData;

    dmsDVB_SystemSoftwareUpdateInfo();

};


class dmsDVB_DescDataBroadcastId : public dmsData
{
public:
    u16      DataBroadcastId;
    dmsData* IdSelector;

    dmsDVB_DescDataBroadcastId(dmsMPEG_Descriptor *desc);

    bool Load(wxXmlNode *node);
};


#endif /* _DMS_LIB_DVB_DESCRIPTOR_H_ */
