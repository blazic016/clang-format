/****************************************************************************
** @file GeniusCarousel.cpp
**
** @brief
**   Genius Carousel generation.
**
**
** @ingroup GENIUS CAROUSEL
**
** @version $Rev: 61909 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/lib/GeniusCarousel/GeniusCarousel.cpp $
**          $Date: 2011-06-10 10:47:16 +0200 (ven., 10 juin 2011) $
**
** @author  SmarDTV Rennes - LIPPA
**
** COPYRIGHT:
**   2011 SmarDTV
**
** @history
**  - COF   - Iwedia  - v 0    - 05/2005 - Creation
**  - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
**
******************************************************************************/


#include <Tools/Tools.h>
#include <Tools/File.h>
#include <Tools/Xml.h>

#include <DataCarousel/DataCarousel.h>

#include <MPEG/MPEG.h>
#include <MPEG/Multiplexer.h>

#include "GeniusImagesData.h"
#include "GeniusCarousel.h"


/* ========================================================================
   Downloader Data Block
   ======================================================================== */
class cGeniusDownloaderDataBlock : public dmsData
{
public:
    u32      UpdateId;
    u32      ModuleVersion;
    u32      ModuleOffset;
    u32      ModuleSize;
    dmsData *Data;

   cGeniusDownloaderDataBlock (dmsMPEG_Section *section) : dmsData()
   {
      HDR_INIT(UpdateId,      32);
      HDR_INIT(ModuleVersion, 32);
      HDR_INIT(ModuleOffset,  32);
      HDR_INIT(ModuleSize,    32);
      HDR_INIT(Data,          0);

      Data = new dmsDataHexa();
      section->TableId          = 0x84;
      section->PrivateIndicator = 1;
      section->SetName(&section->TableIdExtension,"ModuleId");
      section->SetName("DLB_Section");
      SetName("DLB_Data");
   }
};

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

cGeniusCarousel::cGeniusCarousel(dmsDataCarousel* carousel)
{
    m_poImage        = NULL;
    m_poDataCarousel = carousel;
}


cGeniusCarousel::~cGeniusCarousel()
{
    DELNUL(m_poImage);
}

/* ========================================================================

   ======================================================================== */


bool cGeniusCarousel::Load(wxXmlNode *node)
{
    wxXmlNode *child;

    if ((child = node->Find("GeniusImagesData")))
    {
        m_poImage = new cGeniusImagesData(NULL);

        if (!m_poImage->Init(child)) return false;
        if (!m_poImage->Load(child)) return false;
    }

    return true;
}


bool cGeniusCarousel::Compile()
{
    if (!m_poImage->Compile()) return false;

    m_poImage->TraceRec(stdout);
    return true;
}

void cGeniusCarousel::Generate()
{
    dmsModuleData             *p_data;
    cGeniusImagesBlock        *p_block;
    dmsMPEG_Section            section;
    cGeniusDownloaderDataBlock dlb(&section);
    int                        num;

    section.SetData(&section.Data,&dlb);
    section.m_iPID = m_poDataCarousel->m_iPID;

    FOREACH(dmsModuleDataList, m_poImage->m_oChildList, p_data)
    {
        p_block = (cGeniusImagesBlock*)p_data;

        section.TableIdExtension = p_block->Number();

        dlb.UpdateId      = m_poImage->UpdateId();
        dlb.ModuleVersion = 0;
        dlb.ModuleSize    = p_data->m_oBuffer.Len();
        dlb.ModuleOffset  = 0;

        num=0;
        while (dlb.ModuleOffset < dlb.ModuleSize)
        {
            dlb.Data->m_oBuffer.Set(p_data->m_oBuffer, dlb.ModuleOffset, 4066);

            dlb.ModuleOffset += dlb.Data->m_oBuffer.Len();

            section.Generate1();

            section.m_oTraceFilename = STR("blb-%s-%02d", p_data->m_oOutputFilename, num);

            m_poDataCarousel->m_poMux->Add(section);

            if (m_poImage->m_oOutputDir.Len() && m_poDataCarousel->m_oOutputDir.Len())
            {
                section.Trace(m_poDataCarousel->m_oOutputDir+"/GeniusPrivateSections");
            }
            num++;
        }
    }

    section.SetData(&section.Data, NULL);
}


/* GeniusCarousel.cpp */
