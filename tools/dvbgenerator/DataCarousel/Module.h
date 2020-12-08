/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _LIB_DATA_CAROUSEL_MODULE_H_
#define _LIB_DATA_CAROUSEL_MODULE_H_

class wxXmlNode;

#include <MPEG/DESC.h>

#include <DataCarousel/DII.h>
#include <DataCarousel/DDB.h>

#include "ModuleData.h"

class dmsDataCarousel;
class dmsSuperGroup;
class dmsGroup;

class dmsModule
{
public:
    dmsDII_Module*          DIIM;
    dmsMPEG_DescriptorList* m_poDescList;

public:
    int      m_iDDBOutputFrequency;
    int      m_iDDBDebugMissing;
    wxString m_oDataFile;
    int      m_iCompressionMethod;
    wxString m_oFirstFileName;

    wxString m_oOutputDir;
    wxString m_oId;

    dmsDataCarousel*  m_poDataCarousel;
    dmsSuperGroup*    m_poSuperGroup;
    dmsGroup*         m_poGroup;
    dmsModuleData*    m_poModuleData;

    wxXmlNode* m_poXmlDef;

    dmsDDB_SectionList m_oBlockList;

public:
    dmsModule(dmsGroup *group);
    virtual ~dmsModule();

    bool Load(wxXmlNode *node);

    void AddCompress();
    bool Compile();
    bool Generate();
    dmsModuleData *GetLastDataOfType(EnumModuleDataType type);

    void Affect(dmsModuleData* moduleData);
};


WX_DECLARE_LIST(dmsModule, dmsModuleList);

#endif /* _LIB_DATA_CAROUSEL_MODULE_H_ */
