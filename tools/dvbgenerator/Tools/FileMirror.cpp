/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/05 - Creation
   ************************************************************************ */

#include "Tools.h"

#include "FileMirror.h"
#include "FileManager.h"


/* ########################################################################

   ######################################################################## */


#define DMS_FILE_MIRROR_OUPUT_EVENT(_id) \
{if (m_poManager && m_poManager->m_poOutput)\
m_poManager->m_poOutput->Add(new dmsEvent(_id, this));}


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsFileMirror::dmsFileMirror()
{
    m_poRootSrc = NULL;
    m_poRootDst = NULL;
    m_poManager = NULL;
    m_bManagerOwner = false;
}


dmsFileMirror::~dmsFileMirror()
{
    DELNUL(m_poRootSrc);
    DELNUL(m_poRootDst);

    if (m_bManagerOwner)
        DELNUL(m_poManager);
}


/* ========================================================================

   ======================================================================== */

void dmsFileMirror::SetRootSrc(const wxString &filename)
{
    DELNUL(m_poRootSrc);

    m_poRootSrc = new dmsFileNode(NULL, filename);
}


void dmsFileMirror::SetRootDst(const wxString &filename)
{
    DELNUL(m_poRootDst);

    m_poRootDst = new dmsFileNode(NULL, filename);
}


void dmsFileMirror::SetManager(dmsFileManager *manager)
{
    if (m_poManager)
    {
        LOG0(L"MANAGER ALREADY DEFINED");
        return;
    }

    m_poManager     = manager;
    m_bManagerOwner = false;
}

void dmsFileMirror::CreateManager()
{
    if (m_poManager)
    {
        LOG0(L"MANAGER ALREADY DEFINED");
        return;
    }

    m_poManager     = new dmsFileManager;
    m_bManagerOwner = true;
}


/* ========================================================================

   ======================================================================== */

void dmsFileMirror::ListSrc()
{
    if (m_poRootSrc == NULL) return;

    m_poRootSrc->Init();
    m_poRootSrc->Expand();
    m_poRootSrc->SetSubChildInfo();
}


void dmsFileMirror::ListDst()
{
    if (m_poRootDst == NULL) return;

    dmsAssumeDir(m_poRootDst->m_oInfo.m_oFilename);

    m_poRootDst->Init();
    m_poRootDst->Expand();
    m_poRootDst->SetSubChildInfo();
}


void dmsFileMirror::List()
{
    if (m_poRootSrc == NULL || m_poRootDst == NULL) return;

    ListSrc();
    ListDst();

    DMS_FILE_MIRROR_OUPUT_EVENT(DMS_EVENT_FILE_MIRROR_LIST_DONE);
}

void dmsFileMirror::Push()
{
    if (m_poRootSrc == NULL || m_poRootDst == NULL) return;

    dmsFileNode *srcNode = m_poRootSrc;
    dmsFileNode *dstNode;
    wxString from, to;

    if (m_poManager == NULL) CreateManager();

    m_poManager->m_iProgressGlobalSize  = m_poRootSrc->m_iSubChildVolume;
    m_poManager->m_iProgressGlobalCount = 0;

    int begin = m_poRootSrc->m_oInfo.m_oFilename.Len()+1;

    m_poRootDst->SetMarkRec(0);

    while ((srcNode = srcNode->GetNextRec()))
    {
        from = srcNode->m_oInfo.m_oFilename;
        to   = m_poRootDst->m_oInfo.m_oFilename + "/" +  from.Mid(begin);

        dstNode = m_poRootDst->AssumeNode(to);

        m_poManager->Copy(from, to, false);

        dstNode->SetMark(1);
    }

    DMS_FILE_MIRROR_OUPUT_EVENT(DMS_EVENT_FILE_MIRROR_PUSH_DONE);
}


void dmsFileMirror::Delete()
{
    if (m_poRootSrc == NULL || m_poRootDst == NULL) return;

    m_poRootDst->RemoveFileWithMarkRec(0);

    DMS_FILE_MIRROR_OUPUT_EVENT(DMS_EVENT_FILE_MIRROR_DELETE_DONE);
}


void dmsFileMirror::Update()
{
    if (m_poRootSrc == NULL || m_poRootDst == NULL) return;

    List();
    Push();
    Delete();

    DMS_FILE_MIRROR_OUPUT_EVENT(DMS_EVENT_FILE_MIRROR_UPDATE_DONE);
}


/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsFileMirrorEvent::dmsFileMirrorEvent(int id, void *data, void *subdata)
: dmsEvent(id, data, subdata)
{

}


dmsFileMirrorEvent::~dmsFileMirrorEvent()
{

}


/* ========================================================================

   ======================================================================== */

void dmsFileMirrorEvent::Run()
{
    dmsFileMirror *mir = (dmsFileMirror*) m_poData;

    switch (m_iId)
    {
    case DMS_EVENT_FILE_MIRROR_LIST:   mir->List(); break;
    case DMS_EVENT_FILE_MIRROR_PUSH:   mir->Push(); break;
    case DMS_EVENT_FILE_MIRROR_DELETE: mir->Delete(); break;
    case DMS_EVENT_FILE_MIRROR_UPDATE: mir->Update(); break;
    }
}
