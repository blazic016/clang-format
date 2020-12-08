/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.00 - 10/2011 - Support 2 PMT and 2 data Carousel
   ************************************************************************ */

#include <Tools/Tools.h>
#include <Tools/Xml.h>

#include <DVB/DESC.h>

#include "PMT.h"

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsMPEG_PMT_Data::dmsMPEG_PMT_Data(dmsMPEG_Section* section, int numPMT) : dmsData()
{
   char name[50];

    HDR_INIT(Reserved1,         3);
    HDR_INIT(PCR_PID,           13);
    HDR_INIT(Reserved2,         4);
    HDR_INIT(ProgramInfoLength, 12);
    HDR_INIT(Descriptors,       0);
    HDR_INIT(ElementaryStreams, 0);

    section->TableId = 0x2;
    section->m_iPID  = 0x0000;
   if (numPMT < 0) strcpy(name,"PMT_Section");
   else sprintf(name,"PMT%d_Section",numPMT);
    section->SetName(name);
    section->SetName(&section->TableIdExtension, "ProgramNumber");

   if (numPMT < 0) strcpy(name,"PMT_Data");
   else sprintf(name,"PMT%d_Data",numPMT);
    SetName(name);

    Reserved1 = 7;
    Reserved2 = 15;

    Descriptors       = new dmsDVB_DescriptorList();
    ElementaryStreams = new dmsMPEG_PMT_StreamList();

    SetLoad(&PCR_PID, true);

    SetLenLimit(&ProgramInfoLength, &Descriptors);

    SetLoad(&Descriptors,       true);
    SetLoad(&ElementaryStreams, true);
}


dmsData *dmsMPEG_PMT_StreamList::Create(void *pt, const char *tag)
{
    return new dmsMPEG_PMT_Stream();
}


dmsMPEG_PMT_Stream::dmsMPEG_PMT_Stream() : dmsData()
{
    HDR_INIT(StreamType,    8);
    HDR_INIT(Reserved1,     3);
    HDR_INIT(ElementaryPID, 13);
    HDR_INIT(Reserved2,     4);
    HDR_INIT(ES_InfoLength, 12);
    HDR_INIT(Descriptors,   0);

    Reserved1 = 7;
    Reserved2 = 15;

    Descriptors = new dmsDVB_DescriptorList();

    SetLoad(&StreamType,    false);
    SetLoad(&ElementaryPID, false);
    SetLoad(&Descriptors,   true);

    SetLenLimit(&ES_InfoLength, &Descriptors);
}



