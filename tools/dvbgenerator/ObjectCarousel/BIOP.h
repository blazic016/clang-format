/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _DMS_DSMCC_HEADER_BIOP_H_
#define _DMS_DSMCC_HEADER_BIOP_H_

#include <MPEG/DESC.h>
#include <DataCarousel/DII.h>

#include "CORBA.h"


class dmsBiopModuleInfo;
class dmsBiopServiceGatewayInfo;
class dmsBiopDirMessage;
class dmsBiopFileMessage;
class dmsFileNode;
class dmsModule;
class dmsModuleData;

class dmsBiopMessage;
class dmsObjectCarousel;


WX_DECLARE_LIST(dmsBiopMessage, dmsBiopMessageList);


class dmsBIOP_Tap : public dmsData
{
public:
    u16      Id;
    u16      Use;
    u16      AssocTag;
    u_8      SelectorLength;
    dmsData* SelectorData;    // Not implemented

public:
    dmsBIOP_Tap();
};

class dmsBIOP_TapList : public dmsDataList
{
public:
    dmsBIOP_TapList():dmsDataList(){;}
    dmsData *Create(void *pt, const char *tag){return new dmsBIOP_Tap();}
};





class dmsBiopModuleInfo : public dmsData
{
public:
    u32                     ModuleTimeout;
    u32                     BlockTimeout;
    u32                     MinBlockTimeout;
    u_8                     TapsCount;
    dmsBIOP_TapList*        TapList;               // Must be >= 1
    u_8                     UserInfoLength;
    dmsMPEG_DescriptorList* UserInfoDescList;

public:
    dmsObjectCarousel* m_poOC;

    dmsBiopModuleInfo(dmsModule *Module);
};


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsBiopObjectLocation : public dmsData
{
public:
    u32      ComponentIdTag;
    u_8      ComponentDataLength;
    u32      CarouselId;
    u16      ModuleId;
    u_8      VersionMajor;
    u_8      VersionMinor;
    u_8      ObjectKeyLength;
    dmsData* ObjectKey;

public:
    dmsBiopObjectLocation();
};

class dmsDSM_Tap : public dmsData
{
public:
    u16 Id;
    u16 Use;
    u16 AssocTag;
    u_8 SelectorLength;
    u16 SelectorType;
    u32 TransactionId;
    u32 Timeout;

public:
    dmsDSM_Tap();
};


class dmsDSM_TapList : public dmsDataList
{
public:
    dmsDSM_TapList():dmsDataList(){;}
};

class dmsDSM_ConnBinder : public dmsData
{
public:
    u32             ComponentIdTag;
    u_8             ComponentDataLength;
    u_8             TapsCount;
    dmsDSM_TapList* TapList;

public:
    dmsDSM_ConnBinder();

    dmsDSM_Tap* m_poFirstTap;
};


class dmsBiopProfileBody : public dmsData
{
public:
    u32                    ProfileIdTag;
    u32                    ProfileDataLength;
    u_8                    ProfileDataByteOrder;
    u_8                    LiteComponentCount;
    dmsDataList*           LiteComponentList;

public:
    dmsBiopProfileBody(dmsBiopMessage *Message);

    dmsBiopObjectLocation* m_poObjectLocation;
    dmsDSM_ConnBinder*     m_poConnBinder;
    dmsBiopMessage*        m_poMessage;

    bool Update();
};


class dmsBiopServiceGatewayInfo : public dmsData
{
public:
    dmsCorbaIOR* IOR;
    u_8          DownloadTapsCount; // List not implemented
    u_8          ServiceContextList; // List not implemented
    u16          UserInfoLength;     // List not implemented
    dmsDataList* UserInfo;

public:
    dmsObjectCarousel*  m_poOC;
    dmsBiopProfileBody* m_poProfileBody;

    dmsBiopServiceGatewayInfo(dmsObjectCarousel *oc, dmsBiopMessage *Message);

    bool Update();
};



/* ------------------------------------------------------------------------
   0xE0 : ManufacturerInformation
   ------------------------------------------------------------------------ */


class dmsBiop_ManufacturerInformation : public dmsData
{
public:
    u8       ManufacturerInfoTag;
    u16      ManufacturerInfoLength;
    u24      ManufacturerId;
    u32      VersionId;
    dmsData* ManufacturerData;

public:
    dmsBiop_ManufacturerInformation();
};


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */




class dmsBiopBinding : public dmsData
{
public:
    u_8          NameComponentsCount;
    u_8          IdLength;
    dmsData*     IdData;
    u_8          KindLength;
    dmsData*     KindData;
    u_8          BindingType;
    dmsCorbaIOR* IOR;
    u16          ObjectInfoLength;
    dmsData*     DSM_FileContentSize;

public:
    dmsBiopMessage* m_poMessage;
public:
    dmsBiopProfileBody* m_poProfileBody;

    dmsBiopBinding(dmsBiopMessage* Message);

    void SetSingleBiopName(const char *Id);
    void SetFileContentSize(u32 Size);
};


class dmsBiopBindingList : public dmsDataList
{
public:
    dmsBiopBindingList():dmsDataList(){;}
};

class dmsBiopServiceContext : public dmsData
{
public:
    u16      ServiceID_DataBroadcastId;
    u16      ServiceID_ApplicationTypeCode;
    u16      ApplicationSpecificDataLength;
    dmsData* ApplicationSpecificData;

public:
    dmsBiopServiceContext();
};


class dmsBiopServiceContextList : public dmsDataList
{
public:
    dmsBiopServiceContextList():dmsDataList(){;}
};


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsBiopMessage : public dmsData
{
public:
    dmsData* Magic;             // 4 Bytes
    u_8      BicpVersionMajor;
    u_8      BicpVersionMinor;
    u_8      ByteOrder;
    u_8      MessageType;
    u32      MessageSize;
    u_8      ObjectKeyLength;
    dmsData* ObjectKeyData;
    u32      ObjectKindLength;
    dmsData* ObjectKindData;
    dmsData* Data;

public:
    dmsObjectCarousel* m_poOC;
    dmsModuleData* m_poModuleData;

    int      m_iKey;
    wxString m_oFilename;

    dmsBiopMessage(dmsObjectCarousel* oc);

    void SetModuleData(dmsModuleData *moduleData);
    void SetKey(int value);
    void SetKind(char *value);
};



class dmsBiopFileMessage : public dmsData
{
public:
    u16                        ObjectInfoLength;
    dmsData*                   DSM_FileContentSize;     // 8 Bytes
    dmsData*                   ObjectInfoData;          // Not implemented
    u_8                        ServiceContextListCount;
    dmsBiopServiceContextList* ServiceContextList;
    u32                        MessageBodyLength;
    u32                        ContentLength;
    dmsData*                   Content;

public:
    dmsBiopMessage* m_poHeader;
    dmsBiopBinding* m_poBinding;

public:
    dmsBiopFileMessage(dmsBiopMessage *Header);

    void SetContent(dmsBuffer *buffer=NULL);
};


class dmsBiopDirMessage : public dmsData
{
public:
    u16                        ObjectInfoLength;
    dmsData*                   ObjectInfoData;
    u_8                        ServiceContextListCount;
    dmsBiopServiceContextList* ServiceContextList;
    u32                        MessageBodyLength;
    u16                        BindingsCount;
    dmsBiopBindingList*        BindingsList;

public:
    dmsBiopMessage* m_poHeader;

public:
    dmsBiopDirMessage(dmsBiopMessage *Header);
    virtual ~dmsBiopDirMessage();
};


#endif /* _DMS_DSMCC_HEADER_BIOP_H_ */
