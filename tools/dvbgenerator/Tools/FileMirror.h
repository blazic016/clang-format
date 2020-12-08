/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/05 - Creation
   ************************************************************************ */


#include "File.h"
#include "Event.h"

class dmsFileManager;

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

enum
{
    DMS_EVENT_FILE_MIRROR_LIST_DONE = DMS_EVENT_MANAGER_USER,
    DMS_EVENT_FILE_MIRROR_PUSH_DONE,
    DMS_EVENT_FILE_MIRROR_DELETE_DONE,
    DMS_EVENT_FILE_MIRROR_UPDATE_DONE,
};

class dmsFileMirror
{
public:
    dmsFileNode    *m_poRootSrc;
    dmsFileNode    *m_poRootDst;
    dmsFileManager *m_poManager;
    bool            m_bManagerOwner;

public:
    dmsFileMirror();
    virtual ~dmsFileMirror();

    void SetRootSrc(const wxString &filename);
    void SetRootDst(const wxString &filename);
    void SetManager(dmsFileManager *manager);

    void CreateManager();

    void ListSrc();
    void ListDst();

    void List();
    void Push();
    void Delete();

    void Update();
};


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

enum
{
    DMS_EVENT_FILE_MIRROR_LIST = 0,
    DMS_EVENT_FILE_MIRROR_PUSH,
    DMS_EVENT_FILE_MIRROR_DELETE,
    DMS_EVENT_FILE_MIRROR_UPDATE,
};


class dmsFileMirrorEvent : public dmsEvent
{
public:
    dmsFileMirrorEvent(int id, void *data=NULL, void *subdata=NULL);
    virtual ~dmsFileMirrorEvent();

    virtual void Run();
};
