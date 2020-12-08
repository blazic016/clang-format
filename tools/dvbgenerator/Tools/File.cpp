/* ************************************************************************
   SmarDTV

   Description : Extensions au système de fenetrage de wxWindows

   Historique :
   - COF   - Iwedia  - v 0    - 05/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifdef __UNIX__
#include <sys/types.h>
#include <utime.h>
#else
#include <sys/utime.h>
#include <errno.h>
#endif

#include <errno.h>
#include <ctype.h>

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>

#include "Tools.h"
#include "File.h"
#include "Event.h"
#include "Conversion.h"
#include "Tlv.h"

#ifdef WIN32
#include <windows.h>
#endif


/* ########################################################################

   ######################################################################## */


/* ========================================================================

   ======================================================================== */

bool dmsCheckDir(const wxString &dirname, const wxString &labelname)
{
    if (dirname.IsEmpty())
    {
        LOGE(L"Directory '%s' has no value", (const char *)labelname);
        return false;
    }

    if (! wxDirExists(dirname))
    {
        LOGE(L"Directory '%s' does not exists (%s)", (const char *)labelname, (const char *)dirname);
        return false;
    }

    return true;
}


/* ========================================================================

   ======================================================================== */

bool dmsSimilarFiles(const wxString &filename1, const wxString &filename2)
{
    dmsFile file1, file2;
    bool result;

    if (! wxFileExists(filename1) || ! wxFileExists(filename2))
        return false;

    if (! file1.Open(filename1, dmsFile::read))
        return false;

    if (! file2.Open(filename2, dmsFile::read))
    {
        file1.Close();
        return false;
    }

    result =
        (file1.Length() == file2.Length());

    file1.Close();
    file2.Close();

    return result;
}


/* ------------------------------------------------------------------------
   Return just the directory, or NULL if no directory
   ------------------------------------------------------------------------ */


#define _MAXPATHLEN 500

wxString dmsPathOnly (const wxString& path)
{
    if (path != wxT(""))
    {
        wxChar buf[_MAXPATHLEN];

        // Local copy
        wxStrcpy (buf, WXSTRINGCAST path);

        int l = path.Length();
        bool done = FALSE;

        int i = l - 1;

        // Search backward for a backward or forward slash
        while (!done && i > -1)
        {
            if (path[i] == wxT('/') || path[i] == wxT('\\'))
            {
                done = TRUE;
                buf[i] = 0;
                return wxString(buf);
            }
            else i --;
        }

#if defined(__WXMSW__) || defined(__WXPM__)
        // Try Drive specifier
        if (wxIsalpha(buf[0]) && buf[1] == wxT(':'))
        {
            // A:junk --> A:. (since A:.\junk Not A:\junk)
            buf[2] = wxT('\0');
            return wxString(buf);
        }
#endif
    }

    return wxString(wxT(""));
}







/* ------------------------------------------------------------------------
   S'assure de la présence d'un répertoire avec création si absent
   ------------------------------------------------------------------------ */


bool dmsAssumeDir(const wxString &dir, bool log)
{
    wxString path;

    // Fin de la récursivité
    if (dir == "") return true;
    if (wxDirExists(dir)) return true;

    // Un fichier de meme nom ne doit pas exister sur disque
    if (wxFileExists(dir))
    {
        LOGE(L"File [%s] is not a directory", (const char *)path);
        return false;
    }

    path = dmsPathOnly(dir);

    // On est remonté jusqu'a une racine non accessible
    if (path == dir)
    {
        LOGE(L"Drive [%s]' is not accessible", (const char *)dir);
        return false;
    }

    // Recursivite : on crée d'abord les répertoires pères
    if (! dmsAssumeDir(path)) return false;

    // Le path existe : on rajoute le répertoire courant
    if (!wxMkdir(dir))
    {
        LOGE(L"Can not create directory [%s]", (const char *)dir);
        return false;
    }
    else if (log)
    {
        LOG0(L"Path [%s] created", (const char *)dir);
    }

    return true;
}


/* ------------------------------------------------------------
   Fixe la date d'un fichier
   ------------------------------------------------------------ */

bool dmsSetFileModificationTime(const wxString &filename, time_t time)
{
    if (!wxFileExists(filename)) return false;

#ifndef __UNIX__
    struct _utimbuf times;

    times.actime = time;
    times.modtime = time;

    if (_utime(filename.c_str(), &times ) == -1)
    {
        LOGE(L"Set time of file '%s' failed (errno %u)", (const char *)filename, errno);
        return false;
    }
#else
    struct utimbuf times;

    times.actime = time;
    times.modtime = time;

    if (utime(filename.c_str(), &times ) == -1)
    {
        LOGE(L"Set time of file '%s' failed (errno %u)", (const char *)filename, errno);
        return false;
    }

#endif
    return true;
}

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

void dmsStandardFilename(wxString &filename)
{
    filename.Replace("\\", "/");

    while (filename.Len() && filename[filename.Len()] == '/')
    {
        filename = filename.RemoveLast();
    }

    if (filename.IsEmpty()) { filename = "."; return; }
}

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

void dmsRemoveIfExists(wxString &filename)
{
    if (wxFileExists(filename)) wxRemoveFile(filename);
}


bool dmsFileSize(wxString &filename, off_t &size)
{
    if (! wxFileExists(filename)) return false;

    wxFile file;

    if (! file.Open(filename)) return false;

    size = file.Length();

    return true;
}



/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsFileInfo::dmsFileInfo()
{
    Init();
}

dmsFileInfo::dmsFileInfo(const wxString &filename, bool init)
{
    m_oFilename = filename;

    if (init)
        Init();
}

dmsFileInfo::~dmsFileInfo()
{

}


/* ========================================================================

   ======================================================================== */

void dmsFileInfo::Init()
{
    m_iSize = 0;

    if (m_oFilename.IsEmpty()) return;

    if (wxFileExists(m_oFilename))
    {
        dmsFile file;

        m_eType = DMS_FILE_TYPE_STANDARD;

        if (file.Open(m_oFilename, dmsFile::read))
        {
            m_iSize = file.Length();
        }
        else
        {
            m_iSize = -1;
            // Echec en lecture : le fichier est protégé
            // ou bien il est en cours de construction (windows)
        }

        m_oDate   = wxFileModificationTime(m_oFilename);
    }
    else if (wxDirExists(m_oFilename))
    {
        m_eType   = DMS_FILE_TYPE_DIRECTORY;
        m_oDate   = wxFileModificationTime(m_oFilename);
    }
    else
    {
        m_oDate.Set((time_t)0);
        m_eType = DMS_FILE_TYPE_UNKNOWN;
    }
}

void dmsFileInfo::Init(const wxString &filename)
{
    m_oFilename = filename;

    Init();
}

dmsFileInfo &dmsFileInfo::Set(const dmsFileInfo &other)
{
    m_oFilename = other.m_oFilename;
    m_iSize     = other.m_iSize;
    m_eType     = other.m_eType;
    m_oDate     = other.m_oDate;

    return (*this);
}


bool dmsFileInfo::Equals(const dmsFileInfo &other) const
{
    return
        m_oFilename == other.m_oFilename &&
        m_iSize     == other.m_iSize &&
        m_eType     == other.m_eType &&
        m_oDate     == other.m_oDate;
}

bool dmsFileInfo::Similar(const dmsFileInfo &other) const
{
    return
        m_iSize     == other.m_iSize &&
        m_eType     == other.m_eType &&
        m_oDate     == other.m_oDate;
}

/* ========================================================================

   ======================================================================== */

wxString dmsFileInfo::SizeWithUnit() const
{
    return dmsFileSizeWithUnit(m_iSize);
}

wxString dmsFileInfo::TypeAbbrev() const
{
    char* str[] = {(char *)"?", (char *)"F", (char *)"D"};
    return str[m_eType];
}



/* ########################################################################

   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsFileList);

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsFileNode::dmsFileNode(dmsFileNode *parent, const wxString &filename, bool init)
: m_oChildList()
{
    m_oChildList.DeleteContents(true);

    m_poParent = parent;
    m_oInfo.m_oFilename = filename;

    m_eScanState   = DMS_FILE_SCAN_STATE_NEW;
    m_bScanAllowed = true;

    if (init)
        m_oInfo.Init();
}


dmsFileNode::~dmsFileNode()
{

}


/* ========================================================================

   ======================================================================== */


void dmsFileNode::SetFilename(const wxString &name)
{
    m_oInfo.m_oFilename = name;
}


/* ========================================================================

   ======================================================================== */

void dmsFileNode::Init()
{
    m_oInfo.Init();
}

bool dmsFileNode::Expand(const wxString &filter)
{
    if (m_oInfo.m_eType != DMS_FILE_TYPE_DIRECTORY) return false;

    wxDir dir(m_oInfo.m_oFilename);

    if (! dir.IsOpened()) return false;

    wxString filename;
    dmsFileNode *node;
    bool cont = dir.GetFirst(&filename, filter);

    while (cont)
    {
        filename = m_oInfo.m_oFilename + "/" + filename;

        node = Find(filename);

        if (node == NULL)
        {
            node = new dmsFileNode(this, filename, true);
            Append(node);
        }

        node->Expand();

        cont = dir.GetNext(&filename);
    }

    return true;
}

/* ------------------------------------------------------------------------
   Scan du répertoire avec déclenchement des évenements
   - NEW
   - MODIFIED
   - DELETED
   Gestion de la purges des noeuds quand leur traitement DELETED à été
   effectué (m_bScanAllowed = true)
   ------------------------------------------------------------------------ */

void dmsFileNode::Scan(dmsEventManager *manager)
{
    dmsFileNode *node;

    // Le scan ne détecte les différences qui si la précédente différence
    // à été traité (m_bScanAllowed)

    if (m_bScanAllowed)
    {
        // Rechargement des propriétés

        dmsFileInfo info = m_oInfo;

        m_oInfo.Init();

        if (! info.Similar(m_oInfo))
        {
            m_bScanAllowed = false;
            // S'il y a une changement, on le notifie
            if (m_oInfo.m_eType != info.m_eType)
            {
                // Le fichier n'est plus sur disque (ou à changé de type)
                m_eScanState = DMS_FILE_SCAN_STATE_DELETED;
                if (manager) manager->Add(new dmsEvent(DMS_EVT_FILE_SCAN_EVT_NODE, this));
                return;
            }
            else
            {
                // L'info de date ou taille à changé
                m_eScanState = DMS_FILE_SCAN_STATE_MODIFIED;
                if (manager) manager->Add(new dmsEvent(DMS_EVT_FILE_SCAN_EVT_NODE, this));
            }
        }
    }

    // Si le fichier est un fichier standard, on sort

    if (m_oInfo.m_eType != DMS_FILE_TYPE_DIRECTORY) return;

    // Si le fichier est un répertoire, on explore son contenu

    wxDir dir(m_oInfo.m_oFilename);

    if (! dir.IsOpened())
    {
        LOGE(L"Error opening directory %s", (const char *)m_oInfo.m_oFilename);
        return;
    }

    // Exploration du répertoire sur le disque

    wxString          filename;
    bool              cont = dir.GetFirst(&filename);
    dmsFileList::Node *node1, *node2;
    size_t            index;
    dmsSleepCpt       sleep(1, 1);

    node1 = m_oChildList.GetFirst();
    index = 0;

    while (cont)
    {
        // Version optimisée. Assure que l'ordre dans le ChildList
        // soit le même que dans le wxDir

        filename = m_oInfo.m_oFilename + "/" + filename;

        if (node1==NULL || node1->GetData()->m_oInfo.m_oFilename != filename)
        {
            node2 = FindNode(filename);
            if (node1 && node2)
            {
                // Le noeud existe mais il n'est pas au bon endroit
                dmsFileNode *tmp = node1->GetData();
                node1->SetData(node2->GetData());
                node2->SetData(tmp);
            }
            else
            {
                // Si le fichier est nouveau, on l'ajoute et on le notifie
                m_bScanAllowed = false;
                node = new dmsFileNode(this, filename, true);
                m_oChildList.Insert(index, node);
                if (manager) manager->Add(new dmsEvent(DMS_EVT_FILE_SCAN_EVT_NODE, node));
            }
        }
        cont = dir.GetNext(&filename);
        if (node1) node1 = node1->GetNext();
        index++;
        sleep++;
    }

    // Récursivité

    FOREACH(dmsFileList, m_oChildList, node)
    {
        // On regarde si une notification de purge à été traitée
        if (node->m_bScanAllowed && node->m_eScanState == DMS_FILE_SCAN_STATE_DELETED)
        {
            // Dans ce cas on gère la purge
            TODO();// Bug : vérifier que l'aborescence n'a pas de résidu dans la liste
            // des messages
            m_oChildList.DeleteObject(node);
        }
        else
        {
            // Sinon, on regarde en profondeur
            node->Scan(manager);
        }
        sleep++;
    }
}





void dmsFileNode::Reduce()
{
    m_oChildList.Clear();
}


void dmsFileNode::Append(dmsFileNode *node)
{
    m_oChildList.Append(node);
}




dmsFileNode *dmsFileNode::GetNextRec()
{
    if (m_oChildList.GetCount())
        return m_oChildList[0];

    dmsFileNode *parent = m_poParent;
    dmsFileNode *node   = this;

    while (parent)
    {
        dmsFileList::Node *tmp = parent->m_oChildList.Find(node);

        if (tmp->GetNext())
            return tmp->GetNext()->GetData();

        node   = parent;
        parent = node->m_poParent;
    }

    return NULL;
}




dmsFileNode *dmsFileNode::AssumeNode(const wxString &filename)
{
    wxString subdir;
    dmsFileNode *subnode;

    if (! filename.StartsWith(m_oInfo.m_oFilename)) return NULL;

    if (filename.Len() == m_oInfo.m_oFilename.Len()) return this;

    int index = m_oInfo.m_oFilename.Len()+1;

    while (filename[index] != 0 && filename[index] != '/') index++;

    subdir = filename.Mid(0, index);

    subnode = Find(subdir);

    if (subnode == NULL)
    {
        subnode = new dmsFileNode(this, subdir);

        Append(subnode);
    }

    return subnode->AssumeNode(filename);
}


dmsFileNode *dmsFileNode::Find(const wxString &filename)
{
    dmsFileNode *file;

    FOREACH(dmsFileList, m_oChildList, file)
    {
        if (file->m_oInfo.m_oFilename == filename) return file;
    }

    return NULL;
}

dmsFileList::Node *dmsFileNode::FindNode(const wxString &filename)
{
    dmsFileNode *file;

    FOREACH(dmsFileList, m_oChildList, file)
    {
        if (file->m_oInfo.m_oFilename == filename) return _node;
    }

    return NULL;
}




void dmsFileNode::SetMark(int mark)
{
    m_iMark = mark;
}

void dmsFileNode::SetMarkRec(int mark)
{
    dmsFileNode *child;

    SetMark(mark);

    FOREACH(dmsFileList, m_oChildList, child)
    {
        child->SetMarkRec(mark);
    }
}

int dmsFileNode::GetMark()
{
    return m_iMark;
}


void dmsFileNode::RemoveFile()
{
    if (m_oInfo.m_eType == DMS_FILE_TYPE_STANDARD)
        wxRemoveFile(m_oInfo.m_oFilename);
    else if (m_oInfo.m_eType == DMS_FILE_TYPE_DIRECTORY)
        wxRmdir(m_oInfo.m_oFilename);
}



void dmsFileNode::RemoveFileWithMarkRec(int mark)
{
    dmsFileNode *child;

    FOREACH(dmsFileList, m_oChildList, child)
    {
        if (child->GetMark() == mark)
        {
            child->RemoveFileWithMarkRec(mark);
            child->RemoveFile();
            m_oChildList.DeleteObject(child);
        }
    }
}




void dmsFileNode::SetSubChildInfo()
{
    dmsFileNode *child;

    m_iSubChildCount  = 1;
    m_iSubChildVolume = m_oInfo.m_iSize;

    FOREACH(dmsFileList, m_oChildList, child)
    {
        child->SetSubChildInfo();
        m_iSubChildCount  += child->m_iSubChildCount;
        m_iSubChildVolume += child->m_iSubChildVolume;
    }
}


/* ========================================================================
   Persistence
   ======================================================================== */

enum {
    TLV_FILE_ID,
    TLV_FILE_NAME,
    TLV_FILE_SIZE,
    TLV_FILE_TYPE,
    TLV_FILE_DATE,
    TLV_FILE_CHILDREN,
};

void dmsFileNode::Load(dmsTlv &tlv)
{
    tlv.BeginGroup(0);
    {
        tlv.Read(TLV_FILE_NAME, m_oInfo.m_oFilename);
        tlv.Read(TLV_FILE_SIZE, m_oInfo.m_iSize);
        tlv.Read(TLV_FILE_TYPE, (int&) m_oInfo.m_eType);
        tlv.Read(TLV_FILE_DATE, m_oInfo.m_oDate);
        tlv.BeginGroup(TLV_FILE_CHILDREN);
        {
            dmsFileNode *node;
            while (! tlv.EndOfGroup())
            {
                node = new dmsFileNode(this);
                node->Load(tlv);
                Append(node);
            }
        }
    }
    tlv.EndGroup();
}

void dmsFileNode::Save(dmsTlv &tlv)
{
    tlv.BeginGroup(0);
    {
        tlv.Write(TLV_FILE_NAME, m_oInfo.m_oFilename);
        tlv.Write(TLV_FILE_SIZE, m_oInfo.m_iSize);
        tlv.Write(TLV_FILE_TYPE, m_oInfo.m_eType);
        tlv.Write(TLV_FILE_DATE, m_oInfo.m_oDate);
        tlv.BeginGroup(TLV_FILE_CHILDREN);
        {
            dmsFileNode *child;
            FOREACH(dmsFileList, m_oChildList, child)
            {
                child->Save(tlv);
            }
        }
        tlv.EndGroup();
    }
    tlv.EndGroup();
}

void dmsFileNode::Load(const wxString &filename)
{
    dmsTlv tlv(filename, dmsTlv::read);

    Load(tlv);
}

void dmsFileNode::Save(const wxString &filename)
{
    dmsTlv tlv(filename, dmsTlv::write);

    Save(tlv);
}

/* ========================================================================
   Debug
   ======================================================================== */

wxString dmsFileNode::StrTrace() const
{
    return STR("%s %s %s %s",
        m_poParent ? m_oInfo.ShortName() : m_oInfo.m_oFilename,
        m_oInfo.IsoDate(),
        m_oInfo.SizeWithUnit(),
        m_oInfo.TypeAbbrev());
}

void dmsFileNode::Trace(const wxString &prefix) const
{
    dmsFileNode *child;

    LOG0(L"%s- %s", (const char *)prefix, (const char *)StrTrace());

    FOREACH(dmsFileList, m_oChildList, child)
    {
        child->Trace(prefix + "  ");
    }
}



/* ########################################################################
   Copie / Move de fichiers sur un thread
   ######################################################################## */
#if 0
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsFileCopyList);


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsFileCopyThread::dmsFileCopyThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_oList.DeleteContents(true);
    m_iCurrentOffset = 0;
    m_iCurrentSize   = 1;
}


dmsFileCopyThread::~dmsFileCopyThread()
{

}


/* ========================================================================

   ======================================================================== */

void dmsFileCopyThread::Add(const wxString &from, const wxString &to, dmsFileCopyMode mode)
{
    dmsFileInfo fromInfo(from, true);
    dmsFileInfo toInfo(to, true);

    //if (fromInfo.m_iSize == toInfo.m_iSize) return;

    m_oCS.Enter();
    Append(new dmsFileCopyElement(from, to, mode));
    m_oCS.Leave();
}

bool dmsFileCopyThread::GetFirst(wxString &from, wxString &to, dmsFileCopyMode &mode)
{
    dmsFileCopyList::Node *node;
    bool result;

    m_oCS.Enter();
    node = m_oList.GetFirst();
    if (node)
    {
        node->GetData()->Get(from, to, mode);
        result = true;
    }
    else
    {
        result = false;
    }
    m_oCS.Leave();

    return result;
}


void dmsFileCopyThread::Shift()
{
    dmsFileCopyList::Node *node;
    m_oCS.Enter();
    node = m_oList.GetFirst();
    if (node) m_oList.DeleteNode(node);
    m_oCS.Leave();
}



bool dmsFileCopyThread::IsActive()
{
    bool result;

    m_oCS.Enter();
    result = (m_oList.GetCount() > 0);
    m_oCS.Leave();

    return result;
}
/* ========================================================================

   ======================================================================== */

bool dmsFileCopyThread::CopyFile(const wxString &from, const wxString &to)
{
    dmsFile src;
    dmsFile dst;

    OnCopyBegin(from, to);
    m_oChrono.Start(0);

    if (TestDestroy())
    {
        //LOG4("Copy file '%s' canceled", wxFileNameFromPath(from));
        m_oChrono.Pause();
        OnCopyError(from, to);
        return false;
    }

    // Ouverture de la source
    if (! src.Open(from, dmsFile::read))
    {
        LOG0("Open file failed on '%s'", from);
        m_oChrono.Pause();
        OnCopyError(from, to);
        return false;
    }

    // Ouverture de la destination
    if (! dst.Open(to, dmsFile::write))
    {
        LOG0("Open file failed on '%s'", from);
        src.Close();
        m_oChrono.Pause();
        OnCopyError(from, to);
        return false;
    }

    // Copie
    int nbRead, nbWrite;
    m_iCurrentOffset = 0;
    m_iCurrentSize   = src.Length();

    while (true)
    {
        nbRead = src.Read(m_zBuffer, DMS_FILE_COPY_BUFFER_SIZE);

        if (nbRead == 0) break;
        if (TestDestroy()) break;

        nbWrite = dst.Write(m_zBuffer, nbRead);

        if (nbWrite != nbRead)
        {
            LOG0("Write failed on '%s'", to);
            break;
        }

        m_iCurrentOffset += nbRead;

        Sleep(1); // Economie CPU
    }

    // Fin
    src.Close();
    dst.Close();

    if (nbRead == 0)
    {
        // Plus rien a lire
        //LOG4("Copy file '%s' done", wxFileNameFromPath(from));
        m_oChrono.Pause();
        OnCopyEnd(from, to);
        return true;
    }
    else
    {
        // On a été coupé dans le traitement
        //LOG4("Copy file '%s' canceled", wxFileNameFromPath(from));
        m_oChrono.Pause();
        OnCopyError(from, to);
        return false;
    }
}

/* ========================================================================

   ======================================================================== */

wxThread::ExitCode dmsFileCopyThread::Entry()
{
    wxString from, to;
    dmsFileCopyMode mode;
    bool allDone = true;

    while (!TestDestroy())
    {
        if (GetFirst(from, to, mode))
        {
            allDone = false;
            switch (mode)
            {
            case DMS_FILE_COPY_MODE_STD:
                {
                    CopyFile(from, to);
                    break;
                }
            default:
                {
                    LOGE("Not implemented");
                    break;
                }
            }
            Shift();
        }
        else
        {
            if (! allDone)
            {
                allDone = true;
                OnAllDone();
            }
            Sleep(1000);
        }
    }

    return NULL;
}


int dmsFileCopyThread::CurrentProgress()
{
   return m_iCurrentOffset * 100 / m_iCurrentSize;
}


wxString dmsFileCopyThread::CurrentFileProgress()
{
    wxString result;
    dmsFileCopyList::Node *node;

    m_oCS.Enter();

    node = m_oList.GetFirst();

    if (node)
    {
        result.Printf("%s (%d%%)", wxFileNameFromPath(node->GetData()->m_oFrom), CurrentProgress());
    }
    else
    {
        result = "";
    }
    m_oCS.Leave();

    return result;
}

#endif



/* ########################################################################

   ######################################################################## */


#ifdef __VISUALC__
    #define   wxTelli64     _telli64
    #define   wxLseeki64    _lseeki64
    #define   wxRead        _read
    #define   wxWrite        _write
#else
    #define     wxTelli64(fd)   lseek64(fd,0,SEEK_CUR)
    #define     wxLseeki64      lseek64
    #define     wxEof           eof
#endif


// standard
#if defined(__WXMSW__) && !defined(__GNUWIN32__) && !defined(__WXWINE__)
  #include  <io.h>

#ifndef __SALFORDC__
    #define   WIN32_LEAN_AND_MEAN
    #define   NOSERVICE
    #define   NOIME
    #define   NOATOM
    #define   NOGDI
    #define   NOGDICAPMASKS
    #define   NOMETAFILE
    #define   NOMINMAX
    #define   NOMSG
    #define   NOOPENFILE
    #define   NORASTEROPS
    #define   NOSCROLL
    #define   NOSOUND
    #define   NOSYSMETRICS
    #define   NOTEXTMETRIC
    #define   NOWH
    #define   NOCOMM
    #define   NOKANJI
    #define   NOCRYPT
    #define   NOMCX
#endif

    #include  <windows.h>     // for GetTempFileName
#elif (defined(__UNIX__) || defined(__GNUWIN32__))
    #include  <unistd.h>
    #ifdef __GNUWIN32__
        #include <windows.h>
    #endif
#elif (defined(__WXPM__))
    #include <io.h>
    #define   W_OK        2
    #define   R_OK        4
#elif (defined(__WXSTUBS__))
    // Have to ifdef this for different environments
    #include <io.h>
#elif (defined(__WXMAC__))
    int access( const char *path, int mode ) { return 0 ; }
    char* mktemp( char * path ) { return path ;}
    #include  <unistd.h>
    #include  <unix.h>
    #define   W_OK        2
    #define   R_OK        4
#else
    #error  "Please specify the header with file functions declarations."
#endif  //Win/UNIX

#include  <stdio.h>       // SEEK_xxx constants
#include  <fcntl.h>       // O_RDONLY &c

#ifndef __MWERKS__
    #include  <sys/types.h>   // needed for stat
    #include  <sys/stat.h>    // stat
#endif

#if defined(__BORLANDC__) || defined(_MSC_VER)
    #define   W_OK        2
    #define   R_OK        4
#endif

// there is no distinction between text and binary files under Unix, so define
// O_BINARY as 0 if the system headers don't do it already
#if defined(__UNIX__) && !defined(O_BINARY)
    #define   O_BINARY    (0)
#endif  //__UNIX__

#ifdef __SALFORDC__
    #include <unix.h>
#endif

#ifndef MAX_PATH
    #define MAX_PATH 512
#endif

// some broken compilers don't have 3rd argument in open() and creat()
#ifdef __SALFORDC__
    #define ACCESS(access)
    #define stat    _stat
#else // normal compiler
    #define ACCESS(access)  , (access)
#endif // Salford C



// ============================================================================
// implementation of dmsFile
// ============================================================================

// ----------------------------------------------------------------------------
// static functions
// ----------------------------------------------------------------------------
bool dmsFile::Exists(const wxChar *name)
{
    wxStructStat st;
#if wxUSE_UNICODE && wxMBFILES
    wxCharBuffer fname = wxConvFile.cWC2MB(name);

#ifdef __WXMAC__
  return !access(wxUnix2MacFilename( name ) , 0) && !stat(wxUnix2MacFilename( name ), &st) && (st.st_mode & S_IFREG);
#else
    return !wxAccess(fname, 0) &&
           !wxStat(wxMBSTRINGCAST fname, &st) &&
           (st.st_mode & S_IFREG);
#endif
#else
#ifdef __WXMAC__
  return !access(wxUnix2MacFilename( name ) , 0) && !stat(wxUnix2MacFilename( name ), &st) && (st.st_mode & S_IFREG);
#else
    return !wxAccess(name, 0) &&
           !wxStat(name, &st) &&
           (st.st_mode & S_IFREG);
#endif
#endif
}



bool dmsFile::Access(const wxChar *name, OpenMode mode)
{
    int how = 0;

    switch ( mode ) {
        case read:
            how = R_OK;
            break;

        case write:
            how = W_OK;
            break;

        default:
            wxFAIL_MSG(wxT("bad dmsFile::Access mode parameter."));
    }

    return wxAccess(wxFNCONV(name), how) == 0;
}

// ----------------------------------------------------------------------------
// opening/closing
// ----------------------------------------------------------------------------

// ctors
dmsFile::dmsFile(const wxChar *szFileName, OpenMode mode)
{
    m_fd = fd_invalid;
    m_error = FALSE;

    Open(szFileName, mode);
}

// create the file, fail if it already exists and bOverwrite
bool dmsFile::Create(const wxChar *szFileName, bool bOverwrite, int accessMode)
{
    // if bOverwrite we create a new file or truncate the existing one,
    // otherwise we only create the new file and fail if it already exists
#ifdef __WXMAC__
    int fd = open(wxUnix2MacFilename( szFileName ), O_CREAT | (bOverwrite ? O_TRUNC : O_EXCL), access);
#else
    int fd = wxOpen(wxFNCONV(szFileName),
                    O_BINARY | O_WRONLY | O_CREAT |
                    (bOverwrite ? O_TRUNC : O_EXCL)
                    ACCESS(accessMode));
#endif
    if ( fd == -1 ) {
        wxLogSysError(_("can't create file '%s'"), szFileName);
        return FALSE;
    }
    else {
        Attach(fd);
        return TRUE;
    }
}

// open the file
bool dmsFile::Open(const wxChar *szFileName, OpenMode mode, int accessMode)
{
    int flags = O_BINARY;

    Close();

    switch (mode)
    {
    case read:
        flags |= O_RDONLY;
        break;

    case write_append:
        if ( dmsFile::Exists(szFileName) )
        {
            flags |= O_WRONLY | O_APPEND;
            break;
        }
        //else: fall through as write_append is the same as write if the
        //      file doesn't exist

    case write:
        flags |= O_WRONLY | O_CREAT | O_TRUNC;
        break;

    case read_write:
        flags |= O_RDWR;
        break;
    }

#ifdef __WXMAC__
    int fd = open(wxUnix2MacFilename( szFileName ), flags, access);
#elif defined(__VISUALC__)
    HANDLE hFile;
    int fd = -1;

    switch (mode)
    {
    case read:
        {
            hFile = CreateFile(wxFNCONV(szFileName),    // file to open
                GENERIC_READ,          // open for reading
                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,       // share for reading
                NULL,                  // default security
                OPEN_EXISTING,         // existing file only
                FILE_ATTRIBUTE_NORMAL, // normal file
                NULL);                 // no attr. template
            if (hFile != INVALID_HANDLE_VALUE) fd=_open_osfhandle((long) hFile, 0);
            break;
        }
    case write:
        {
            hFile = CreateFile(wxFNCONV(szFileName),     // file to create
                GENERIC_WRITE,          // open for writing
                0,                      // do not share
                NULL,                   // default security
                CREATE_ALWAYS,          // overwrite existing
                FILE_ATTRIBUTE_NORMAL | // normal file
                FILE_FLAG_OVERLAPPED,   // asynchronous I/O
                NULL);                  // no attr. template
            if (hFile != INVALID_HANDLE_VALUE) fd=_open_osfhandle((long) hFile, 0);
            break;
        }
    default:
        {
            fd = wxOpen(wxFNCONV(szFileName), flags ACCESS(accessMode));
            break;
        }
    }
#else
    int fd = wxOpen(wxFNCONV(szFileName), flags ACCESS(accessMode));
#endif
    if ( fd == -1 )
    {
        LOGE(L"Can't open file '%s'", (const char *)szFileName);
        return FALSE;
    }
    else {
        Attach(fd);
        return TRUE;
    }
}

// close
bool dmsFile::Close()
{
    if ( IsOpened() ) {
        if ( wxClose(m_fd) == -1 ) {
            wxLogSysError(_("can't close file descriptor %d"), m_fd);
            m_fd = fd_invalid;
            return FALSE;
        }
        else
            m_fd = fd_invalid;
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
// read/write
// ----------------------------------------------------------------------------

// read
off_t dmsFile::Read(void *pBuf, off_t nCount)
{
    wxCHECK( (pBuf != NULL) && IsOpened(), 0 );

    // note: we have to use scope resolution operator because there is also an
    // enum value "read"
#ifdef __MWERKS__
    int iRc = ::read(m_fd, (char *) pBuf, nCount);
#else
    int iRc = wxRead(m_fd, pBuf, nCount);
#endif
    if ( iRc == -1 ) {
        wxLogSysError(_("can't read from file descriptor %d"), m_fd);
        return (wxLongLong_t)-1;
    }
    else
        return (size_t)iRc;
}

// write
size_t dmsFile::Write(const void *pBuf, size_t nCount)
{
    wxCHECK( (pBuf != NULL) && IsOpened(), 0 );

    // have to use scope resolution for the same reason as above
#ifdef __MWERKS__
    int iRc = ::write(m_fd, (const char *) pBuf, nCount);
#else
    int iRc = wxWrite(m_fd, pBuf, nCount);
#endif
    if ( iRc == -1 ) {
        wxLogSysError(_("can't write to file descriptor %d"), m_fd);
        m_error = TRUE;
        return 0;
    }
    else
        return iRc;
}

// flush
bool dmsFile::Flush()
{
    if ( IsOpened() ) {
#if defined(__VISUALC__) || wxHAVE_FSYNC
        if ( wxFsync(m_fd) == -1 )
        {
            wxLogSysError(_("can't flush file descriptor %d"), m_fd);
            return FALSE;
        }
#else // no fsync
        // just do nothing
#endif // fsync
    }

    return TRUE;
}
// ----------------------------------------------------------------------------
// seek
// ----------------------------------------------------------------------------

// seek
wxLongLong_t dmsFile::Seek(wxLongLong_t ofs, wxSeekMode mode)
{
    wxASSERT( IsOpened() );

    int origin;
    switch ( mode ) {
        default:
            wxFAIL_MSG(_("unknown seek origin"));
            break;
        case wxFromStart:
            origin = SEEK_SET;
            break;

        case wxFromCurrent:
            origin = SEEK_CUR;
            break;

        case wxFromEnd:
            origin = SEEK_END;
            break;
    }

    wxLongLong_t iRc = wxLseeki64(m_fd, ofs, origin);
    if ( iRc == -1 ) {
        wxLogSysError(_("can't seek on file descriptor %d"), m_fd);
        return (wxLongLong_t)-1;
    }
    else
        return iRc;
}

// get current off_t
wxLongLong_t dmsFile::Tell() const
{
    wxASSERT( IsOpened() );

    wxLongLong_t iRc = wxTelli64(m_fd);
    if ( iRc == -1 ) {
        wxLogSysError(_("can't get seek position on file descriptor %d"), m_fd);
        return (wxLongLong_t)-1;
    }
    else
        return iRc;
}

// get current file length
wxLongLong_t dmsFile::Length() const
{
    wxASSERT( IsOpened() );

#ifdef __VISUALC__
    wxLongLong_t iRc = (wxLongLong_t)_filelengthi64(m_fd);
#else // !VC++
    wxLongLong_t iRc = wxTelli64(m_fd);
    if ( iRc != -1 ) {
        wxLongLong_t iLen = ((dmsFile *)this)->SeekEnd(); // const_cast
        if ( iLen != -1 ) {
            // restore old position
            if ( ((dmsFile *)this)->Seek(iRc) == -1 ) {
                // error
                iLen = -1;
            }
        }

        iRc = iLen;
    }
#endif  // VC++

    if ( iRc == -1 ) {
        wxLogSysError(_("can't find length of file on file descriptor %d"), m_fd);
        return (wxLongLong_t)-1;
    }
    else
        return iRc;
}

// is end of file reached?
bool dmsFile::Eof() const
{
    wxASSERT( IsOpened() );

    int iRc;

#if defined(__UNIX__) || defined(__GNUWIN32__) || defined( __MWERKS__ ) || defined(__SALFORDC__)
    // FIXME this doesn't work, of course, on unseekable file descriptors
    wxLongLong_t ofsCur = Tell(), ofsMax = Length();
    if ( ofsCur == (wxLongLong_t)-1 || ofsMax == (wxLongLong_t)-1 )
        iRc = -1;
    else
        iRc = ofsCur == ofsMax;
#else  // Windows and "native" compiler
    iRc = wxEof(m_fd);
#endif // Windows/Unix

    switch ( iRc ) {
        case 1:
            break;

        case 0:
            return FALSE;

        case -1:
            wxLogSysError(_("can't determine if the end of file is reached on descriptor %d"), m_fd);
                break;

        default:
            wxFAIL_MSG(_("invalid eof() return value."));
    }

    return TRUE;
}






bool dmsScanDirectory(const wxString &dirname, const wxString &filter, wxStringList &list)
{
    wxDir dir(dirname);
    wxString filename;

    LOG_AF(wxDirExists(dirname), LOGE(L"No directory [%s]", (const char *)dirname));
    LOG_AF(dir.IsOpened(), LOGE(L"Error opening directory [%s]", (const char *)dirname));

    for (bool cont = dir.GetFirst(&filename, filter); cont; cont = dir.GetNext(&filename))
    {
        list.Add(filename);
    }

    return true;
}


bool dmsLoadFile(const wxString &filename, unsigned char *&buffer, size_t &bufferLen)
{
    wxFile File;

    LOG_AF(File.Open(filename, wxFile::read),
        LOGE(L"Error opening file in read mode [%s]", (const char *)filename));

    bufferLen = File.Length();

    buffer = new unsigned char[bufferLen];

    LOG_AF(File.Read(buffer, bufferLen) == (off_t) bufferLen,
        LOGE(L"Error reading %d Bytes in %s", bufferLen, (const char *)filename));

    return true;
}


/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsTraceFile::dmsTraceFile(const wxString &filename)
{
    dmsAssumeDir(wxPathOnly(filename), true);

    m_poFile = fopen(filename, "w");

    if (m_poFile==NULL)
    {
        LOGE(L"Error opening file [%s] in write mode", (const char *)filename);
    }
}


dmsTraceFile::~dmsTraceFile()
{
    if (m_poFile) fclose(m_poFile);
}


/* ========================================================================

   ======================================================================== */

FILE* dmsTraceFile::GetFILE()
{
    return m_poFile;
}





/* ########################################################################

   ######################################################################## */

int SortByNumericExtension(const dmsFileNode **a, const dmsFileNode **b)
{
    wxString ext1, ext2;
    long     i, j;

    wxFileName::SplitPath((**a).m_oInfo.m_oFilename, NULL, NULL, &ext1);
    wxFileName::SplitPath((**b).m_oInfo.m_oFilename, NULL, NULL, &ext2);

    if (ext1.ToLong(&i) && ext2.ToLong(&j))
    {
        if (i>j) return 1;
        if (i<j) return -1;
        return 0;
    }

    return (**a).m_oInfo.m_oFilename.Cmp((**b).m_oInfo.m_oFilename);
}






/* ------------------------------------------------------------------------
   Ouvre l'application

   C'est du spécifique windows
   ------------------------------------------------------------------------ */


class OpenWithDefaultAppThread : public wxThread
{
public:
    wxString m_oFile;

    OpenWithDefaultAppThread(const wxString &oFileName)
        : wxThread()
    {
        m_oFile = oFileName;
    }

    void *Entry()
    {
#ifndef __WXGTK__
        wxString error;
        int res;
        /*SHELLEXECUTEINFO info;
        char verb[] = "open";

          info.cbSize = sizeof(SHELLEXECUTEINFO);
          info.fMask = 0;//SEE_MASK_FLAG_DDEWAIT;
          info.hwnd = NULL;
          info.lpVerb = verb;
          info.lpFile = oFileName;
          info.lpParameters = NULL;
          info.lpDirectory = NULL;
          info.nShow = SW_SHOWNORMAL;
          info.hInstApp = NULL;
          ShellExecuteEx(&info);
        res = (int) info.hInstApp;*/
        res = (int) ShellExecute(NULL, "open", m_oFile, NULL, NULL, SW_SHOWNORMAL);
        switch (res)
        {
        case 0: error = "The system is out of memory or resources"; break;
        case SE_ERR_NOASSOC: error = "There is no association for the specified file type"; break;

        case ERROR_FILE_NOT_FOUND: error = "The specified file was not found"; break;
        case ERROR_PATH_NOT_FOUND: error = "The specified path was not found"; break;
        case ERROR_BAD_FORMAT: error = "The .exe file is invalid (non-Win32 .exe or error in .exe image)"; break;

        default: error = ""; break;
        }

        if (res <= 32)
        {
            if (error == "")
                LOGE("[FILE UTI] Open file '%s' failed (code %d)", (const char *)m_oFile, res);
                else
                LOGE("[FILE UTI] Open file '%s' failed (%s)", (const char *)m_oFile, error);
        }

#endif
        return NULL;
    }
};


bool dmsFileOpenWithDefaultApp(const wxString &oFileName)
{
#ifndef __WXGTK__
    // Cof Mise sur Thread. Evite les bloquages DDE
    OpenWithDefaultAppThread *pt;

    pt = new OpenWithDefaultAppThread(oFileName);
    pt->Create();
    pt->Run();
#endif

    return true;
}
