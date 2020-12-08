/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.11 - 02/2012 - Fix bug Mux sections modules
                                          multiplexed with REPEAT shorty
                                          modules.
   - LIPPA - SmarDTV - v 2.12 - 02/2012 - DSI repetition all seconds.
   - LIPPA - SmarDTV - v 2.13 - 03/2012 - Generate raw sections in a file and
                                          choice generated files extensions.
   - LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
                                          Header in sections file
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/file.h>
#include <wx/ffile.h>
#include <wx/filename.h>

#include <Tools/Tools.h>
#include <Tools/Xml.h>
#include <Tools/Validator.h>
#include <Tools/File.h>


#include <MPEG/MPEG.h>
#include <MPEG/PAT.h>
#include <MPEG/PMT.h>

#include <DVB/NIT.h>
#include <Tools/Header.h>

#include <TS_Writer/TS_WriterVersion.h>

#include "MuxStream.h"
#include "Multiplexer.h"


/* ########################################################################

   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsMuxStreamList);



/* ########################################################################

   dmsMuxStream

   Classe générique, flux de multiplexage

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsMuxStream::dmsMuxStream(dmsMultiplexer *mux)
{
    m_poMux    = mux;

    m_iPID      = -1;
    m_iRate     = 0;

    m_bCarryPCR = false;

    m_bFromTemplate = false;
}


dmsMuxStream::~dmsMuxStream()
{
}

/* ========================================================================

   ======================================================================== */

bool dmsMuxStream::Load(wxXmlNode *node)
{
    dmsXmlContextManager m(node);

    m.Add("Group?", new dmsvString(&m_oGroupName, ""));
    m.Add("Rate?",  new dmsvNumeric(&m_iRate,""));
    m.Add("PID?",   new dmsvNumeric(&m_iPID,":bit:13"));

    if (!m.Validate()) return false;

    m_iRate *= 1000;

    wxString dir, name, ext;
    wxFileName::SplitPath(m_poMux->m_oOutputFile, &dir, &name, &ext);

    return V_Load(node);
}

/* ========================================================================

   ======================================================================== */

bool dmsMuxStream::PlayStart()
{
    m_iPlayLap                       = 0;
    m_iPlayCount                     = 0;
    m_iPlayContinuityCounter         = 0;

    m_iPlayPcr = 0;

    return Open();
}

void dmsMuxStream::PlayNextCount()
{
    m_iPlayCount += 188;

    if (m_iRate)
        m_iPlayPcr = m_iPlayCount*8*1000000*MPEG_PCR_FREQ_MHZ/m_iRate;
}


bool dmsMuxStream::PlayNext(u8* buffer)
{
    if (! ReadTS(buffer)) return false;

    PlayNextCount();

    return true;
}



void dmsMuxStream::PlayStop()
{
    Close();
}

/* ========================================================================

   ======================================================================== */



/* ########################################################################

   Outils pour le multiplexage de sections

   ######################################################################## */

/* ========================================================================

   dmsMuxStreamSectionItem

   Element unitaire d'un flux de sections.

   Contient les paramètres nécessaire au multiplexage de sections.

   ======================================================================== */

/* ------------------------------------------------------------------------
   Declarations
   ------------------------------------------------------------------------ */

class dmsMuxStreamSectionItem;

WX_DECLARE_LIST(dmsMuxStreamSectionItem, dmsMuxStreamSectionItemList);

/* ------------------------------------------------------------------------
   Classe
   ------------------------------------------------------------------------ */


class dmsMuxStreamSectionItem
{
public:
    dmsMuxStream *m_poTS;

public:
    dmsBuffer m_oBuffer;      // Section sous sa forme binaire
    int       m_iFrequency;   // Fréquence de répétition de la section
    wxString  m_oLabel;       // Label (pour les traces)
    int       m_iPID;         // PID de la section (on peut avoir plusieurs PID dans le meme stream

public:
    dmsMuxStreamSectionItem(dmsMuxStream *TS, const wxString &Label, dmsBuffer &Buffer, int Frequency, int pid)
    {
        Buffer.Move(m_oBuffer);

        m_iFrequency = Frequency;
        m_oLabel     = Label;
        m_poTS       = TS;
        m_iPID       = pid;
    }

    ~dmsMuxStreamSectionItem()
    {
    }
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsMuxStreamSectionItemList);

/* ========================================================================

   dmsMuxStreamSectionGroup

   Groupe de sections (ou Flux) à multiplexer.

   Contient les parametres de multiplixage (fréquence, ...)

   ======================================================================== */


/* ------------------------------------------------------------------------
   Declarations
   ------------------------------------------------------------------------ */

class dmsMuxStreamSectionGroup;

WX_DECLARE_LIST(dmsMuxStreamSectionGroup, dmsMuxStreamSectionGroupList);

/* ------------------------------------------------------------------------
   Classe
   ------------------------------------------------------------------------ */

class dmsMuxStreamSectionGroup// : public dmsData
{
public:
    dmsMuxStreamSectionItemList::Node* m_poCurrent;
    dmsMuxStreamSectionItemList        m_oList;
    dmsMuxStreamSection*               m_poMultiplexer;

    int m_iId;
    int m_iStep;
    int m_iNextOffset;
    int m_iMaxFreq;
    int m_iLap;
    int m_iRank;

public:
    dmsMuxStreamSectionGroup(dmsMuxStreamSection* multiplexer, int id)
    {
        m_poMultiplexer = multiplexer;

        m_oList.DeleteContents(true);

        m_iId = id;

        m_poCurrent = NULL;
        m_iMaxFreq = -1;
    }

    bool Init(int MaxGroupSize)
    {
        if (m_oList.GetCount()==0)
        {
            LOGW(L"Empty Mux Group [%d]", m_iId);
            m_iStep = 0;
            return true;
        }

        m_iStep       = MaxGroupSize / m_oList.GetCount();
        m_iNextOffset = 0;
        m_iLap        = 0;
        m_iRank       = 0;

        m_poCurrent = m_oList.GetFirst();

        return true;
    }

    dmsMuxStreamSectionItem *GetGroupItem(int offset)
    {
        dmsMuxStreamSectionItemList::Node* res;

        if (m_iMaxFreq>0) return NULL;

        switch(m_poMultiplexer->m_eMultiplexMode)
        {
        case dmsMuxStreamSection::Stretch:
            break;

        case dmsMuxStreamSection::Repeat:
            m_iStep = 1;
            if (m_poCurrent==NULL)
            {
                m_poCurrent = m_oList.GetFirst();
                m_iLap++;
                m_iRank=0;
            }
            break;

        case dmsMuxStreamSection::Asap:
            m_iStep = 1;
            break;
        }

        if (m_poCurrent && (offset == m_iNextOffset))
        {
            res           = m_poCurrent;
            m_poCurrent   = m_poCurrent->GetNext();
            m_iNextOffset += m_iStep;
            m_iRank++;
        }
        else
        {
            res = NULL;
        }

        return res?res->GetData():NULL;
    }


    dmsMuxStreamSectionItem *GetFreqItem(int offset)
    {
        dmsMuxStreamSectionItemList::Node* res;

        if (m_iMaxFreq==0) return NULL;

        // Dans le cas de la liste à répétition...

        // On passe les fréquences qui n'ont pas expiré

        while (m_poCurrent && (offset%m_poCurrent->GetData()->m_iFrequency>0))
        {
            m_poCurrent=m_poCurrent->GetNext();
        }

        // Retour de l'élément périmé (s'il existe)

        if (m_poCurrent)
        {
            res         = m_poCurrent;
            m_poCurrent = m_poCurrent->GetNext();
        }
        else
        {
            res         = NULL;
            m_poCurrent = m_oList.GetFirst();
        }

        return res?res->GetData():NULL;
    }
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsMuxStreamSectionGroupList);

/* ########################################################################

   ######################################################################## */

WX_DECLARE_LIST(dmsMuxStreamSectionMuxedItem, dmsMuxStreamSectionMuxedItemList);

class dmsMuxStreamSectionMuxedItem
{
public:
    dmsMuxStreamSectionItem*  m_poItem;
    dmsMuxStreamSectionGroup* m_poGroup;
    int                       m_iGroupOffset;
    int                       m_iFrequentOffset;
    int                       m_iLineOffset;
    bool                      m_bLastInGroup;

public:
    dmsMuxStreamSectionMuxedItem(dmsMuxStreamSectionItem* item, dmsMuxStreamSectionGroup* group,
        int groupOffset, int freqOffset, int lineOffset)
    {
        m_poItem          = item;
        m_poGroup         = group;
        m_iGroupOffset    = groupOffset;
        m_iFrequentOffset = freqOffset;
        m_iLineOffset     = lineOffset;

        m_bLastInGroup = (group->m_poCurrent==NULL);
    }

    ~dmsMuxStreamSectionMuxedItem() {;}
};

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsMuxStreamSectionMuxedItemList);

/* ########################################################################

   dmsMuxStreamSection

   Flux de sections (SI, DATA, ...) construites dynamiquement, à partir du
   fichier de configuration.

   Utilise le multiplexage de sections

   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsMuxStreamSectionList);


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsMuxStreamSection::dmsMuxStreamSection(dmsMultiplexer *mux) : dmsMuxStream(mux)
{
    m_eMultiplexMode = Asap;

    m_poGroupList = new dmsMuxStreamSectionGroupList();
    m_poGroupList->DeleteContents(true);

    m_poMuxedList = new dmsMuxStreamSectionMuxedItemList();
    m_poMuxedList->DeleteContents(true);

    GetNextGroup();
}


dmsMuxStreamSection::~dmsMuxStreamSection()
{
    DELNUL(m_poGroupList);
    DELNUL(m_poMuxedList);
}


/* ========================================================================

   ======================================================================== */


void dmsMuxStreamSection::Clear()
{
    m_poGroupList->Clear();
    m_poMuxedList->Clear();
}


void dmsMuxStreamSection::TraceMuxedList(wxFFile &trace)
{
    dmsMuxStreamSectionMuxedItem* item;
    wxString                      frequency;

    trace.Write("+----------------------------------------------------------------------------------+\n");
    trace.Write("|                                                                                  |\n");
    trace.Write("|                                 Multiplex Output                                 |\n");
    trace.Write("|                                                                                  |\n");
    trace.Write("+----------------+------+------+--------+--------------------------------+---------+\n");
    trace.Write("|     Offset     | Mux  | Freq |  PID   | Label                          | Size in |\n");
    trace.Write("| Line.Freq.Group|Group |      |        |                                | bytes   |\n");
    trace.Write("+----------------+------+------+--------+--------------------------------+---------+\n");

    FOREACH(dmsMuxStreamSectionMuxedItemList, *m_poMuxedList, item)
    {
        if (item->m_poItem->m_iFrequency)
            frequency = STR("F%4d", item->m_poItem->m_iFrequency);
        else if (m_eMultiplexMode == Repeat && item->m_poGroup->m_iRank==1)
            frequency = STR("L%4d",item->m_poGroup->m_iLap);
        else if (m_eMultiplexMode == Asap && item->m_bLastInGroup)
            frequency = "    .";
        else if (m_eMultiplexMode == Stretch && item->m_bLastInGroup)
            frequency = "    .";
        else if (item->m_poGroup->m_iStep>1)
            frequency = STR("G%4d",item->m_poGroup->m_iStep);
        else
            frequency = STR("%5s", "");

        trace.Write(STR("| %4d.%4d.%4d | %4d |%s | 0x%04x | %-30s | %7d |\n",
            item->m_iLineOffset,
            item->m_iFrequentOffset,
            item->m_iGroupOffset,
            item->m_poGroup->m_iId,
            frequency,
            item->m_poItem->m_iPID,
            item->m_poItem->m_oLabel,
            item->m_poItem->m_oBuffer.m_iBufferLen));
    }

    trace.Write("+----------------+------+------+--------+--------------------------------+---------+\n");
}

void dmsMuxStreamSection::GenerateFrequent(wxFile &file, bool trace, wxFFile &traceFile, int &frequentOffset, int &groupOffset, int &lineOffset)
{
    dmsMuxStreamSectionGroup* group;
    dmsMuxStreamSectionItem* item;

    FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
    {
        while ((item = group->GetFreqItem(frequentOffset)))
        {
            Generate(file, item->m_oBuffer, item->m_iPID);

            //if (trace) TraceItem(traceFile, groupOffset, frequentOffset, lineOffset, group, item);

            lineOffset++;
        }
    }

    frequentOffset++;
}


int dmsMuxStreamSection::GetSectionCount()
{
    int res = 0;

    dmsMuxStreamSectionGroup* group;

    FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
    {
        res += group->m_oList.GetCount();
    }

    return res;
}

bool dmsMuxStreamSection::Generate(const wxString &Filename, const wxString &traceFileName)
{
    wxFile  file;
    bool    trace = (traceFileName.Len()>0);
    wxFFile traceFile;

    if (GetSectionCount()==0) return true;

    if (! file.Open(Filename, wxFile::write))
    {
        LOGE(L"Error opening file [%s]", Filename);
        return false;
    }

    if (trace)
    {
        if (! traceFile.Open(traceFileName, "w"))
        {
            LOGE(L"Error opening file [%s]", traceFileName);
            return false;
        }
        //TraceHeader(traceFile);
    }

    dmsMuxStreamSectionGroup* group;
    dmsMuxStreamSectionItem* item;

    SetMaxGroupLen();

    m_iWriteCount = 0;

    //while (m_iWriteCount < m_iTsMinimalSize || m_iWriteCount==0)
    {
        int lineOffset = 0;
        int frequentOffset = 0;
        int groupOffset = 0;

        FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
        {
            if (! group->Init(m_iMaxGroupLen)) return false;
        }

        GenerateFrequent(file, trace, traceFile, frequentOffset, groupOffset, lineOffset);

        for (groupOffset=0; groupOffset<m_iMaxGroupLen; groupOffset++)
        {
            FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
            {
                while ((item = group->GetGroupItem(groupOffset)))
                {
                    Generate(file, item->m_oBuffer, item->m_iPID);

                    //if (trace) TraceItem(traceFile, groupOffset, frequentOffset, lineOffset, group, item);

                    lineOffset++;

                    GenerateFrequent(file, trace, traceFile, frequentOffset, groupOffset, lineOffset);
                }
            }
        }

        //if (m_iWriteCount==0) break;

        //if (trace) TraceFooter(traceFile);
    }

    return true;
}



bool dmsMuxStreamSection::V_Load(wxXmlNode *node)
{
    dmsXmlContextManager m(node);

    m.Add("MultiplexMode?", new dmsvEnum(&m_eMultiplexMode, ":Asap:Repeat:Stretch",""));
    m.Add("PID?", new dmsvNumeric(&m_iPID));

    if (!m.Validate()) return false;

    return true;
}



/* ========================================================================

   ======================================================================== */

int dmsMuxStreamSection::GetNextGroup()
{
    int res = m_poGroupList->GetCount();

    m_poGroupList->Append(new dmsMuxStreamSectionGroup(this, res));

    return res;
}



void dmsMuxStreamSection::Generate(wxFile &File, dmsBuffer &Buffer, int pid)
{
    BOOL              LastPacket = false;
    u_8               PacketBuffer[188];
    BCD_mpeg_packet_t Packet;
    int               byteRead=0;
    u8               cc=0;

    TODO();

    while (! LastPacket)
    {
        // read section and build packet
        LastPacket = !BCD_extract_packet_from_buffer(Buffer.Begin(), &Packet, Buffer.Len(), pid, &byteRead, &cc);

        // add packet specific fields
        PacketBuffer[MPEG_SYNC_BYTE_OFFSET]  = SYNC_BYTE;
        PacketBuffer[MPEG_PUSI_PID_OFFSET]   = (unsigned char) (Packet.payload_unit_start_indicator_pid >> 8);
        PacketBuffer[MPEG_PUSI_PID_OFFSET+1] = (unsigned char) (Packet.payload_unit_start_indicator_pid);
        PacketBuffer[MPEG_TSC_AFC_CC_OFFSET] = Packet.tsc_afc_continuity_counter;
        memcpy(&PacketBuffer[MPEG_DATA_BYTE_OFFSET], Packet.packet_data, MPEG_PACKET_DATA_SIZE);

        File.Write(PacketBuffer, 188);

        m_iWriteCount+=188;
    }
}


void dmsMuxStreamSection::Generate(const wxString &Filename, dmsBuffer &Buffer, int pid)
{
    wxFile file(Filename, wxFile::write);

    Generate(file, Buffer, pid);
}

void dmsMuxStreamSection::Generate(const wxString &Filename, dmsMPEG_MuxSection &section)
{
    Generate(Filename, section.m_oBuffer, section.m_iPID);
}



void dmsMuxStreamSection::SetMaxGroupLen()
{
    dmsMuxStreamSectionGroup* group;

    m_iMaxGroupLen = 0;

    FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
    {
        if ((int)group->m_oList.GetCount() > m_iMaxGroupLen)
            m_iMaxGroupLen = group->m_oList.GetCount();
    }
}



void dmsMuxStreamSection::Add(dmsMPEG_MuxSection &section, int position)
{
    if (section.m_iDebugMissing) return;

    dmsMuxStreamSectionItem *item = new dmsMuxStreamSectionItem(this, section.m_oTraceFilename, section.m_oBuffer, section.m_iOutputFrequency, section.m_iPID);

    dmsMuxStreamSectionGroup* group = (*m_poGroupList)[section.m_iGroup];

    if ((group->m_iMaxFreq == 0 && section.m_iOutputFrequency > 0) ||
        (group->m_iMaxFreq > 0  && section.m_iOutputFrequency == 0))
    {
        GetNextGroup();
        group = (*m_poGroupList)[m_poGroupList->GetCount()-1];
    }

    if (section.m_iOutputFrequency > group->m_iMaxFreq)
        group->m_iMaxFreq = section.m_iOutputFrequency;

    if (position==-1)
        group->m_oList.Append(item);
    else
        group->m_oList.Insert((size_t)position, item);
}


/* ========================================================================
   Mux Sections
   ======================================================================== */


void dmsMuxStreamSection::GenerateFrequent(int &frequentOffset, int &groupOffset, int &lineOffset)
{
    dmsMuxStreamSectionGroup* group;
    dmsMuxStreamSectionItem* item;

    FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
    {
        while ((item = group->GetFreqItem(frequentOffset)))
        {
            m_poMuxedList->Append(new dmsMuxStreamSectionMuxedItem(item, group, groupOffset, frequentOffset, lineOffset));

            lineOffset++;
        }
    }

    frequentOffset++;
}


bool dmsMuxStreamSection::MuxSectionsBootLoader()
{
    if (GetSectionCount()==0) return true;

    dmsMuxStreamSectionGroup* group;
    dmsMuxStreamSectionItem* item;

    SetMaxGroupLen();

    m_iWriteCount = 0;

    int lineOffset     = 0;
    int frequentOffset = 0;
    int groupOffset    = 0;

    /*--------------------------------------*/
    /* This code mux like this :            */
    /*  first sections of each group first, */
    /*  then second sections of each group  */
    /*  etc..                               */
    /*--------------------------------------*/
    FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
    {
        if (! group->Init(m_iMaxGroupLen)) return false;
    }

    GenerateFrequent(frequentOffset, groupOffset, lineOffset);

    for (groupOffset=0; groupOffset<m_iMaxGroupLen; groupOffset++)
    {
        FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
        {
            while ((item = group->GetGroupItem(groupOffset)))
            {
                m_poMuxedList->Append(new dmsMuxStreamSectionMuxedItem(item, group, groupOffset, frequentOffset, lineOffset));

                lineOffset++;

                GenerateFrequent(frequentOffset, groupOffset, lineOffset);
            }
        }
    }

    return true;
}

bool dmsMuxStreamSection::MuxSectionsEmbeddedLoader()
{
    if (GetSectionCount()==0) return true;

    dmsMuxStreamSectionGroup* group;
    dmsMuxStreamSectionItem* item;

    SetMaxGroupLen();

    m_iWriteCount = 0;

    int lineOffset     = 0;
    int frequentOffset = 0;
    int groupOffset    = 0;

    /*------------------------------------------*/
    /* This code mux like this :                */
    /*  all sections of the first group first,  */
    /*  then all sections of the second group,  */
    /*  etc..                                   */
    /*------------------------------------------*/

    FOREACH(dmsMuxStreamSectionGroupList, *m_poGroupList, group)
    {
        if (! group->Init(m_iMaxGroupLen)) return false;


    groupOffset    = 0;

            while ((item = group->GetGroupItem(groupOffset)))
            {
                m_poMuxedList->Append(new dmsMuxStreamSectionMuxedItem(item, group, groupOffset, frequentOffset, lineOffset));

                lineOffset++;

                GenerateFrequent(frequentOffset, groupOffset, lineOffset);

                groupOffset++;

            }
    }


    return true;
}



/* ========================================================================
   Lecture de flux TS
   ======================================================================== */
bool dmsMuxStreamSection::Open()
{
   dmsMuxStreamSectionMuxedItem *p_item;

   switch (m_poMux->m_iLoaderType)
   {
   case kTS_WRITER_LOADER_TYPE_EMBEDDED_LOADER:
      /* Mux sections module by module */
      if (! MuxSectionsEmbeddedLoader()) return false;
      break;

   case kTS_WRITER_LOADER_TYPE_DOWNLOADER_NASC:
      /* Mux sections modules multiplexed with REPEAT shorty modules */
      m_eMultiplexMode = Repeat;
      if (! MuxSectionsBootLoader()) return false;
      break;

   case kTS_WRITER_LOADER_TYPE_DOWNLOADER_BASIC:
   default:
      /* Mux sections modules multiplexed */
      if (! MuxSectionsBootLoader()) return false;
   }

   Reset();

   /**
    * LIPPA v 2.21: Generate file sections
    *   Play each sections of list in multiplexer to generate the section files.
    *   In multiplexer not in section file generation mode the method PlaySectionsFile()
    *   is inoperenta eand return OK
   */
   FOREACH(dmsMuxStreamSectionMuxedItemList, *m_poMuxedList, p_item)
   {
      if (!m_poMux->PlaySectionsFile(p_item->m_poItem->m_oBuffer.m_poBuffer,
                                     (size_t)p_item->m_poItem->m_oBuffer.m_iBufferLen)) return false;
   }

   // Trace sections
   if (m_poMux->m_oOutputDir.Len())
   {
      dmsAssumeDir(m_poMux->m_oOutputDir);

      wxFFile f(STR("%s/%s.mux.txt", m_poMux->m_oOutputDir, m_oGroupName.Len()?m_oGroupName:"sections"), "w");

      TraceMuxedList(f);
   }

   return true;
}

void dmsMuxStreamSection::Close()
{
}

void dmsMuxStreamSection::SetLabel()
{
    if (m_poPlayNode==NULL) return;

    dmsMuxStreamSectionMuxedItem* item = ((dmsMuxStreamSectionMuxedItemList::Node*) m_poPlayNode)->GetData();

    m_oLabel.Printf("%s-%d", item->m_poItem->m_oLabel, m_iPlayOffset);
}


bool dmsMuxStreamSection::ReadTS(u8* buffer)
{
    BCD_mpeg_packet_t Packet;

    dmsMuxStreamSectionMuxedItem* item = ((dmsMuxStreamSectionMuxedItemList::Node*) m_poPlayNode)->GetData();

    bool last = !BCD_extract_packet_from_buffer(
        item->m_poItem->m_oBuffer.Begin(),
        &Packet,
        item->m_poItem->m_oBuffer.Len(),
        item->m_poItem->m_iPID,
        &m_iPlayOffset,
        &m_iPlayContinuityCounter);

    // Ajout de champs specifiques

    buffer[MPEG_SYNC_BYTE_OFFSET]  = SYNC_BYTE;
    buffer[MPEG_PUSI_PID_OFFSET]   = (unsigned char) (Packet.payload_unit_start_indicator_pid >> 8);
    buffer[MPEG_PUSI_PID_OFFSET+1] = (unsigned char) (Packet.payload_unit_start_indicator_pid);
    buffer[MPEG_TSC_AFC_CC_OFFSET] = Packet.tsc_afc_continuity_counter;
    memcpy(&buffer[MPEG_DATA_BYTE_OFFSET], Packet.packet_data, MPEG_PACKET_DATA_SIZE);

    //

    if (last) m_poPlayNode = ((dmsMuxStreamSectionMuxedItemList::Node*) m_poPlayNode)->GetNext();

#ifdef _DEBUG
    SetLabel();
#endif

    return true;
}

bool dmsMuxStreamSection::EofTS()
{
    return (m_poPlayNode==NULL);
}

void dmsMuxStreamSection::Reset()
{
    m_poPlayNode  = m_poMuxedList->GetFirst();
    m_iPlayOffset = 0;

#ifdef _DEBUG
    SetLabel();
#endif
}




/* ########################################################################

   Flux élémentaire présent sur disque (généré par un outils externe)

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsMuxStreamFile::dmsMuxStreamFile(dmsMultiplexer *mux) : dmsMuxStream(mux)
{
    m_poFile = new wxFile;
}


dmsMuxStreamFile::~dmsMuxStreamFile()
{
    DELNUL(m_poFile);
}


/* ========================================================================

   ======================================================================== */

bool dmsMuxStreamFile::V_Load(wxXmlNode *node)
{
    dmsXmlContextManager m(node);

    m.Add("Name", new dmsvFilename(&m_oName));

    if (!m.Validate()) return false;

    m_oLabel = wxFileNameFromPath(m_oName);

    return true;
}

/* ========================================================================
   Lecture de flux TS
   ======================================================================== */

bool dmsMuxStreamFile::Open()
{
    if (! m_poFile->Open(m_oName))
    {
        LOGE(L"No file [%s]", m_oName);
        return false;
    }
    return true;
}

void dmsMuxStreamFile::Close()
{
    m_poFile->Close();
}

bool dmsMuxStreamFile::ReadTS(u8* buffer)
{
    if (m_poFile->Read(buffer, 188)!=188)
    {
        LOGE(L"[%s] Not multiple of 188 Bytes", m_oLabel);
        return true;
    }

    return true;
}

bool dmsMuxStreamFile::EofTS()
{
    return m_poFile->Eof();
}

void dmsMuxStreamFile::Reset()
{
    m_poFile->Seek(0);
}

/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsMuxStreamStuffing::dmsMuxStreamStuffing(dmsMultiplexer *mux) : dmsMuxStream(mux)
{
    m_iPID = 0x1FFF;
}


dmsMuxStreamStuffing::~dmsMuxStreamStuffing()
{

}


/* ========================================================================

   ======================================================================== */

bool dmsMuxStreamStuffing::V_Load(wxXmlNode *node)
{
    dmsXmlContextManager m(node);

    m.Add("CarryPcr?", new dmsvBool(&m_bCarryPCR));

    if (!m.Validate()) return false;

    if (m_bCarryPCR)
    {
        m_iPlayPcrStep = (u64)(MPEG_PACKET_SIZE)*8*1000000*MPEG_PCR_FREQ_MHZ/m_iRate;

        if (m_iPlayPcrStep > m_poMux->m_iPcrInterval*MPEG_PCR_FREQ_MHZ)
        {
            LOGE(L"Rate of stuffing stream must be greater than [%d Kb/s] to carry PCR",
                MPEG_PACKET_SIZE*8*1000/m_poMux->m_iPcrInterval);
            return false;
        }
    }

    return true;
}

bool dmsMuxStreamStuffing::Open()
{
    m_iPlayNextCarriedPcr = 0;
    m_iPlayLap            = 1;

    return true;
}



bool dmsMuxStreamStuffing::ReadTS(u8* buffer)
{
    if (m_bCarryPCR && m_iPlayPcr>=m_iPlayNextCarriedPcr)
    {
        dmsMpegGetStuffingPcrPacket(buffer, m_iPID, m_iPlayPcr, &m_iPlayContinuityCounter);

        m_iPlayNextCarriedPcr = m_iPlayPcr + m_poMux->m_iPcrInterval*MPEG_PCR_FREQ_MHZ - m_iPlayPcrStep;
    }
    else
    {
        dmsMpegGetStuffingPacket(buffer, &m_iPlayContinuityCounter);
    }

    return true;
}

bool dmsMuxStreamStuffing::EofTS()
{
    return false;
}
