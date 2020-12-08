/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DVB_HEADER_DSI_H_
#define _DVB_HEADER_DSI_H_

#include <Tools/Header.h>

#include "DESC.h"

class dmsDSMCC_Data;
class dmsDSI_Data;
class dmsMPEG_Section;
class dmsGroup;

class dmsDSI_Section
{
public:
    dmsMPEG_Section* MPEG;
    dmsDSMCC_Data*   DSMCC;
    dmsDSI_Data*     DSI;

public:
    dmsDSI_Section();
    virtual ~dmsDSI_Section();
};


class dmsDSI_Data : public dmsData
{
public:
    dmsData* ServerId;
    u16      CompatibilityDescriptorLength;
    u16      PrivateDataLength;
    dmsData* PrivateData;

public:
    dmsDSMCC_Data *m_poHeader;

    dmsDSI_Data(dmsDSMCC_Data *Header);
    bool Update();
};


class dmsDSI_GroupInfoIndicationItem : public dmsData
{
public:
    u32                             GroupId;
    u32                             GroupSize;
    dmsDSM_CompatibilityDescriptor* GroupCompatibility;
    u16                             GroupInfoLength;
    dmsDSMCC_DescriptorList*        GroupInfo;
    u16                             PrivateDataLength;
    dmsData*                        Private;

public:
    dmsDSI_GroupInfoIndicationItem();

    dmsGroup* m_poGroup;
    bool Update();
};



HDR_DEFINE_LIST(dmsDSI_GroupInfoIndicationItem);



class dmsDSI_GroupInfoIndication : public dmsData
{
public:
    u16                                 NumberOfGroups;
    dmsDSI_GroupInfoIndicationItemList* GroupInfoList;

public:
    dmsDSI_GroupInfoIndication();
};





#endif /* _DVB_HEADER_DSI_H_ */
