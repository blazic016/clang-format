/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DVB_HEADERS_SDT_H_
#define _DVB_HEADERS_SDT_H_

#include <Tools/Header.h>

#include <MPEG/MPEG.h>
#include <MPEG/DESC.h>


class wxXmlNode;
class dmsMPEG_Section;




class dmsDVB_SDT_TS_Item : public dmsData
{
public:
    u16                     ServiceId;
    u6                      Reserved;
    u1                      EIT_ScheduleFlag;
    u1                      EIT_PresentFollowingFlag;
    u3                      RunningStatus;
    u1                      FreeCA_Mode;
    u12                     DescriptorLoopLength;
    dmsMPEG_DescriptorList* DescriptorLoop;

    dmsDVB_SDT_TS_Item();
};

HDR_DEFINE_LIST(dmsDVB_SDT_TS_Item);


class dmsDVB_SDT_Data : public dmsData
{
public:
    u16                     OriginalNetworkId;
    u8                      Reserved;
    dmsDVB_SDT_TS_ItemList* Loop;

    int m_iPID;
    int m_iOutputFrequency;

    dmsMPEG_Section *m_poParent;

public:
   dmsDVB_SDT_Data(dmsMPEG_Section* section);
};



#endif /* _DVB_HEADERS_SDT_H_ */
