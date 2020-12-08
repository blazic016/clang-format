/****************************************************************************
** @file TS_WriterVersion.h
**
** @brief
**   Genius Core Generator TS Writer Version definition and
**   functionnality/option setting.
**
**   Note: the Version in the interger form on 4 bytes is the version set
**         on the header of data generated (chunk=module=block 0).
**         This version have the followinf format:
**         0xVVVVRRSS noted in ascci "V.R.S"
**         Example the version 5.23.12 is in integer 0x000517C
**
** @ingroup TS WRITER
**
** @see
**
** @version $Rev: 62361 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/lib/TS_Writer/TS_WriterVersion.h $
**          $Date: 2013-03-11 16:06:08 +0100 (lun., 11 mars 2013) $
**
** @author  SmarDTV Rennes - LIPPA
**
** COPYRIGHT:
**   2011 SmarDTV
**
** @history
**   LIPPA - SmarDTV - v 2.00 - 05/2011 - Creation
**   LIPPA - SmarDTV - v 2.00 - 10/2011 - Support 2 PMT and 2 data Carousel
**                                      - Add Service List Descriptor (tag
**                                        0x41) and the specific Nagra
**                                        Channel Descriptor (tag 0x82)
**                                      - New tab 'Additional Streams' to
**                                        inject streams audio and video in
**                                        stream generated.
**   LIPPA - SmarDTV - v 2.10 - 12/2011 - Add DSI Subdescriptor Update ID
**                                        and Usage ID
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
**   LIPPA - SmarDTV - v 2.33 - 01/2014 - Hardware and Product Versions became
**                                        a mask on target versions to support
**                                        several versions in same carousel.
**                                        Suppress the NASC 3 module ID 4
**                                        'application kernel'
**
******************************************************************************/


#ifndef _TS_WRITER_VERSION_H_
#define _TS_WRITER_VERSION_H_

/**
 *  @brief
 *    Printable version of TS_Writer
 *    Current is the 2.3.0.
*/
#define kTS_WRITER_VERSION_STRING     "3.0.0"

/**
 *  @brief
 *    Versionning. Version code registered in header of chunk generated.
 *    = version of SmartDTV carousel Image
 *    Current is the 2.0.0.
*/
#define kTS_WRITER_STVD_CARO_VERSION_VALUE   0x00020000


/**
 *  @brief
 *    Indicate the TS Writer supported image (partition) with size not
 *    multiple of block size.
*/
#define kTS_WRITER_IMAGE_SIZE_NOT_MULTIPLE_BLOCK_SIZE_ENABLE   1

/**
 *  @brief
 *    - Maximum number of images basic (partition) supported.
 *    - Maximum number of images by zone ID (partition) supported.
 *    - Maximum images defined in TS Writer XML max of the 2 previous max.
*/
#define kTS_WRITER_MAX_IMAGES_BASIC        8
#define kTS_WRITER_MAX_IMAGES_BY_ZONE_ID  16
#define kTS_WRITER_MAX_IMAGES_SUPPORTED   16

/**
 *  @brief
 *    Maximum number of pre-defined partition supported.
*/
#define kTS_WRITER_MAX_PARTITION_PREDEF  8

/**
 *  @brief
 *    Maximum number of additionnal Software Compatibility Descriptor supported.
*/
#define kTS_WRITER_MAX_ADD_SW_COMPATIBILITY_DESC  16

/**
 *  @brief
 *    Signature type enable
 *    Define in Integer to generator and string to generate input XML in UI manager
*/
#define kTS_WRITER_SIGNATURE_NONE                0
#define kTS_WRITER_SIGNATURE_AUTOMATIC           1
#define kTS_WRITER_SIGNATURE_FROMFILE            2
#define kTS_WRITER_SIGNATURE_NONE_STRING        "0"
#define kTS_WRITER_SIGNATURE_AUTOMATIC_STRING   "1"
#define kTS_WRITER_SIGNATURE_FROMFILE_STRING    "2"

/**
 *  @brief
 *    Image location type
 *    Define in Integer to generator and string to generate input XML in UI manager
*/
#define kTS_WRITER_IMG_LOCATION_BY_OFFSET          0
#define kTS_WRITER_IMG_LOCATION_BY_PARTID          1
#define kTS_WRITER_IMG_LOCATION_BY_OFFSET_STRING  "0"
#define kTS_WRITER_IMG_LOCATION_BY_PARTID_STRING  "1"

/**
 *  @brief
 *    ICS Module Identifiers values: The SSA library uses some unique identifiers
 *    for Application Modules. These identifiers are used to build the SBP
 *    proprietary section.
 *    Define only the ICS downloadable => not define:
 *    - kTS_WRITER_ICS_ID_FACTORY_DOWNLOADER (1)
 *    - kTS_WRITER_ICS_ID_FACTORY_CONFIG_FILE (2)
 *    - kTS_WRITER_ICS_ID_EMS_REDUNDANT_CONFIG_FILE (5)
 *
*/
#define kTS_WRITER_ICS_ID_DOWNLOADER              0x0003
#define kTS_WRITER_ICS_ID_EMS_CONFIG_FILE         0x0004
#define kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_0  0x0006
#define kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_1  0x0007
#define kTS_WRITER_ICS_ID_EXE_APPLICATION_USER_2  0x0008
#define kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_0   0x0009
#define kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_1   0x000A
#define kTS_WRITER_ICS_ID_EXE_APPLICATION_ENG_2   0x000B
#define kTS_WRITER_ICS_ID_EXE_ASSET_USER_0        0x000C
#define kTS_WRITER_ICS_ID_EXE_ASSET_USER_1        0x000D
#define kTS_WRITER_ICS_ID_EXE_ASSET_USER_2        0x000E
#define kTS_WRITER_ICS_ID_SSA_DA2                 0x0090

/**
 *  @brief
 *    Module NASC 3.0 Identifiers values
 *    Define the Id defined in enum tSDTVD_NASC_CF_ModuleId of
 *    nasc driver (sdtvdnasc.h) except the module not downloadable:
 *    - USB_DOWNLOADER (0x06)
 *    - FACTORY_BOOTLOADER (0xF0)
 *    - FACTORY_CONFIG_FILE (0xF1)
 *    - FACTORY_DOWNLOADER (0xF2)
 *
 *    + Add some Module Id to have a genius quickly open to other ids.
 *
*/
#define kTS_WRITER_NASC3_ID_BOOTLOADER              0x0000
#define kTS_WRITER_NASC3_ID_CONFIG_FILE             0x0001
#define kTS_WRITER_NASC3_ID_DOWNLOADER              0x0002
#define kTS_WRITER_NASC3_ID_APPLI_KERNEL            0x0003
#define kTS_WRITER_NASC3_ID_APPLI_MIDDLEWARE        0x0004
#define kTS_WRITER_NASC3_ID_SERVICE_MODE            0x0005
#define kTS_WRITER_NASC3_ID_MODULE_07               0x0007
#define kTS_WRITER_NASC3_ID_MODULE_08               0x0008
#define kTS_WRITER_NASC3_ID_MODULE_09               0x0009
#define kTS_WRITER_NASC3_ID_MODULE_10               0x000A
#define kTS_WRITER_NASC3_ID_MODULE_11               0x000B
#define kTS_WRITER_NASC3_ID_MODULE_12               0x000C
#define kTS_WRITER_NASC3_ID_MODULE_13               0x000D
#define kTS_WRITER_NASC3_ID_MODULE_14               0x000E
#define kTS_WRITER_NASC3_ID_MODULE_15               0x000F
#define kTS_WRITER_NASC3_ID_MODULE_16               0x0010


/**
 *  @brief
 *    Maximum number of additional elementary streams in mux supported (external file).
*/
#define kTS_WRITER_MAX_MUX_ELEMENTARY_STREAMS          8

/**
 *  @brief
 *    Default stuffing PID.
*/
#define kTS_WRITER_DEFAULT_STUFFING_PID           0x1FFE

/**
 *  @brief
 *    Loader type.
*/
#define kTS_WRITER_LOADER_TYPE_DOWNLOADER_BASIC        0
#define kTS_WRITER_LOADER_TYPE_EMBEDDED_LOADER         1
#define kTS_WRITER_LOADER_TYPE_DOWNLOADER_NASC         2

/**
 *  @brief
 *    Sections File Generation method.
*/
#define kTS_WRITER_SEC_FILE_MODE_NOT_GENERATED         0
#define kTS_WRITER_SEC_FILE_MODE_ADD_SIGNAL_TABLE      1
#define kTS_WRITER_SEC_FILE_MODE_USB_HEADER            2
#define kTS_WRITER_SEC_FILE_MODE_ONLY_SECTIONS         3

/**
 *  @brief
 *    DVB Generic OUI value for all OUIs
*/
#define kTS_WRITER_DVB_GENERIC_OUI              0x00015A


#endif /* _TS_WRITER_VERSION_H_ */

/* TS_WriterVersion.h */
