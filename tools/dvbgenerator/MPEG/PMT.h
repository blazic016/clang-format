/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.00 - 10/2011 - Support 2 PMT and 2 data Carousel
   ************************************************************************ */

#ifndef _DVB_HEADERS_PMT_H_
#define _DVB_HEADERS_PMT_H_

#include <Tools/Header.h>

#include <MPEG/MPEG.h>
#include <MPEG/DESC.h>


class wxXmlNode;
class dmsMPEG_Section;

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */


class dmsMPEG_PMT_Stream : public dmsData
{
public:
    u8       StreamType;
    u3       Reserved1;
    u13      ElementaryPID;
    u4       Reserved2;
    u12      ES_InfoLength;
    dmsData* Descriptors;

    dmsMPEG_PMT_Stream();
};

class dmsMPEG_PMT_StreamList : public dmsDataList
{
public:
    dmsMPEG_PMT_StreamList():dmsDataList(){;}
    dmsData *Create(void *pt, const char *tag);
};

class dmsMPEG_PMT_Data : public dmsData
{
public:
    u3       Reserved1;
    u13      PCR_PID;
    u4       Reserved2;
    u12      ProgramInfoLength;
    dmsMPEG_DescriptorList* Descriptors;
    dmsMPEG_PMT_StreamList* ElementaryStreams;

    /*dmsMPEG_PMT_Data(dmsMPEG_Section* section);*/
    dmsMPEG_PMT_Data(dmsMPEG_Section* section, int numPMT = -1);
};



#endif /* _DVB_HEADERS_PMT_H_ */
