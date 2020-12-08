/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/03 - Creation
   ************************************************************************ */


#include "Tools.h"
#include "Scheduler.h"

/* ########################################################################

   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsSchedulerList);

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

void dmsSchedulerEvent::ConstructorInit()
{
    m_oDate.SetToCurrent();

    m_poScheduler = NULL;
    m_iId         = 0;
}

dmsSchedulerEvent::dmsSchedulerEvent()
{
    ConstructorInit();
}

dmsSchedulerEvent::dmsSchedulerEvent(const wxDateTime &date)
{
    ConstructorInit();

    m_oDate = date;
}

dmsSchedulerEvent::dmsSchedulerEvent(int delay)
{
    ConstructorInit();

    m_oDate += wxTimeSpan(0,0,delay,0);
}



/* ========================================================================

   ======================================================================== */


void dmsSchedulerEvent::Retry(const wxDateTime &date)
{
    m_oDate = date;
}

void dmsSchedulerEvent::Retry(int secDelay)
{
    Retry(wxDateTime::Now() + wxTimeSpan(0,0,secDelay,0));
}

/* ========================================================================

   ======================================================================== */

dmsSchedulerEvent *dmsSchedulerEvent::SetName(const wxString &name)
{
    m_oName = name;
    return this;
}


/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsScheduler::dmsScheduler(void *owner) : wxThread(wxTHREAD_JOINABLE)
{
    m_poOwner = owner;
}


dmsScheduler::~dmsScheduler()
{

}


/* ========================================================================

   ======================================================================== */


dmsSchedulerEvent *dmsScheduler::Shift()
{
    dmsSchedulerEvent *result;
    dmsSchedulerList::Node *node;

    m_oCS.Enter();
    node = m_oList.GetFirst();
    if (node)
    {
        if (! node->GetData()->m_oDate.IsLaterThan(wxDateTime::Now()))
        {
            result = node->GetData();
            m_oList.DeleteNode(node);
        }
        else
        {
            result = NULL;
        }
    }
    else
    {
        result = NULL;
    }
    m_oCS.Leave();

    return result;
}

wxThread::ExitCode dmsScheduler::Entry()
{
    dmsSchedulerEvent *event;

    while (!TestDestroy())
    {
        event = Shift();

        if (event)
        {
            wxDateTime saveDate = event->m_oDate;
            wxStopWatch sw;

            sw.Start();
            //LOG4("Exec %s", event->m_oName);
            event->Notify();
            if (sw.Time() > 1000)
            {
                //LOG(INF4, "Exec in %d ms", sw.Time());
            }
            if (saveDate != event->m_oDate)
            {
                Add(event);
            }
            else
            {
                DELNUL(event);
            }
            //Trace();
        }
        else
        {
            Sleep(1000);
        }
    }

    return NULL;
}






void dmsScheduler::Add(dmsSchedulerEvent *event)
{
    dmsSchedulerEvent *current;

    m_oCS.Enter();

    // Ajout trié
    size_t position = 0;
    FOREACH(dmsSchedulerList, m_oList, current)
    {
        if (event->m_oDate.IsEarlierThan(current->m_oDate))
            break;
        position++;
    }

    m_oList.Insert(position, event);

    event->m_poScheduler = this;

    m_oCS.Leave();
}


void dmsScheduler::Remove(int id)
{
    dmsSchedulerEvent *event;

    m_oCS.Enter();

    FOREACH(dmsSchedulerList, m_oList, event)
    {
        if (event->m_iId == id)
        {
            m_oList.DeleteNode(_node);
            DELNUL(event);
        }
    }

    m_oCS.Leave();
}




void dmsScheduler::Trace()
{
    dmsSchedulerEvent *current;

    m_oCS.Enter();

    int i = 0;
    LOG4(L"Scheduler list %d", m_oList.GetCount());
    FOREACH(dmsSchedulerList, m_oList, current)
    {
        LOG4(L"Scheduled Task %d %s : %s", i++, current->m_oName, current->m_oDate.Format());
    }

    m_oCS.Leave();
}
