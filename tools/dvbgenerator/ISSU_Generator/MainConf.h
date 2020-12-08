/****************************************************************************
** @file MainConf.h
**
** @brief
**   Management genius configuration file Profile.ini (loading and save).
**
** @ingroup GENIUS USER INTERFACE
**
** @see Profile.ini
**
** @version $Rev: 62361 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/soft/ISSU_Generator/MainConf.h $
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
**   LIPPA - SmarDTV - v 2.10 - 12/2011 - Add DSI Subdescriptor Update ID
**                                        and Usage ID
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
**
******************************************************************************/

#ifndef _MAIN_CONF_H_
#define _MAIN_CONF_H_

#include <map>
#include <fstream>

#include <wx/wx.h>

#include <Tools/Tools.h>

#include <TS_Writer/TS_WriterVersion.h>

class wxFileConfig;


class dmsValidator;

/**
    \brief Configuration. Sauvée dans "profile.ini".
*/
//namespace std{
//template<
//    class CharT,
//    class Traits
//	> class basic_fstream;
//typedef basic_fstream<char> fstream;
//}

class dmsConfBase {
public:
	   dmsConfBase();
	   virtual ~dmsConfBase();
	   int           m_iProfile;
	   bool          m_bDebug;
	   wxString      m_oOutputDir;
	   wxString      m_oExtFileOAD;
	   wxString      m_oExtFileSections;
	   int           m_iModeFileSections;
	   int           m_iNO_NIT_BAT;
	   int           m_iSAT_CABLE_TERR;
	   int           m_iLoaderType;
	   int           m_iCarouselType_DC_OC_GC;
	   int           m_iTriggerType_UK_SSU_SSUE;
	   int           m_iFLASH_LOCATION_TYPE;
	   int           m_iCompatibilitySW_Model;
	   int           m_iCompatibilitySW_Version;
	   bool          m_bInternalSig;
	   bool          m_bWithPcr;
	   bool          m_bAutoSign;
	   wxDateTime    m_oDiffusionDate;
	   int           m_iProtocolId;
	   int           m_iModuleSize;
	   wxDateTime    m_oSSUE_StartDiffusionDate;
	   int           m_iOUI;
	   bool          m_bAddUsageIdFile;
	   /* Lippa v 2.0.0 Images Basic Mapping */
	   int           m_tiImagesBasicFlashOffset[kTS_WRITER_MAX_IMAGES_BASIC];
	   int           m_tiImagesBasicPartitionId[kTS_WRITER_MAX_IMAGES_BASIC];
	   int           m_tiImagesBasicICSModuleId[kTS_WRITER_MAX_IMAGES_BASIC];
	   /* Lippa v 2.2.0 Images by Zone ID Mapping (add Zone ID of config file and DA2) */
	   int           m_tiImagesByZidICSModuleId[kTS_WRITER_MAX_IMAGES_BY_ZONE_ID];
	   int           m_tiImagesByZidZoneId[kTS_WRITER_MAX_IMAGES_BY_ZONE_ID];
	   /* Lippa v 2.3.0 Flag to use DBV generic OUI */
	   bool          m_bSIG_UseGenericOUI;
	   /* Lippa v 2.3.1 Images Basic by NASC 3.0 Module ID Mapping */
	   int           m_tiImagesBasicNASC3ModuleId[kTS_WRITER_MAX_IMAGES_BASIC];

	   /* BELB 06/08/08 _ Add partitionId management for the 8 possible partitions */
	   int           m_tiPartitionIdent[kTS_WRITER_MAX_PARTITION_PREDEF];
	   int           m_tiPartitionSize[kTS_WRITER_MAX_PARTITION_PREDEF];
	   dmsValidatorManager* m_poValidManager;

	   virtual void Load          () = 0;
	   virtual void Save          () = 0;
	   virtual void Load          (const wxString &profil, dmsValidatorManager &manager) = 0;
	   virtual void Save          (const wxString &profil, dmsValidatorManager &manager) = 0;
	   virtual void RemoveProfile (const wxString &profil) = 0;
	   virtual void GetGroupList  (wxArrayString &tab) = 0;
};


class dmsConf : public dmsConfBase
{
public:
   dmsConf();
   virtual ~dmsConf();

    void Load          () override;
    void Save          () override;
    void Load          (const wxString &profil, dmsValidatorManager &manager) override;
    void Save          (const wxString &profil, dmsValidatorManager &manager) override;
    void RemoveProfile (const wxString &profil) override;
    void GetGroupList  (wxArrayString &tab) override;

   wxFileConfig *config;

};


class dmsCliConf : public dmsConfBase
{
private:
    bool isFileOpened = false;
    std::string config_name;
    std::map<std::string, std::string> mapa;

public:
    dmsCliConf(const std::string& file = std::string("Default.conf"));
    virtual ~dmsCliConf();

    std::fstream config;
    void Load          () override;
    void Save          () override;
    void Load          (const wxString &profil, dmsValidatorManager &manager) override;
    void Save          (const wxString &profil, dmsValidatorManager &manager) override;
    void RemoveProfile (const wxString &profil) override;
    void GetGroupList  (wxArrayString &tab) override;
    bool isFileOpen	   () const;
    void openConfig    (std::string name);
};



extern dmsConfBase *g_poMainConf;



#endif /* _MAIN_CONF_H_ */

/* MainConf.h */
