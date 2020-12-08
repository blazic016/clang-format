/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DVB_HEADERS_BAT_H_
#define _DVB_HEADERS_BAT_H_

#include <Tools/Header.h>

#include <MPEG/MPEG.h>
#include <MPEG/DESC.h>

class wxXmlNode;
class dmsMPEG_Section;

class dmsDVB_BAT_TS_Item : public dmsData
{
public:
    u16 TransportStreamId;
    u16 OriginalNetworkId;
    u4 Reserved;
    u12 TransportDescriptorsLength;
    dmsData* TransportDescriptors;

    dmsDVB_BAT_TS_Item();
};

class dmsDVB_BAT_TS_List : public dmsDataList
{
public:
    dmsDVB_BAT_TS_List():dmsDataList(){;}
    dmsData *Create(void *pt, const char *tag);
};

class dmsDVB_BAT_Data : public dmsData
{
public:
    u4                      Reserved1;
    u12                     NetworkDescriptorsLength;
    dmsMPEG_DescriptorList* NetworkDescriptors;
    u4                      Reserved2;
    u12                     TransportStreamLoopLength;
    dmsDVB_BAT_TS_List*     TransportStreamLoop;

    int m_iPID;
    int m_iOutputFrequency;

    dmsMPEG_Section *m_poParent;

public:
   dmsDVB_BAT_Data(dmsMPEG_Section* section);
};



#endif /* _DVB_HEADERS_BAT_H_ */
