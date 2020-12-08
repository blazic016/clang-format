/****************************************************************************
** @file MainFrame.cpp
**
** @brief
**   Management user input: Read, check and XML conversion in TS Writer format.
**
** @ingroup GENIUS USER INTERFACE
**
** @see MainFrame.xml and TS_Writer.xml
**
** @version $Rev: 62361 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/soft/ISSU_Generator/MainFrame.cpp $
**          $Date: 2013-03-11 16:06:08 +0100 (lun., 11 mars 2013) $
**
** @author  SmarDTV Rennes - LIPPA
**
** COPYRIGHT:
**   2011 SmarDTV
**
** @history
**   COF   - Iwedia  - v 0    - 05/2003 - Creation
**   LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
**   LIPPA - SmarDTV - v 2.00 - 10/2011 - New tab 'Additional Streams' to
**                                        inject streams audio and video in
**                                        stream generated.
**   LIPPA - SmarDTV - v 2.10 - 12/2011 - Add DSI Subdescriptor Update ID
**                                        and Usage ID
**   LIPPA - SmarDTV - v 2.10 - 01/2012 - Add Usage ID in PMT in Protocol ID 3
**   LIPPA - SmarDTV - v 2.11 - 02/2012 - Fix bug Mux sections modules
**                                        multiplexed with REPEAT shorty
**                                        modules.
**   LIPPA - SmarDTV - v 2.12 - 02/2012 - DSI repetition all seconds.
**   LIPPA - SmarDTV - v 2.13 - 03/2012 - Generate raw sections in a file and
**                                        choice generated files extensions.
**   LIPPA - SmarDTV - v 2.20 - 03/2012 - Merge Telefonica functionality:
**                                        - Manage 16 images max
**                                        - Manage Zone ID with config file
**                                          and DA2
**                                        - Manage max 16 additionals software
**                                          compatibility descriptors.
**                                        Corrupt Chunk CRC option for QA Test
**                                        (enable only in version DEBUG).
**   LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
**                                        Header in sections file
**   LIPPA - SmarDTV - v 2.30 - 03/2013 - Use DVB generic OUI 0x15A for all
**                                        OUIs in signaling table (PMT, NIT
**                                        and BAT).
**   LIPPA - SmarDTV - v 2.31 - 07/2013 - Manage Flash Location NASC 3.0
**                                        (modules NASC 3.0).
**   LIPPA - SmarDTV - v 2.32 - 09/2013 - Support Module Size 512 K and 1 M.
**
******************************************************************************/

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/regex.h>
#include <wx/filename.h>
#include <wx/busyinfo.h>
#include <wx/ffile.h>

#include <Tools/Tools.h>
#include <Tools/Xml.h>
#include <Tools/Validator.h>
#include <Tools/Conversion.h>

#include <ToolsGui/Window.h>
#include <ToolsGui/Log.h>

#include <TS_Writer/TS_Writer.h>
#include <TS_Writer/Config.h>
#include <TS_Writer/TS_WriterVersion.h>

#include "MainApp.h"
#include "MainConf.h"

#include <Resources/iWedia.xpm>

#include <sstream>


/* ========================================================================
   Declarations
   ======================================================================== */

// Loader Type (value of combobox 'LDTYPE', see MainFrame.xml)
#define kMFRAMEi_LOADER_TYPE_DOWNLOADER_BASIC        0
#define kMFRAMEi_LOADER_TYPE_EMBEDDED_LOADER         1
#define kMFRAMEi_LOADER_TYPE_DOWNLOADER_NASC         2

// Carousel Type (value of combobox 'CAROTYPE_DC_OC_GC', see MainFrame.xml)
#define kMFRAMEi_CARO_TYPE_DATA_CAROUSEL             0
#define kMFRAMEi_CARO_TYPE_OBJECT_CAROUSEL           1
#define kMFRAMEi_CARO_TYPE_GENIUS_CAROUSEL           2

// Flash Location Type (value of combobox 'FLASH_LOCATION_TYPE', see MainFrame.xml)
#define kMFRAMEi_FLASH_LOC_TYPE_LOGICAL_OFFSET       0
#define kMFRAMEi_FLASH_LOC_TYPE_PARTITION_ID         1
#define kMFRAMEi_FLASH_LOC_TYPE_ICS_ID_NASC          2
#define kMFRAMEi_FLASH_LOC_TYPE_ICS_AND_ZONEID_NASC  3
#define kMFRAMEi_FLASH_LOC_TYPE_NASC3_MODULE_ID      4

// Sections File Generation Method (value of combobox 'SEC_FILE_MODE', see MainFrame.xml)
#define kMFRAMEi_SEC_FILE_MODE_NOT_GENERATED         0
#define kMFRAMEi_SEC_FILE_MODE_ADD_SIGNAL_TABLE      1
#define kMFRAMEi_SEC_FILE_MODE_USB_HEADER            2
#define kMFRAMEi_SEC_FILE_MODE_ONLY_SECTIONS         3

// ICS Module NASC Id of each image location (value of combobox 'ImageICSIdN' with N in [1..8], see MainFrame.xml)
#define kMFRAMEi_IMAGE_ICS_ID_DOWNLOADER             0
#define kMFRAMEi_IMAGE_ICS_ID_EMS_CONFIG_FILE        1
#define kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_0 2
#define kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_1 3
#define kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_2 4
#define kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_0  5
#define kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_1  6
#define kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_2  7
#define kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_0       8
#define kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_1       9
#define kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_2      10
#define kMFRAMEi_IMAGE_ICS_ID_SSA_DA2               11

// NASC 3.0 Module ID of each image location (value of combobox 'ImageNASC3ModIdN' with N in [1..8], see MainFrame.xml)
#define kMFRAMEi_IMAGE_NASC3_ID_BOOTLOADER         0
#define kMFRAMEi_IMAGE_NASC3_ID_CONFIG_FILE        1
#define kMFRAMEi_IMAGE_NASC3_ID_DOWNLOADER         2
#define kMFRAMEi_IMAGE_NASC3_ID_APPLI_KERNEL       3
#define kMFRAMEi_IMAGE_NASC3_ID_APPLI_MIDDLEWARE   4
#define kMFRAMEi_IMAGE_NASC3_ID_SERVICE_MODE       5
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_07          6
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_08          7
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_09          8
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_10          9
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_11         10
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_12         11
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_13         12
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_14         13
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_15         14
#define kMFRAMEi_IMAGE_NASC3_ID_MODULE_16         15


dmsMainFrame *g_poMainFrame = NULL;

/* ========================================================================
   Frame
   ======================================================================== */

#include <wx/notebook.h>
dmsCliContextManager * dm;
dmsMainCli::dmsMainCli(const wxString& title)
{
	shouldUseWait = false;
	m_poManager = new dmsCliContextManager();
	dmsMain::m_poManager = m_poManager;
	wxXmlDocument doc;
	dmsBuffer     buf;
	int           i;
	char          label[50];

	if (!buf.LoadResource("Resources/MainFrame.xml")) return;

	wxString str = (char*) buf.m_poBuffer;

	str.Replace("$SSU_GEN_VERSION$",
	     STR("Version %s --- %s", SOFT_VERSION, __DATE__));

	doc.LoadText(str);

	if (! m_poManager->Load(doc.GetRoot())) return;

	dmsValidatorManager *v = m_poManager->m_poValidatorManager;

	  v->SetMapping("Profile",                &g_poMainConf->m_iProfile);
	    v->SetMapping("OutputDir",              &g_poMainConf->m_oOutputDir);
	    v->SetMapping("OADFileExtension",       &g_poMainConf->m_oExtFileOAD);
	    v->SetMapping("SEC_FILE_EXTENSION",     &g_poMainConf->m_oExtFileSections);
	    v->SetMapping("SEC_FILE_MODE",          &g_poMainConf->m_iModeFileSections);
	    v->SetMapping("NO_NIT_BAT",             &g_poMainConf->m_iNO_NIT_BAT);
	    v->SetMapping("LDTYPE",                 &g_poMainConf->m_iLoaderType);
	    v->SetMapping("CAROTYPE_DC_OC_GC",      &g_poMainConf->m_iCarouselType_DC_OC_GC);
	    v->SetMapping("TriggerType_UK_SSU_SSUE",&g_poMainConf->m_iTriggerType_UK_SSU_SSUE);
	    v->SetMapping("FLASH_LOCATION_TYPE",    &g_poMainConf->m_iFLASH_LOCATION_TYPE);
	    v->SetMapping("SAT_CABLE_TERR",         &g_poMainConf->m_iSAT_CABLE_TERR);
	    v->SetMapping("SW_Model",               &g_poMainConf->m_iCompatibilitySW_Model);
	    v->SetMapping("SW_Version",             &g_poMainConf->m_iCompatibilitySW_Version);
	    v->SetMapping("OUI",                    &g_poMainConf->m_iOUI);
	    v->SetMapping("INTERNAL_SIG",           &g_poMainConf->m_bInternalSig);
	    v->SetMapping("DoMultiplex",            &g_poMainConf->m_bWithPcr);
	    v->SetMapping("AUTO_SIGN",              &g_poMainConf->m_bAutoSign);
	    v->SetMapping("DiffusionDate",          &g_poMainConf->m_oDiffusionDate);
	    v->SetMapping("SSUE_StartDiffusionDate",&g_poMainConf->m_oSSUE_StartDiffusionDate);
	    v->SetMapping("Protocol",                    &g_poMainConf->m_iProtocolId);
	    v->SetMapping("DEBUG_REQUIRED",         &g_poMainConf->m_bDebug);
	    v->SetMapping("ModuleSize",             &g_poMainConf->m_iModuleSize);
	    v->SetMapping("ADD_USAGE_ID",           &g_poMainConf->m_bAddUsageIdFile);
	    v->SetMapping("SIG_UseGenericOUI",      &g_poMainConf->m_bSIG_UseGenericOUI);

	    for (i = 0; i < kTS_WRITER_MAX_IMAGES_BASIC; i++)
	    {
	       sprintf(label,"ImageFlashOffset%d",i+1);
	       v->SetMapping(label,g_poMainConf->m_tiImagesBasicFlashOffset+i);
	       sprintf(label,"ImagePartitionId%d",i+1);
	       v->SetMapping(label,g_poMainConf->m_tiImagesBasicPartitionId+i);
	       sprintf(label,"ImageICSId%d",i+1);
	       v->SetMapping(label,g_poMainConf->m_tiImagesBasicICSModuleId+i);
	       sprintf(label,"ImageNASC3ModId%d",i+1);
	       v->SetMapping(label,g_poMainConf->m_tiImagesBasicNASC3ModuleId+i);
	    }

	    for (i = 0; i < kTS_WRITER_MAX_IMAGES_BY_ZONE_ID; i++)
	    {
	       sprintf(label,"ibzImageICSId%d",i+1);
	       v->SetMapping(label,g_poMainConf->m_tiImagesByZidICSModuleId+i);
	       sprintf(label,"ibzImageZoneId%d",i+1);
	       v->SetMapping(label,g_poMainConf->m_tiImagesByZidZoneId+i);
	    }

	    for (i = 0; i < kTS_WRITER_MAX_PARTITION_PREDEF; i++)
	    {
	       sprintf(label,"Partition_Id%d",i+1);
	       v->SetMapping(label,g_poMainConf->m_tiPartitionIdent+i);
	       sprintf(label,"Partition_Sz%d",i+1);
	       v->SetMapping(label,g_poMainConf->m_tiPartitionSize+i);
	    }
	    /*  LLLLLLLLLLLLLLLLLLLLLLLLLLLL */
}

#include <iostream>
void dmsMainCli::printGenerationMsg(const wxString& msg, int icon) const
{
	std::string out_file("Output_Info");
	std::string message(msg);
	std::istringstream stream(message);
	std::string line;

	for(int i = 0; i < 4; i++)
	{
		std::getline(stream, line);
		std::cout << line << std::endl;
	}

	std::ofstream file;
	file.open(out_file, std::ios_base::out | std::ios_base::trunc);

	std::cout << "Full Output Info in " << out_file << std::endl;

	file << message;
}

dmsMainFrame::dmsMainFrame(const wxString& title) : dmsDialog(NULL, title)
{
	shouldUseWait = true;
	dmsMain::m_poManager = dmsDialog::m_poManager;

    wxXmlDocument doc;
    dmsBuffer     buf;
   int           i;
   char          label[50];

    if (!buf.LoadResource("Resources/MainFrame.xml")) return;

    wxString str = (char*) buf.m_poBuffer;

    str.Replace("$SSU_GEN_VERSION$",
        STR("Version %s --- %s", SOFT_VERSION, __DATE__));

    doc.LoadText(str);

    if (! m_poManager->Load(doc.GetRoot())) return;

    dmsValidatorManager *v = m_poManager->m_poValidatorManager;

    v->SetMapping("Profile",                &g_poMainConf->m_iProfile);

    v->SetMapping("OutputDir",              &g_poMainConf->m_oOutputDir);
    v->SetMapping("OADFileExtension",       &g_poMainConf->m_oExtFileOAD);
    v->SetMapping("SEC_FILE_EXTENSION",     &g_poMainConf->m_oExtFileSections);
    v->SetMapping("SEC_FILE_MODE",          &g_poMainConf->m_iModeFileSections);
    v->SetMapping("NO_NIT_BAT",             &g_poMainConf->m_iNO_NIT_BAT);
    v->SetMapping("LDTYPE",                 &g_poMainConf->m_iLoaderType);
    v->SetMapping("CAROTYPE_DC_OC_GC",      &g_poMainConf->m_iCarouselType_DC_OC_GC);
    v->SetMapping("TriggerType_UK_SSU_SSUE",&g_poMainConf->m_iTriggerType_UK_SSU_SSUE);
    v->SetMapping("FLASH_LOCATION_TYPE",    &g_poMainConf->m_iFLASH_LOCATION_TYPE);
    v->SetMapping("SAT_CABLE_TERR",         &g_poMainConf->m_iSAT_CABLE_TERR);
    v->SetMapping("SW_Model",               &g_poMainConf->m_iCompatibilitySW_Model);
    v->SetMapping("SW_Version",             &g_poMainConf->m_iCompatibilitySW_Version);
    v->SetMapping("OUI",                    &g_poMainConf->m_iOUI);
    v->SetMapping("INTERNAL_SIG",           &g_poMainConf->m_bInternalSig);
    v->SetMapping("DoMultiplex",            &g_poMainConf->m_bWithPcr);
    v->SetMapping("AUTO_SIGN",              &g_poMainConf->m_bAutoSign);
    v->SetMapping("DiffusionDate",          &g_poMainConf->m_oDiffusionDate);
    v->SetMapping("SSUE_StartDiffusionDate",&g_poMainConf->m_oSSUE_StartDiffusionDate);
    v->SetMapping("Protocol",                    &g_poMainConf->m_iProtocolId);
    v->SetMapping("DEBUG_REQUIRED",         &g_poMainConf->m_bDebug);
    v->SetMapping("ModuleSize",             &g_poMainConf->m_iModuleSize);
    v->SetMapping("ADD_USAGE_ID",           &g_poMainConf->m_bAddUsageIdFile);
    v->SetMapping("SIG_UseGenericOUI",      &g_poMainConf->m_bSIG_UseGenericOUI);

    for (i = 0; i < kTS_WRITER_MAX_IMAGES_BASIC; i++)
    {
       sprintf(label,"ImageFlashOffset%d",i+1);
       v->SetMapping(label,g_poMainConf->m_tiImagesBasicFlashOffset+i);
       sprintf(label,"ImagePartitionId%d",i+1);
       v->SetMapping(label,g_poMainConf->m_tiImagesBasicPartitionId+i);
       sprintf(label,"ImageICSId%d",i+1);
       v->SetMapping(label,g_poMainConf->m_tiImagesBasicICSModuleId+i);
       sprintf(label,"ImageNASC3ModId%d",i+1);
       v->SetMapping(label,g_poMainConf->m_tiImagesBasicNASC3ModuleId+i);
    }

    for (i = 0; i < kTS_WRITER_MAX_IMAGES_BY_ZONE_ID; i++)
    {
       sprintf(label,"ibzImageICSId%d",i+1);
       v->SetMapping(label,g_poMainConf->m_tiImagesByZidICSModuleId+i);
       sprintf(label,"ibzImageZoneId%d",i+1);
       v->SetMapping(label,g_poMainConf->m_tiImagesByZidZoneId+i);
    }

    for (i = 0; i < kTS_WRITER_MAX_PARTITION_PREDEF; i++)
    {
       sprintf(label,"Partition_Id%d",i+1);
       v->SetMapping(label,g_poMainConf->m_tiPartitionIdent+i);
       sprintf(label,"Partition_Sz%d",i+1);
       v->SetMapping(label,g_poMainConf->m_tiPartitionSize+i);
    }

    SetIcon(wxICON(iWedia));
    dmsLoadGeometry(this, "/Window/Main");
}


dmsMainFrame::~dmsMainFrame()
{
    dmsSaveGeometry(this, "/Window/Main");
}


/* ========================================================================

   ======================================================================== */


void dmsMainFrame::OnEvent(dmsDialogContext* ctx)
{
    wxString name;

    if (ctx)
        name = ctx->m_oName;
    else
        name = "Quit";

    if (name=="Quit")
    {
        if (! AssumeProfilSaved()) return;

        Destroy();
        return;
    }
    if (name=="Profile")
    {
        dmsDialogContext* ctx = (dmsDialogContext*) m_poManager->Find("Profile");
        dmsvEnum*         map = (dmsvEnum*) ctx->m_poValidator;

        wxString old = map->Get();

        ctx->Set();

        if (! AssumeProfilSaved(old))
        {
            ctx->Set(old);
            m_poManager->m_poGetCxt = ctx;
            return;
        }

        g_poMainConf->Load(map->Get(), *m_poManager->m_poValidatorManager);
        m_poManager->m_poValidatorManager->InitReset();
        m_poManager->Get();

        return;
    }
    if (name=="AddProfile")
    {
        if (! AssumeProfilSaved()) return;

        wxTextEntryDialog input(NULL,
            "New profile",
            "Enter new profile name",
            "");

        if (input.ShowModal() != wxID_OK) return;

        dmsDialogContext* ctx = (dmsDialogContext*) m_poManager->Find("Profile");
        dmsvEnum* map   = (dmsvEnum*) ctx->m_poValidator;

        if (! map->AddUniqValue(input.GetValue()))
        {
            wxMessageDialog dialog(NULL,
                map->GetError(),
                "Input error",
                wxOK | wxCENTRE | wxICON_ERROR);

            dialog.ShowModal();

            return;
        }

        ctx->Get();

        g_poMainConf->Save(map->Get(), *m_poManager->m_poValidatorManager);

        return;
    }
    if (name=="SaveProfile")
    {
        dmsValidator* map = m_poManager->m_poValidatorManager->Find("Profile");
        g_poMainConf->Save(map->Get(), *m_poManager->m_poValidatorManager);

        wxMessageDialog dialog(NULL,
            "Profile Saved",
            "Profile",
            wxOK | wxCENTRE | wxICON_INFORMATION);

        dialog.ShowModal();

        return;
    }
    if (name=="ResetProfile")
    {
        dmsValidator* map = m_poManager->m_poValidatorManager->Find("Profile");

        wxMessageDialog dialog(NULL,
            STR("Do you confirm reseting profile '%s' ?", map->Get()),
            "Reset profile",
            wxNO_DEFAULT|wxYES_NO|wxICON_WARNING);

        if (dialog.ShowModal()!=wxID_YES) return;

        m_poManager->m_poValidatorManager->Reset();
        m_poManager->Get();
        return;
    }
    if (name=="DelProfile")
    {
        dmsDialogContext* ctx = (dmsDialogContext*) m_poManager->Find("Profile");
        dmsvEnum*         map = (dmsvEnum*) ctx->m_poValidator;

        g_poMainConf->RemoveProfile(map->Get());
        g_poMainConf->Load(map->Get(), *m_poManager->m_poValidatorManager);
        m_poManager->m_poValidatorManager->InitReset();
        m_poManager->Get();

        return;
    }
    if (name=="CreateStream")
    {
        if (m_poManager->Validate())
            CreateStream();
    }
}

dmsMain::dmsMain() {

}

bool dmsMain::AssumeProfilSaved(const wxString &name)
{
    if (g_poMainConf==NULL) return true;

    if (! m_poManager->m_poValidatorManager->IsModified()) return true;

    dmsvEnum* map = (dmsvEnum*) m_poManager->m_poValidatorManager->Find("Profile");

    wxString realname = name;

    if (realname.IsEmpty()) realname = map->Get();

    wxMessageDialog confirm(NULL,
        STR("Do you want to save profile '%s'", realname),
        "Input error",
        wxYES|wxNO|wxCANCEL|wxICON_QUESTION);

    switch (confirm.ShowModal())
    {
    case wxID_NO:
        return true;
    case wxID_CANCEL:
        return false;
    case wxID_YES:
        g_poMainConf->Save(realname, *m_poManager->m_poValidatorManager);
        return true;
    }

    wxMessageBox("dmsMainFrame::AssumeProfilSaved");

    return true;
}


bool dmsMain::CreateStream()
{
    wxXmlDocument doc;
    dmsBuffer     buf;
    bool          ok = true;

    if (! buf.LoadResource("Resources/TS_Writer.xml")) return false;

    m_oConf = (char*) buf.m_poBuffer;

    buf.Clear();

    /* BELB Comment : Create the XML for TS_Writter with the new config from IHM */
    if (! ReplaceConf())
    {
        LOGE(L"Internal Error");
        ok = false;
    }

    wxString debug = g_poMainConf->m_bDebug ? "\n\n\nXML\n===\n\n"+m_oConf:"";


    /* BELB Comment _ Enable this part to get the XML generate and send to TS Writter
       usefull when the TS_Writter crash
    dmsTextDialog dialogBeforeTSWritter("XML Generate",
        STR(
        "%s",
        debug),
        wxICON_INFORMATION);

    dialogBeforeTSWritter.ShowModal();
    */

    if (ok)
    {
        g_poLogManager->Clear();

        doc.LoadText(m_oConf);

        dmsTS_Writer writer;

        if(shouldUseWait)
        	wxBeginBusyCursor();

        if (writer.Load(doc.GetRoot()) && writer.Generate())
        {
            if (g_poMainConf->m_bDebug)
            {
                debug = "\n\n\n" + g_poLogManager->Format("- %s\n") + debug;
            }

            wxFileName filename(writer.m_poConfig->m_oOutputFile);

            filename.MakeAbsolute();

         if ((writer.m_poConfig->m_oOutputSectionsFile.Len() > 0) &&
             (writer.m_poConfig->m_iModeSectionsFile != kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED))
         {
            wxFileName fsectname(writer.m_poConfig->m_oOutputSectionsFile);

            fsectname.MakeAbsolute();


            printGenerationMsg(STR("Sucessfull stream generation.\n\n"
                    "Stream generated in file [%s]\n"
                    "Data Sections generated in file [%s]%s",
                    filename.GetFullPath(),
                    fsectname.GetFullPath(),
                    debug),
            		wxICON_INFORMATION);

         }
         else
         {
            printGenerationMsg(STR("Sucessfull stream generation.\n\n"
                   "Stream generated in file [%s]%s",
                   filename.GetFullPath(), debug),
                   wxICON_INFORMATION);
         }


            ok = true;
        }
        else
        {
            ok = false;
        }
        if(shouldUseWait)
        	wxEndBusyCursor();
    }

    if (!ok)
    {
        printGenerationMsg(STR("Error in stream generation :\n\n%s%s",
                g_poLogManager->Format("- %s\n", DMS_LOG_LEVEL_ERR),
                debug),
            wxICON_ERROR);
    }

    return ok;
}

void dmsMainFrame::printGenerationMsg(const wxString& msg, int icon) const {
    dmsTextDialog dialog("Generation",
    	   msg,
           icon);
    dialog.ShowModal();
}


void MFi_SSUE_TranslateInHour(int  iHour,
                                    int  iHourAmPm,
                                    unsigned char *pucHour)
{
    *pucHour = iHour;
    if (iHourAmPm != 0)
    {
        *pucHour = *pucHour  + 12;
    }

    return;
}

void MFi_SSUE_ComputeDuration(unsigned char ucStartHour,
                              unsigned char ucStopHour,
                              unsigned short *pusDuration)
{
    if (ucStartHour > ucStopHour)
    {
        *pusDuration = (24-ucStartHour) + ucStopHour;
    }
    else
    {
        *pusDuration = ucStopHour - ucStartHour ;
    }

    return;
}

int
MFi_SSUE_ComputeActionTypeOuiHash(int OUI)
{
    int iResult;

    iResult = (OUI >> 16) & 0xFF;
    iResult ^= (OUI >> 8) & 0xFF;
    iResult ^= OUI & 0xFF;

    iResult ^= 0x0100;

    return (iResult);
}


/**
 * PRIVATE METHODS
 *
 * Functions used to create the XML given to the TS_Writter (see TS_Writter.xml)
 * with user input
*/

bool dmsMain::ReplaceConf()
{
    dmsConfBase  *c = g_poMainConf;
   int       nb_images,nb_add_streams,nb_add_sw_cd;
   int       rate, repRate, iBlockSize;
    int       iSSUE_MJD_Start;
   bool      is_images_by_zone_id = false;
   char      label[50];
   int       i,j;

   /* Convert UI Loader type in TS Writer loader type */
   switch (c->m_iLoaderType)
   {
   case kMFRAMEi_LOADER_TYPE_DOWNLOADER_BASIC:
      if (! ReplaceItem("LOADER_TYPE",kTS_WRITER_LOADER_TYPE_DOWNLOADER_BASIC)) return false;
      break;
   case kMFRAMEi_LOADER_TYPE_EMBEDDED_LOADER:
      if (! ReplaceItem("LOADER_TYPE",kTS_WRITER_LOADER_TYPE_EMBEDDED_LOADER)) return false;
      break;
   case kMFRAMEi_LOADER_TYPE_DOWNLOADER_NASC:
      if (! ReplaceItem("LOADER_TYPE",kTS_WRITER_LOADER_TYPE_DOWNLOADER_NASC)) return false;
      break;
   default:
        LOGE(L"Loader type %d invalid",c->m_iLoaderType);
        return false;
   }

   if (c->m_iLoaderType == kMFRAMEi_LOADER_TYPE_DOWNLOADER_NASC)
   {
      // In Downloader NASC the image are already signed
      m_oConf.Replace("$SignatureType$",kTS_WRITER_SIGNATURE_NONE_STRING);
      m_oConf.Replace("$InputKeyFile$", "");
      m_oConf.Replace("$InputSignFile$","");
   }
   else if (c->m_bAutoSign)
   {
      // Process signature
      m_oConf.Replace("$SignatureType$",kTS_WRITER_SIGNATURE_AUTOMATIC_STRING);
      m_oConf.Replace("$InputSignFile$","");
   }
   else
   {
      // Add signature added in a file
      m_oConf.Replace("$SignatureType$",kTS_WRITER_SIGNATURE_FROMFILE_STRING);
      m_oConf.Replace("$InputKeyFile$", "");
   }

   // Flash location type and images
   switch (c->m_iFLASH_LOCATION_TYPE)
   {
   case kMFRAMEi_FLASH_LOC_TYPE_LOGICAL_OFFSET:
      is_images_by_zone_id = false;
      m_oConf.Replace("$LocationType$",kTS_WRITER_IMG_LOCATION_BY_OFFSET_STRING);
      break;
   case kMFRAMEi_FLASH_LOC_TYPE_PARTITION_ID:
      is_images_by_zone_id = false;
      m_oConf.Replace("$LocationType$",kTS_WRITER_IMG_LOCATION_BY_OFFSET_STRING);
      break;
   case kMFRAMEi_FLASH_LOC_TYPE_ICS_ID_NASC:
      is_images_by_zone_id = false;
      m_oConf.Replace("$LocationType$",kTS_WRITER_IMG_LOCATION_BY_PARTID_STRING);
      break;
   case kMFRAMEi_FLASH_LOC_TYPE_ICS_AND_ZONEID_NASC:
      is_images_by_zone_id = true;;
      m_oConf.Replace("$LocationType$",kTS_WRITER_IMG_LOCATION_BY_PARTID_STRING);
      break;
   case kMFRAMEi_FLASH_LOC_TYPE_NASC3_MODULE_ID:
      is_images_by_zone_id = false;
      m_oConf.Replace("$LocationType$",kTS_WRITER_IMG_LOCATION_BY_PARTID_STRING);
      break;
   default:
        LOGE(L"Flash location type %d invalid",c->m_iFLASH_LOCATION_TYPE);
        return false;
   }

   // Get and check the images numbers
   if (is_images_by_zone_id)
   {
      nb_images = atoi(m_poManager->m_poValidatorManager->Find("ibzN_IMAGES")->Get());
      if (nb_images > kTS_WRITER_MAX_IMAGES_BY_ZONE_ID)
      {
           LOGE(L"Multiple images by zone id download number %d too big (max %d)",nb_images,kTS_WRITER_MAX_IMAGES_BY_ZONE_ID);
           return false;
      }
   }
   else
   {
      nb_images = atoi(m_poManager->m_poValidatorManager->Find("N_IMAGES")->Get());
      if (nb_images > kTS_WRITER_MAX_IMAGES_BASIC)
      {
           LOGE(L"Multiple images basic download number %d too big (max %d)",nb_images,kTS_WRITER_MAX_IMAGES_BASIC);
           return false;
      }
   }
   if (nb_images < 1)
   {
        LOGE(L"No image download defined");
        return false;
   }

   // Check first if the carroussel type is not Genius Caroussel (not able to manage with multiple image in G.C.)
    if ((g_poMainConf->m_iCarouselType_DC_OC_GC == kMFRAMEi_CARO_TYPE_GENIUS_CAROUSEL) && (nb_images > 1))
    {
        LOGE(L"Multiple image download cannot be used with Genius Caroussel (only Data or Object)");
        return false;
    }

   // Additionnal compatibility descriptor enable only if location by ICS and Zone ID
   if (c->m_iFLASH_LOCATION_TYPE == kMFRAMEi_FLASH_LOC_TYPE_ICS_AND_ZONEID_NASC)
   {
      nb_add_sw_cd = atoi(m_poManager->m_poValidatorManager->Find("N_SW_ADDITIONAL")->Get());
      if (nb_add_sw_cd > kTS_WRITER_MAX_ADD_SW_COMPATIBILITY_DESC)
      {
           LOGE(L"Additional Software Compatibility Descripor number %d too big (max %d)",
              nb_add_sw_cd,kTS_WRITER_MAX_ADD_SW_COMPATIBILITY_DESC);
           return false;
      }
   }
   else nb_add_sw_cd = 0;

   // Get and check the additional streams images numbers
   /*>>>>> NOT AVAILABLE - BUG
   nb_add_streams = atoi(m_poManager->m_poValidatorManager->Find("N_ADDSTREAMS")->Get());
   if (nb_add_streams > kTS_WRITER_MAX_MUX_ELEMENTARY_STREAMS)
   {
        LOGE("Additional Streams number %d too big (max %d)",nb_add_streams,kTS_WRITER_MAX_MUX_ELEMENTARY_STREAMS);
        return false;
   }
   <<<<<*/
   nb_add_streams = 0;

   // Undefine all non significant images
   ReplaceUndefinedImages(nb_images);

   // Define the N additionnal Software Compatibily Descriptor and undefine the other
   for (i = 0; i < kTS_WRITER_MAX_ADD_SW_COMPATIBILITY_DESC; i++)
   {
      sprintf(label,"AddSW%d",i+1);
      if (i < nb_add_sw_cd) ReplaceItem(label,"SW");
      else ReplaceItem(label,"NO_SW");
   }

   // Undefine all non significant additional stream
   ReplaceUndefinedAddStreams(nb_add_streams);

   // Replacement generique
    dmsValidator* map;

    FOREACH(dmsValidatorList, m_poManager->m_poValidatorManager->m_oList, map)
    {
        m_oConf.Replace(STR("$%s$", map->GetName()), map->Get(), true);
    }

    wxString VersionNoX = m_poManager->m_poValidatorManager->Find("SoftVersion")->Get().Mid(2);
    ReplaceItem("SoftVersion_nox", (const char *)VersionNoX);


    if (c->m_iNO_NIT_BAT == 3)
    {
        wxString Manufacturer = m_poManager->m_poValidatorManager->Find("ManName")->Get();

      if (c->m_oExtFileOAD.Len() > 0)
      {
         if (!ReplaceItem("OUTPUT_FILE", (const char *)STR("%s/%s_%02d%d%04d.%s",
                          c->m_oOutputDir,Manufacturer,
                          c->m_oDiffusionDate.GetWeekOfYear(),
                          c->m_oDiffusionDate.GetWeekDay(),
                          c->m_oDiffusionDate.GetYear(),c->m_oExtFileOAD))) return false;
      }
      else
      {
         if (!ReplaceItem("OUTPUT_FILE", (const char *)STR("%s/%s_%02d%d%04d.trp",
                          c->m_oOutputDir,Manufacturer,
                          c->m_oDiffusionDate.GetWeekOfYear(),
                          c->m_oDiffusionDate.GetWeekDay(),
                          c->m_oDiffusionDate.GetYear()))) return false;
      }

        if (! ReplaceItem("OUTPUT_SECTIONS_FILE", "")) return false;
      if (! ReplaceItem("SECTIONS_FILE_MODE",kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED)) return false;
    }
    else
    {
      if ((c->m_oExtFileOAD.Len() > 0) && (c->m_oExtFileSections.Len() > 0))
      {
         if (c->m_oExtFileOAD == c->m_oExtFileSections)
         {
              LOGE(L"OAD file extension equal sections file extension");
              return false;
         }
      }

      if (c->m_oExtFileOAD.Len() > 0)
      {
         if (!ReplaceItem("OUTPUT_FILE", (const char *)STR("%s/%s.%s",
                          c->m_oOutputDir,VersionNoX,c->m_oExtFileOAD))) return false;
      }
      else
      {
         if (!ReplaceItem("OUTPUT_FILE", (const char *)STR("%s/%s.mpg",
                          c->m_oOutputDir,VersionNoX))) return false;
      }

      if (c->m_oExtFileSections.Len() > 0)
      {
         if (! ReplaceItem("OUTPUT_SECTIONS_FILE", (const char *)STR("%s/%s.%s",
                           c->m_oOutputDir,VersionNoX,c->m_oExtFileSections))) return false;
      }
      else
      {
         if (! ReplaceItem("OUTPUT_SECTIONS_FILE", (const char *)STR("%s/%s.sec",
                           c->m_oOutputDir,VersionNoX))) return false;
      }

      switch (c->m_iModeFileSections)
      {
      case kMFRAMEi_SEC_FILE_MODE_NOT_GENERATED:
         if (! ReplaceItem("SECTIONS_FILE_MODE",kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED)) return false;
         break;
      case kMFRAMEi_SEC_FILE_MODE_ADD_SIGNAL_TABLE:
         if (! ReplaceItem("SECTIONS_FILE_MODE",kTS_WRITER_SEC_FILE_MODE_ADD_SIGNAL_TABLE)) return false;
         break;
      case kMFRAMEi_SEC_FILE_MODE_USB_HEADER:
         if (! ReplaceItem("SECTIONS_FILE_MODE",kTS_WRITER_SEC_FILE_MODE_USB_HEADER)) return false;
         break;

      case kMFRAMEi_SEC_FILE_MODE_ONLY_SECTIONS:
         if (! ReplaceItem("SECTIONS_FILE_MODE",kTS_WRITER_SEC_FILE_MODE_ONLY_SECTIONS)) return false;
         break;
      default:
           LOGE(L"Sections File generation mode %d invalid",c->m_iModeFileSections);
           return false;
      }
    }

   if (! ReplaceItem("DATA_RATE", (const char *)m_poManager->m_poValidatorManager->Find("DataRate")->Get())) return false;

    /* BELB Comment _ DSI DII repetition rate is defined in accordance with data rate :
       repetition_rate = ((data_rate * 5) / (8*4096)) - 2
       where : data_rate * 5 to get the number of data in 5 secondes
               / (8 * 4096) to get the number of 4k section in 5 secondes
               -2 to decrease the value for DSI & DII */
   rate = atoi(m_poManager->m_poValidatorManager->Find("DataRate")->Get());
   repRate = ((rate*1000* 5) / (8*4096)) - 2;
   if (repRate < 3) repRate = 3;
   if (! ReplaceItem("DII_RepetitionRate", repRate)) return false;

   /* LIPPA 2.12 Comment _ DSI: all seconds => DII rate / 5 */
   repRate = repRate / 5;
   if (repRate < 1) repRate = 1;
   if (! ReplaceItem("DSI_RepetitionRate", repRate)) return false;


   /* BELB Comment _ Module Size can be an user selection (64/128/256) */
    switch (c->m_iModuleSize)
    {
   case 1: /* 128 K */
      if (! ReplaceItem("BlockSize","131072")) return false;
        iBlockSize = 131072;
        break;

   case 2: /* 256 K */
      if (! ReplaceItem("BlockSize","262144")) return false;
        iBlockSize = 262144;
        break;

   case 3: /* 512 K */
      if (! ReplaceItem("BlockSize","524288")) return false;
        iBlockSize = 524288;
        break;

   case 4: /* 1 M */
      if (! ReplaceItem("BlockSize","1048576")) return false;
        iBlockSize = 1048576;
        break;

   case 5: /* 2 M */
      if (! ReplaceItem("BlockSize","2097152")) return false;
        iBlockSize = 2097152;
        break;

   case 6: /* 4 M */
      if (! ReplaceItem("BlockSize","4194304")) return false;
        iBlockSize = 4194304;
        break;

   case 7: /* 8 M */
      if (! ReplaceItem("BlockSize","8388608")) return false;
        iBlockSize = 8388608;
        break;

   case 0: /* 64 K */
   default:
      if (! ReplaceItem("BlockSize", "65536")) return false;
        iBlockSize = 65536;
    }

    if (! ReplaceItem("TRACE",       c->m_bDebug)) return false;

   /**
    *  LIPPA 2.3.0
    *  When use DVB generic OUI set the generic OUI in OUI of signalization
    *  and the manufacturer OUI in OUI of carousel (DSI & DII), NOT set
    *  the selector in PMT
    *  Else (= before version 2.3.0) set the manufacturer OUI in all OUI, set
    *  the selector in PMT
    *
   */
   if (c->m_bSIG_UseGenericOUI)
   {
      if (! ReplaceItem("OUI_SIG",   kTS_WRITER_DVB_GENERIC_OUI)) return false;
      if (! ReplaceItem("OUI_TARGET",c->m_iOUI)) return false;
   }
   else
   {
      if (! ReplaceItem("OUI_SIG",   c->m_iOUI)) return false;
      if (! ReplaceItem("OUI_TARGET",c->m_iOUI)) return false;
   }

   wxString wxGroupIdList;
    wxString wxGroupIdFile;
    wxFFile file;

   switch(c->m_iProtocolId)
    {
    case 0: /* Choice 0x0001 */
      if (c->m_bSIG_UseGenericOUI)
         m_oConf.Replace("$PMT_SELECTOR_DFT$", "NO_Selector");
      else
         m_oConf.Replace("$PMT_SELECTOR_DFT$", "Selector");
        m_oConf.Replace("$PMT_SELECTOR_PROTO3$", "NO_Selector");
        m_oConf.Replace("$GROUP_ID_LIST$", "<GroupIdLoop></GroupIdLoop>");
        break;
    case 1: /* Choice 0x0002 */
      if (c->m_bSIG_UseGenericOUI)
         m_oConf.Replace("$PMT_SELECTOR_DFT$", "NO_Selector");
      else
         m_oConf.Replace("$PMT_SELECTOR_DFT$", "Selector");
        m_oConf.Replace("$PMT_SELECTOR_PROTO3$", "NO_Selector");
        wxGroupIdFile = m_poManager->m_poValidatorManager->Find("GroupIDInputFile")->Get();
        if (!file.Open(wxGroupIdFile, "r"))
        {
            LOGE(L"Error opening file [%s]", wxGroupIdFile);
            return false;
        }
        file.ReadAll(&wxGroupIdList);
        file.Close();
        m_oConf.Replace("$GROUP_ID_LIST$", wxGroupIdList);
        break;
    case 2: /* Choice 0x0003 */
        m_oConf.Replace("$PMT_SELECTOR_DFT$", "NO_Selector");
      if (c->m_bSIG_UseGenericOUI)
         m_oConf.Replace("$PMT_SELECTOR_PROTO3$","NO_Selector");
      else
           m_oConf.Replace("$PMT_SELECTOR_PROTO3$","Selector");
        m_oConf.Replace("$GROUP_ID_LIST$", "<GroupIdLoop></GroupIdLoop>");
        break;
    default :
      if (c->m_bSIG_UseGenericOUI)
         m_oConf.Replace("$PMT_SELECTOR_DFT$", "NO_Selector");
      else
         m_oConf.Replace("$PMT_SELECTOR_DFT$", "Selector");
        m_oConf.Replace("$PMT_SELECTOR_PROTO3$", "NO_Selector");
        m_oConf.Replace("$GROUP_ID_LIST$", "<GroupIdLoop></GroupIdLoop>");
        break;
    }
   if (c->m_bAddUsageIdFile)
   {
      wxString wxUsageIdList;
      wxString wxUsageIdFile = m_poManager->m_poValidatorManager->Find("UsageIDInputFile")->Get();
      if (!file.Open(wxUsageIdFile, "r"))
      {
         LOGE(L"Error opening file [%s]", wxUsageIdFile);
         return false;
      }
      file.ReadAll(&wxUsageIdList);
      file.Close();
      m_oConf.Replace("$USAGE_ID_LIST$", wxUsageIdList);

      if ((c->m_iProtocolId == 2) && (!c->m_bSIG_UseGenericOUI)) /* Choice 0x0003 */
      {
         m_oConf.Replace("$PMT_USAGE_ID_LIST$",wxUsageIdList);
      }
      else
      {
         m_oConf.Replace("$PMT_USAGE_ID_LIST$","");
      }
   }
   else
   {
      m_oConf.Replace("$USAGE_ID_LIST$","");
      m_oConf.Replace("$PMT_USAGE_ID_LIST$","");
   }

    switch (c->m_iNO_NIT_BAT)
    {
    case 0:
        ReplaceItem("NIT_BAT", "NO_NIT_BAT");
        ReplaceItem("SDT", "NO_SDT");
        ReplaceItem("NIT_BAT_TABLE_ID", "0x00");
        ReplaceItem("TableIdExtension", "TableIdExtension");
        ReplaceItem("PSI", "NO_PSI");
        ReplaceItem("SIG_Section", "NO_Section");
        ReplaceItem("OUTPUT_SIG", "");
      ReplaceItem("TDT",  "NO_TDT");
        break;
    case 1:
        ReplaceItem("NIT_BAT", "NIT");
        ReplaceItem("SDT", "SDT");
        ReplaceItem("NIT_BAT_TABLE_ID", "0x40");
        ReplaceItem("TableIdExtension", "NetworkId");
        ReplaceItem("PSI", "PSI");
        ReplaceItem("SIG_Section", "Section");
        ReplaceItem("OUTPUT_SIG", c->m_bInternalSig?"":"sig");
      ReplaceItem("TDT",  "TDT");
        break;
    case 2:
        ReplaceItem("NIT_BAT", "BAT");
        ReplaceItem("SDT", "SDT");
        ReplaceItem("NIT_BAT_TABLE_ID", "0x4A");
        ReplaceItem("TableIdExtension", "BouquetId");
        ReplaceItem("PSI", "PSI");
        ReplaceItem("SIG_Section", "Section");
        ReplaceItem("OUTPUT_SIG", c->m_bInternalSig?"":"sig");
      ReplaceItem("TDT",  "TDT");
        break;
    case 3:
        ReplaceItem("NIT_BAT", "NO_NIT_BAT");
        ReplaceItem("SDT", "NO_SDT");
        ReplaceItem("NIT_BAT_TABLE_ID", "0x00");
        ReplaceItem("TableIdExtension", "TableIdExtension");
        ReplaceItem("PSI", "PSI");
        ReplaceItem("SIG_Section", "Section");
        ReplaceItem("OUTPUT_SIG", "");
      ReplaceItem("TDT",  "TDT");
    }

    switch (c->m_iSAT_CABLE_TERR)
    {
    case 0: ReplaceItem("SATELLITE_DELIVERY_SYSTEM",   "SatelliteDeliverySystem");   break;
    case 1: ReplaceItem("CABLE_DELIVERY_SYSTEM",       "CableDeliverySystem");       break;
    case 2: ReplaceItem("TERRESTRIAL_DELIVERY_SYSTEM", "TerrestrialDeliverySystem"); break;
   }

    m_oConf.Replace("$SATELLITE_DELIVERY_SYSTEM$",   "NO_SatelliteDeliverySystem", true);
    m_oConf.Replace("$CABLE_DELIVERY_SYSTEM$",       "NO_CableDeliverySystem", true);
    m_oConf.Replace("$TERRESTRIAL_DELIVERY_SYSTEM$", "NO_TerrestrialDeliverySystem", true);

    if (! ReplaceItem("COMP_SW_MODEL_VERSION",(c-> m_iCompatibilitySW_Model<<16)|c->m_iCompatibilitySW_Version)) return false;

    switch (c->m_iCarouselType_DC_OC_GC)
    {
    case kMFRAMEi_CARO_TYPE_DATA_CAROUSEL:
      ReplaceItem("DATA_CAROUSEL_STD", "DataCarousel");
      break;
    case kMFRAMEi_CARO_TYPE_OBJECT_CAROUSEL:
      ReplaceItem("DATA_CAROUSEL_OC",  "DataCarousel");
      break;
    case kMFRAMEi_CARO_TYPE_GENIUS_CAROUSEL:
      ReplaceItem("DATA_CAROUSEL_GC",  "DataCarousel");
      break;
    }
    m_oConf.Replace("$DATA_CAROUSEL_STD$","NO_DataCarousel", true);
    m_oConf.Replace("$DATA_CAROUSEL_OC$", "NO_DataCarousel", true);
    m_oConf.Replace("$DATA_CAROUSEL_GC$", "NO_DataCarousel", true);

    switch (c->m_iTriggerType_UK_SSU_SSUE)
    {
    case 0:
      ReplaceItem("DataBroadcastId0111",  "DataBroadcastId");
      ReplaceItem("Linkage81",  "Linkage");
      ReplaceItem("UPDATE_ID_TYPE",  "0x1");
      if (! ReplaceItem("SSU", "NO_SSU")) return false;
      if (! ReplaceItem("UNT",   "NO_UNT")) return false;
      if (! ReplaceItem("PMT_SSU_ENHANCED", "NO_PMT_SSU_ENHANCED")) return false;
      if (! ReplaceItem("PMT_SSU_DTG", "PMT")) return false;
      if (! ReplaceItem("Action_Type_OUI_HASH", "")) return false;
      break;
    case 1:
      ReplaceItem("DataBroadcastId000A",  "DataBroadcastId");
      ReplaceItem("Linkage09",  "Linkage");
      ReplaceItem("UPDATE_ID_TYPE",  "0x1");
      if (! ReplaceItem("SSU", "NO_SSU")) return false;
      if (! ReplaceItem("UNT",   "NO_UNT")) return false;
      if (! ReplaceItem("PMT_SSU_ENHANCED", "NO_PMT_SSU_ENHANCED")) return false;
      if (! ReplaceItem("PMT_SSU_DTG", "PMT")) return false;
      if (! ReplaceItem("Action_Type_OUI_HASH", "")) return false;
      break;
    case 2:
      ReplaceItem("DataBroadcastId000A",  "DataBroadcastId");
      ReplaceItem("Linkage09",  "Linkage"); /* BELB Comment _ Not sure of this value */
      ReplaceItem("UPDATE_ID_TYPE",  "0x2");
      if (! ReplaceItem("SSU", "SSU")) return false;
      if (! ReplaceItem("UNT", "UNT")) return false;
      if (! ReplaceItem("PMT_SSU_ENHANCED", "PMT")) return false;
      if (! ReplaceItem("PMT_SSU_DTG", "NO_PMT_SSU_DTG")) return false;

      int iActionTypeOuiHash = MFi_SSUE_ComputeActionTypeOuiHash(c->m_iOUI);
      if (! ReplaceItem("Action_Type_OUI_HASH", iActionTypeOuiHash)) return false;
      break;
    }

    iSSUE_MJD_Start = (int) c->m_oSSUE_StartDiffusionDate.GetMJD();
   if (c->m_iTriggerType_UK_SSU_SSUE == 2)
   {
      /* SSU_Enhanced */
      int iSSUE_MJD_Stop = (int) iSSUE_MJD_Start + atoi(m_poManager->m_poValidatorManager->Find("SSUE_NbDayOfDiffsuion")->Get());

      ReplaceItem("SSUE_MJD_Start", iSSUE_MJD_Start);
      ReplaceItem("SSUE_MJD_End", iSSUE_MJD_Stop);

      ReplaceItem("SSUE_PID", (const char *)m_poManager->m_poValidatorManager->Find("SSUE_UNT_PID")->Get());

      unsigned char  ucStartHour;
      unsigned char  ucStopHour;

      MFi_SSUE_TranslateInHour(atoi(m_poManager->m_poValidatorManager->Find("SSUE_StartHourUser")->Get()),
                              (strcmp(m_poManager->m_poValidatorManager->Find("SSUE_StartHourAmPm")->Get(),"am")),
                              &ucStartHour);

      ReplaceItem("SSUE_StartHour",  ucStartHour);

      MFi_SSUE_TranslateInHour(atoi(m_poManager->m_poValidatorManager->Find("SSUE_StopHourUser")->Get()),
                              (strcmp(m_poManager->m_poValidatorManager->Find("SSUE_StopHourAmPm")->Get(),"am")),
                              &ucStopHour);

      ReplaceItem("SSUE_StopHour",  ucStopHour);


      unsigned short usDuration;
      MFi_SSUE_ComputeDuration(ucStartHour,
                              ucStopHour,
                              &usDuration);

      ReplaceItem("SSUE_Duration",  usDuration);  /* Duration of the download in hour */
      ReplaceItem("SSUE_Message", (const char *)m_poManager->m_poValidatorManager->Find("SSUE_AdditionMessage")->Get());
   }
   else
   {
      ReplaceItem("SSUE_PID", "0");

      /* Force the date MJD to be a valid one */
      ReplaceItem("SSUE_MJD_Start", iSSUE_MJD_Start);
      ReplaceItem("SSUE_MJD_End", "0");

      ReplaceItem("SSUE_StartHour",  "0");

      ReplaceItem("SSUE_StopHour",  "0");
      ReplaceItem("SSUE_Duration", "0");
      ReplaceItem("SSUE_Message", "");
   }

    m_oConf.Replace("$DataBroadcastId0111$", "NO_DataBroadcastId0111", true);
    m_oConf.Replace("$DataBroadcastId000A$", "NO_DataBroadcastId000A", true);
    m_oConf.Replace("$Linkage81$",           "NO_Linkage81", true);
    m_oConf.Replace("$Linkage09$",           "NO_Linkage09", true);

   if (c->m_iNO_NIT_BAT == 3)
   {
      if (! ReplaceItem("WITH_MUX", "NO_Mux")) return false;
      if (! ReplaceItem("NO_MUX",   "NO_Mux")) return false;
      if (! ReplaceItem("PAD_MUX",  "Mux")) return false;
   }
   else if (c->m_bWithPcr)
    {
        if (! ReplaceItem("WITH_MUX", "Mux")) return false;
        if (! ReplaceItem("NO_MUX",   "NO_Mux")) return false;
        if (! ReplaceItem("PAD_MUX",  "NO_Mux")) return false;
    }
    else
    {
        if (! ReplaceItem("WITH_MUX", "NO_Mux")) return false;
        if (! ReplaceItem("NO_MUX",   "Mux")) return false;
        if (! ReplaceItem("PAD_MUX",  "NO_Mux")) return false;
    }

   // Define images
   if (is_images_by_zone_id)
   {
      /* Define images by zone id (16 max) */
      if (!ReplaceDefinedImagesByZoneId(nb_images,iBlockSize)) return false;
   }
   else
   {
      /* Define images basic (8 max) */
      if (!ReplaceDefinedImagesBasic(nb_images,iBlockSize)) return false;
   }
   ReplaceItem("NumImages",nb_images);

   // Define additional streams in mux
   if (!ReplaceDefinedAddStreams(nb_add_streams)) return false;

   i = m_oConf.Find("$");
    if (i > 0)
    {
        j = i + 1;
        while (m_oConf[j] != 0 && m_oConf[j] != '$') j++;
        LOGE(L"Missing replacement of [%s]", m_oConf.Mid(i, j-i+1));
        return false;
    }

    return true;
}

bool dmsMain::ReplaceDefinedImagesBasic (int numImages, int blockSize)
{
    wxString img_filename;
    wxFFile  img_ffile;
   int      img_size,img_offset,img_partid,part_size;
   char     label[50];
   int      i = 0;

   for (i = 0; i < numImages; i++)
   {
      // Load file and get the length
      sprintf(label,"BinaryFile%d",i+1);
      img_filename = m_poManager->m_poValidatorManager->Find(label)->Get();
      if (!img_ffile.Open(img_filename, "r"))
      {
         LOGE(L"Error opening file %d [%s]",i+1,img_filename);
         return false;
      }

      img_size = (int)img_ffile.Length();
      img_ffile.Close();

#ifndef kTS_WRITER_IMAGE_SIZE_NOT_MULTIPLE_BLOCK_SIZE_ENABLE
      if ((img_size % blockSize) != 0)
      {
         LOGE(L"Error with file %d [%s] _ The length of the file (%d) is not a multiple of block size (%d)",
                  i+1,img_filename,imgSize,blockSize);
         return false;
      }
#endif

      // Flash location
      img_offset = 0;
      img_partid = 0;

      switch (g_poMainConf->m_iFLASH_LOCATION_TYPE)
      {
      case kMFRAMEi_FLASH_LOC_TYPE_LOGICAL_OFFSET:
         // Get the flash offset
         img_offset = g_poMainConf->m_tiImagesBasicFlashOffset[i];
         break;

      case kMFRAMEi_FLASH_LOC_TYPE_PARTITION_ID:
         // Check file size compatible with partition size
         part_size = GetPartitionSize(g_poMainConf->m_tiImagesBasicPartitionId[i]);

         if (part_size  == 0) return false;
         else if (part_size < img_size)
         {
            LOGE(L"Error with file %d [%s] _ Size of file (0x%x) greater than size of targetted "
                 "partition (partition 0x%x size=0x%x)",
                 i+1,img_filename,img_size,g_poMainConf->m_tiImagesBasicPartitionId[i],part_size);
            return false;
         }

         // Deduce the logical address (offset from begining of the flash)
         img_offset = GetPartitionLogicalAddr(g_poMainConf->m_tiImagesBasicPartitionId[i]);
         break;

      case kMFRAMEi_FLASH_LOC_TYPE_ICS_ID_NASC:
         switch (g_poMainConf->m_tiImagesBasicICSModuleId[i])
         {
         case kMFRAMEi_IMAGE_ICS_ID_DOWNLOADER:
            img_partid = kTS_WRITER_ICS_ID_DOWNLOADER;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EMS_CONFIG_FILE:
            img_partid = kTS_WRITER_ICS_ID_EMS_CONFIG_FILE;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_0:
            img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_0;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_1:
            img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_1;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_2:
            img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_2;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_0:
            img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_0;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_1:
            img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_1;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_2:
            img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_2;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_0:
            img_partid = kTS_WRITER_ICS_ID_EXE_ASSET_USER_0;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_1:
            img_partid = kTS_WRITER_ICS_ID_EXE_ASSET_USER_1;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_2:
            img_partid = kTS_WRITER_ICS_ID_EXE_ASSET_USER_2;
            break;
         case kMFRAMEi_IMAGE_ICS_ID_SSA_DA2:
            img_partid = kTS_WRITER_ICS_ID_SSA_DA2;
            break;

         default:
            LOGE(L"Error with file %d [%s] _ ICS Module %d UNSUPPORTED",
                 i+1,img_filename,g_poMainConf->m_tiImagesBasicICSModuleId[i]);
            return false;
         }
         break;

      case kMFRAMEi_FLASH_LOC_TYPE_NASC3_MODULE_ID:
         switch (g_poMainConf->m_tiImagesBasicNASC3ModuleId[i])
         {
         case kMFRAMEi_IMAGE_NASC3_ID_BOOTLOADER:
            img_partid = kTS_WRITER_NASC3_ID_BOOTLOADER;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_CONFIG_FILE:
            img_partid = kTS_WRITER_NASC3_ID_CONFIG_FILE;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_DOWNLOADER:
            img_partid = kTS_WRITER_NASC3_ID_DOWNLOADER;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_APPLI_KERNEL:
            img_partid = kTS_WRITER_NASC3_ID_APPLI_KERNEL;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_APPLI_MIDDLEWARE:
            img_partid = kTS_WRITER_NASC3_ID_APPLI_MIDDLEWARE;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_SERVICE_MODE:
            img_partid = kTS_WRITER_NASC3_ID_SERVICE_MODE;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_07:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_07;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_08:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_08;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_09:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_09;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_10:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_10;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_11:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_11;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_12:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_12;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_13:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_13;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_14:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_14;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_15:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_15;
            break;
         case kMFRAMEi_IMAGE_NASC3_ID_MODULE_16:
            img_partid = kTS_WRITER_NASC3_ID_MODULE_16;
            break;
         default:
            LOGE(L"Error with file %d [%s] _ NASC 3.0 Module %d UNSUPPORTED",
                 i+1,img_filename,g_poMainConf->m_tiImagesBasicNASC3ModuleId[i]);
            return false;
         }
         break;

      default:
         LOGE(L"Error with file %d [%s] _ Flash Location Type %d UNSUPPORTED",
              i+1,img_filename,g_poMainConf->m_iFLASH_LOCATION_TYPE);
         return false;
      }

      sprintf(label,"ImageSize%d",i+1);
        ReplaceItem(label,img_size);
      sprintf(label,"FlashOffset%d",i+1);
        ReplaceItem(label,img_offset);
      sprintf(label,"FlashPartId%d",i+1);
      ReplaceItem(label,img_partid);
      sprintf(label,"File%d",i+1);
      ReplaceItem(label,"File");
      sprintf(label,"BinFileName%d",i+1);
      ReplaceItem(label, (const char *)img_filename);
   }

   return true;
}

bool dmsMain::ReplaceDefinedImagesByZoneId (int numImages, int blockSize)
{
    wxString img_filename;
    wxFFile  img_ffile;
   int      img_size,img_partid;
   char     label[50];
   int      i = 0;

   for (i = 0; i < numImages; i++)
   {
      // Load file and get the length
      sprintf(label,"ibzBinaryFile%d",i+1);
      img_filename = m_poManager->m_poValidatorManager->Find(label)->Get();
      if (!img_ffile.Open(img_filename, "r"))
      {
         LOGE(L"Error opening file %d [%s]",i+1,img_filename);
         return false;
      }

      img_size = (int)img_ffile.Length();
      img_ffile.Close();

#ifndef kTS_WRITER_IMAGE_SIZE_NOT_MULTIPLE_BLOCK_SIZE_ENABLE
      if ((img_size % blockSize) != 0)
      {
         LOGE("Error with file %d [%s] _ The length of the file (%d) is not a multiple of block size (%d)",
                  i+1,img_filename,imgSize,blockSize);
         return false;
      }
#endif

      // Flash location by parition ID (and zone ID)
      img_partid = 0;
      switch (g_poMainConf->m_tiImagesByZidICSModuleId[i])
      {
      case kMFRAMEi_IMAGE_ICS_ID_DOWNLOADER:
         img_partid = kTS_WRITER_ICS_ID_DOWNLOADER;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EMS_CONFIG_FILE:
         /* Manage a configuration file ICS Id by country case Zone ID value */
         switch (g_poMainConf->m_tiImagesByZidZoneId[i])
         {
         case  0:
            img_partid = kTS_WRITER_ICS_ID_EMS_CONFIG_FILE;
            break;
         case  1:
         case  2:
         case  3:
         case  4:
         case  5:
         case  6:
         case  7:
         case  8:
         case  9:
         case 10:
         case 11:
         case 12:
         case 13:
         case 14:
         case 15:
            img_partid = 0x80 + g_poMainConf->m_tiImagesByZidZoneId[i];
            break;
         default:
            LOGE(L"Error with file %d [%s] _ Zone ID %d UNSUPPORTED",
                 i+1,img_filename,g_poMainConf->m_tiImagesByZidZoneId[i]);
            return false;
         }
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_0:
         img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_0;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_1:
         img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_1;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_USER_2:
         img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_2;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_0:
         img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_0;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_1:
         img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_1;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_APPLICATION_ENG_2:
         img_partid = kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_2;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_0:
         img_partid = kTS_WRITER_ICS_ID_EXE_ASSET_USER_0;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_1:
         img_partid = kTS_WRITER_ICS_ID_EXE_ASSET_USER_1;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_EXE_ASSET_USER_2:
         img_partid = kTS_WRITER_ICS_ID_EXE_ASSET_USER_2;
         break;
      case kMFRAMEi_IMAGE_ICS_ID_SSA_DA2:
         /* Manage a configuration file ICS Id by country case Zone ID value */
         switch (g_poMainConf->m_tiImagesByZidZoneId[i])
         {
         case  0:
            img_partid = kTS_WRITER_ICS_ID_SSA_DA2;
            break;
         case  1:
         case  2:
         case  3:
         case  4:
         case  5:
         case  6:
         case  7:
         case  8:
         case  9:
         case 10:
         case 11:
         case 12:
         case 13:
         case 14:
         case 15:
            img_partid = 0x90 + g_poMainConf->m_tiImagesByZidZoneId[i];
            break;
         default:
            LOGE(L"Error with file %d [%s] _ Zone ID %d UNSUPPORTED",
                 i+1,img_filename,g_poMainConf->m_tiImagesByZidZoneId[i]);
            return false;
         }
         break;

      default:
         LOGE(L"Error with file %d [%s] _ ICS Module %d UNSUPPORTED",
              i+1,img_filename,g_poMainConf->m_tiImagesByZidICSModuleId[i]);
         return false;
      }

      sprintf(label,"ImageSize%d",i+1);
        ReplaceItem(label,img_size);
      sprintf(label,"FlashOffset%d",i+1);
        ReplaceItem(label,0);
      sprintf(label,"FlashPartId%d",i+1);
      ReplaceItem(label,img_partid);
      sprintf(label,"File%d",i+1);
      ReplaceItem(label,"File");
      sprintf(label,"BinFileName%d",i+1);
      ReplaceItem(label, (const char *)img_filename);

   }

   return true;
}


void dmsMain::ReplaceUndefinedImages (int numImages)
{
   char label[50];
   int  i = numImages;

   while (i < kTS_WRITER_MAX_IMAGES_SUPPORTED)
   {
      i++;
      sprintf(label,"ImageSize%d",i);
      ReplaceItem(label,0);
      sprintf(label,"FlashOffset%d",i);
      ReplaceItem(label,0);
      sprintf(label,"FlashPartId%d",i);
      ReplaceItem(label,0);
      sprintf(label,"BinaryFile%d",i);
      ReplaceItem(label,"");
      sprintf(label,"File%d",i);
      ReplaceItem(label,"NO_File");
      sprintf(label,"BinFileName%d",i);
      ReplaceItem(label,"");
   }
}

bool dmsMain::ReplaceDefinedAddStreams (int numStreams)
{
    wxString filename;
    wxFFile  ffile;
   int      size;
   char     label[50];
   int      i = 0;

   while (i < numStreams)
   {
      i++;

      // Load file and get the length
      sprintf(label,"StreamFileName%d",i);
      filename = m_poManager->m_poValidatorManager->Find(label)->Get();
      if (!ffile.Open(filename, "r"))
      {
         LOGE(L"Error opening file %d [%s]",i,filename);
         return false;
      }

      size = (int)ffile.Length();
      ffile.Close();

      LOG0(L"Additional Stream %2d: '%s' size=%d",i,filename,size);

      sprintf(label,"MuxFile%d",i);
      ReplaceItem(label,"File");
      sprintf(label,"PMT_Stub_ES%d",i);
      ReplaceItem(label,"Stream");
   }

   return true;
}

void dmsMain::ReplaceUndefinedAddStreams (int numStreams)
{
   char label[50];
   int  i = numStreams;

   // Set stuffing stream in ES 1 if no additionnal stream
   if (numStreams < 1)
   {
      ReplaceItem("PCR_PID",     "0x1ffe");
      ReplaceItem("PMT_Stub_ES1","Stream");
      ReplaceItem("StreamType1", "0x01");
      ReplaceItem("StreamPID1",  "0x1ffb");
   }
   else
   {
      wxString p = m_poManager->m_poValidatorManager->Find("StreamPID1")->Get();
      ReplaceItem("PCR_PID", (const char *)p);
   }

   while (i < kTS_WRITER_MAX_MUX_ELEMENTARY_STREAMS)
   {
      i++;
      sprintf(label,"MuxFile%d",i);
      ReplaceItem(label,"NO_File");
      sprintf(label,"StreamFileName%d",i);
      ReplaceItem(label,"");
      sprintf(label,"StreamRate%d",i);
      ReplaceItem(label,"");
      sprintf(label,"StreamPID%d",i);
      ReplaceItem(label,"");
      sprintf(label,"StreamType%d",i);
      ReplaceItem(label,"");

      if (i > 0)
      {
         sprintf(label,"PMT_Stub_ES%d",i);
         ReplaceItem(label,"NO_Stream");
      }
   }
}

int dmsMain::GetPartitionSize (int partitionId)
{
   int i;

   for (i = 0; i < kTS_WRITER_MAX_PARTITION_PREDEF; i++)
   {
      if (partitionId == g_poMainConf->m_tiPartitionIdent[i])
      {
         return(g_poMainConf->m_tiPartitionSize[i]);
      }
   }

   LOGE(L"Unable to found PartitionId 0x%x in the PartitionId List defined ", partitionId);

   return(0);
}



int dmsMain::GetPartitionLogicalAddr (int partitionId)
{
   int i;
   int laddr = 0;

   for (i = 0; i < kTS_WRITER_MAX_PARTITION_PREDEF; i++)
   {
      if (partitionId == g_poMainConf->m_tiPartitionIdent[i])
      {
         return(laddr);
      }

      laddr += g_poMainConf->m_tiPartitionSize[i];
   }

   /* Shall never occurs ... */
   return(laddr);
}

bool dmsMain::ReplaceItem(const wxString &name, const char* value)
{
    if (m_oConf.Replace(STR("$%s$", name), value, true)==0)
    {
        LOGE(L"No replacement for [%s] (%s)", (const char *)name, value);
        return false;
    }

    return true;
}

bool dmsMain::ReplaceItem(const wxString &name, bool value)
{
    return ReplaceItem(name, (const char *)STR("%s", value?"true":"false"));
}

bool dmsMain::ReplaceItem(const wxString &name, int value)
{
    return ReplaceItem(name, (const char *)STR("0x%x", value));
}

/* MainFrame.cpp */
