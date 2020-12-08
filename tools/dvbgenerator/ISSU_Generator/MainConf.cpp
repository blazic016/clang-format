/****************************************************************************
** @file MainConf.cpp
**
** @brief
**   Management genius configuration file Profile.ini (loading and save).
**
** @ingroup GENIUS USER INTERFACE
**
** @see Profile.ini
**
** @version $Rev: 62361 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/soft/ISSU_Generator/MainConf.cpp $
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
**
******************************************************************************/

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <fstream>
#include <string>
#include <regex>
#include <map>

#include <Tools/Validator.h>


#include "MainConf.h"

dmsConfBase *g_poMainConf = NULL;


/* ========================================================================
   Constructeur/Destructeur
   ======================================================================== */


dmsConfBase::dmsConfBase()
{
	   int i;

	   m_iNO_NIT_BAT               = 0;
	   m_iSAT_CABLE_TERR           = 0;
	   m_iLoaderType               = 0;
	   m_iCarouselType_DC_OC_GC    = 0;
	   m_iTriggerType_UK_SSU_SSUE  = 0;
	   m_iCompatibilitySW_Model    = 0;
	   m_iCompatibilitySW_Version  = 0;
	   m_iProfile                  = 0;
	   m_bWithPcr                  = false;
	   m_bAutoSign                 = true;
	   m_oDiffusionDate            = wxDateTime::Today();
	   m_iProtocolId               = 0;
	   m_iModuleSize               = 0x40;
	   m_bDebug                    = false;
	   m_iFLASH_LOCATION_TYPE      = 0;
	   m_oSSUE_StartDiffusionDate  = wxDateTime::Today();
	   m_iModeFileSections         = 0;
	   m_bAddUsageIdFile           = false;
	   m_bSIG_UseGenericOUI        = false;

	   for (i = 0; i < kTS_WRITER_MAX_IMAGES_BASIC; i++)
	   {
	      m_tiImagesBasicFlashOffset[i]   = 0;
	      m_tiImagesBasicPartitionId[i]   = 0;
	      m_tiImagesBasicICSModuleId[i]   = 0;
	      m_tiImagesBasicNASC3ModuleId[i] = 0;
	   }

	   for (i = 0; i < kTS_WRITER_MAX_IMAGES_BY_ZONE_ID; i++)
	   {
	      m_tiImagesByZidICSModuleId[i] = 0;
	      m_tiImagesByZidZoneId[i]      = 0;
	   }

	   for (i = 0; i < kTS_WRITER_MAX_PARTITION_PREDEF; i++)
	   {
	      m_tiPartitionIdent[0] = 0;
	      m_tiPartitionSize[0]  = 0;
	   }

}


dmsConf::dmsConf() : dmsConfBase()
{
	 config = new wxFileConfig("", "", "Resources/Profile.ini", "", wxCONFIG_USE_RELATIVE_PATH);
}




dmsConfBase::~dmsConfBase()
{
}

dmsConf::~dmsConf()
{
    DELNUL(config);
}


/* ========================================================================
   Persistance
   ======================================================================== */


void dmsConf::Load()
{
    wxConfigBase* config = wxConfigBase::Get();

    config->Read("/Config/Profile", &m_iProfile, 0);

    m_bDebug = true;
}

void dmsConf::Save()
{
    wxConfigBase* config = wxConfigBase::Get();

    config->Write("/Config/Profile", m_iProfile);
}


void dmsConf::Load(const wxString &profil, dmsValidatorManager &manager)
{
    config->SetPath(STR("/Config/PROFILE_LIST/%s", profil));

    dmsValidator* map;
    wxString tmp;

    FOREACH(dmsValidatorList,
            manager.m_oList,
            map)
    {
        if (map->GetName() == "Profile") continue;

        config->Read(map->GetName(),
                     &tmp,
                     map->Get());

        map->SetNoError(tmp);
    }
}


void dmsConf::Save(const wxString &profil, dmsValidatorManager &manager)
{
    config->SetPath(STR("/Config/PROFILE_LIST/%s", profil));

    dmsValidator* map;

    FOREACH(dmsValidatorList,
            manager.m_oList,
            map)
    {
        if (map->GetName() == "Profile") continue;

        config->Write(map->GetName(),
                      map->Get());
    }

    config->Flush();
}


void dmsConf::RemoveProfile(const wxString &profil)
{
    config->DeleteGroup(STR("/Config/PROFILE_LIST/%s", profil));

    config->Flush();
}



void dmsConf::GetGroupList(wxArrayString &tab)
{
    wxString value;
    long index = 0;

    tab.Clear();

    config->SetPath("Config/PROFILE_LIST");

    for (bool cont = config->GetFirstGroup(value, index);
         cont;
         cont = config->GetNextGroup(value, index))
    {
        tab.Add(value);
    }
}




//dmsCliConf
dmsCliConf::dmsCliConf(const std::string& file) : dmsConfBase(), config_name(file)
{
	config.open(file);
	isFileOpened = config.is_open();
}

dmsCliConf::~dmsCliConf()
{
}


void dmsCliConf::Load()
{
	m_bDebug = true;
}


void dmsCliConf::Save()
{
	std::string file_name = config_name.substr(0, config_name.size()-5) + "_complete_configuration.conf";
	std::ofstream conf_file(file_name);

	for(auto const& pair: mapa)
	{
		conf_file<<pair.first<<"="<<pair.second<<std::endl;
	}

	conf_file.close();
}

void dmsCliConf::openConfig(std::string name)
{
	config.close();
	config_name = name;
	config.open(config_name);
	isFileOpened = config.is_open();
}

void dmsCliConf::Load(const wxString &profil, dmsValidatorManager &manager)
{
	std::string line;

	std::regex reg("([a-zA-Z0-9_]+)=(.*)");
	std::smatch match;


	while(
			std::getline(config, line) &&
			std::regex_match(line, reg)
		)
	{
		std::regex_search(line, match, reg);
		mapa[match[1]] = match[2];
	}

	dmsValidator* validator;

	FOREACH(dmsValidatorList,
	           manager.m_oList,
	           validator)
	{

		if (validator->GetName() == "Profile") continue;

		std::string name = std::string(validator->GetName());

		auto search = mapa.find(name);

		if(search != mapa.end())
		{
			validator->SetNoError(search->second.c_str());
			std::cout << name << std::string("=") << search->second.c_str() << std::endl;
		}
		else
		{
			validator->SetNoError(validator->Get().c_str());
			std::cout << name << std::string("=") << validator->Get().c_str() << std::endl;
		}

	 }
}


void dmsCliConf::Save(const wxString &profil, dmsValidatorManager &manager)
{
}


void dmsCliConf::RemoveProfile(const wxString &profil)
{
}



void dmsCliConf::GetGroupList(wxArrayString &tab)
{
}

bool dmsCliConf::isFileOpen()const {
	if(!isFileOpened){
		LOGE(L"File %s failed to open for loading configuration!!!", config_name.c_str());
		return false;
	}
	return true;
}


/* MainConf.cpp */
