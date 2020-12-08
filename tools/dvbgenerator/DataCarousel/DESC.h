/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.10 - 12/2011 - Add DSI Subdescriptor Update ID
                                          and Usage ID
   ************************************************************************ */


#ifndef _LIB_DVB_DATA_CAROUSEL_DESC_H_
#define _LIB_DVB_DATA_CAROUSEL_DESC_H_

#include <MPEG/DESC.h>
#include <Tools/Header.h>

class dmsModule;


/* ========================================================================

   ======================================================================== */

class dmsDSMCC_DescriptorList : public dmsMPEG_DescriptorList
{
public:
    dmsModule *m_poModule;

    dmsDSMCC_DescriptorList(dmsModule *Module);

    dmsData* CreateData(dmsMPEG_Descriptor *desc, u8 descTag);
};


/* ========================================================================

   ======================================================================== */

/* ------------------------------------------------------------------------
   0x01 : Type
   ------------------------------------------------------------------------ */

class dmsDSMCC_DescType : public dmsData
{
public:
    dmsData *Text;

public:
    dmsDSMCC_DescType(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x02 : Name
   ------------------------------------------------------------------------ */

class dmsDSMCC_DescName : public dmsData
{
public:
    dmsData *Text;

public:
    dmsModule *m_poModule;
    dmsDSMCC_DescName(dmsMPEG_Descriptor *desc, dmsModule *Module);

    bool Update();
};


/* ------------------------------------------------------------------------
   0x04 : Module Link
   ------------------------------------------------------------------------ */

class dmsDSMCC_DescModuleLink : public dmsData
{
public:
    u_8 Position;
    u16 ModuleId;

public:
    dmsModule *m_poModule;
    dmsDSMCC_DescModuleLink(dmsMPEG_Descriptor *desc, dmsModule *Module);

    bool Update();
};

/* ------------------------------------------------------------------------
   0x05 : CRC32
   ------------------------------------------------------------------------ */

class dmsDSMCC_DescCRC32 : public dmsData
{
public:
    u32 CRC32;

public:
    dmsModule *m_poModule;
    dmsDSMCC_DescCRC32(dmsMPEG_Descriptor *desc, dmsModule *Module);

    bool Update();
};

/* ------------------------------------------------------------------------
   0x06 : Location
   ------------------------------------------------------------------------ */

class dmsDSMCC_DescLocation : public dmsData
{
public:
    u_8 LocationTag;

public:
    dmsDSMCC_DescLocation(dmsMPEG_Descriptor *desc);
};

/* ------------------------------------------------------------------------
   0x09 : Compressed
   ------------------------------------------------------------------------ */

class dmsDSMCC_DescCompressedModule : public dmsData
{
public:
    u_8 CompressionMethod;
    u32 OriginalSize;

public:
    dmsModule *m_poModule;
    dmsDSMCC_DescCompressedModule(dmsMPEG_Descriptor *desc, dmsModule *Module);

    bool Load(wxXmlNode *node);
    bool Update();
};

/* ========================================================================

   ======================================================================== */
class dmsDSM_SubDescUpdateID : public dmsData
{
public:
    u8  descriptorType;
    u8  descriptorLength;
    u32 update_id;

   dmsDSM_SubDescUpdateID();
};

class dmsDSM_SubDescUsageID : public dmsData
{
public:
    u8  descriptorType;
    u8  descriptorLength;
    u8  usage_id;

   dmsDSM_SubDescUsageID();
};

class dmsDSM_SubDescriptorList : public dmsDataList
{
public:
    dmsDSM_SubDescriptorList():dmsDataList(){;}
    dmsData *Create(void *pt, const char *tag);
};

class dmsDSM_CompatibilityInfo : public dmsData
{
public:
    u8           DescriptorType;
    u8           DescriptorLength;
    u8           SpecifierType;
    u24          SpecifierData;
    u16          Model;
    u16          Version;
    u8           SubDescriptorCount;
    dmsDSM_SubDescriptorList* SubDescriptorList;

public:
    dmsDSM_CompatibilityInfo();
};



HDR_DEFINE_LIST(dmsDSM_CompatibilityInfo);


class dmsDSM_CompatibilityDescriptor : public dmsData
{
public:
    u16                           CompatibilityDescriptorLength;
    u16                           DescriptorCount;
    dmsDSM_CompatibilityInfoList* DescriptorList;

    dmsDSM_CompatibilityDescriptor();
};





#endif /* _LIB_DVB_DATA_CAROUSEL_DESC_H_ */

