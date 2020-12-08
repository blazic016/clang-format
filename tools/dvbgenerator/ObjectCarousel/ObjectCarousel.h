/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _DVB_LIB_OBJECT_CAROUSEL_H_
#define _DVB_LIB_OBJECT_CAROUSEL_H_

#include "BIOP.h"

class dmsFileNode;
class dmsBiopMessage;
class dmsModuleDataList;

class dmsObjectCarousel
{
public:
    int      m_iId;
    u24      m_iOUI;
    u32      m_iVersion;

    wxString m_oDirname;
    wxString m_oFilter;
    int      m_iAssocTag;

    dmsFileNode*       m_poRoot;
    dmsBiopMessageList m_oFileMessageList;
    dmsBiopMessageList m_oDirMessageList;
    dmsBiopMessage*    m_poSrgMessage;

    int GetNextId();

public:
    dmsObjectCarousel();
    virtual ~dmsObjectCarousel();

public:
    bool Load(wxXmlNode *node);
    bool LoadDir();

    dmsBiopMessage* Load(dmsFileNode *node);

    void SetSrg(dmsBiopMessage *root);

    dmsBiopDirMessage*  AddDirMess(const wxString &name, dmsBiopDirMessage* Dir);
    dmsBiopFileMessage* AddFileMess(const wxString &name, dmsBiopDirMessage* Dir);

    dmsBiopBinding*     AddBinding(dmsBiopMessage* Message, dmsBiopDirMessage* Dir);
};

#endif /* _DVB_LIB_OBJECT_CAROUSEL_H_ */
