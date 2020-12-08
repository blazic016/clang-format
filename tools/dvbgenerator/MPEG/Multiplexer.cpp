/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
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
#include <wx/filename.h>

#include <Tools/Log.h>
#include <Tools/Xml.h>
#include <Tools/Validator.h>

#include <TS_Writer/TS_WriterVersion.h>

#include "Multiplexer.h"

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsMultiplexer::dmsMultiplexer()
{
   m_iTsMinimalSize      = 0;
   m_iRate               = 0;
   m_iPcrInterval        = 36000;
   m_iLoaderType         = kTS_WRITER_LOADER_TYPE_DOWNLOADER_BASIC;
   m_iModeSectionsFile   = kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED;
   m_bIsFileSectionsOpen = false;
   m_u32NumSections      = 0;
   m_u32SizeSections     = 0;

   m_oStreamList.DeleteContents(true);
}


dmsMultiplexer::~dmsMultiplexer()
{
   if (m_bIsFileSectionsOpen)
   {
      /**
       *  Section always open.
       *  Abnormal, error detected during generation
       *  => Close the file and delete it
      */
      m_oFileSections.Close();
      wxRemoveFile(m_oOutputSectionsFile);
#ifdef _DEBUG
      LOG0(L"[LIPPA-DBG] dmsMultiplexer: destructor: Error generation => Delete section file '%s'", (const char *)m_oOutputSectionsFile);
#endif
   }
}


/* ========================================================================

   ======================================================================== */

bool dmsMultiplexer::Load(wxXmlNode *node)
{
    wxXmlNode *child;

    dmsXmlContextManager m(node);

    m.Add("MinimalOutputFileSize?", new dmsvNumeric(&m_iTsMinimalSize));

    if ((child = node->Find("Mux")))
    {
        wxXmlNode *child2;
        dmsMuxStream *stream;

        for (child2 = child->GetChildren(); child2; child2=child2->GetNext())
        {
            child2->Used();

            if (child2->GetName()=="Section")
                stream = NewSectionMux();
            else if (child2->GetName()=="File")
                stream = NewFileMux();
            else if (child2->GetName()=="Stuffing")
                stream = NewStuffingMux();
            else
            {
                LOGE(L"Unknown tag [%s]", child2->GetLongName());
                return false;
            }

            if (! stream->Load(child2)) return false;
        }
    }

    if (!m.Validate()) return false;

    m_iTsMinimalSize *= 1024;

    return true;
}

/* ------------------------------------------------------------------------
   Crée un nouveau multiplex de section et l'ajoute dans les bonnes listes
   ------------------------------------------------------------------------ */

dmsMuxStreamSection* dmsMultiplexer::NewSectionMux()
{
    dmsMuxStreamSection* res = new dmsMuxStreamSection(this);

    m_oStreamList.Append(res);
    m_oStreamSectionList.Append(res);

    return res;
}

/* ------------------------------------------------------------------------
   Cree un nouveau noeud et l'ajoute en queue du 'groupe'
   ------------------------------------------------------------------------ */

dmsMuxStreamSection* dmsMultiplexer::NewSectionMuxTemplateIntance()
{
    dmsMuxStreamSection* res = new dmsMuxStreamSection(this);
    dmsMuxStream*        templat = NULL;;

    m_oStreamSectionList.Append(res);

    dmsMuxStreamList::Node* node;

    for (node=m_oStreamList.GetFirst(); node; node=node->GetNext())
    {
        if (templat && !node->GetData()->m_bFromTemplate) break;
        if (node->GetData()->m_iPID == -1) templat = node->GetData();
    }

    if (node)
        m_oStreamList.Insert(node, res);
    else
        m_oStreamList.Append(res);

    res->m_bFromTemplate = true;

    return res;
}

/* ------------------------------------------------------------------------
   Crée un mux de type "fichier"
   ------------------------------------------------------------------------ */

dmsMuxStreamFile* dmsMultiplexer::NewFileMux()
{
    dmsMuxStreamFile* res = new dmsMuxStreamFile(this);

    m_oStreamList.Append(res);

    return res;
}

/* ------------------------------------------------------------------------
   Crée un mux de type "stuffing"
   ------------------------------------------------------------------------ */

dmsMuxStreamStuffing* dmsMultiplexer::NewStuffingMux()
{
    dmsMuxStreamStuffing* res = new dmsMuxStreamStuffing(this);

    m_oStreamList.Append(res);

    return res;
}

/* ------------------------------------------------------------------------
   Retourne le multiplexe de section ayant pour nom "name".

   Si pas trouvé, retourne le multiplex de nom vide.

   Si pas encore trouvé, retourne NULL
   ------------------------------------------------------------------------ */

dmsMuxStreamSection* dmsMultiplexer::FindSectionMux(const wxString &name)
{
    dmsMuxStreamSection* res, *noname;

    noname=NULL;
    FOREACH(dmsMuxStreamSectionList, m_oStreamSectionList, res)
    {
        if (res->m_oName==name) return res;
        if (noname==NULL && res->m_oName.IsEmpty()) noname=res;
    }

    return noname;
}


dmsMuxStreamSection* dmsMultiplexer::FindSectionMux(int pid, bool force)
{
    dmsMuxStreamSection* res, *nopid;

    nopid=NULL;
    FOREACH(dmsMuxStreamSectionList, m_oStreamSectionList, res)
    {
        if (res->m_iPID==pid) return res;
        if (nopid==NULL && res->m_iPID==-1) nopid=res;
    }

    if (nopid && force)
    {
        dmsMuxStreamSection* other = NewSectionMuxTemplateIntance();

        other->m_iPID       = pid;
        other->m_iRate      = nopid->m_iRate;
        other->m_oGroupName = nopid->m_oGroupName;

#ifdef _DEBUG
        dmsMuxStream* stream;

        LOG0(L"---------------");
        FOREACH(dmsMuxStreamList, m_oStreamList, stream)
        {
           LOG0(L"dmsMultiplexer::FindSectionMux Stream PID=0x%04X, Rate=%5d Kb/s, Group='%s' (%s)",
                stream->m_iPID, stream->m_iRate/1000, (const char *)stream->m_oGroupName, (const char *)stream->m_oLabel);
        }
#endif

        return other;
    }

    return nopid;
}


/* ------------------------------------------------------------------------
   Retourne le flux ayant plus haut débit (ie : référence de temps)
   ------------------------------------------------------------------------ */

dmsMuxStream* dmsMultiplexer::GetMaxRateStream()
{
    dmsMuxStream* res = NULL;
    dmsMuxStream* stream;

    FOREACH(dmsMuxStreamList, m_oStreamList, stream)
    {
        if (res==NULL || stream->m_iRate > res->m_iRate)
        {
            LOG0(L"dmsMultiplexer::GetMaxRateStream pid=0x%x rate=%d\n",
                 stream->m_iPID,
                 stream->m_iRate);
            res=stream;
        }

    }

    return res;
}


int dmsMultiplexer::GetMaxRate()
{
    dmsMuxStream* stream = GetMaxRateStream();

    return stream?stream->m_iRate:0;
}

/* ========================================================================

   ======================================================================== */

bool dmsMultiplexer::AddSection(dmsMPEG_MuxSection &section, int position)
{
    dmsMuxStreamSection* mux = FindSectionMux(section.m_iPID, true);

    if (mux==NULL)
    {
        LOGE(L"Section [%s] with pid [0x%x] has no mux associated",
            section.m_oOutputName,
            section.m_iPID);


        return false;
    }
    mux->Add(section, position);

    return true;
}


/* ========================================================================
   Multiplexage ! ! !
   ======================================================================== */


bool dmsMultiplexer::Play(const dmsMuxStreamList& list, const wxString &filename)
{
   dmsMuxStream* stream;
   dmsBuffer     buffer;
   int           count;
   u64           lastPcr;
   wxFile        output;

#ifdef _DEBUG
   LOG0(L"[LIPPA-DBG] dmsMultiplexer: Play file '%s'...", (const char *)filename);
#endif

   if (!output.Open(filename+".tmp", wxFile::write))
   {
      LOGE(L"Error opening file [%s] for mpeg generation", filename);
      return false;
   }

   // Demarre la generation du fichiers des sections
   if (!PlaySectionsFileStart()) return false;

   // Go
   buffer.Alloc(188);

   // Tous les flux sont a consommer
   FOREACH(dmsMuxStreamList, list, stream)
   {
      if (! stream->PlayStart()) return false;
   }

   count   = 0;
   lastPcr = 0;

   while (1)
   {
      dmsMuxStream* minPcr1; // Prochain flux a consommer (pcr la plus proche)
      dmsMuxStream* minPcr2; // Flux suivant a consommer (ie : fin du flux "minPcr1")
      int nbLap0 = 0;        // Nombre de flux sur leur lap 0 (pas consommés entierement)

      // Recherche des flux "minPcr1" et "minPcr2"
      minPcr1 = NULL;

      FOREACH(dmsMuxStreamList, list, stream)
      {
         if (!minPcr1 || stream->m_iPlayPcr < minPcr1->m_iPlayPcr)
         {
            minPcr2 = minPcr1;
            minPcr1 = stream;
         }
         else if (!minPcr2 || stream->m_iPlayPcr < minPcr2->m_iPlayPcr)
         {
            minPcr2  = stream;
         }

         if (stream->m_iPlayLap==0) nbLap0++;
      }

      if (nbLap0==0 && count > m_iTsMinimalSize) break;

      if (minPcr1 == NULL) break;

      // On consomme "a fond" le flux "minPcr1" jusqu'a l'instant du flux "minPcr2"
      // (Optimisation si un flux du mux est a très haut débit)
      while (1)
      {
         // Lecture & Traitement du TS
         if (! minPcr1->PlayNext(buffer.m_poBuffer)) return false;

         output.Write(buffer.m_poBuffer, 188);
         count += 188;
         lastPcr = minPcr1->m_iPlayPcr;

         // Gestion du bouclage des flux
         if (minPcr1->EofTS())
         {
            if (minPcr1->m_iRate==0) // Les flux à débit null ne sont mis qu'une fois au début
            {
               minPcr1->m_iPlayPcr=-1;
               break;
            }
            minPcr1->m_iPlayLap++;    // Notification de fin de flux
            if (minPcr2==NULL && count>m_iTsMinimalSize) break; // Flux unique : on quitte

            minPcr1->Reset();
            if (minPcr1->EofTS()) break; // Flux vide : on quitte
         }

         // Test de passage au flux suivant
         if (minPcr2 && minPcr2->m_iPlayPcr <= minPcr1->m_iPlayPcr) break;
      }

      if (count==0)
      {
         LOG0(L"Dev Error : no bytes writed by multiplexer");
         return false;
      }
   }

   // Termine la generation du fichiers des sections
   if (!PlaySectionsFileStop()) return false;

   LOG0(L"Mux generated (%.3f KB, %.3f sec) in [%s]",
        count/1000.0,
        ((int)(lastPcr/1000/MPEG_PCR_FREQ_MHZ))/1000.0,
        (const char *)filename);

   FOREACH2(dmsMuxStreamList, list, stream) stream->PlayStop();

   output.Close();
   wxRenameFile(filename+".tmp", filename);

#ifdef _DEBUG
   LOG0(L"[LIPPA-DBG] dmsMultiplexer: Play file '%s' OKAY",(const char *)filename);
#endif

    return true;
}


bool dmsMultiplexer::Play()
{
    dmsMuxStreamList notPlayedList, playingList;
    dmsMuxStream*    stream;
    wxString         groupName;
    wxString         filename;
    bool             withPcr;
    bool             withIncompatiblePcr;

    FOREACH2(dmsMuxStreamList, m_oStreamList, stream)
    {
        if (stream->m_iPID != -1)
        {
            notPlayedList.Append(stream);

            LOG0(L"dmsMultiplexer::Play _ Stream PID=0x%04X, Rate=%5d Kb/s, Group=%s",
                stream->m_iPID, stream->m_iRate/1000, (const char *)stream->m_oGroupName);
        }
    }

    while (notPlayedList.GetCount())
    {
        groupName = notPlayedList[0]->m_oGroupName;

        if (groupName.IsEmpty())
            filename = m_oOutputFile;
        else
        {
            wxString path, name, ext;
            wxFileName::SplitPath(m_oOutputFile, &path, &name, &ext);

            filename = STR("%s/%s.%s.%s", path, name, groupName, ext);
        }

        withPcr             = false;
        withIncompatiblePcr = false;
        FOREACH(dmsMuxStreamList, notPlayedList, stream)
        {
            if (stream->m_oGroupName==groupName)
            {
                notPlayedList.DeleteNode(_node);
                playingList.Append(stream);

                if (stream->m_bCarryPCR) withPcr = true;
                if (stream->m_iRate==0) withIncompatiblePcr = true;
                if (stream->m_iPID<0)   withIncompatiblePcr = true;
            }
        }

        if (withPcr && withIncompatiblePcr)
        {
            LOGE(L"Stream pcr must not be mixed with Stream with null rate or multi-pid");
            return false;
        }

        if (! Play(playingList, filename)) return false;

        playingList.Clear();
    }

    return true;
}

/**
 *  LIPPA v 2.21: Sections Files Management
 *  ---------------------------------------
*/

/**
 *  @brief
 *    Memory implementation of USB Header of section file with the
 *    following format:
 *     usb_file_header () {
 *        version                                32  uimsbf
 *        usb_header_id                          32  uimsbf
 *        update_id                              32  uimsbf
 *        OUI                                    24  uimsbf
 *        platform_model                         16  uimsbf
 *        platform_version                       16  uimsbf
 *        product_model                          16  uimsbf
 *        product_version                        16  uimsbf
 *        sections_number                        32  uimsbf
 *        sections_size                          32  uimsbf
 *        reserved                              360  uimsbf
 *        crc32                                  32  uimsbf
 *     }
 *
 *    with:
 *    - version:          The version of header. Equal 0x00000001.
 *    - usb_header_id:    Identifier of this type of header. Equal 0x555343ED
 *    - update_id:        Identification of the set images to be downloaded in this USB file
 *    - OUI:              Organization Unique Identifier of the receceiver Manufacturer
 *    - platform_model:   The receiver Manufacturer main platform identifier
 *    - platform_version: The receiver Manufacturer platform sub-identifier
 *    - product_model:    The receiver Manufacturer main product identifier
 *    - product_version:  The receiver Manufacturer product sub-identifier
 *    - sections_number:  Number of sections
 *    - sections_size:    Size of sections in bytes
 *    - reserved:         45 bytes equal 0x00
 *    - crc32:            The  CRC 32 of the USB file header
 *
 *    Fixed size of the header = 640 bits = 80 bytes
*/

#define kSECTION_FILE_USB_HEAD_VERSION_SIZE    4
#define kSECTION_FILE_USB_HEAD_ID_SIZE         4
#define kSECTION_FILE_USB_HEAD_UPDATE_ID_SIZE  4
#define kSECTION_FILE_USB_HEAD_OUI_SIZE        3
#define kSECTION_FILE_USB_HEAD_PLTF_MODEL      2
#define kSECTION_FILE_USB_HEAD_PLTF_VERSION    2
#define kSECTION_FILE_USB_HEAD_PROD_MODEL      2
#define kSECTION_FILE_USB_HEAD_PROD_VERSION    2
#define kSECTION_FILE_USB_HEAD_SEC_NUM_SIZE    4
#define kSECTION_FILE_USB_HEAD_SEC_SIZE_SIZE   4
#define kSECTION_FILE_USB_HEAD_RESERVED_SIZE  45
#define kSECTION_FILE_USB_HEAD_CRC32_SIZE      4
#define kSECTION_FILE_USB_HEAD_SIZE           (kSECTION_FILE_USB_HEAD_VERSION_SIZE+kSECTION_FILE_USB_HEAD_ID_SIZE+\
                                               kSECTION_FILE_USB_HEAD_UPDATE_ID_SIZE+kSECTION_FILE_USB_HEAD_OUI_SIZE\
                                               +kSECTION_FILE_USB_HEAD_PLTF_MODEL+kSECTION_FILE_USB_HEAD_PLTF_VERSION\
                                               +kSECTION_FILE_USB_HEAD_PROD_MODEL+kSECTION_FILE_USB_HEAD_PROD_VERSION\
                                               +kSECTION_FILE_USB_HEAD_SEC_NUM_SIZE+kSECTION_FILE_USB_HEAD_SEC_SIZE_SIZE\
                                               +kSECTION_FILE_USB_HEAD_RESERVED_SIZE+kSECTION_FILE_USB_HEAD_CRC32_SIZE)

#define kSECTION_FILE_USB_HEAD_CURR_VERSION   0x00000001
#define kSECTION_FILE_USB_HEAD_HEADER_ID      0x555343ED
#define kSECTION_FILE_USB_HEAD_RESERVED_BYTE           0


class cSectionsFileHeaderUSB : public dmsData
{
public:
    u32      version;
    u32      usb_header_id;
    u32      update_id;
    u32      OUI;
    u16      platform_model;
    u16      platform_version;
    u16      product_model;
    u16      product_version;
    u32      sections_number;
    u32      sections_size;
    dmsData *reserved;
    u32      crc32;

public:
   cSectionsFileHeaderUSB()
   {
      HDR_INIT(version,         kSECTION_FILE_USB_HEAD_VERSION_SIZE*8);
        HDR_INIT(usb_header_id,   kSECTION_FILE_USB_HEAD_ID_SIZE*8);
        HDR_INIT(update_id,       kSECTION_FILE_USB_HEAD_UPDATE_ID_SIZE*8);
        HDR_INIT(OUI,             kSECTION_FILE_USB_HEAD_OUI_SIZE*8);
        HDR_INIT(platform_model,  kSECTION_FILE_USB_HEAD_PLTF_MODEL*8);
        HDR_INIT(platform_version,kSECTION_FILE_USB_HEAD_PLTF_VERSION*8);
        HDR_INIT(product_model,   kSECTION_FILE_USB_HEAD_PROD_MODEL*8);
        HDR_INIT(product_version, kSECTION_FILE_USB_HEAD_PROD_VERSION*8);
        HDR_INIT(sections_number, kSECTION_FILE_USB_HEAD_SEC_NUM_SIZE*8);
        HDR_INIT(sections_size,   kSECTION_FILE_USB_HEAD_SEC_SIZE_SIZE*8);
        HDR_INIT(reserved,         0);
        HDR_INIT(crc32,           kSECTION_FILE_USB_HEAD_CRC32_SIZE*8);

      version       = kSECTION_FILE_USB_HEAD_CURR_VERSION;
      usb_header_id = kSECTION_FILE_USB_HEAD_HEADER_ID;
        reserved = new dmsData();
      reserved->m_oBuffer.Set(kSECTION_FILE_USB_HEAD_RESERVED_BYTE,
                              kSECTION_FILE_USB_HEAD_RESERVED_SIZE);
    }
};


/**
 *  Demarre la generation du fichier sections:
 *  Si a generer:
 *  - Initialise les variables de travail
 *  - Creer le fichier si necessaire
 *  - Si mode avec entete USB, ecrit une entete vide au debut
 *
 *  Note: Le fichier est fermer par la methode PlaySectionsFileStop()
 *        Si cette methode n'a pas ete appelee, c'est qu'une erreur
 *        a ete detectee pendant la generation et le fichier
 *        sera automatiquement ferme et detruit dans le desctructeur
 *        de cete objet.
*/
bool dmsMultiplexer::PlaySectionsFileStart()
{
   if (m_bIsFileSectionsOpen)
   {
      LOGE(L"ERROR: Start play sections but sections file %s already open",
              (const char *)m_oOutputSectionsFile);
      return false;
   }

   m_bIsFileSectionsOpen = false;
   m_u32NumSections      = 0;
   m_u32SizeSections     = 0;

   switch (m_iModeSectionsFile)
   {
   case kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED:
      // Nothing to do
#ifdef _DEBUG
      LOG0(L"[LIPPA-DBG] dmsMultiplexer: Not generate section file");
#endif
      return true;
      break;

   case kTS_WRITER_SEC_FILE_MODE_ADD_SIGNAL_TABLE:
#ifdef _DEBUG
      LOG0(L"[LIPPA-DBG] dmsMultiplexer: Generate sections file '%s' with signalisation tables",
              (const char *)m_oOutputSectionsFile);
#endif
      m_bIsFileSectionsOpen = true;
      break;

   case kTS_WRITER_SEC_FILE_MODE_USB_HEADER:
#ifdef _DEBUG
      LOG0(L"[LIPPA-DBG] dmsMultiplexer: Generate sections file '%s' with USB header",
              (const char *)m_oOutputSectionsFile);
#endif
      m_bIsFileSectionsOpen = true;
      break;

   case kTS_WRITER_SEC_FILE_MODE_ONLY_SECTIONS:
#ifdef _DEBUG
      LOG0(L"[LIPPA-DBG] dmsMultiplexer: Generate sections file '%s' with only carousel sections",
           (const char *)m_oOutputSectionsFile);
#endif
      m_bIsFileSectionsOpen = true;
      break;

   default:
      // Warning => Nothing to do
      LOGW(L"Warning Mode generation sections generation %d unsupported",
           m_iModeSectionsFile);
      return true;
   }

   if (!m_oFileSections.Open(m_oOutputSectionsFile,wxFile::write))
   {
      LOGE(L"Error create file [%s] for sections generation",(const char *)m_oOutputSectionsFile);
      return false;
   }

   // ok
   m_bIsFileSectionsOpen = true;

   if (m_iModeSectionsFile == kTS_WRITER_SEC_FILE_MODE_USB_HEADER)
   {
      // Initialize the USB header to 0 and write it (rewritten with good values at the end)
      uchar  usb_head[kSECTION_FILE_USB_HEAD_SIZE];
      size_t nw;

#ifdef _DEBUG
      LOG0(L"[LIPPA-DBG] dmsMultiplexer: Write USB header initialized with %d 0x%02X",
           kSECTION_FILE_USB_HEAD_SIZE,kSECTION_FILE_USB_HEAD_RESERVED_BYTE);
#endif
      memset(usb_head,kSECTION_FILE_USB_HEAD_RESERVED_BYTE,
             kSECTION_FILE_USB_HEAD_SIZE);
      nw = m_oFileSections.Write(usb_head,kSECTION_FILE_USB_HEAD_SIZE);

      if (nw != kSECTION_FILE_USB_HEAD_SIZE)
      {
         LOGE(L"Initialize USB header in sections file [%s] failure "
              "(write only %d bytes/%d)",
              (const char *)m_oOutputSectionsFile,nw,kSECTION_FILE_USB_HEAD_SIZE);
         return false;
      }
   }

   return true;
}


/**
 *  Termine la generation du fichier sections:
 *  Si le fichier a ete genere (ouvert):
 *  - Si mode avec entete USB, ecrit l'entete valide au debut du fichier
 *  - Ferme le fichier
*/
bool dmsMultiplexer::PlaySectionsFileStop()
{
   cSectionsFileHeaderUSB *p_header = NULL;
   dmsHtoN                 hton;
   size_t                  nw;

   if (!m_bIsFileSectionsOpen) return true;

   switch (m_iModeSectionsFile)
   {
   case kTS_WRITER_SEC_FILE_MODE_ADD_SIGNAL_TABLE:
      LOG0(L"Signalisation tables and carousel sections generated "
           "(%lu sections on %lu bytes=%.3f KB) in [%s]",
           m_u32NumSections,m_u32SizeSections,(double)m_u32SizeSections/1000.0,
           (const char *)m_oOutputSectionsFile);
      m_bIsFileSectionsOpen = true;
      break;

   case kTS_WRITER_SEC_FILE_MODE_USB_HEADER:
      // Genere l'entete USB
      p_header = new cSectionsFileHeaderUSB();
      if (p_header == NULL)
      {
            LOGE(L"Create USB header failure");
            return false;
      }

      p_header->SetName("headerUSB");
      p_header->update_id        = m_u32UpdateId;
      p_header->OUI              = m_u32OUI;
      p_header->platform_model   = m_u16PlateformModel;
      p_header->platform_version = m_u16PlateformVersion;
      p_header->product_model    = m_u16ProductModel;
      p_header->product_version  = m_u16ProductVersion;
      p_header->sections_number  = m_u32NumSections;
      p_header->sections_size    = m_u32SizeSections;

      p_header->Generate1(hton);
      hton.UpdateCRC32(p_header->crc32);
      hton.Copy(p_header->m_oBuffer);

#ifdef _DEBUG
      LOG0(L"[LIPPA-DBG] Write USB Header:");
      LOG0(L"[LIPPA-DBG] - version..........%#x",p_header->version);
      LOG0(L"[LIPPA-DBG] - usb_header_id....%#x",p_header->usb_header_id);
      LOG0(L"[LIPPA-DBG] - update_id........%#x",p_header->update_id);
      LOG0(L"[LIPPA-DBG] - OUI..............%#x",p_header->OUI);
      LOG0(L"[LIPPA-DBG] - platform_model...%#x",p_header->platform_model);
      LOG0(L"[LIPPA-DBG] - platform_version.%#x",p_header->platform_version);
      LOG0(L"[LIPPA-DBG] - product_model....%#x",p_header->product_model);
      LOG0(L"[LIPPA-DBG] - product_version..%#x",p_header->product_version);
      LOG0(L"[LIPPA-DBG] - sections_number..%lu",p_header->sections_number);
      LOG0(L"[LIPPA-DBG] - sections_size....%lu (=%lu Kb)",p_header->sections_size,p_header->sections_size/1024);
      LOG0(L"[LIPPA-DBG] - crc32............%#x",p_header->crc32);
      LOG0(L"[LIPPA-DBG] - Header size......%d",p_header->m_oBuffer.m_iBufferLen);
#endif

      // Ecrire l'entete USB au debut du fichier
      nw = m_oFileSections.Seek(0,wxFromStart);
      if (nw != 0)
      {
         LOGE(L"Seek begining sections file [%s] to write USB header failure "
              "(seek result = %d)",
              (const char *)m_oOutputSectionsFile,nw);
         delete p_header;
         return false;
      }

      nw = m_oFileSections.Write(p_header->m_oBuffer.m_poBuffer,
                                 p_header->m_oBuffer.m_iBufferLen);

      if (nw != (size_t)p_header->m_oBuffer.m_iBufferLen)
      {
         LOGE(L"Write USB header in sections file [%s] failure "
              "(write only %d bytes/%d)",
              (const char *)m_oOutputSectionsFile,nw,p_header->m_oBuffer.m_iBufferLen);
         delete p_header;
         return false;
      }

      delete p_header;

      LOG0(L"Raw carousel sections with USB header generated "
           "(header on %d bytes + %u sections on %u bytes=%.3f KB) in [%s]",
           kSECTION_FILE_USB_HEAD_SIZE,m_u32NumSections,
           m_u32SizeSections,(double)m_u32SizeSections/1024.0,
           (const char *)m_oOutputSectionsFile);
      break;

   case kTS_WRITER_SEC_FILE_MODE_ONLY_SECTIONS:
      LOG0(L"Raw carousel sections generated "
           "(%u sections on %u bytes=%.3f KB) in [%s]",
           m_u32NumSections,m_u32SizeSections,(double)m_u32SizeSections/1024.0,
           (const char *)m_oOutputSectionsFile);
      break;

   case kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED:
   default:
      // Warning, Not close => file automatcly closed and removed in destructor
      LOGW(L"Sections file %s abnormaly open in mode %d without generation",
              (const char *)m_oOutputSectionsFile,m_iModeSectionsFile);
      return true;
   }

   // Close the file
   m_oFileSections.Close();
   m_bIsFileSectionsOpen = false;
   m_u32NumSections      = 0;
   m_u32SizeSections     = 0;

   return true;
}

/**
 *  Generation du fichier sections:
 *  Si le fichier est ouvert (generation demarre), ajoute a la fin du fichier
 *  la section passee en entree.
 *
 *  En mode 'seulement les sections du carousel' et 'avec USB header' on enregistre
 *  seulement sections DSI (tables id 0x3B), DII (tables id 0x3B)
 *  et DDB (tables id 0x3C).
 *
 *  En mode 'avec signalisation' on enregistre en plus, les sections de la
 *  signalisation PSI (PMT, PAT) et de la signalisation SI (NIT ou BAT, SDT, TDT)
 *
*/
bool dmsMultiplexer::PlaySectionsFile (uchar *pucSectionBuffer,
                                       size_t iSectionSize)
{
   size_t sz;

   if (!m_bIsFileSectionsOpen) return true;
   if (pucSectionBuffer == NULL) return true;
   if (iSectionSize < 1) return true;

   switch (pucSectionBuffer[0])
   {
   case 0x3B: // Carousel DSI or DII
   case 0x3C: // Carousel DSI or DII
      // Write in each case
      break;

   default:
      if (m_iModeSectionsFile != kTS_WRITER_SEC_FILE_MODE_ADD_SIGNAL_TABLE)
      {
         // Section carries a signalisation table, not write it in this mode
#ifdef _DEBUG
         LOG0(L"[LIPPA-DBG] dmsMultiplexer: Trash section with table ID %02X",
              pucSectionBuffer[0]);
#endif
         return true;
      }

#ifdef _DEBUG
      LOG0(L"[LIPPA-DBG] dmsMultiplexer: Write signalisation table ID %02X in section file (%d bytes)",
           pucSectionBuffer[0],iSectionSize);
#endif
   }

   sz = m_oFileSections.Write(pucSectionBuffer,iSectionSize);

   if (sz != iSectionSize)
   {
      LOGE(L"Store section %02X in sections file [%s] failure "
           "(write only %d bytes/%d)",
           pucSectionBuffer[0],(const char *)m_oOutputSectionsFile,
           sz,iSectionSize);
      return false;
   }

   m_u32NumSections  += 1;
   m_u32SizeSections += (u32)iSectionSize;

   return true;
}
