/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/05 - Creation
   ************************************************************************ */

#include "Tools.h"
#include "Event.h"


#define DMS_FILE_COPY_BUFFER_SIZE (65536)



/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsFileManager : public dmsEventManager
{
public:
    char *m_poBuffer;
    int  m_iBufferSize;

    wxString m_oProgressName;
    int64    m_iProgressSize;
    int64    m_iProgressCount;
    int64    m_iProgressGlobalSize;
    int64    m_iProgressGlobalCount;

public:
    dmsFileManager();
    virtual ~dmsFileManager();

    void SetBuffer(int size = DMS_FILE_COPY_BUFFER_SIZE);

    bool Copy(const wxString &from, const wxString &to, bool force=false);
    bool Move(const wxString &from, const wxString &to);

    void Cancel();

    virtual void GetPrivateProgress(wxString &log);
};

