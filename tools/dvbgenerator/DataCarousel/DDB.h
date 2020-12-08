/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 11/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DVB_HEADER_DDB_H_
#define _DVB_HEADER_DDB_H_

/* ------------------------------------------------------------------------
   Download Data Block (DDB) Message
   ------------------------------------------------------------------------ */

#include <Tools/Header.h>

class dmsDSMCC_Data;
class dmsDDB_Data;



class dmsModule;

class dmsDDB_Section
{
public:
    dmsMPEG_Section* MPEG;
    dmsDSMCC_Data*   DSMCC;
    dmsDDB_Data*     DDB;

public:
    dmsDDB_Section(dmsModule* module, int number, int lastBlockNumber);
    virtual ~dmsDDB_Section();

    wxString m_oOutputDir;
    wxString m_oId;

    bool Generate();
};


WX_DECLARE_LIST(dmsDDB_Section, dmsDDB_SectionList);



class dmsDDB_Data : public dmsData
{
public:
    u16  ModuleId;
    u_8  ModuleVersion;
    u_8  Reserved;
    u16  BlockNumber;
    dmsData* Data;              //  RECOMMENDED N=4066 Bytes for fast access

public:
    dmsModule* m_poModule;

    dmsDSMCC_Data *m_poHeader;

    dmsDDB_Data(dmsDSMCC_Data *Header);

    bool Update();
};


#endif /* _DVB_HEADER_DDB_H_ */
