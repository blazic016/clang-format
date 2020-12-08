/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 11/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Rename object 'cIwediaXXX' to
                                          'cGeniusXXX'
   ************************************************************************ */

#include <wx/file.h>
#include <wx/wx.h>

#include <Tools/Xml.h>

#include <MPEG/Multiplexer.h>

#include "DataCarousel.h"
#include "SuperGroup.h"

#include <DataCarousel/DDB.h>
#include <DataCarousel/DSI.h>
#include <DataCarousel/DSMCC.h>

#include <GeniusCarousel/GeniusCarousel.h>
#include <ObjectCarousel/ObjectCarousel.h>

#include <MPEG/MPEG.h>

/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsDataCarousel::dmsDataCarousel()
{
    m_poMux = NULL;
    m_poSuperGroup = NULL;
    m_poOC = NULL;
    m_poGC = NULL;

    m_iPID = -1;
}

dmsDataCarousel::~dmsDataCarousel()
{
    DELNUL(m_poSuperGroup);
    DELNUL(m_poOC);
    DELNUL(m_poGC);
}

/* ========================================================================

   ======================================================================== */

bool dmsDataCarousel::Load(wxXmlNode* node)
{
    wxXmlNode* child;

    node->Read("PID?", &m_iPID);

    if ((child = node->Find("GeniusCarousel")))
    {
        m_poGC = new cGeniusCarousel(this);
        return m_poGC->Load(child);
    }

    if ((child = node->Find("ObjectCarousel")))
    {
        m_poOC = new dmsObjectCarousel();
        m_poOC->Load(child);
    }
    if ((child = node->Find("SuperGroup")))
    {
        m_poSuperGroup = new dmsSuperGroup(this);
        if (!m_poSuperGroup->Load(child))
            return false;
    }
    else
    {
        LOGE(L"No super group found");
        return false;
    }

    return true;
}

bool dmsDataCarousel::Compile()
{
    if (m_poSuperGroup)
        return m_poSuperGroup->Compile();
    if (m_poGC)
        return m_poGC->Compile();
    return true;
}

void dmsDataCarousel::Generate()
{
    if (m_poSuperGroup)
        m_poSuperGroup->Generate();
    if (m_poGC)
        m_poGC->Generate();
}
