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
#include <Tools/Conversion.h>

#include <MPEG/MPEG.h>
#include <MPEG/Multiplexer.h>
#include <DataCarousel/DSMCC.h>
#include <DataCarousel/DDB.h>
#include <ObjectCarousel/BIOP.h>

#include "Module.h"
#include "SuperGroup.h"
#include "Group.h"
#include "ModuleData.h"
#include "DataCarousel.h"



#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsModuleList);


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModule::dmsModule(dmsGroup *group)
{
    DIIM = new dmsDII_Module();
    DIIM->m_poModule = this;

    m_iDDBOutputFrequency  = 0;
    m_poXmlDef             = NULL;

    m_poGroup        = group;
    m_poSuperGroup   = group->m_poSuperGroup;
    m_poDataCarousel = group->m_poDataCarousel;

    m_oBlockList.DeleteContents(true);

    m_poDescList         = NULL;
    m_poModuleData       = NULL;

    m_iCompressionMethod = -1;
}


dmsModule::~dmsModule()
{
    DELNUL(m_poModuleData);
}


/* ========================================================================

   ======================================================================== */

bool dmsModule::Load(wxXmlNode *node)
{
    wxXmlNode* child;
    LOG_AF(node, LOGE(L"No Module"));

    dmsModuleData* data = new dmsModuleData(NULL, MODULE_DATA_TYPE_DATA);

    data->m_poDataCarousel = m_poDataCarousel;

    m_poModuleData = data;

    node->Read("ModuleId",          &DIIM->ModuleId);
    node->Read("Version",           &DIIM->ModuleVersion);
    node->Read("OutputFrequency?",  &m_iDDBOutputFrequency, 0);
    node->Read("Debug_TS_Missing?", &m_iDDBDebugMissing, 0);

    LOG_AF(node->Find("Data", child), LOGE(L"No module data for [%s]", node->GetLongName()));

    if (! m_poModuleData->Load(child)) return false;

    wxXmlNode *list;

    if ((list = node->Find("BiopModuleInfo")))
    {
        LOG_AF(m_poDataCarousel->m_poOC, LOGE(L"No object carousel defined"));
        dmsBiopModuleInfo *elt = new dmsBiopModuleInfo(this);

        if (! elt->Load(list)) return false;

        DIIM->ModuleInfo = elt;
        m_poDescList     = elt->UserInfoDescList;
    }
    else if ((list = node->Find("ModuleInfo")))
    {
        dmsDSMCC_DescriptorList *elt = new dmsDSMCC_DescriptorList(this);

        if (! elt->Load(list)) return false;

        DIIM->ModuleInfo = elt;
        m_poDescList     = elt;
    }
    else
    {
        LOGE(L"No Module info for node [%s]", node->GetLongName());
        return false;
    }


    return true;
}

/* ------------------------------------------------------------------------
   Regarde s'il ajouter la compression du module
   ------------------------------------------------------------------------ */

void dmsModule::AddCompress()
{
    if (m_iCompressionMethod < 0) return;

    dmsModuleDataCompress *data = new dmsModuleDataCompress(NULL);

    m_poModuleData->MoveTo(data);

    m_poModuleData = data;
}



bool dmsModule::Compile()
{
    return m_poModuleData->Compile();
}



bool dmsModule::Generate()
{
    dmsModuleData *data;

    // Object Carousel
    // Pour les modules contenant les BIOP_DirMessage et les BIOP_ServiceGatewayMessage,
    // on effectue la generation en utilisant les donnees du dernier fils.
    if ((m_poDataCarousel->m_poOC == NULL) ||
        (!(((data = GetLastDataOfType(MODULE_DATA_TYPE_BIOP_DIR_MESSAGE)) != NULL) ||
           ((data = GetLastDataOfType(MODULE_DATA_TYPE_BIOP_SERVICE_GATEWAY_MESSAGE)) != NULL))))
    {
        data = m_poModuleData;
    }

    if (data->m_oBuffer.IsEmpty())
    {
        LOGE(L"Module [%s] : empty data", m_oId);
        return false;
    }

    int lastBlockNumber = (data->m_oBuffer.Len()-1) / m_poGroup->m_poSection->DII->BlockSize;
    int i=0;

    int group = m_poDataCarousel->m_poMux->GetNextGroup();

    for (int num=0; num<=lastBlockNumber; num++)
    {
        dmsDDB_Section* block = new dmsDDB_Section(this, num, lastBlockNumber);

        block->MPEG->m_iGroup = group;
        block->MPEG->m_iDebugMissing = m_iDDBDebugMissing;

        if (m_iDDBDebugMissing > 0) m_iDDBDebugMissing--;

        block->DDB->Data->m_oBuffer.Set(data->m_oBuffer, i, m_poGroup->m_poSection->DII->BlockSize);

        m_oBlockList.Append(block);

        i+=m_poGroup->m_poSection->DII->BlockSize;

        block->Generate();
    }

    if (m_oOutputDir.Len())
    {
        data->m_oBuffer.Save(m_oOutputDir+"/data.bin");

        wxFFile file;

        if (! file.Open(m_oOutputDir+"/data-info.txt", "w"))
        {
            LOGE(L"Error opening file [%s]", m_oOutputDir+"/data-info.txt");
            return false;
        }
        data->TraceRec(file.fp(),"");
        file.Close();
    }

    return true;
}


void dmsModule::Affect(dmsModuleData* moduleData)
{
    m_oId = STR("%s-%03x", m_poGroup->m_oId, DIIM->ModuleId);

    if (m_poGroup->m_oOutputDir.Len())
        m_oOutputDir = m_poGroup->m_oOutputDir + "/" + STR("module-%s",m_oId);

    m_poModuleData = moduleData;

    AddCompress();

    m_poModuleData->AffectRec(this);
}


dmsModuleData *dmsModule::GetLastDataOfType(EnumModuleDataType type)
{
    dmsModuleData *data, *current;

    data = NULL;
    current = m_poModuleData;

    while ((current != NULL) && (current = current->FindType(type)) != NULL)
    {
        data = current;
        if ((data->m_oChildList).GetCount() == 0)
        {
            current = NULL;
        }
        else
        {
            current = data->m_oChildList[0];
        }
    }

    return data;
}
