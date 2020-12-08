/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#include <wx/wx.h>
#include <wx/file.h>
#include <wx/ffile.h>

#include <Tools/Tools.h>
#include <Tools/Xml.h>
#include <Tools/File.h>

#include <MPEG/Multiplexer.h>

#include <ObjectCarousel/ObjectCarousel.h>

#include "SuperGroup.h"
#include "DataCarousel.h"

#include <DataCarousel/DSI.h>
#include <DataCarousel/DSMCC.h>


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsSuperGroup::dmsSuperGroup(dmsDataCarousel* DataCarousel)
{
    m_poDataCarousel = DataCarousel;
    m_poSection      = new dmsDSI_Section();

    m_poGII = NULL;
    m_poSGI = NULL;

    m_oGroupList.DeleteContents(true);
}


dmsSuperGroup::~dmsSuperGroup()
{
    DELNUL(m_poSection)
}

/* ========================================================================

   ======================================================================== */

bool dmsSuperGroup::Load(wxXmlNode *node)
{
    wxString value;
    wxXmlNode *child, *list;

    LOG_AF(node, LOGE(L"No SuperGroup"));

    node->Read("TransactionId",     &m_poSection->DSMCC->TransactionId);
    node->Read("OutputFrequency?",  &m_poSection->MPEG->m_iOutputFrequency, 0);
    node->Read("Debug_TS_Missing?", &m_poSection->MPEG->m_iDebugMissing, 0);

    int id = m_poSection->DSMCC->TransactionId;

    LOG_AF((id&0xFFFF) < 2, LOGE(L"Transaction ID & 0xFFFF for DSI must be < 2"));

    m_oId = STR("%02x", id);

    m_poSection->MPEG->m_oTraceFilename = STR("dsi-%s", m_oId);
    if (m_poDataCarousel->m_oOutputDir.Len())
        m_oOutputDir = m_poDataCarousel->m_oOutputDir + "/" + m_poSection->MPEG->m_oTraceFilename;

    if ((list = node->Find("BiopServiceGatewayInfo")))
    {
        m_poSGI = new dmsBiopServiceGatewayInfo(
            m_poDataCarousel->m_poOC,
            m_poDataCarousel->m_poOC->m_poSrgMessage);

        if (! m_poSGI->Load(list)) return false;

        m_poSection->DSI->PrivateData = m_poSGI;
        m_poSGI->SetName("BiopServiceGatewayInfo");
    }
    else if ((list = node->Find("GroupInfoIndication")))
    {
        m_poGII = new dmsDSI_GroupInfoIndication();
        m_poSection->DSI->PrivateData = m_poGII;
        m_poGII->SetName("GroupInfoIndication");
    }
    else
    {
        LOGE(L"No private data in DSI");
        return false;
    }

    if ((list = node->Find("GroupList")))
    {
        for (child = list->GetChildren(); child; child=child->GetNext())
        {
            if (child->GetName() == "Group")
            {
                dmsGroup* Group;

                Group = new dmsGroup(this);

                if (! Group->Load(child)) return false;

                m_oGroupList.Append(Group);
            }
        }
    }
    else
    {
        LOGE(L"Empty super group");
        return false;
    }

    return true;
}


bool dmsSuperGroup::Compile()
{
    dmsGroup* Group;

    FOREACH(dmsGroupList, m_oGroupList, Group)
    {
        if (! Group->Compile()) return false;
    }

    return true;
}

void dmsSuperGroup::Generate()
{
    dmsGroup* Group;

    FOREACH(dmsGroupList, m_oGroupList, Group)
    {
        Group->Generate();
    }

    m_poSection->MPEG->Generate1();

    if (m_oOutputDir.Len()) m_poSection->MPEG->Trace(m_oOutputDir);

    m_poSection->MPEG->m_iPID = m_poDataCarousel->m_iPID;
    m_poDataCarousel->m_poMux->Add(*m_poSection->MPEG, 0);
}
