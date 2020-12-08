/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Tools/Header.h>
#include <Tools/File.h>
#include <Tools/Xml.h>

#include <DataCarousel/DataCarousel.h>
#include <DataCarousel/Module.h>
#include <DataCarousel/ModuleData.h>
#include <DataCarousel/SuperGroup.h>
#include <DataCarousel/DSMCC.h>
#include <DataCarousel/DESC.h>


#include "ObjectCarousel.h"



#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsBiopMessageList);

/* ########################################################################

   ######################################################################## */


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

dmsBIOP_Tap::dmsBIOP_Tap() : dmsData()
{
    HDR_INIT(Id,             16);
    HDR_INIT(Use,            16);
    HDR_INIT(AssocTag,       16);
    HDR_INIT(SelectorLength, 8);
    HDR_INIT(SelectorData,   0);

    Id  = 0x0000;
    Use = 0x0017;

    SetLoad(&AssocTag, false);
}


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */


dmsBiopModuleInfo::dmsBiopModuleInfo(dmsModule *Module) : dmsData()
{
    HDR_INIT(ModuleTimeout,    32);
    HDR_INIT(BlockTimeout,     32);
    HDR_INIT(MinBlockTimeout,  32);
    HDR_INIT(TapsCount,        8);
    HDR_INIT(TapList,          0);
    HDR_INIT(UserInfoLength,   8);
    HDR_INIT(UserInfoDescList, 0);

    TapList          = new dmsBIOP_TapList();
    UserInfoDescList = new dmsDSMCC_DescriptorList(Module);

    SetLenLimit(&UserInfoLength, &UserInfoDescList);
    SetListCount(&TapsCount,     TapList);

    SetLoad(&ModuleTimeout,   false);
    SetLoad(&BlockTimeout,    false);
    SetLoad(&MinBlockTimeout, false);

    SetLoad(&UserInfoDescList, "UserInfo");
    SetLoad(&TapList, true);

    m_poOC = Module->m_poDataCarousel->m_poOC;
}





/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsBiopServiceGatewayInfo::dmsBiopServiceGatewayInfo(dmsObjectCarousel *oc, dmsBiopMessage *Message)
{
    HDR_INIT(IOR,                0);
    HDR_INIT(DownloadTapsCount,  8);
    HDR_INIT(ServiceContextList, 8);
    HDR_INIT(UserInfoLength,     16);
    HDR_INIT(UserInfo,           0);

    IOR             = new dmsCorbaIOR();
    UserInfo        = new dmsMPEG_DescriptorList();
    m_poProfileBody = new dmsBiopProfileBody(Message);

    IOR->TaggedProfileList->Append(m_poProfileBody);

    m_poOC = oc;

    IOR->SetTypeId("srg");

    dmsBiop_ManufacturerInformation* info = new dmsBiop_ManufacturerInformation();

    info->ManufacturerId = oc->m_iOUI;
    info->VersionId = oc->m_iVersion;

    UserInfo->Append(info);

    SetLenLimit(&UserInfoLength, &UserInfo);
}



bool dmsBiopServiceGatewayInfo::Update()
{
    if (m_poOC->m_poSrgMessage==NULL)
        return false;

    if (m_poProfileBody->m_poMessage==NULL)
        m_poProfileBody->m_poMessage = m_poOC->m_poSrgMessage;

    return true;
}



/* ========================================================================
   0xE0 : ManufacturerInformation
   ======================================================================== */

dmsBiop_ManufacturerInformation::dmsBiop_ManufacturerInformation() : dmsData()
{
    HDR_INIT(ManufacturerInfoTag,    8);
    HDR_INIT(ManufacturerInfoLength, 16);
    HDR_INIT(ManufacturerId,         24);
    HDR_INIT(VersionId,              32);
    HDR_INIT(ManufacturerData,       0);

    ManufacturerInfoTag = 0xE0;

    SetLoad(&ManufacturerId, false);
    SetLoad(&VersionId,      false);

    SetLenLimit(&ManufacturerInfoLength, &ManufacturerData);
}


dmsBiopProfileBody::dmsBiopProfileBody(dmsBiopMessage *Message) : dmsData()
{
    HDR_INIT(ProfileIdTag,         32);
    HDR_INIT(ProfileDataLength,    32);
    HDR_INIT(ProfileDataByteOrder, 8);
    HDR_INIT(LiteComponentCount,   8);
    HDR_INIT(LiteComponentList,    0);

    SetName("BiopProfileBody");

    ProfileIdTag         = 0x49534F06;
    ProfileDataByteOrder = 0x00;

    m_poObjectLocation = new dmsBiopObjectLocation();
    m_poConnBinder     = new dmsDSM_ConnBinder();

    LiteComponentList = new dmsDataList();
    LiteComponentList->Append(m_poObjectLocation);
    LiteComponentList->Append(m_poConnBinder);

    m_poObjectLocation->ComponentIdTag = 0x49534F50;
    m_poObjectLocation->VersionMajor   = 0x01;
    m_poObjectLocation->VersionMinor   = 0x00;

    m_poConnBinder->m_poFirstTap = new dmsDSM_Tap();
    m_poConnBinder->m_poFirstTap->Id             = 0x0000;
    m_poConnBinder->m_poFirstTap->Use            = 0x0016;
    m_poConnBinder->m_poFirstTap->SelectorLength = 0x0A;
    m_poConnBinder->m_poFirstTap->SelectorType   = 0x0001;

    m_poConnBinder->TapList->Append(m_poConnBinder->m_poFirstTap);
    m_poConnBinder->ComponentIdTag = 0x49534F40;

    SetLenLimit(&ProfileDataLength, &LiteComponentList);
    SetListCount(&LiteComponentCount, LiteComponentList);

    m_poMessage = Message;
}



bool dmsBiopProfileBody::Update()
{
    if (m_poMessage==NULL || m_poMessage->m_poModuleData==NULL) return false;

    // Contenu

    m_poObjectLocation->ObjectKey->m_oBuffer.Set(m_poMessage->ObjectKeyData->m_oBuffer);

    if (m_poMessage->m_poModuleData->m_poModule==NULL) return false;

    // Identification

    m_poObjectLocation->CarouselId = m_poMessage->m_poOC->m_iId;
    m_poObjectLocation->ModuleId   = m_poMessage->m_poModuleData->m_poModule->DIIM->ModuleId;

    m_poConnBinder->m_poFirstTap->AssocTag      = m_poMessage->m_poOC->m_iAssocTag;
    m_poConnBinder->m_poFirstTap->TransactionId = m_poMessage->m_poModuleData->m_poModule->m_poGroup->m_poSection->DSMCC->TransactionId;

    return true;
}





dmsBiopObjectLocation::dmsBiopObjectLocation() : dmsData()
{
    HDR_INIT(ComponentIdTag,      32);
    HDR_INIT(ComponentDataLength, 8);
    HDR_INIT(CarouselId,          32);
    HDR_INIT(ModuleId,            16);
    HDR_INIT(VersionMajor,        8);
    HDR_INIT(VersionMinor,        8);
    HDR_INIT(ObjectKeyLength,     8);
    HDR_INIT(ObjectKey,           0);

    ObjectKey = new dmsData();

    SetLenLimit(&ComponentDataLength, &ObjectKey);
    SetLenLimit(&ObjectKeyLength, &ObjectKey);

    SetName("BIOP_ObjectLocation");
}


dmsDSM_Tap::dmsDSM_Tap() : dmsData()
{
    HDR_INIT(Id,             16);
    HDR_INIT(Use,            16);
    HDR_INIT(AssocTag,       16);
    HDR_INIT(SelectorLength, 8);
    HDR_INIT(SelectorType,   16);
    HDR_INIT(TransactionId,  32);
    HDR_INIT(Timeout,        32);

    SetLenLimit(&SelectorLength, &Timeout);

    SetName("DSM_Tap");
}

dmsDSM_ConnBinder::dmsDSM_ConnBinder() : dmsData()
{
    HDR_INIT(ComponentIdTag,      32);
    HDR_INIT(ComponentDataLength, 8);
    HDR_INIT(TapsCount,           8);
    HDR_INIT(TapList,             0);

    TapList = new dmsDSM_TapList();

    SetLenLimit(&ComponentDataLength, &TapList);
    SetListCount(&TapsCount, TapList);

    SetName("DSM_ConnBinder");
}


/* ========================================================================

   ======================================================================== */



dmsBiopMessage::dmsBiopMessage(dmsObjectCarousel* oc) : dmsData()
{
    HDR_INIT(Magic,            0);
    HDR_INIT(BicpVersionMajor, 8);
    HDR_INIT(BicpVersionMinor, 8);
    HDR_INIT(ByteOrder,        8);
    HDR_INIT(MessageType,      8);
    HDR_INIT(MessageSize,      32);
    HDR_INIT(ObjectKeyLength,  8);
    HDR_INIT(ObjectKeyData,    0);
    HDR_INIT(ObjectKindLength, 32);
    HDR_INIT(ObjectKindData,   0);
    HDR_INIT(Data,             0);

    m_poOC         = oc;
    m_poModuleData = NULL;

    Magic          = new dmsData();
    ObjectKeyData  = new dmsData();
    ObjectKindData = new dmsData();

    ObjectKeyData->m_oBuffer.Set(0, 4);

    BicpVersionMajor   = 0x01;

    Magic->m_oBuffer.Set("BIOP", false);

    SetLenLimit(&ObjectKeyLength, &ObjectKeyData);
    SetLenLimit(&ObjectKindLength, &ObjectKindData);
    SetLenLimit(&MessageSize, &Data);

    SetName("BIOP_Message");
    m_iKey  = 0;

}

void dmsBiopMessage::SetModuleData(dmsModuleData *moduleData)
{
    //if (GeneratedError()) return;

    m_poModuleData = moduleData;
    /*
    m_oTraceFilename = STR("biop-%s-%s-%s",
        moduleData->m_poModule->m_oId,
        ObjectKeyData->m_oBuffer.m_poBuffer,
        ObjectKindData->m_oBuffer.m_poBuffer);
    */
}


void dmsBiopMessage::SetKey(int value)
{
    if (value==-1) return;

    if (GeneratedError()) return;

    m_iKey = value;

    sprintf((char*) ObjectKeyData->m_oBuffer.m_poBuffer, "%d", value);
}

void dmsBiopMessage::SetKind(char *value)
{
    if (GeneratedError()) return;

    ObjectKindData->m_oBuffer.Set(value, true);
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsBiopFileMessage::dmsBiopFileMessage(dmsBiopMessage *Header) : dmsData()
{
    HDR_INIT(ObjectInfoLength,        16);
    HDR_INIT(DSM_FileContentSize,     0);
    HDR_INIT(ObjectInfoData,          0);
    HDR_INIT(ServiceContextListCount, 8);
    HDR_INIT(ServiceContextList,      0);
    HDR_INIT(MessageBodyLength,       32);
    HDR_INIT(ContentLength,           32);
    HDR_INIT(Content,                 0)

    DSM_FileContentSize = new dmsData();
    ServiceContextList  = new dmsBiopServiceContextList();
    Content             = new dmsData();

    DSM_FileContentSize->m_oBuffer.Set(0, 8);

    SetLenLimit(&ObjectInfoLength, &DSM_FileContentSize);
    SetListCount(&ServiceContextListCount, ServiceContextList);
    SetLenLimit(&MessageBodyLength, &Content);
    SetLenLimit(&ContentLength, &Content);

    Header->SetName("BIOP_FileMessage");
    Header->SetKind((char *)"fil");

    SetName("BIOP_FileData");


    m_poHeader = Header;

    Header->Data = this;

    m_poBinding = NULL;
}




void dmsBiopFileMessage::SetContent(dmsBuffer *buffer)
{
    if (GeneratedError()) return;

    if (buffer)
        Content->m_oBuffer.Set(*buffer);
    else
        Content->m_oBuffer.Load(m_poHeader->m_oFilename);

    sprintf((char*)DSM_FileContentSize->m_oBuffer.m_poBuffer, "%d", Content->m_oBuffer.Len());

    if (m_poBinding)
        m_poBinding->SetFileContentSize(Content->m_oBuffer.Len());
}



/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsBiopDirMessage::dmsBiopDirMessage(dmsBiopMessage *Header) : dmsData()
{
    HDR_INIT(ObjectInfoLength,        16);
    HDR_INIT(ObjectInfoData,          0);
    HDR_INIT(ServiceContextListCount, 8);
    HDR_INIT(ServiceContextList,      0);
    HDR_INIT(MessageBodyLength,       32);
    HDR_INIT(BindingsCount,           16);
    HDR_INIT(BindingsList,            0);

    m_poHeader  = Header;

    ServiceContextList = new dmsBiopServiceContextList();
    BindingsList       = new dmsBiopBindingList();

    SetLenLimit(&ObjectInfoLength, &ObjectInfoData);
    SetListCount(&ServiceContextListCount, ServiceContextList);
    SetLenLimit(&MessageBodyLength, &BindingsList);
    SetListCount(&BindingsCount,    BindingsList);

    Header->SetKind((char *)"dir");

    Header->SetName("BIOP_DirMessage");
    Header->Data = this;

    SetName("BIOP_DirData");
}



dmsBiopDirMessage::~dmsBiopDirMessage()
{

}


/* ========================================================================

   ======================================================================== */


/* ########################################################################

   ######################################################################## */


dmsBiopBinding::dmsBiopBinding(dmsBiopMessage* Message) : dmsData()
{
    HDR_INIT(NameComponentsCount, 8);
    HDR_INIT(IdLength,            8);
    HDR_INIT(IdData,              0);
    HDR_INIT(KindLength,          8);
    HDR_INIT(KindData,            0);
    HDR_INIT(BindingType,         8);
    HDR_INIT(IOR,                 0);
    HDR_INIT(ObjectInfoLength,    16);
    HDR_INIT(DSM_FileContentSize, 0);

    IOR                 = new dmsCorbaIOR();
    IdData              = new dmsData();
    KindData            = new dmsData();
    DSM_FileContentSize = new dmsData();

    m_poProfileBody = new dmsBiopProfileBody(Message);

    IOR->TaggedProfileList->Append(m_poProfileBody);


    BindingType         = 1; // No object
    NameComponentsCount = 1;

    SetLenLimit(&IdLength,   &IdData);
    SetLenLimit(&KindLength, &KindData);
    SetLenLimit(&ObjectInfoLength, &DSM_FileContentSize);

    SetName("Binding");

    KindData->m_oBuffer.Set(Message->ObjectKindData->m_oBuffer);
    IOR->SetTypeId(Message->ObjectKindData);

    m_poMessage = Message;
}

void dmsBiopBinding::SetSingleBiopName(const char *Id)
{
    if (GeneratedError()) return;

    IdData->m_oBuffer.Set(Id, false);
}


void dmsBiopBinding::SetFileContentSize(u32 Size)
{
    if (GeneratedError()) return;

    DSM_FileContentSize->m_oBuffer.Set(0, 8);

    sprintf((char*)DSM_FileContentSize->m_oBuffer.m_poBuffer, "%u", Size);
}



dmsBiopServiceContext::dmsBiopServiceContext() : dmsData()
{
    HDR_INIT(ServiceID_DataBroadcastId,     16);
    HDR_INIT(ServiceID_ApplicationTypeCode, 16);
    HDR_INIT(ApplicationSpecificDataLength, 16);
    HDR_INIT(ApplicationSpecificData,       0);

    ServiceID_DataBroadcastId     = 0x0106;
    ServiceID_ApplicationTypeCode = 0x0102;

    SetLenLimit(&ApplicationSpecificDataLength, &ApplicationSpecificData);

    SetName("ServiceContext");
}
