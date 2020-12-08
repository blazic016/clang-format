/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/03 - Creation
   ************************************************************************ */


#include "Tools.h"
#include "Event.h"


/* ########################################################################

   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsEventList);


/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsEvent::dmsEvent(int id, void *data, void *subdata)
{
    m_iId       = id;
    m_poData    = data;
    m_poSubData = subdata;
}


dmsEvent::~dmsEvent()
{

}


/* ========================================================================

   ======================================================================== */

wxString dmsEvent::GetDebugInfo()
{
    return STR("%d with data %p/%p", m_iId, m_poData, m_poSubData);
}

/* ========================================================================

   ======================================================================== */

void dmsEvent::Run()
{
    TODO();
}


/* ########################################################################

   MANAGER D'EVENTS

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsEventManager::dmsEventManager()
{
    m_poThread       = NULL;
    m_poCurrentEvent = NULL;
    m_poOutput       = NULL;
    m_bOutputOwner   = false;
    m_bDebug         = false;
    m_bCanceled      = false;
}

dmsEventManager::~dmsEventManager()
{
    if (m_poThread)
        m_poThread->Delete();

    if (m_bOutputOwner)
        DELNUL(m_poOutput);

    DELNUL(m_poThread);
}

/* ========================================================================
   Accesseurs
   ======================================================================== */

void dmsEventManager::SetName(const wxString &value)
{
    m_oName = value;
}

void dmsEventManager::SetDebug(bool value)
{
    m_bDebug = value;
}

/* ========================================================================

   ======================================================================== */

/* ------------------------------------------------------------------------
   Ajout d'un event
   ------------------------------------------------------------------------ */

void dmsEventManager::Add(dmsEvent *event)
{
    m_bCanceled = false;

    if (m_bDebug)
    {
        LOG0(L"[%s] Add  Task : %s", m_oName, event->GetDebugInfo());
    }
    m_oCS.Enter();
    m_oList.Append(event);
    event->m_poManager = this;
    m_oCS.Leave();
}

/* ------------------------------------------------------------------------
   Retire l'élément suivant de la liste
   ------------------------------------------------------------------------ */

dmsEvent *dmsEventManager::Shift(dmsEvent **event)
{
    dmsEventList::Node *node;
    dmsEvent *tmpEvent;

    if (event == NULL) event = &tmpEvent;

    m_oCS.Enter();
    node = m_oList.GetFirst();
    if (node)
    {
        *event = node->GetData();
        m_oList.DeleteNode(node);
    }
    else
    {
        *event = NULL;
    }
    m_oCS.Leave();

    if (m_bDebug && *event)
    {
        LOG0(L"**** [%s] Exec Task : %s", m_oName, (*event)->GetDebugInfo());
    }

    return *event;
}

/* ------------------------------------------------------------------------
   Retire et exécute l'élément suivant de la liste
   ------------------------------------------------------------------------ */

bool dmsEventManager::ShiftAndProcess()
{
    dmsEvent *event;

    Shift(&event);

    if (event)
    {
        ProcessEvent(event);
        DELNUL(event);
        return true;
    }

    return false;
}

/* ------------------------------------------------------------------------
   Retire et exécute l'élément suivant de la liste.

   Cette forme est "multi-thread safe".

   Elle permet l'exécution sur sur un thread dédié et sécurise la
   supervision de l'évenement courant.
   ------------------------------------------------------------------------ */

bool dmsEventManager::ShiftAndProcessOnThread()
{
    Shift(&m_poCurrentEvent);

    if (m_poCurrentEvent)
    {
        ProcessEvent(m_poCurrentEvent);
        m_oCS.Enter();
        DELNUL(m_poCurrentEvent);
        m_oCS.Leave();

        if (m_poOutput && IsEmpty())
        {
            m_poOutput->Add(new dmsEvent(DMS_EVENT_MANAGER_EMPTY, this));
        }

        return true;
    }

    return false;
}

void dmsEventManager::ProcessEvent(dmsEvent *event)
{
    event->Run();
}

void dmsEventManager::Clear()
{
    dmsEvent *event;

    m_bCanceled = true;

    while (true)
    {
        event = Shift();
        if (event == NULL) break;
        DELNUL(event);
    }

    while (m_poCurrentEvent)
    {
        wxThread::Sleep(1);
    }
}

void dmsEventManager::CreateThread()
{
    if (m_poThread) return;

    m_poThread = new dmsEventThread(this);

    m_poThread->Create();
    m_poThread->Run();
}

void dmsEventManager::CreateOutput()
{
    if (m_poOutput)
    {
        LOG0(L"OUPUT ALREADY DEFINED");
        return;
    }

    m_poOutput     = new dmsEventManager();
    m_bOutputOwner = true;
}

void dmsEventManager::SetOutput(dmsEventManager *value)
{
    if (m_poOutput)
    {
        LOG0(L"OUPUT ALREADY DEFINED");
        return;
    }

    m_poOutput     = value;
    m_bOutputOwner = false;
}

bool dmsEventManager::IsEmpty()
{
    wxCriticalSectionLocker lock(m_oCS);

    return
        (m_poCurrentEvent == NULL) &&
        (m_oList.GetCount() == 0);
}


bool dmsEventManager::GetProgress(wxString &log)
{
    wxCriticalSectionLocker lock(m_oCS);

    if ((m_poCurrentEvent == NULL) && (m_oList.GetCount() == 0))
        return false;
    else
    {
        GetPrivateProgress(log);
        return true;
    }

}

void dmsEventManager::GetPrivateProgress(wxString &log)
{
    log.Printf("Remains %d tasks", m_oList.GetCount() + m_poCurrentEvent ? 1 : 0);
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsEventThread::dmsEventThread(dmsEventManager *manager) : wxThread(wxTHREAD_JOINABLE)
{
    m_poManager = manager;
    m_iDelay    = 1;
}


dmsEventThread::~dmsEventThread()
{
}

/* ========================================================================

   ======================================================================== */

wxThread::ExitCode dmsEventThread::Entry()
{
    while (!TestDestroy())
    {
        if (! m_poManager->ShiftAndProcessOnThread())
        {
            wxThread::Sleep(m_iDelay);
        }
    }

    return 0;
}
