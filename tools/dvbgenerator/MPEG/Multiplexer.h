/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.12 - 02/2012 - DSI repetition all seconds.
   - LIPPA - SmarDTV - v 2.13 - 03/2012 - Generate raw sections in a file and
                                          choice generated files extensions.
   - LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
                                          Header in sections file
   ************************************************************************ */

#ifndef _LIB_DATA_CAROUSEL_TRANSPORT_STREAM_H_
#define _LIB_DATA_CAROUSEL_TRANSPORT_STREAM_H_

#include <wx/file.h>
#include "MuxStream.h"


#define MPEG_PCR_FREQ_MHZ 27
#define MPEG_PACKET_SIZE  188


class dmsMultiplexer
{
public:
   wxString m_oOutputFile;
   wxString m_oOutputSectionsFile;
   int      m_iModeSectionsFile;
   u32      m_u32UpdateId;
   u32      m_u32OUI;
   u16      m_u16PlateformModel;
   u16      m_u16PlateformVersion;
   u16      m_u16ProductModel;
   u16      m_u16ProductVersion;
   int      m_iTsMinimalSize;
   int      m_iRate;
   int      m_iPcrInterval;
   int      m_iLoaderType;

   wxString m_oOutputDir;

   dmsMuxStreamList        m_oStreamList;        // Tous les flux
   dmsMuxStreamSectionList m_oStreamSectionList; // Tous les flux de type "section"

public:
   dmsMultiplexer();
   virtual ~dmsMultiplexer();

    bool Load(wxXmlNode *node);

   dmsMuxStreamSection*  NewSectionMux();
   dmsMuxStreamSection*  NewSectionMuxTemplateIntance();
   dmsMuxStreamFile*     NewFileMux();
   dmsMuxStreamStuffing* NewStuffingMux();

   dmsMuxStreamSection* FindSectionMux(const wxString &name);
   dmsMuxStreamSection* FindSectionMux(int pid, bool force=false);

   dmsMuxStream*        GetMaxRateStream();
   int GetMaxRate();

   bool Play();

   bool AddSection(dmsMPEG_MuxSection &section, int position=-1);

private:
   bool Play(const dmsMuxStreamList& list, const wxString &filename);

   // LIPPA v 2.21: Sections Files Management
   bool    m_bIsFileSectionsOpen;
   wxFile  m_oFileSections;
   u32     m_u32NumSections;
   u32     m_u32SizeSections;

   bool PlaySectionsFileStart ();
   bool PlaySectionsFileStop  ();

public:
   bool PlaySectionsFile      (uchar *pucSectionBuffer,
                               size_t iSectionSize);

};


#endif /* _LIB_DATA_CAROUSEL_TRANSPORT_STREAM_H_ */
