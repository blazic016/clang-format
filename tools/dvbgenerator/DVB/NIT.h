/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.00 - 10/2011 - Add Service List Descriptor (tag
                                          0x41) and the specific Nagra
                                          Channel Descriptor (tag 0x82)
   ************************************************************************ */

#ifndef _DVB_HEADERS_NIT_H_
#define _DVB_HEADERS_NIT_H_

#include <Tools/Header.h>

#include <MPEG/MPEG.h>
#include <MPEG/DESC.h>
#include <DVB/DESC.h>


class wxXmlNode;
class dmsMPEG_Section;




class dmsDVB_NIT_TS_Item : public dmsData
{
public:
    u16 TransportStreamId;
    u16 OriginalNetworkId;
    u4 Reserved;
    u12 TransportDescriptorsLength;
    dmsData* TransportDescriptors;
    dmsDVB_ServiceListDescriptor* ServiceListDescriptor;
    dmsDVB_NagraChannelDescriptor* NagraChannelDescriptor;

   dmsDVB_NIT_TS_Item();
};

class dmsDVB_NIT_TS_List : public dmsDataList
{
public:
    dmsDVB_NIT_TS_List():dmsDataList(){;}
    dmsData *Create(void *pt, const char *tag);
};

class dmsDVB_NIT_Data : public dmsData
{
public:
    u4                      Reserved1;
    u12                     NetworkDescriptorsLength;
    dmsMPEG_DescriptorList* NetworkDescriptors;
    u4                      Reserved2;
    u12                     TransportStreamLoopLength;
    dmsDVB_NIT_TS_List*     TransportStreamLoop;

    int m_iPID;
    int m_iOutputFrequency;

    dmsMPEG_Section *m_poParent;

public:
   dmsDVB_NIT_Data(dmsMPEG_Section* section);
};



#endif /* _DVB_HEADERS_NIT_H_ */
