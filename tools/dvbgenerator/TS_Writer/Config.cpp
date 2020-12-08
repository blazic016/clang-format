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

#include <Tools/Xml.h>

#include "TS_WriterVersion.h"
#include "Config.h"


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsTS_WriterConf::dmsTS_WriterConf()
{
   m_bTrace              = false;
   m_iLoader             = 0;
   m_iModeSectionsFile   = kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED;
   m_u32UpdateId         = 0;
   m_u32OUI              = 0;
   m_u16PlateformModel   = 0;
   m_u16PlateformVersion = 0;
   m_u16ProductModel     = 0;
   m_u16ProductVersion   = 0;
}


dmsTS_WriterConf::~dmsTS_WriterConf()
{

}


/* ========================================================================

   ======================================================================== */

/* BELB Comment _ Get conf parameters from XML */
bool dmsTS_WriterConf::Load(wxXmlNode *node)
{
    wxXmlNode *sections_node;

    LOG_AF(node, LOGE(L"No Configuration"));

    node->ReadSFN("Output",        &m_oOutputFile);
    node->ReadSFN("OutputSections",&m_oOutputSectionsFile);
    node->ReadSFN("OutputSig?",    &m_oOutputSigFile);
    node->Read("Trace",            &m_bTrace);
    node->Read("Loader",           &m_iLoader);

    // Get if USB header in sections file and get IDs
    if ((sections_node = node->Find("OutputSections")))
    {
        sections_node->Read("generationMode",&m_iModeSectionsFile);
        sections_node->Read("updateId",      &m_u32UpdateId);
        sections_node->Read("oui",           &m_u32OUI);
        sections_node->Read("pltfModel",     &m_u16PlateformModel);
        sections_node->Read("pltfVersion",   &m_u16PlateformVersion);
        sections_node->Read("prodModel",     &m_u16ProductModel);
        sections_node->Read("prodVersion",   &m_u16ProductVersion);
    }
   else m_iModeSectionsFile = kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED;


   return true;
}

