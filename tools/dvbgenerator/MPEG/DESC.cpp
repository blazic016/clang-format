/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#include <Tools/Tools.h>
#include <Tools/Xml.h>
#include <Tools/Conversion.h>

#include "DESC.h"


/* ########################################################################

   ######################################################################## */


dmsMPEG_Descriptor::dmsMPEG_Descriptor() : dmsData()
{
    HDR_INIT(DescriptorTag,    8);
    HDR_INIT(DescriptorLength, 8);
    HDR_INIT(Data,             0);

    SetLoad(&Data, "");

    SetLenLimit(&DescriptorLength, &Data);
}

/* ########################################################################

   ######################################################################## */


/* ========================================================================

   ======================================================================== */

static dmsNameRank MPEG_DescriptorNames[] =
{
    {0, NULL}
};


dmsMPEG_DescriptorList::dmsMPEG_DescriptorList() : dmsDataList()
{
    m_poNameTab = MPEG_DescriptorNames;
}

dmsData* dmsMPEG_DescriptorList::Create(void *pt, const char *tag)
{
    return Create(tag);
}

/* ========================================================================

   ======================================================================== */

dmsData* dmsMPEG_DescriptorList::CreateData(dmsMPEG_Descriptor *desc, u8 descTag)
{
    return NULL;
}

/* ========================================================================

   ======================================================================== */

dmsMPEG_Descriptor* dmsMPEG_DescriptorList::Create(u8 descTag)
{
    dmsMPEG_Descriptor *res = new dmsMPEG_Descriptor();

    if (descTag==255)
    {
        TODO();
    }

    dmsData *data = CreateData(res, descTag);

    data->m_poParent = res;

    if (data==NULL)
    {
        delete res;
        LOGE(L"Descriptor [0x%02x] not implemented", descTag);
        return NULL;
    }

    res->DescriptorTag = descTag;

    res->SetLoad(&res->Data, "", data);

    return res;
}

dmsMPEG_Descriptor* dmsMPEG_DescriptorList::Create(const char* descName)
{
    int rank = dmsGetStrRank(descName, m_poNameTab);

    if (rank<0)
    {
        LOGE(L"Descriptor [%s] not implemented", descName);
        LOGE(L"Implemented Descriptors:");
        for (int i=0; m_poNameTab[i].name; i++)
        {
            LOGE(L"- 0x%02X : %s", m_poNameTab[i].rank, m_poNameTab[i].name);
        }
        return NULL;
    }

    return Create(rank);
}

dmsMPEG_Descriptor* dmsMPEG_DescriptorList::FindDesc(u8 tag)
{
    dmsData *res;

    FOREACH(dmsDataListInternal, m_oList, res)
    {
        if (((dmsMPEG_Descriptor*)res)->DescriptorTag == tag)
        {
            return ((dmsMPEG_Descriptor*)res);
        }
    }
    return NULL;
}

void dmsMPEG_DescriptorList::AddData(dmsData *data)
{
    dmsMPEG_Descriptor *res = new dmsMPEG_Descriptor;

    res->SetLoad(&res->Data, "", data);
}
