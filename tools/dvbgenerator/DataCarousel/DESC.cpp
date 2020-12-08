/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.10 - 12/2011 - Add DSI Subdescriptor Update ID
                                          and Usage ID
   ************************************************************************ */


#include <Tools/Tools.h>
#include <Tools/Xml.h>
#include <Tools/Conversion.h>

#include "DESC.h"
#include "DataCarousel.h"
#include "Group.h"
#include "Module.h"
#include "ModuleData.h"

/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsNameRank DSMCC_DescriptorNames[] =
{
    {0x01, (char *)"Type"},
    {0x02, (char *)"Name"},
    {0x04, (char *)"ModuleLink"},
    {0x05, (char *)"CRC32"},
    {0x06, (char *)"Location"},
    {0x09, (char *)"CompressedModule"},
    {0,NULL}
};

dmsDSMCC_DescriptorList::dmsDSMCC_DescriptorList(dmsModule *Module) : dmsMPEG_DescriptorList()
{
    m_poNameTab = DSMCC_DescriptorNames;
    m_poModule  = Module;
}

dmsData* dmsDSMCC_DescriptorList::CreateData(dmsMPEG_Descriptor *desc, u8 descTag)
{
    switch (descTag)
    {
    case 0x01: return new dmsDSMCC_DescType(desc);
    case 0x02: return new dmsDSMCC_DescName(desc, m_poModule);
    case 0x04: return new dmsDSMCC_DescModuleLink(desc, m_poModule);
    case 0x05: return new dmsDSMCC_DescCRC32(desc, m_poModule);
    case 0x06: return new dmsDSMCC_DescLocation(desc);
    case 0x09: return new dmsDSMCC_DescCompressedModule(desc, m_poModule);
    default:
        return NULL;
    }
}

/* ########################################################################

   ######################################################################## */




/* ========================================================================
   Initialisations
   ======================================================================== */

/* ------------------------------------------------------------------------
   0x01 : Type
   ------------------------------------------------------------------------ */

dmsDSMCC_DescType::dmsDSMCC_DescType(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(Text, 0);

    Text = new dmsDataAscii();

    SetLoad((void*)&Text, "", false);
}

/* ------------------------------------------------------------------------
   0x02 : Name
   ------------------------------------------------------------------------ */

dmsDSMCC_DescName::dmsDSMCC_DescName(dmsMPEG_Descriptor *desc, dmsModule *Module) : dmsData()
{
    HDR_INIT(Text, 0);

    Text = new dmsDataAscii();

    SetLoad(&Text, "");

    m_poModule = Module;
}

bool dmsDSMCC_DescName::Update()
{
    if (m_poModule==NULL) return false;

    if (Text->m_oBuffer.Len()==0||Text->m_oBuffer.m_poBuffer[0]==0)
        Text->m_oBuffer.Set(wxFileNameFromPath(m_poModule->m_oFirstFileName), true);

    return true;
}


/* ------------------------------------------------------------------------
   0x04 : ModuleLink
   ------------------------------------------------------------------------ */

dmsDSMCC_DescModuleLink::dmsDSMCC_DescModuleLink(dmsMPEG_Descriptor *desc, dmsModule *Module) : dmsData()
{
    HDR_INIT(Position,  8);
    HDR_INIT(ModuleId, 16);

    SetLoad(&Position, true);
    SetLoad(&ModuleId, true);

    m_poModule = Module;
}


bool dmsDSMCC_DescModuleLink::Update()
{
    if (m_poModule==NULL) return false;

    dmsModuleList::Node* node = m_poModule->m_poGroup->m_oModuleList.Find(m_poModule);
    dmsModuleList::Node* next = node->GetNext();

    if (node->GetPrevious() == NULL) Position = 0x00;
    else if (next)                Position = 0x01;
    else                          Position = 0x02;

    ModuleId = next?next->GetData()->DIIM->ModuleId:0;

    return true;
}

/* ------------------------------------------------------------------------
   0x05 : CRC32
   ------------------------------------------------------------------------ */

dmsDSMCC_DescCRC32::dmsDSMCC_DescCRC32(dmsMPEG_Descriptor *desc, dmsModule *Module) : dmsData()
{
    HDR_INIT(CRC32, 32);

    SetLoad(&CRC32, true);

    m_poModule = Module;
}


bool dmsDSMCC_DescCRC32::Update()
{
    if (m_poModule==NULL || m_poModule->m_poModuleData==NULL) return false;

    dmsModuleData *data;

    // Object Carousel
    // Pour les modules contenant les BIOP_DirMessage et les BIOP_ServiceGatewayMessage,
    // on effectue la generation en utilisant les donnees du dernier fils.
    if ((m_poModule->m_poDataCarousel->m_poOC == NULL) ||
        (!(((data = m_poModule->GetLastDataOfType(MODULE_DATA_TYPE_BIOP_DIR_MESSAGE)) != NULL) ||
           ((data = m_poModule->GetLastDataOfType(MODULE_DATA_TYPE_BIOP_SERVICE_GATEWAY_MESSAGE)) != NULL))))
    {
        data = m_poModule->m_poModuleData;
    }

    CRC32 = data->m_oBuffer.CRC32();

    return true;
}

/* ------------------------------------------------------------------------
   0x06 : Location
   ------------------------------------------------------------------------ */

dmsDSMCC_DescLocation::dmsDSMCC_DescLocation(dmsMPEG_Descriptor *desc) : dmsData()
{
    HDR_INIT(LocationTag, 8);

    SetLoad(&LocationTag, "", false);
}

/* ------------------------------------------------------------------------
   0x09 : Compressed Module
   ------------------------------------------------------------------------ */

dmsDSMCC_DescCompressedModule::dmsDSMCC_DescCompressedModule(dmsMPEG_Descriptor *desc, dmsModule *Module) : dmsData()
{
    HDR_INIT(CompressionMethod, 8);
    HDR_INIT(OriginalSize, 32);

    SetLoad(&CompressionMethod, false);
    SetLoad(&OriginalSize, true);

    m_poModule = Module;
}


bool dmsDSMCC_DescCompressedModule::Load(wxXmlNode *node)
{
    dmsData::Load(node);

    m_poModule->m_iCompressionMethod = CompressionMethod;

    return true;
}


bool dmsDSMCC_DescCompressedModule::Update()
{
    if (m_poModule==NULL || m_poModule->m_poModuleData==NULL) return false;

    OriginalSize = m_poModule->m_poModuleData->m_iOriginalSize;

    return true;
}


/* ########################################################################

   ######################################################################## */



dmsDSM_CompatibilityDescriptor::dmsDSM_CompatibilityDescriptor() : dmsData()
{
    HDR_INIT(CompatibilityDescriptorLength, 16);
    HDR_INIT(DescriptorCount,               16);
    HDR_INIT(DescriptorList,                0);

    DescriptorList = new dmsDSM_CompatibilityInfoList();

    SetLenLimit(&CompatibilityDescriptorLength, &DescriptorList);
    SetListCount(&DescriptorCount, DescriptorList);

    SetLoad(&DescriptorList, "");
}



dmsNameRank CompatibilityDescriptorType[] =
{
    {0, (char *)"Pad"},
    {1, (char *)"HW"},
    {2, (char *)"SW"},
    {0, NULL}
};


dmsNameRank CompatibilitySpecifierType[] =
{
    {1, (char *)"OUI"},
    {0, NULL}
};

dmsDSM_CompatibilityInfo::dmsDSM_CompatibilityInfo() : dmsData()
{
    HDR_INIT(DescriptorType,     8);
    HDR_INIT(DescriptorLength,   8);
    HDR_INIT(SpecifierType,      8);
    HDR_INIT(SpecifierData,      24);
    HDR_INIT(Model,              16);
    HDR_INIT(Version,            16);
    HDR_INIT(SubDescriptorCount, 8);
    HDR_INIT(SubDescriptorList,  0);

   SubDescriptorList = new dmsDSM_SubDescriptorList();
   SetLenLimit(&DescriptorLength, &SubDescriptorList);

    SetValues(&DescriptorType, "XmlTagName", CompatibilityDescriptorType);
    SetValues(&SpecifierType, CompatibilitySpecifierType);
    SetLoad(&SpecifierData, "OUI", false);
    SetLoad(&Model, false);
    SetLoad(&Version, false);

    SetLoad(&SpecifierType, true); SpecifierType = 1;
    SetLoad(&SubDescriptorList, true);
    SetListCount(&SubDescriptorCount, SubDescriptorList);
}

dmsData *dmsDSM_SubDescriptorList::Create(void *pt, const char *tag)
{
   if (tag != NULL)
   {
      if (strcmp(tag,"SubDescUpdateID") == 0)
      {
         return new dmsDSM_SubDescUpdateID();
      }
      else if (strcmp(tag,"SubDescUsageID") == 0)
      {
         return new dmsDSM_SubDescUsageID();
      }
   }

   return NULL;
}


dmsDSM_SubDescUpdateID::dmsDSM_SubDescUpdateID() : dmsData()
{
    HDR_INIT(descriptorType,  8);
    HDR_INIT(descriptorLength,8);
    HDR_INIT(update_id,      32);

   descriptorType   = 1;
   descriptorLength = 4;
    SetLoad(&update_id,false);
}

dmsDSM_SubDescUsageID::dmsDSM_SubDescUsageID() : dmsData()
{
    HDR_INIT(descriptorType,  8);
    HDR_INIT(descriptorLength,8);
    HDR_INIT(usage_id,        8);

   descriptorType   = 2;
   descriptorLength = 1;
    SetLoad(&usage_id,false);
}
