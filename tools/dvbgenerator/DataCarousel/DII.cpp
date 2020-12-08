/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <MPEG/DESC.h>
#include <ObjectCarousel/BIOP.h>

#include "DSMCC.h"
#include "DII.h"
#include "Module.h"
#include "ModuleData.h"



dmsDII_Section::dmsDII_Section()
{
    MPEG  = new dmsMPEG_Section();
    DSMCC = new dmsDSMCC_Data(MPEG);
    DII   = new dmsDII_Data(DSMCC);
}


dmsDII_Section::~dmsDII_Section()
{
    DELNUL(MPEG);
}

dmsDII_Data::dmsDII_Data(dmsDSMCC_Data *Header) : dmsData()
{
    HDR_INIT(DownloadId,              32);
    HDR_INIT(BlockSize,               16);
    HDR_INIT(WindowSize,              8);
    HDR_INIT(AckPeriod,               8);
    HDR_INIT(TC_DownloadWindow,       32);
    HDR_INIT(TC_DownloadScenario,     32);
    HDR_INIT(CompatibilityDescriptor, 16);
    HDR_INIT(NumberOfModules,         16);
    HDR_INIT(ModuleList,              0);
    HDR_INIT(PrivateDataLength,       16);
    HDR_INIT(PrivateData,             0);

    ModuleList = new dmsDII_ModuleList();

    SetLenLimit(&PrivateDataLength, &PrivateData);
    SetListCount(&NumberOfModules, ModuleList);

    Header->m_poSection->SetName("DII_Section");
    Header->m_poSection->TableId = 0x3B;

    Header->SetName("DII_Message");
    Header->ProtocolDiscriminator = 0x11;
    Header->DsmccType             = 0x03;
    Header->MessageId             = 0x1002;
    Header->Reserved              = 0xFF;
    Header->AdaptationLength      = 0x00;
    Header->Data                  = this;

    SetName("DII_Data");

    m_poHeader = Header;
}


bool dmsDII_Data::Update()
{
    m_poHeader->m_poSection->TableIdExtension = m_poHeader->TransactionId & 0xFFFF;

    return true;
}



dmsDII_Module::dmsDII_Module() : dmsData()
{
    HDR_INIT(ModuleId,          16);
    HDR_INIT(ModuleSize,        32);
    HDR_INIT(ModuleVersion,     8);
    HDR_INIT(ModuleInfoLength,  8);
    HDR_INIT(ModuleInfo,        0);

    SetLenLimit(&ModuleInfoLength, &ModuleInfo);

    m_poModule = NULL;

    SetName("Module");
}


bool dmsDII_Module::Update()
{
    if (m_poModule==NULL || m_poModule->m_poModuleData==NULL) return false;

    ModuleSize = m_poModule->m_poModuleData->m_oBuffer.Len();

    return true;
}
