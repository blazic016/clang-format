/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 11/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <MPEG/MPEG.h>
#include <MPEG/Multiplexer.h>

#include "DDB.h"
#include "DSMCC.h"
#include "DataCarousel.h"
#include "Group.h"
#include "Module.h"
#include "SuperGroup.h"

/* ########################################################################

   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsDDB_SectionList);

/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsDDB_Section::dmsDDB_Section(dmsModule* module,
                               int        number,
                               int        lastBlockNumber)
{
    MPEG = new dmsMPEG_Section();
    DSMCC = new dmsDSMCC_Data(MPEG);
    DDB = new dmsDDB_Data(DSMCC);

    MPEG->LastSectionNumber = lastBlockNumber > 0xFF ? 0xFF : lastBlockNumber;
    DDB->m_poModule = module;
    DDB->BlockNumber = number;

    m_oId = STR("%s-%03d_%03d", module->m_oId, number, lastBlockNumber);

    MPEG->m_oTraceFilename = STR("ddb-%s", m_oId);
    if (module->m_oOutputDir.Len())
        m_oOutputDir = module->m_oOutputDir + "/" + MPEG->m_oTraceFilename;
}

dmsDDB_Section::~dmsDDB_Section()
{
    DELNUL(MPEG);
}

/* ========================================================================

   ======================================================================== */

bool dmsDDB_Section::Generate()
{
    MPEG->Generate1();

    if (m_oOutputDir.Len())
        MPEG->Trace(m_oOutputDir);

    DDB->m_poModule->m_poDataCarousel->m_poMux->Add(*MPEG);

    return true;
}

/* ########################################################################

   ######################################################################## */

dmsDDB_Data::dmsDDB_Data(dmsDSMCC_Data* Header) : dmsData()
{
    HDR_INIT(ModuleId, 16);
    HDR_INIT(ModuleVersion, 8);
    HDR_INIT(Reserved, 8);
    HDR_INIT(BlockNumber, 16);
    HDR_INIT(Data, 0);

    Header->SetName(&Header->TransactionId, "DownloadId");

    Header->m_poSection->SetName("DDB_Section");
    Header->m_poSection->TableId = 0x3C;

    Header->SetName("DDB_Message");
    Header->ProtocolDiscriminator = 0x11;
    Header->DsmccType = 0x03;
    Header->MessageId = 0x1003;
    Header->Reserved = 0xFF;
    Header->AdaptationLength = 0x00;
    Header->Data = this;

    SetName("DDB_Data");

    Reserved = 0xFF;

    m_poHeader = Header;

    Data = new dmsData();
}

bool dmsDDB_Data::Update()
{
    if (m_poModule != NULL)
    {
        m_poHeader->m_poSection->m_iPID = m_poModule->m_poDataCarousel->m_iPID;
        m_poHeader->m_poSection->m_iOutputFrequency =
            m_poModule->m_iDDBOutputFrequency;
        m_poHeader->m_poSection->TableIdExtension = m_poModule->DIIM->ModuleId;
        m_poHeader->m_poSection->VersionNumber =
            m_poModule->DIIM->ModuleVersion & 0x1F;

        m_poHeader->m_poSection->SectionNumber = BlockNumber & 0x00FF;

        m_poHeader->TransactionId =
            m_poModule->m_poGroup->m_poSection->DII->DownloadId;

        ModuleId = m_poModule->DIIM->ModuleId;
        ModuleVersion = m_poModule->DIIM->ModuleVersion;

        return true;
    }
    else
    {
        return false;
    }
}
