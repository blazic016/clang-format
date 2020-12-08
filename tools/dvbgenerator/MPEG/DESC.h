/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _DVB_MPEG_DESCRIPTOR_H_
#define _DVB_MPEG_DESCRIPTOR_H_

#include <wx/wx.h>
#include <Tools/Header.h>


class dmsMPEG_Descriptor;

dmsData* dmsMPEG_CreateDescriptorData(dmsMPEG_Descriptor *desc, const wxString &name);

/* ========================================================================
   Declaration
   ======================================================================== */

class dmsMPEG_Descriptor : public dmsData
{
public:
    u8 DescriptorTag;
    u8 DescriptorLength;
    dmsData* Data;

    dmsMPEG_Descriptor();
};


class dmsMPEG_DescriptorList : public dmsDataList
{
public:
    dmsNameRank* m_poNameTab;

    dmsMPEG_DescriptorList();

    dmsData* Create(void *pt, const char *tag);

    dmsMPEG_Descriptor* Create(const char* descName);
    dmsMPEG_Descriptor* Create(u8 descTag);

    dmsMPEG_Descriptor *FindDesc(u8 tag);

    void AddData(dmsData *data);

    virtual dmsData* CreateData(dmsMPEG_Descriptor *desc, u8 descTag);
};

/* ========================================================================
   Descripteurs MPEG
   ======================================================================== */


#endif /* _DVB_MPEG_DESCRIPTOR_H_ */
