/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/03 - Creation
   ************************************************************************ */

#ifndef _TOOLS_SCHEDULER_H_
#define _TOOLS_SCHEDULER_H_

#include <wx/wx.h>
#include <wx/datetime.h>
#include <wx/thread.h>

class dmsScheduler;

class dmsSchedulerEvent : public wxObject
{
public:
    wxDateTime    m_oDate;
    dmsScheduler *m_poScheduler;
    wxString      m_oName;
    int           m_iId;

public:
    void ConstructorInit();
    dmsSchedulerEvent();
    dmsSchedulerEvent(const wxDateTime &date);
    dmsSchedulerEvent(int delay);

    void SetId(int id) {m_iId = id;}

    void Retry(const wxDateTime &date);
    void Retry(int secDelay);

    virtual void Notify() = 0;

    dmsSchedulerEvent *SetName(const wxString &name);
};


WX_DECLARE_LIST(dmsSchedulerEvent, dmsSchedulerList);


class dmsScheduler : public wxThread
{
private:
    dmsSchedulerList  m_oList;
    wxCriticalSection m_oCS;

private:
    dmsSchedulerEvent *Shift();

public:
    void *m_poOwner;

public:
    dmsScheduler(void *owner);
    virtual ~dmsScheduler();

    void Add(dmsSchedulerEvent *event);
    void Remove(int id);


    void Trace();

    ExitCode Entry();
};


#endif /* _TOOLS_SCHEDULER_H_ */
