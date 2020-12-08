/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DVB_HEADER_DII_H_
#define _DVB_HEADER_DII_H_

#include <MPEG/MPEG.h>

class dmsDSMCC_Data;
class dmsDII_Data;
class dmsMPEG_Section;
class dmsModule;


class dmsDII_Section
{
public:
    dmsMPEG_Section* MPEG;
    dmsDSMCC_Data*   DSMCC;
    dmsDII_Data*     DII;

public:
    dmsDII_Section();
    virtual ~dmsDII_Section();
};



class dmsDII_Module : public dmsData
{
public:
    u16      ModuleId;
    u32      ModuleSize;
    u_8      ModuleVersion;
    u_8      ModuleInfoLength; // Pas plutot 16 ??
    dmsData* ModuleInfo;

public:
    dmsDII_Module();

    bool Update();

    dmsModule* m_poModule;
};


HDR_DEFINE_LIST(dmsDII_Module);


class dmsDII_Data : public dmsData
{
public:
    u32                DownloadId;
    u16                BlockSize;
    u_8                WindowSize;
    u_8                AckPeriod;
    u32                TC_DownloadWindow;
    u32                TC_DownloadScenario;
    u16                CompatibilityDescriptor;
    u16                NumberOfModules;
    dmsDII_ModuleList* ModuleList;
    u16                PrivateDataLength;
    dmsData*           PrivateData;

public:
    dmsDSMCC_Data *m_poHeader;

    dmsDII_Data(dmsDSMCC_Data *Header);
    bool Update();
};



#endif /* _DVB_HEADER_DII_H_ */
