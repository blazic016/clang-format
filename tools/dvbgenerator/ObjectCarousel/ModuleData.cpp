/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <Tools/File.h>
#include <DataCarousel/Module.h>
#include "ModuleData.h"
#include "ObjectCarousel.h"


/* ########################################################################

   ######################################################################## */


dmsModuleDataBiopData::dmsModuleDataBiopData(dmsModuleData* parent, dmsObjectCarousel* carousel)
: dmsModuleData(parent, MODULE_DATA_TYPE_BIOP_DATA)
{
    m_poCarousel = carousel;

    m_bAllowAppend = false;
}



bool dmsModuleDataBiopData::GetTree(dmsBiopDirMessage* dir, dmsModuleData* ref)
{
    dmsModuleDataList List;
    dmsModuleData*    child;

    ref->GetChildrenAndCompiled(List);

    dmsFileInfo info;

    FOREACH(dmsModuleDataList, List, child)
    {

        info.Init(child->m_oOutputFilename);

        dmsModuleData* data = NULL;

        switch (info.m_eType)
        {
        case DMS_FILE_TYPE_STANDARD:
            {
                dmsBiopFileMessage* File = m_poCarousel->AddFileMess(info.m_oFilename, dir);

                File->SetContent(&(child->m_oBuffer));
                child->m_oBuffer.Clear();
                data = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_FILE_MESSAGE, File->m_poHeader, m_poCarousel);
                data->m_oOutputFilename = File->m_poHeader->m_oFilename;
                m_oCompiledList.Append(data);
            }
            break;
        case DMS_FILE_TYPE_DIRECTORY:
            {
                dmsBiopDirMessage* newDir = m_poCarousel->AddDirMess(info.m_oFilename, dir);

                //data = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_DIR_MESSAGE, newDir->m_poHeader, m_poCarousel);
                //data->m_oOutputFilename = newDir->m_poHeader->m_oFilename;
                //data->m_oRefList.Append(child);
                //m_oCompiledList.Append(data);

                if (! GetTree(newDir, child)) return false;
            }
            break;
        default:
            {
                dmsBiopFileMessage* File = m_poCarousel->AddFileMess(info.m_oFilename, dir);

                File->SetContent(&(child->m_oBuffer));
                child->m_oBuffer.Clear();
                data = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_FILE_MESSAGE, File->m_poHeader, m_poCarousel);
                data->m_oOutputFilename = File->m_poHeader->m_oFilename;
                m_oCompiledList.Append(data);
            }
        }
    }

    return true;
}



bool dmsModuleDataBiopData::CompileBuffer()
{
    if (m_poCarousel->m_poSrgMessage==NULL)
    {
        dmsBiopDirMessage* dir = m_poCarousel->AddDirMess("OUI", NULL);

        dmsModuleData*      child;

        dmsModuleDataList notEmptyList;

        GetNotEmpty(notEmptyList);

        FOREACH(dmsModuleDataList, notEmptyList, child)
        {
            dmsBiopFileMessage* File = m_poCarousel->AddFileMess(child->m_oOutputFilename, dir);

            File->SetContent(&(child->m_oBuffer));

            child->m_oBuffer.Clear();

            dmsModuleData* data = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_FILE_MESSAGE, File->m_poHeader, m_poCarousel);

            data->m_oOutputFilename = File->m_poHeader->m_oFilename;

            m_oCompiledList.Append(data);
        }

        //if (! GetTree(dir, this)) return false;

        m_poCarousel->SetSrg(dir->m_poHeader);
    }

    return true;
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModuleDataBiopRef::dmsModuleDataBiopRef(dmsModuleData* parent, EnumModuleDataType type, dmsBiopMessage *Message, dmsObjectCarousel* carousel)
: dmsModuleData(parent, type)
{
    m_poMessage  = Message;
    m_poCarousel = carousel;
}

dmsModuleDataBiopRef::~dmsModuleDataBiopRef()
{
    if (m_poMessage && m_poMessage->m_poModuleData == this)
    {
        m_poMessage->m_bGenerated   = false;
        m_poMessage->m_poModuleData = NULL;
    }
}

/* ========================================================================

   ======================================================================== */


bool dmsModuleDataBiopRef::CompileBuffer()
{
    LOG_AF(m_oChildList.IsEmpty(), LOGE(L"No child allowed under File Tag"));

    switch (m_eType)
    {
    case MODULE_DATA_TYPE_BIOP_FILE_MESSAGE:
        break;

    case MODULE_DATA_TYPE_BIOP_DIR_MESSAGE:
        if (m_poMessage==NULL)
        {
            dmsBiopMessage *mess;

            FOREACH(dmsBiopMessageList, m_poCarousel->m_oDirMessageList, mess)
            {
                dmsModuleData* child = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_DIR_MESSAGE, mess, m_poCarousel);

                child->m_oOutputFilename = mess->m_oFilename;

                m_oCompiledList.Append(child);
            }

            return true;
        }
        break;
    case MODULE_DATA_TYPE_BIOP_SERVICE_GATEWAY_MESSAGE:
        if (m_poMessage==NULL)
        {
            m_poMessage = m_poCarousel->m_poSrgMessage;
        }
        break;
    default:
        LOGE(L"Module Data Biop ref undefined");
        return false;
    }

    if (m_poMessage==NULL)
    {
        LOGE(L"No message for Biop Ref");
        return true;
    }

    m_poMessage->SetModuleData(this);
    m_poMessage->Generate1();
    m_oBuffer.Set(m_poMessage->m_oBuffer);

    m_bUpdated = m_poMessage->m_bUpdateComplete;

    return true;
}


bool dmsModuleDataBiopRef::Affect(dmsModule *module)
{
    m_poModule = module;

    m_oDirTrace = m_poModule->m_oOutputDir;

    return true;
}


void dmsModuleDataBiopRef::FileTrace()
{
    if (m_poMessage)
    {
        m_poMessage->m_oOutputName += STR("%04d", m_poMessage->m_iKey);
        m_poMessage->Trace(m_oDirTrace);
    }
}


wxString dmsModuleDataBiopRef::TraceName()
{
    if (m_poMessage==NULL) return "";

    return m_poMessage->m_oOutputName;

    // Old

    wxString name = wxFileNameFromPath(m_poMessage->m_oFilename);

    switch (m_eType)
    {
    case MODULE_DATA_TYPE_BIOP_FILE_MESSAGE: return STR("BiopFileMessage_"+name);
    case MODULE_DATA_TYPE_BIOP_DIR_MESSAGE: return STR("BiopDirMessage_"+name);
    case MODULE_DATA_TYPE_BIOP_SERVICE_GATEWAY_MESSAGE: return STR("BiopServiceGatewayMessage_"+name);
    default: return "???";
    }
}
