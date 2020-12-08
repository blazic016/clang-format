/* ************************************************************************
   SmarDTV

   Description : Gestion des codes et des messages d'erreurs

   Historique :
   - COF   - Iwedia  - v 0    - 09/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/filename.h>

#ifdef __WXMSW__
#include <windows.h>
#endif

#include <wx/file.h>


#include "Log.h"
#include "File.h"

#define DMS_LOG_BUFFER_SIZE   (4096) // Taille max d'un log


/* ########################################################################

   Les LOGS

   ######################################################################## */


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsLogList);


char dmsLogLevelLetter[] = {'E', 'W', '0', '1', '2', '3', '4'};



/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

#ifdef _DEBUG
dmsLog::dmsLog(EnumDmsLogLevel level, wxString file, int line)
{
    // Mise a jour champs debug
    m_oSrcFile  = file;
    m_iSrcLine  = line;
#else
dmsLog::dmsLog(EnumDmsLogLevel level)
{
#endif
    // Mise a jour du niveau et de la date
    m_eLevel    = level;
    m_oDateTime.SetToCurrent();

    // Autres infos
    m_poThread = wxThread::This();
}



dmsLog::~dmsLog()
{

}



dmsLog *dmsLog::SetText(const wxChar *format, ...)
{
    // Déclarations statiques (optimisation)
    static char              zFormatBuffer[DMS_LOG_BUFFER_SIZE];
    static wxCriticalSection oFormatCS;

    // Formatage
    oFormatCS.Enter(); // Entree section critique

    va_list argptr;
    va_start(argptr, format);

    if (wxVsnprintf(zFormatBuffer, DMS_LOG_BUFFER_SIZE, format, argptr) < 0 )
    {
        zFormatBuffer[DMS_LOG_BUFFER_SIZE-1] = 0;
    }

    m_oMessage = zFormatBuffer;

    oFormatCS.Leave(); // Sortie section critique

    g_poLogManager->Append(this);

    va_end(argptr);

    return this;
}



/* ========================================================================

   ======================================================================== */

wxString dmsLog::GetMessage()
{
    return m_oMessage;
}



wxString dmsLog::GetFilePos()
{
#ifdef _DEBUG
    wxString name;

    wxFileName::SplitPath(m_oSrcFile, NULL, &name, NULL);

    return STR("%s:%d", name, m_iSrcLine);
#else
    return "";
#endif
}



/* ########################################################################

   Le gestionnaire de LOGS

   ######################################################################## */


dmsLogManager *g_poLogManager = NULL;



/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsLogManager::dmsLogManager()
{
}


dmsLogManager::~dmsLogManager()
{
    Clear();
}


/* ========================================================================

   ======================================================================== */


void dmsLogManager::Clear()
{
    m_oCS.Enter();
    m_oList.DeleteContents(true);
    m_oList.Clear();
    m_oList.DeleteContents(false);
    m_oCS.Leave();
}

void dmsLogManager::Append(dmsLog *log)
{
    m_oCS.Enter();
    m_oList.Append(log);
    m_oCS.Leave();

#if (wxUSEGUI==0)
#ifndef _CONSOLE
#define _CONSOLE
#endif
#endif

#ifdef _CONSOLE
#ifdef _DEBUG
    printf("%s [%c] %s - %s\n",
        (const char *)log->m_oDateTime.FormatTime(),
        dmsLogLevelLetter[log->m_eLevel],
        (const char *)log->m_oMessage,
        (const char *)log->GetFilePos());

#ifdef __WXMSW__
    {
        char prefix[256];
        sprintf(prefix, "%s(%d):%s [%c]",
            log->m_oSrcFile.c_str(), log->m_iSrcLine,
            log->m_oDateTime.FormatTime().c_str(),
            dmsLogLevelLetter[log->m_eLevel]);
        OutputDebugString(prefix);
        OutputDebugString(log->m_oMessage.c_str());
        OutputDebugString("\n");
    }
#endif
#else
    printf("%s [%c] %s\n",
        (const char *)log->m_oDateTime.FormatTime(),
        dmsLogLevelLetter[log->m_eLevel],
        (const char *)log->m_oMessage);
#endif
#else
#ifdef _DEBUG
    wxLogDebug("%s [%c] %s                (%s)",
        log->m_oDateTime.FormatTime(),
        dmsLogLevelLetter[log->m_eLevel],
        log->m_oMessage.c_str(),
        log->GetFilePos());
#endif
#endif
}


dmsLog *dmsLogManager::Shift()
{
    dmsLog           *result;
    dmsLogList::Node *node;

    m_oCS.Enter();
    node = m_oList.GetFirst();
    if (node)
    {
        result = node->GetData();
        m_oList.DeleteNode(node);
    }
    else
    {
        result = NULL;
    }
    m_oCS.Leave();

    return result;
}



int dmsLogManager::GetCount()
{
    int result;

    m_oCS.Enter();
    result = m_oList.GetCount();
    m_oCS.Leave();

    return result;
}


wxString dmsLogManager::Format(const char *format, EnumDmsLogLevel level)
{
    wxString result;
    dmsLog *log;

    m_oCS.Enter();
    FOREACH(dmsLogList, m_oList, log)
    {
        if (log->m_eLevel <= level && log->m_poThread == wxThread::This())
        {
            result += STR(format, log->GetMessage());
        }
    }
    m_oCS.Leave();

    return result;
}




void dmsLogManager::KeepLogs()
{
    m_oKeepLogsCS.Enter();
}

void dmsLogManager::RestoreLogs(wxString &logs, const char *format, EnumDmsLogLevel level)
{
    logs = Format(format, level);

    m_oKeepLogsCS.Leave();
}






/* ########################################################################

   Thread de sauvegarde des logs

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsSaveLogThread::dmsSaveLogThread() : wxThread(wxTHREAD_JOINABLE)
{
    m_iDelay = 10;
    m_iTTL   = 1;
}


dmsSaveLogThread::~dmsSaveLogThread()
{

}


/* ========================================================================
   Accesseurs
   ======================================================================== */

void dmsSaveLogThread::SetDirectory(const wxString &dirname)
{
    m_oDirname = dirname;
    dmsAssumeDir(dirname);
}



/* ========================================================================

   ======================================================================== */


wxThread::ExitCode dmsSaveLogThread::Entry()
{
    if (m_oDirname.IsEmpty())
    {
        LOGE(L"LOG No directory to save data");
        return NULL;
    }

    dmsAssumeDir(m_oDirname);

    while (!TestDestroy())
    {
        Save();
        Sleep(m_iDelay*1000);
    }
    Save();

    return NULL;
}


void dmsSaveLogThread::Save()
{
    wxFile   file;
    wxString filename;

    if (g_poLogManager == NULL) return;
    if (! g_poLogManager->GetCount()) return;

    filename.Printf("%s/log-%s.txt", m_oDirname, wxDateTime::Now().Format("%Y-%m-%d"));

    if (! file.Open(filename, wxFile::write_append))
    {
        LOGE(L"Unable to open file %s", filename);
    }
    else
    {
        wxCriticalSectionLocker lock(g_poLogManager->m_oKeepLogsCS);
        wxString line;

        dmsLog *log;

        while (1)
        {
            log = g_poLogManager->Shift();
            if (log == NULL) break;

            line.Printf("%s [%c] %s\n",
                log->m_oDateTime.Format(),
                dmsLogLevelLetter[log->m_eLevel],
                log->m_oMessage.c_str());

            file.Write(line.c_str(), line.Len());

            DELNUL(log);
        }
    }
}
