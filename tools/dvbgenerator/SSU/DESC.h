/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DMS_LIB_DVB_SSU_UNT_DESCRIPTOR_H_
#define _DMS_LIB_DVB_SSU_UNT_DESCRIPTOR_H_


#include <MPEG/DESC.h>
#include <DVB/Date.h>
#include <Tools/Header.h>

/* ========================================================================
   Creation de descripteurs
   ======================================================================== */

class dmsSSU_UNT_DescriptorList : public dmsMPEG_DescriptorList
{
public:
    dmsSSU_UNT_DescriptorList();

    dmsData* CreateData(dmsMPEG_Descriptor *desc, u8 descTag);
};

/* ========================================================================
   Descripteurs
   ======================================================================== */

/* ------------------------------------------------------------------------
   0x01 : Scheduling
   ------------------------------------------------------------------------ */


class dmsSSU_UNT_DescScheduling : public dmsData
{
public:
    dmsDVB_UTC_Time* StartDateTime;
    dmsDVB_UTC_Time* EndDateTime;
    u1               FinalAvailability;
    u1               PeriodicityFlag;
    u2               PeriodUnit;
    u2               DurationUnit;
    u2               EstimatedCycleTimeUnit;
    u8               Period;
    u8               Duration;
    u8               EstimatedCycleTime;
    dmsData*         PrivateData;

public:
    dmsSSU_UNT_DescScheduling(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x02 : Update
   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescUpdate : public dmsData
{
public:
    u2       UpdateFlag;
    u4       UpdateMethod;
    u2       UpdatePriority;
    dmsData* PrivateData;

public:
    dmsSSU_UNT_DescUpdate(dmsMPEG_Descriptor *desc);
};
/* ------------------------------------------------------------------------
   0x03 : SSU_Location
   ------------------------------------------------------------------------ */


class dmsSSU_UNT_DescSSU_Location0x000A : public dmsData
{
public:
    u16 AssociationTag;

    dmsSSU_UNT_DescSSU_Location0x000A();
};


class dmsSSU_UNT_DescSSU_Location : public dmsData
{
public:
    u16      DataBroadcastId;
    dmsData* DataBroadcastIdSubData;
    dmsData* PrivateData;

public:
    dmsSSU_UNT_DescSSU_Location(dmsMPEG_Descriptor *desc);

    bool Load(wxXmlNode *node);
};

/* ------------------------------------------------------------------------
   0x04 : Message
   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescMessage : public dmsData
{
public:
    u4       DescriptorNumber;
    u4       LastDescriptorNumber;
    dmsData* ISO_639_LanguageCode;
    dmsData* Text;

public:
    dmsSSU_UNT_DescMessage(dmsMPEG_Descriptor *desc);
};
/* ------------------------------------------------------------------------
   0x05 : SSU_EventName
   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescSSU_EventName : public dmsData
{
public:
    dmsData* ISO_639_LanguageCode;
    u8       NameLength;
    dmsData* Name;
    u8       TextLength;
    dmsData* Text;

public:
    dmsSSU_UNT_DescSSU_EventName(dmsMPEG_Descriptor *desc);
};
/* ------------------------------------------------------------------------
   0x06 : TargetSmartCard
   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescTargetSmartCard : public dmsData
{
public:
    u32 SuperCA_SystemId;
    dmsData*    PrivateData;

public:
    dmsSSU_UNT_DescTargetSmartCard(dmsMPEG_Descriptor *desc);
};
/* ------------------------------------------------------------------------
   0x07 : TargetMAC_Address
   ------------------------------------------------------------------------ */


class dmsSSU_UNT_DescTargetMAC_AddressItem : public dmsData
{
public:
    dmsData *MAC_AddrMatch; // 48 bits

    dmsSSU_UNT_DescTargetMAC_AddressItem();
};


HDR_DEFINE_LIST(dmsSSU_UNT_DescTargetMAC_AddressItem);


class dmsSSU_UNT_DescTargetMAC_Address : public dmsData
{
public:
    dmsData*                                  MAC_AddrMask; // 48 bits
    dmsSSU_UNT_DescTargetMAC_AddressItemList* MAC_AddrMatchList;

public:
    dmsSSU_UNT_DescTargetMAC_Address(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x08 : TargetSerialNumber
   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescTargetSerialNumber : public dmsData
{
public:
    dmsData *SerialData;

public:
    dmsSSU_UNT_DescTargetSerialNumber(dmsMPEG_Descriptor *desc);
};
/* ------------------------------------------------------------------------
   0x09 : TargetIP_Address
   ------------------------------------------------------------------------ */


class dmsSSU_UNT_DescTargetIP_AddressItem : public dmsData
{
public:
    dmsData *IP_AddrMatch;

    dmsSSU_UNT_DescTargetIP_AddressItem();
};


HDR_DEFINE_LIST(dmsSSU_UNT_DescTargetIP_AddressItem);

class dmsSSU_UNT_DescTargetIP_Address : public dmsData
{
public:
    dmsData*                                 IP_AddrMask;
    dmsSSU_UNT_DescTargetIP_AddressItemList* IP_AddrMatchList;

public:
    dmsSSU_UNT_DescTargetIP_Address(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x0A : Target_IPv6_Address
   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescTargetIPv6_AddressItem : public dmsData
{
public:
    dmsData *IPv6_AddrMatch;

    dmsSSU_UNT_DescTargetIPv6_AddressItem();
};


HDR_DEFINE_LIST(dmsSSU_UNT_DescTargetIPv6_AddressItem);

class dmsSSU_UNT_DescTarget_IPv6_Address : public dmsData
{
public:
    dmsData*                                   IPv6_AddrMask;
    dmsSSU_UNT_DescTargetIPv6_AddressItemList* IPv6_AddrMatchList;

public:
    dmsSSU_UNT_DescTarget_IPv6_Address(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x0B : SSU_SubGroupAssociation
   ------------------------------------------------------------------------ */


class dmsSSU_UNT_DescSSU_SubGroupTag : public dmsData
{
public:
    u24 OUI;
    u16 SubGroupAssociationDescriptor;

    dmsSSU_UNT_DescSSU_SubGroupTag();
};


class dmsSSU_UNT_DescSSU_SubGroupAssociation : public dmsData
{
public:
    dmsSSU_UNT_DescSSU_SubGroupTag *SubGroupTag;

public:
    dmsSSU_UNT_DescSSU_SubGroupAssociation(dmsMPEG_Descriptor *desc);
};
/* ------------------------------------------------------------------------
   0x57 : TelephoneDescriptor
   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescTelephoneDescriptor : public dmsData
{
public:
    u2       Reserved1;
    u1       ForeignAvailability;
    u5       ConnectionType;
    u1       Reserved2;
    u2       CountryPrefixLength;
    u3       InternationalAreaCodeLength;
    u2       OperatorCodeLength;
    u1       Reserved3;
    u3       NationalAreaCodeLength;
    u4       CoreNumberLength;
    dmsData* CountryPrefix;
    dmsData* InternationalAreaCode;
    dmsData* OperatorCode;
    dmsData* NationalAreaCode;
    dmsData* CoreNumber;

public:
    dmsSSU_UNT_DescTelephoneDescriptor(dmsMPEG_Descriptor *desc);
};
/* ------------------------------------------------------------------------
   0x5F : PrivateDataSpecifier
   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescPrivateDataSpecifier : public dmsData
{
public:
    u32 PrivateDataSpecifier;

public:
    dmsSSU_UNT_DescPrivateDataSpecifier(dmsMPEG_Descriptor *desc);
};

#endif /* _DMS_LIB_DVB_SSU_UNT_DESCRIPTOR_H_ */
