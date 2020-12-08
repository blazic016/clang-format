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

#include <MPEG/Multiplexer.h>

#include <DataCarousel/DSMCC.h>
#include <DataCarousel/DII.h>

#include "DataCarousel.h"
#include "SuperGroup.h"
#include "Group.h"
#include "ModuleData.h"



#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsGroupList);



/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsGroup::dmsGroup(dmsSuperGroup* supergroup)
{
    m_poSection = new dmsDII_Section();

    m_poSuperGroup  = supergroup;
    m_poDataCarousel = supergroup->m_poDataCarousel;

    m_oModuleList.DeleteContents(true);
}


dmsGroup::~dmsGroup()
{
    DELNUL(m_poSection);
}


/* ========================================================================

   ======================================================================== */



bool dmsGroup::Load(wxXmlNode *node)
{
    dmsModule* module;

    LOG_AF(node, LOGE(L"No Group Configuration"));

    node->Read("DownloadId",    &m_poSection->DII->DownloadId);
    node->Read("TransactionId", &m_poSection->DSMCC->TransactionId);
    node->Read("BlockDataSize", &m_poSection->DII->BlockSize);
    node->Read("OutputFrequency?", &m_poSection->MPEG->m_iOutputFrequency, 0);
    node->Read("Debug_TS_Missing?", &m_poSection->MPEG->m_iDebugMissing, 0);

    int id = m_poSection->DII->DownloadId;

    LOG_AF((m_poSection->DSMCC->TransactionId&0xFFFF) >= 2,
        LOGE(L"Transaction ID(%d) & 0xFFFF  for DII must be >= 2", m_poSection->DSMCC->TransactionId));

    m_oId = STR("%s-%02x", m_poSuperGroup->m_oId, id);

    m_poSection->MPEG->m_oTraceFilename = STR("dii-%s",m_oId);
    if (m_poSuperGroup->m_oOutputDir.Len())
        m_oOutputDir = m_poSuperGroup->m_oOutputDir + "/" + m_poSection->MPEG->m_oTraceFilename;

    wxXmlNode *child, *list;

    if ((child = node->Find("GII_Item")))
    {
        wxXmlNode *subchild;

        LOG_AF(m_poSuperGroup->m_poGII, LOGE(L"Not group info indication defined in SuperGroup"));

        dmsDSI_GroupInfoIndicationItem* item = new dmsDSI_GroupInfoIndicationItem();
        item->m_poGroup = this;

        m_poSuperGroup->m_poGII->GroupInfoList->Append(item);

        if ((subchild = child->Find("GroupCompatibility", true)))
        {
            item->GroupCompatibility->Load(subchild);
        }

        if ((subchild = child->Find("GroupInfo")))
        {
            item->GroupInfo->Load(subchild);
        }
    }

    if ((list = node->Find("ModuleList")))
    {
        wxXmlNode *child;

        for (child = list->GetChildren(); child; child=child->GetNext())
        {
            if (child->GetName() == "Module")
            {
                module = new dmsModule(this);

                if (! module->Load(child)) return false;

                module->m_poXmlDef = child;

                m_oModuleList.Append(module);
            }
        }
    }
    else
    {
        LOG_AF(list, LOGE(L"Empty Module List"));
    }


    return true;
}



dmsModule* dmsGroup::FindModule(int id)
{
    dmsModule* module;

    FOREACH(dmsModuleList, m_oModuleList, module)
    {
        if (module->DIIM->ModuleId == id) return module;
    }

    return NULL;
}





bool dmsGroup::Compile()
{
    dmsModule* Module;

    int maxId=0;
    {
        FOREACH(dmsModuleList, m_oModuleList, Module)
            if (Module->DIIM->ModuleId > maxId)
                maxId = Module->DIIM->ModuleId;
    }

    FOREACH2(dmsModuleList, m_oModuleList, Module)
    {
        dmsModuleDataList notEmptyList;
        dmsModuleData*    data;
        dmsModuleData*    save;
        dmsModule*        Mod;
        int               num;

        if (! Module->Compile()) return false;

        save = Module->m_poModuleData;

        //LOG0("---"); Module->m_poModuleData->TraceRec(stdout, "********> ");

        Module->m_poModuleData->GetNotEmpty(notEmptyList);

        num=0;

        FOREACH(dmsModuleDataList, notEmptyList, data)
        {
            if (num==0)
            {
                Mod = Module;
            }
            else
            {
                Mod = new dmsModule(this);

                if (! Mod->Load(Module->m_poXmlDef)) return false;

                DELNUL(Mod->m_poModuleData);

                Mod->DIIM->ModuleId = ++maxId;

                if (_node2_next)
                    m_oModuleList.Insert(_node2_next, Mod);
                else
                    m_oModuleList.Append(Mod);
            }

            data->Remove();

            Mod->Affect(data);

            num++;
        }

        if (save->m_iSize == 0) delete save;
    }

    {
        FOREACH(dmsModuleList, m_oModuleList, Module)
        {
            if (! Module->m_poModuleData->Compile()) return false;

            if (! Module->m_poModuleData->m_bUpdated)
            {
                LOGE(L"Error updating module [%d]", Module->DIIM->ModuleId);
                //return false;
            }
        }
    }

    // Mise à jour DII

    {
        FOREACH(dmsModuleList, m_oModuleList, Module)
        {
            m_poSection->DII->ModuleList->Append(Module->DIIM);
        }
    }

#ifdef _DEBUG
    TraceModules();
#endif
    return true;
}

bool dmsGroup::Generate()
{
    dmsModule* Module;

    FOREACH3(dmsModuleList, m_oModuleList, Module)
    {
        Module->Generate();
    }

    m_poSection->MPEG->Generate1();
    if (m_oOutputDir.Len()) m_poSection->MPEG->Trace(m_oOutputDir);

    m_poSection->MPEG->m_iPID = m_poDataCarousel->m_iPID;
    m_poDataCarousel->m_poMux->Add(*m_poSection->MPEG, 0);

    return true;
}

void dmsGroup::TraceModules()
{
    dmsModule* Module;

    LOG0(L"%d modules", m_oModuleList.GetCount());
    FOREACH2(dmsModuleList, m_oModuleList, Module)
    {
        fprintf(stdout, "- Module %d (%d B)\n", Module->DIIM->ModuleId, Module->m_poModuleData->m_oBuffer.Len());
        Module->m_poModuleData->TraceRec(stdout, "  | ");
    }
}

u32 dmsGroup::GetCumulativeSize()
{
    dmsModule* Module;
    u32        res;

    res=0;
    FOREACH(dmsModuleList, m_oModuleList, Module)
    {
        res += Module->m_poModuleData->m_oBuffer.Len();
    }

    return res;
}
