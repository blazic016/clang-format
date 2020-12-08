/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/05 - Creation
   ************************************************************************ */

#include "Log.h"
#include "File.h"
#include "FileManager.h"

/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsFileManager::dmsFileManager() : dmsEventManager()
{
    m_poBuffer          = NULL;
    m_iProgressSize     = 0;
    m_iProgressCount    = 0;
}


dmsFileManager::~dmsFileManager()
{
    if (m_poBuffer) {
        delete [] m_poBuffer;
        m_poBuffer = NULL;
    }
}

/* ========================================================================

   ======================================================================== */

void dmsFileManager::SetBuffer(int size)
{
    if (m_poBuffer) {
        delete [] m_poBuffer;
        m_poBuffer = NULL;
    }

    m_poBuffer    = new char[size];
    m_iBufferSize = size;
}




bool dmsFileManager::Copy(const wxString &from, const wxString &to, bool force)
{
    dmsFileInfo    srcInfo, dstInfo;
    dmsFile        srcFile, dstFile;
    int            nbRead, nbWrite;
    bool           ok;

    // Vérification sur les fichiers

    if (m_bCanceled) return false;

    srcInfo.Init(from);
    dstInfo.Init(to);

    if (srcInfo.m_eType != DMS_FILE_TYPE_STANDARD)
    {
        // C'est un répertoire
        m_iProgressGlobalCount += srcInfo.m_iSize;
        return false;
    }

    if (!force && srcInfo.Similar(dstInfo))
    {
        // Pas besoin de copier, existe déjà sur disque
        m_iProgressGlobalCount += srcInfo.m_iSize;
        return true;
    }

    dmsAssumeDir(wxPathOnly(to));

    // Ouverture de la source

    if (! srcFile.Open(from, dmsFile::read))
    {
        LOG0(L"Open file failed on '%s'", from);
        m_iProgressGlobalCount += srcInfo.m_iSize;
        return false;
    }

    // Ouverture de la destination

    if (! dstFile.Open(to, dmsFile::write))
    {
        LOG0(L"Open file failed on '%s'", from);
        srcFile.Close();
        m_iProgressGlobalCount += srcInfo.m_iSize;
        return false;
    }

    // Préparation

    if (m_poBuffer == NULL) SetBuffer();

    m_oProgressName  = STR("Copy %s", wxFileNameFromPath(from));
    m_iProgressSize  = srcFile.Length();
    m_iProgressCount = 0;

    // Copie

    ok = true;

    while (true)
    {
        if (m_bCanceled)
        {
            LOG0(L"Copy canceled on '%s'", from);
            ok = false; break;
        }

        nbRead = srcFile.Read(m_poBuffer, m_iBufferSize);

        if (nbRead == 0) break;

        nbWrite = dstFile.Write(m_poBuffer, nbRead);

        if (nbWrite != nbRead)
        {
            LOG0(L"Write failed on '%s'", to);
            ok = false; break;
        }

        m_iProgressCount += nbRead;
        m_iProgressGlobalCount += nbRead;

        wxThread::Sleep(1); // Economie CPU
    }

    srcFile.Close();
    dstFile.Close();

    // Terminaison

    if (ok)
    {
        dmsSetFileModificationTime(to, wxFileModificationTime(from));
    }
    else
    {
        wxRemoveFile(to);
    }

    return ok;
}

/* ------------------------------------------------------------------------
   Supervision
   ------------------------------------------------------------------------ */

void dmsFileManager::GetPrivateProgress(wxString &log)
{
    if (m_iProgressGlobalSize == 0)
    {
        if (m_iProgressSize)
            log.Printf("%s %d%%", m_oProgressName, m_iProgressCount*100/m_iProgressSize);
        else
            log.Clear();
    }
    else
    {
        log.Printf("%d%%", m_iProgressGlobalCount*100/m_iProgressGlobalSize);

        if (m_iProgressSize)
            log << STR(" (%s %d%%)", m_oProgressName, m_iProgressCount*100/m_iProgressSize);
    }
}
