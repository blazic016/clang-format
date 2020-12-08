/* ************************************************************************
   SmarDTV

   Description : Extensions au système de fenetrage de wxWindows

   Historique :
   - COF   - Iwedia  - v 0    - 05/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _TOOLS_FILE_H_
#define _TOOLS_FILE_H_


#include <wx/wx.h>
#include <wx/datetime.h>
#include <wx/thread.h>
#include <wx/timer.h>

#include "Tools.h"

/* ------------------------------------------------------------------------
   Déclarations
   ------------------------------------------------------------------------ */

class dmsEventManager;
class dmsTlv;

class dmsFileNode;
class dmsFileManager;

/* ------------------------------------------------------------------------
   Outils sur répertoires / fichiers
   ------------------------------------------------------------------------ */

bool dmsCheckDir(const wxString &dirname, const wxString &labelname);
bool dmsAssumeDir(const wxString &dir, bool log=false);
bool dmsSimilarFiles(const wxString &filename1, const wxString &filename2);
bool dmsSetFileModificationTime(const wxString &filename, time_t time);
void dmsStandardFilename(wxString &filename);
void dmsRemoveIfExists(wxString &filename);
bool dmsFileSize(wxString &filename, off_t &size);

/* ------------------------------------------------------------------------
   Class "FileInfo".

   Contient les infos des fichiers
   ------------------------------------------------------------------------ */

typedef enum {
    DMS_FILE_TYPE_UNKNOWN,
    DMS_FILE_TYPE_STANDARD,
    DMS_FILE_TYPE_DIRECTORY,
} dmsFileType;

class dmsFileInfo : public wxObject
{
public:
    wxString     m_oFilename;
    wxLongLong_t m_iSize;
    dmsFileType  m_eType;
    wxDateTime   m_oDate;

public:
    dmsFileInfo();
    dmsFileInfo(const wxString &filename, bool init=false);
    virtual ~dmsFileInfo();

    void Init();
    void Init(const wxString &filename);

    dmsFileInfo &Set(const dmsFileInfo &other);

    wxString ShortName() const {return wxFileNameFromPath(m_oFilename);}
    wxString IsoDate() const {return m_oDate.Format("%Y-%m-%d %H:%M:%S");}
    wxString SizeWithUnit() const;
    wxString TypeAbbrev() const;

    bool Equals(const dmsFileInfo &other) const;
    bool Similar(const dmsFileInfo &other) const;

    dmsFileInfo& operator=(const dmsFileInfo &other) {return Set(other);}

};

inline bool operator==(const dmsFileInfo &a, const dmsFileInfo &b)
{
    return a.Equals(b);
}

inline bool operator!=(const dmsFileInfo &a, const dmsFileInfo &b)
{
    return !a.Equals(b);
}


/* ------------------------------------------------------------------------
   Class "FileNode"

   Permet d'avoir une vue hiérarchique d'une arborscence disque.
   ------------------------------------------------------------------------ */

WX_DECLARE_LIST(dmsFileNode, dmsFileList);

typedef enum {
    DMS_FILE_SCAN_STATE_UNKNOWN,
    DMS_FILE_SCAN_STATE_NEW,
    DMS_FILE_SCAN_STATE_MODIFIED,
    DMS_FILE_SCAN_STATE_DELETED,
} dmsFileScanState;

enum {
    DMS_EVT_FILE_SCAN_UNKNOWN,
    DMS_EVT_FILE_SCAN_EVT_NODE,
    DMS_EVT_FILE_SCAN_EVT_END_OF_SCAN,
};

class dmsFileNode
{
public:
    dmsFileInfo      m_oInfo;
    dmsFileList      m_oChildList;
    dmsFileNode*     m_poParent;
    int              m_iMark;
    int64            m_iSubChildCount;
    int64            m_iSubChildVolume;

    dmsFileScanState m_eScanState;
    bool             m_bScanAllowed;

public:
    dmsFileNode(dmsFileNode *parent = NULL, const wxString &filename=wxEmptyString, bool init=false);
    virtual ~dmsFileNode();

    void SetFilename(const wxString &name);

    void Init();
    bool Expand(const wxString &filter = "*.*");
    void Scan(dmsEventManager *manager = NULL);
    void Reduce();

    void Append(dmsFileNode *node);

    dmsFileNode* GetNextRec();
    dmsFileNode* Find(const wxString &filename);
    dmsFileList::Node* FindNode(const wxString &filename);
    dmsFileNode* AssumeNode(const wxString &filename);

    void SetMark(int mark);
    void SetMarkRec(int mark);
    int  GetMark();

    void RemoveFileWithMarkRec(int mark);
    void RemoveFile();

    void SetSubChildInfo();

    wxString StrTrace() const;
    void Trace(const wxString &prefix = wxEmptyString) const;

    void Load(dmsTlv &tlv);
    void Save(dmsTlv &tlv);
    void Load(const wxString &filename);
    void Save(const wxString &filename);
};

/* ========================================================================
   Threads spécialisés sur du traitement fichier
   ======================================================================== */

/* ------------------------------------------------------------------------
   Copie / Move d'une liste de fichiers
   ------------------------------------------------------------------------ */

#if 0

typedef enum
{
    DMS_FILE_COPY_MODE_STD,
    DMS_FILE_COPY_MODE_MOVE,
    DMS_FILE_COPY_MODE_FTP,
} dmsFileCopyMode;


class dmsFileCopyThread;

// Copie d'un élément

class dmsFileCopyElement
{
    friend dmsFileCopyThread;

private:
    wxString        m_oFrom;
    wxString        m_oTo;
    dmsFileCopyMode m_eMode;

public:
    dmsFileCopyElement(const wxString &from, const wxString &to, dmsFileCopyMode mode)
    {
        m_oFrom = from; m_oTo = to; m_eMode = mode;
    }

    void Get(wxString &from, wxString &to, dmsFileCopyMode &mode)
    {
        from = m_oFrom; to = m_oTo; mode = m_eMode;
    }

    ~dmsFileCopyElement() {;}
};

WX_DECLARE_LIST(dmsFileCopyElement, dmsFileCopyList);

// Copie de la liste d'éléments

class dmsFileCopyThread : public wxThread
{
private:
    dmsFileCopyList   m_oList;
    wxCriticalSection m_oCS;
    char              m_zBuffer[DMS_FILE_COPY_BUFFER_SIZE];

    wxLongLong_t      m_iCurrentOffset;
    wxLongLong_t      m_iCurrentSize;

    wxLongLong_t      m_iGlobalOffset;
    wxLongLong_t      m_iGlobalSize;

    bool CopyFile(const wxString &from, const wxString &to);
    bool GetFirst(wxString &from, wxString &to, dmsFileCopyMode &mode);
    void Shift();

    // Evenements

    virtual void OnCopyBegin(const wxString &from, const wxString &to) {;}
    virtual void OnCopyEnd(const wxString &from, const wxString &to)   {;}
    virtual void OnCopyError(const wxString &from, const wxString &to) {;}
    virtual void OnAllDone()   {;}

public:
    wxStopWatch       m_oChrono;

public:
    dmsFileCopyThread();
    virtual ~dmsFileCopyThread();

    void Add(const wxString &from, const wxString &to, dmsFileCopyMode mode = DMS_FILE_COPY_MODE_STD);
    bool IsActive();

    int  CurrentProgress();
    wxString CurrentFileProgress();

    ExitCode Entry();
};
#endif


/* ========================================================================
   Classe "FILE 64 bits"

   Copie du code source de la classe "wxFile"
   ======================================================================== */

/* ------------------------------------------------------------------------
   Constantes
   ------------------------------------------------------------------------ */

// we redefine these constants here because S_IREAD &c are _not_ standard
// however, we do assume that the values correspond to the Unix umask bits
#define wxS_IRUSR 00400
#define wxS_IWUSR 00200
#define wxS_IXUSR 00100

#define wxS_IRGRP 00040
#define wxS_IWGRP 00020
#define wxS_IXGRP 00010

#define wxS_IROTH 00004
#define wxS_IWOTH 00002
#define wxS_IXOTH 00001

// default mode for the new files: corresponds to umask 022
#define wxS_DEFAULT   (wxS_IRUSR | wxS_IWUSR | wxS_IRGRP | wxS_IWGRP |\
                       wxS_IROTH | wxS_IWOTH)

/* ------------------------------------------------------------------------
   class wxFile: raw file IO

   NB: for space efficiency this class has no virtual functions, including
       dtor which is _not_ virtual, so it shouldn't be used as a base class.
   ------------------------------------------------------------------------ */

class  dmsFile
{
public:
  // more file constants
  // -------------------
    // opening mode
  enum OpenMode { read, write, read_write, write_append };
    // standard values for file descriptor
  enum { fd_invalid = -1, fd_stdin, fd_stdout, fd_stderr };

  // static functions
  // ----------------
    // check whether a regular file by this name exists
  static bool Exists(const wxChar *name);
    // check whetther we can access the given file in given mode
    // (only read and write make sense here)
  static bool Access(const wxChar *name, OpenMode mode);

  // ctors
  // -----
    // def ctor
  dmsFile() { m_fd = fd_invalid; }
    // open specified file (may fail, use IsOpened())
  dmsFile(const wxChar *szFileName, OpenMode mode = read);
    // attach to (already opened) file
  dmsFile(int fd) { m_fd = fd; }

  // open/close
    // create a new file (with the default value of bOverwrite, it will fail if
    // the file already exists, otherwise it will overwrite it and succeed)
  bool Create(const wxChar *szFileName, bool bOverwrite = FALSE,
              int access = wxS_DEFAULT);
  bool Open(const wxChar *szFileName, OpenMode mode = read,
            int access = wxS_DEFAULT);
  bool Close();  // Close is a NOP if not opened

  // assign an existing file descriptor and get it back from wxFile object
  void Attach(int fd) { Close(); m_fd = fd; }
  void Detach()       { m_fd = fd_invalid;  }
  int  fd() const { return m_fd; }

  // read/write (unbuffered)
    // returns number of bytes read or ofsInvalid on error
  off_t Read(void *pBuf, off_t nCount);
    // returns the number of bytes written
  size_t Write(const void *pBuf, size_t nCount);
    // returns true on success
  bool Write(const wxString& s, wxMBConv& conv = wxConvLibc)
  {
      const wxWX2MBbuf buf = s.mb_str(conv);
      size_t size = strlen(buf);
      return Write((const char *) buf, size) == size;
  }
    // flush data not yet written
  bool Flush();

  // file pointer operations (return ofsInvalid on failure)
    // move ptr ofs bytes related to start/current off_t/end of file
  wxLongLong_t Seek(wxLongLong_t ofs, wxSeekMode mode = wxFromStart);
    // move ptr to ofs bytes before the end
  wxLongLong_t SeekEnd(wxLongLong_t ofs = 0) { return Seek(ofs, wxFromEnd); }
    // get current off_t
  wxLongLong_t Tell() const;
    // get current file length
  wxLongLong_t Length() const;

  // simple accessors
    // is file opened?
  bool IsOpened() const { return m_fd != fd_invalid; }
    // is end of file reached?
  bool Eof() const;
    // has an error occured?
  bool Error() const { return m_error; }

  // dtor closes the file if opened
  virtual ~dmsFile() { Close(); }

private:
  // copy ctor and assignment operator are private because
  // it doesn't make sense to copy files this way:
  // attempt to do it will provoke a compile-time error.
  dmsFile(const dmsFile&);
  dmsFile& operator=(const dmsFile&);

  int m_fd; // file descriptor or INVALID_FD if not opened
  bool m_error; // error memory
};


bool dmsScanDirectory(const wxString &dirname, const wxString &filter, wxStringList &list);
bool dmsLoadFile(const wxString &filename, unsigned char *&buffer, size_t &bufferLen);


class dmsTraceFile
{
public:
    FILE    *m_poFile;
public:
    dmsTraceFile(const wxString &filename);

    virtual ~dmsTraceFile();

    FILE *GetFILE();
};


int SortByNumericExtension(const dmsFileNode **a, const dmsFileNode **b);


bool dmsFileOpenWithDefaultApp(const wxString &oFileName);



#endif /* _TOOLS_FILE_H_ */
