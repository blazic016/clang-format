/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _DMS_DVB_LIB_OBJECT_CAROUSEL_MODULE_DATA_H_
#define _DMS_DVB_LIB_OBJECT_CAROUSEL_MODULE_DATA_H_


#include <DataCarousel/ModuleData.h>

class dmsObjectCarousel;
class dmsBiopDirMessage;

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsModuleDataBiopData : public dmsModuleData
{
public:
    dmsObjectCarousel *m_poCarousel;

public:
    dmsModuleDataBiopData(dmsModuleData* parent, dmsObjectCarousel* carousel);

    bool CompileBuffer();

    bool GetTree(dmsBiopDirMessage* dir, dmsModuleData* ref);
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsModuleDataBiopRef : public dmsModuleData
{
public:
    dmsBiopMessage*    m_poMessage;
    dmsObjectCarousel* m_poCarousel;

public:
    dmsModuleDataBiopRef(dmsModuleData* parent, EnumModuleDataType type, dmsBiopMessage *Message, dmsObjectCarousel* carousel);
    virtual ~dmsModuleDataBiopRef();

    bool CompileBuffer();
    bool Affect(dmsModule *module);
    virtual wxString TraceName();
    virtual void FileTrace();
};



#endif /* _DMS_DVB_LIB_OBJECT_CAROUSEL_MODULE_DATA_H_ */
