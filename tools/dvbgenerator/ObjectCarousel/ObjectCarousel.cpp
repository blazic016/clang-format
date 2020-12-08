/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <Tools/File.h>
#include <Tools/Xml.h>

#include <DataCarousel/ModuleData.h>


#include "ObjectCarousel.h"

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsObjectCarousel::dmsObjectCarousel()
{
    m_iId       = 0;
    m_iAssocTag = 0;

    m_poRoot       = NULL;
    m_poSrgMessage = NULL;

    m_oFileMessageList.DeleteContents(true);
    m_oDirMessageList.DeleteContents(true);
}


dmsObjectCarousel::~dmsObjectCarousel()
{
    DELNUL(m_poRoot);
    DELNUL(m_poSrgMessage);
}

/* ========================================================================

   ======================================================================== */

bool dmsObjectCarousel::Load(wxXmlNode *node)
{
    node->Read("Id",       &m_iId);
    node->Read("Dirname?", &m_oDirname);
    node->Read("AssocTag", &m_iAssocTag);
    node->Read("OUI",      &m_iOUI);
    node->Read("Version",  &m_iVersion);

    if (m_oDirname.Len()) {if (!LoadDir()) return false;}

    return true;
}



bool dmsObjectCarousel::LoadDir()
{
    dmsStandardFileName(m_oDirname);

    m_poRoot = new dmsFileNode(NULL, m_oDirname, true);

    LOG_AF(m_poRoot->Expand(m_oFilter), LOGE(L"Error listing directory [%s]", m_oDirname));

#ifdef _DEBUG
    //m_poRoot->Trace();
#endif

    dmsBiopMessage* Root = Load(m_poRoot);

    SetSrg(Root);

    return true;
}


void dmsObjectCarousel::SetSrg(dmsBiopMessage *root)
{
    m_poSrgMessage = new dmsBiopMessage(this);
    dmsBiopDirMessage* Srg = new dmsBiopDirMessage(m_poSrgMessage);

    m_poSrgMessage->m_oFilename = "SRG";
    root->m_oFilename = "OUI";

    m_poSrgMessage->SetKind((char *)"srg");
    m_poSrgMessage->SetKey(0);
    m_poSrgMessage->SetName("BIOP_ServiceGatewayMessage");

    dmsBiopBinding* Binding = new dmsBiopBinding(root);

    Binding->SetSingleBiopName(STR("%06x", m_iOUI));

    Srg->BindingsList->Append(Binding);

    dmsBiopServiceContext* context = new dmsBiopServiceContext();
    Srg->ServiceContextList->Append(context);
}


int dmsObjectCarousel::GetNextId()
{
    int res =
        m_oFileMessageList.GetCount()+
        m_oDirMessageList.GetCount()+1;

    return res;
}



dmsBiopDirMessage* dmsObjectCarousel::AddDirMess(const wxString &name, dmsBiopDirMessage* Dir)
{
    dmsBiopMessage*    Msg    = new dmsBiopMessage(this);
    dmsBiopDirMessage* SubDir = new dmsBiopDirMessage(Msg);

    Msg->SetKey(GetNextId());
    Msg->m_oFilename = name;
    m_oDirMessageList.Insert(Msg);

    return SubDir;
}


dmsBiopFileMessage* dmsObjectCarousel::AddFileMess(const wxString &name, dmsBiopDirMessage* Dir)
{
    dmsBiopMessage*     Elt  = new dmsBiopMessage(this);
    dmsBiopFileMessage* File = new dmsBiopFileMessage(Elt);

    Elt->SetKey(GetNextId());
    Elt->m_oFilename = name;
    m_oFileMessageList.Append(Elt);

    AddBinding(Elt, Dir);

    return File;
}


dmsBiopBinding* dmsObjectCarousel::AddBinding(dmsBiopMessage* Message, dmsBiopDirMessage* Dir)
{
    dmsBiopBinding* Binding = new dmsBiopBinding(Message);

    Binding->SetSingleBiopName(wxFileNameFromPath(Message->m_oFilename).c_str());

    Dir->BindingsList->Append(Binding);

    return Binding;
}




dmsBiopMessage* dmsObjectCarousel::Load(dmsFileNode *node)
{
    dmsBiopDirMessage* Dir = AddDirMess(node->m_oInfo.m_oFilename, NULL);
    dmsFileNode*       child;

    FOREACH(dmsFileList, node->m_oChildList, child)
    {
        if (child->m_oInfo.m_eType == DMS_FILE_TYPE_DIRECTORY)
            AddBinding(Load(child), Dir);
        else
            AddFileMess(child->m_oInfo.m_oFilename, Dir)->SetContent();
    }

    return Dir->m_poHeader;
}
