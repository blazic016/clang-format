/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/03 - Creation
   ************************************************************************ */


#ifndef _TOOLS_EVENT_H_
#define _TOOLS_EVENT_H_

#include <wx/wx.h>

class dmsEventThread;
class dmsEventManager;


enum
{
    DMS_EVENT_MANAGER_EMPTY = 0,
    DMS_EVENT_MANAGER_USER,
};

/* ------------------------------------------------------------------------
   L'évenement
   ------------------------------------------------------------------------ */

class dmsEvent : public wxObject
{
public:
    dmsEventManager* m_poManager;
    int              m_iId;
    void*            m_poData;
    void*            m_poSubData;

public:
    dmsEvent(int id, void *data=NULL, void *subdata=NULL);
    virtual ~dmsEvent();

    wxString GetDebugInfo();

    virtual void Run();
};

WX_DECLARE_LIST(dmsEvent, dmsEventList);


/* ------------------------------------------------------------------------
   Le gestionnaire d'évenement
   ------------------------------------------------------------------------ */

class dmsEventManager : public wxObject
{
public:
    dmsEventList      m_oList;
    wxCriticalSection m_oCS;
    dmsEventThread*   m_poThread;
    dmsEvent*         m_poCurrentEvent;
    wxString          m_oName;
    bool              m_bDebug;
    bool              m_bCanceled;

    dmsEventManager*  m_poOutput;
    bool              m_bOutputOwner;

public:
    dmsEventManager();
    virtual ~dmsEventManager();

    // Accesseurs
    void SetName(const wxString &value);
    void SetDebug(bool value);
    void SetOutput(dmsEventManager *value);

    // Initialisation
    void CreateThread();
    void CreateOutput();

    // Terminaison
    void Clear();

    // Ajout
    void Add(dmsEvent *event);

    // Execution
    dmsEvent* Shift(dmsEvent **event = NULL);
    bool      ShiftAndProcess();
    bool      ShiftAndProcessOnThread();
    virtual void ProcessEvent(dmsEvent *event);

    // Supervision
    bool GetProgress(wxString &log);
    bool IsEmpty();

    virtual void GetPrivateProgress(wxString &log);
};

/* ------------------------------------------------------------------------
   Le thread pour exécuter les évenements
   ------------------------------------------------------------------------ */

class dmsEventThread : public wxThread
{
public:
    dmsEventManager *m_poManager;
    int             m_iDelay;

public:
    dmsEventThread(dmsEventManager *manager);
    virtual ~dmsEventThread();

    ExitCode Entry();
};


#endif /* _TOOLS_EVENT_H_ */
