/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _SSU_HEADERS_UNT_H_
#define _SSU_HEADERS_UNT_H_

#include <Tools/Header.h>

#include <MPEG/MPEG.h>

#include "DESC.h"


class wxXmlNode;
class dmsMPEG_Section;


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsSSU_UNT_DescriptorLoop : public dmsData
{
public:
    u4                         Reserved;
    u12                        DescriptorLoopLength;
    dmsSSU_UNT_DescriptorList* DescriptorList;

    dmsSSU_UNT_DescriptorLoop(const wxString &name);
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsSSU_UNT_PlateformInfo : public dmsData
{
public:
    dmsSSU_UNT_DescriptorLoop* TargetDescriptorLoop;
    dmsSSU_UNT_DescriptorLoop* OperationalDescriptorLoop;

    dmsSSU_UNT_PlateformInfo();
};

HDR_DEFINE_LIST(dmsSSU_UNT_PlateformInfo);


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsSSU_UNT_CompatibilityItem : public dmsData
{
public:
    u8                         DescriptorType;
    u8                         DescriptorLength;
    u8                         SpecifierType;
    u24                        SpecifierData;
    u16                        Model;
    u16                        Version;
    u8                         SubDescriptorCount;
    dmsSSU_UNT_DescriptorList* SubDescriptorList;

    dmsSSU_UNT_CompatibilityItem();
};

HDR_DEFINE_LIST(dmsSSU_UNT_CompatibilityItem);


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */


class dmsSSU_UNT_CompatibilityDescriptor : public dmsData
{
public:
    u16                               CompatibilityDescriptorLength;
    u16                               DescriptorCount;
    dmsSSU_UNT_CompatibilityItemList* DescriptorList;

    dmsSSU_UNT_CompatibilityDescriptor();
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsSSU_UNT_Item : public dmsData
{
public:
    dmsSSU_UNT_CompatibilityDescriptor* CompatibilityDescriptor;
    u16                                 PlateformLoopLength;
    dmsSSU_UNT_PlateformInfoList*       PlateformInfo;

    dmsSSU_UNT_Item();
};

HDR_DEFINE_LIST(dmsSSU_UNT_Item);

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsSSU_UNT_Data : public dmsData
{
public:
    // Décomposition du table id extension en 2 octets :
    u8  ActionType;
    u8  OUI_Hash;

    //
    u24                        OUI;
    u8                         ProcessingOrder;
    dmsSSU_UNT_DescriptorLoop* CommonDescriptorLoop;
    dmsSSU_UNT_ItemList*       Loop;

    int m_iPID;
    int m_iOutputFrequency;

    dmsMPEG_Section *m_poParent;

public:
   dmsSSU_UNT_Data(dmsMPEG_Section* section);
};



#endif /* _SSU_HEADERS_UNT_H_ */
