/* ************************************************************************
   SmarDTV

   Description : Gestion des codes et des messages d'erreurs

   Historique :
   - COF   - Iwedia  - v 0    - 09/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _TOOLS_ERROR_H_
#define _TOOLS_ERROR_H_

#include <wx/wx.h>
#include <wx/datetime.h>
#include <wx/thread.h>

#include "Tools.h"

class dmsLog;
class dmsLogManager;


/* ------------------------------------------------------------------------
   Variables externes
   ------------------------------------------------------------------------ */

extern dmsLogManager *g_poLogManager;

/* ------------------------------------------------------------------------
   Types
   ------------------------------------------------------------------------ */


typedef enum
{
    DMS_LOG_LEVEL_ERR,
    DMS_LOG_LEVEL_WARN,
    DMS_LOG_LEVEL_INFO0,
    DMS_LOG_LEVEL_INFO1,
    DMS_LOG_LEVEL_INFO2,
    DMS_LOG_LEVEL_INFO3,
    DMS_LOG_LEVEL_INFO4,
} EnumDmsLogLevel;


extern char dmsLogLevelLetter[];

/* ------------------------------------------------------------------------
   MACROS
   ------------------------------------------------------------------------ */

#ifdef _DEBUG
#define DMS_LOG_DEBUG_EXTRA ,__FILE__,__LINE__
#else
#define DMS_LOG_DEBUG_EXTRA
#endif

#define LOGE (new dmsLog(DMS_LOG_LEVEL_ERR   DMS_LOG_DEBUG_EXTRA))->SetText
#define LOGW (new dmsLog(DMS_LOG_LEVEL_WARN  DMS_LOG_DEBUG_EXTRA))->SetText
#define LOG0 (new dmsLog(DMS_LOG_LEVEL_INFO0 DMS_LOG_DEBUG_EXTRA))->SetText
#define LOG1 (new dmsLog(DMS_LOG_LEVEL_INFO1 DMS_LOG_DEBUG_EXTRA))->SetText
#define LOG2 (new dmsLog(DMS_LOG_LEVEL_INFO2 DMS_LOG_DEBUG_EXTRA))->SetText
#define LOG3 (new dmsLog(DMS_LOG_LEVEL_INFO3 DMS_LOG_DEBUG_EXTRA))->SetText
#define LOG4 (new dmsLog(DMS_LOG_LEVEL_INFO4 DMS_LOG_DEBUG_EXTRA))->SetText

// Sortie de fonction en cas d'erreur

#define ERRB(_f) \
    {bool _ret = (_f); if (_ret == false) {LOG4("Exit Function"); return false;}}

#define LOG_AF(_cond,_log) {if (!(_cond)) {_log;return false;}}
#define LOG_AN(_cond,_log) {if (!(_cond)) {_log;return NULL;}}
#define LOG_AV(_cond,_log) {if (!(_cond)) {_log;return;}}

#define TODO() LOGW(L"### TODO ### %s %d", __FILE__, __LINE__);



/* ------------------------------------------------------------------------
   Encapsulation d'une erreur
   ------------------------------------------------------------------------ */

class dmsLog : public wxObject
{
public:
    EnumDmsLogLevel m_eLevel;    // Niveau
    wxDateTime      m_oDateTime; // Date
    wxString        m_oMessage;  // Message

    void           *m_poThread; // Thread d'execution

#ifdef _DEBUG
    wxString        m_oSrcFile; // Debug : fichier source
    int             m_iSrcLine; // Debug : ligne dans le fichier source
#endif

public:
#ifdef _DEBUG
    dmsLog(EnumDmsLogLevel level, wxString file, int line);
#else
    dmsLog(EnumDmsLogLevel level);
#endif

    dmsLog *SetText(const wxChar *format, ...);

    virtual ~dmsLog();

    wxString GetMessage();

    wxString GetFilePos();
};


WX_DECLARE_LIST(dmsLog, dmsLogList);


/* ------------------------------------------------------------------------
   Le Log Manager
   ------------------------------------------------------------------------ */


class dmsLogManager : public wxObject
{
public:
    dmsLogList        m_oList;
    wxCriticalSection m_oCS;
    wxCriticalSection m_oKeepLogsCS;

public:
    dmsLogManager();
    virtual ~dmsLogManager();

    void Append(dmsLog *log);
    dmsLog *Shift();


    int GetCount();
    void Clear();

    wxString Format(const char *format, EnumDmsLogLevel level=DMS_LOG_LEVEL_INFO4);

    void KeepLogs();
    void RestoreLogs(wxString &logs, const char *format, EnumDmsLogLevel level=DMS_LOG_LEVEL_INFO4);
};


/* ------------------------------------------------------------------------
   Sauvegarde des logs
   ------------------------------------------------------------------------ */

class dmsSaveLogThread : public wxThread
{
public:
    wxString m_oDirname;
    int      m_iDelay;
    int      m_iTTL;

public:
    dmsSaveLogThread();
    virtual ~dmsSaveLogThread();

    void SetDirectory(const wxString &dirname);
    void SetDelay(int sec);
    void SetTTL(int days);

    void Save();

    ExitCode Entry();
};


#endif /* _TOOLS_ERROR_H_ */
