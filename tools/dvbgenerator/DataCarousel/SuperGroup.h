/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _LIB_DATA_CAROUSEL_SUPER_GROUP_H_
#define _LIB_DATA_CAROUSEL_SUPER_GROUP_H_

#include "Group.h"

class wxFile;
class wxXmlNode;
class dmsDataCarousel;
class dmsTransportStream;
class dmsDSI_Section;
class dmsDSI_GroupInfoIndication;
class dmsBiopServiceGatewayInfo;

class dmsSuperGroup
{
public:
    dmsDSI_Section*  m_poSection;
    dmsDataCarousel* m_poDataCarousel;

    dmsBiopServiceGatewayInfo*  m_poSGI;
    dmsDSI_GroupInfoIndication* m_poGII;

public:
    dmsGroupList m_oGroupList;
    wxString m_oOutputDir;
    wxString m_oId;

public:
    dmsSuperGroup(dmsDataCarousel* DataCarousel);
    virtual ~dmsSuperGroup();

    bool Compile();

    void Generate();

    bool Load(wxXmlNode *node);
};


#endif /* _LIB_DATA_CAROUSEL_SUPER_GROUP_H_ */
