/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _LIB_DATA_CAROUSEL_GROUP_H_
#define _LIB_DATA_CAROUSEL_GROUP_H_

#include "Module.h"

#include <DataCarousel/DSI.h>

class wxXmlNode;

class dmsDataCarousel;
class dmsSuperGroup;


class dmsGroup
{
public:
    dmsDII_Section*  m_poSection;

public:

    dmsModuleList    m_oModuleList;
    dmsDataCarousel* m_poDataCarousel;
    dmsSuperGroup*   m_poSuperGroup;

    wxString m_oOutputDir;
    wxString m_oId;

public:
    dmsGroup(dmsSuperGroup* supergroup);
    virtual ~dmsGroup();

    bool Load(wxXmlNode *node);

    dmsModule* FindModule(int id);

    bool Compile();
    bool Generate();

    u32  GetCumulativeSize();

    void TraceModules();
};


WX_DECLARE_LIST(dmsGroup, dmsGroupList);


#endif /* _LIB_DATA_CAROUSEL_GROUP_H_ */
