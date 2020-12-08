/****************************************************************************
** @file GeniusImagesData.h
**
** @brief
**   Genius Carousel Images Data generation:
**   Get images list configuration in the TS writer XML configuration and
**   split images list of block (also named chunk) in the Genius proprieraty
**   carousel format.
**
**
** @ingroup GENIUS CAROUSEL
**
** @version $Rev: 62243 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/lib/GeniusCarousel/GeniusImagesData.h $
**          $Date: 2012-03-28 17:00:46 +0200 (mer., 28 mars 2012) $
**
** @author  SmarDTV Rennes - LIPPA
**
** COPYRIGHT:
**   2011 SmarDTV
**
** @history
**   LIPPA - SmarDTV - v 2.00 - 06/2011 - Refactoring module GeniusSplitTool
**                                        - cGeniusDataSplit_Multiple became
**                                          cGeniusImagesData.
**                                        - cGeniusDataSplit_Single not
**                                          implemented replaced by a
**                                          cGeniusImagesData with one image.
**                                        - cGeniusDataBlock_Multiple became
**                                          cGeniusImagesBlock.
**                                        - cGeniusDataBlock_Single not
**                                          implemented replaced by a
**                                          cGeniusImagesBlock with one image.
**   LIPPA - SmarDTV - v 2.20 - 03/2012 - Merge Telefonica functionality:
**                                        - Manage 16 images max
**                                        - Manage Zone ID with config file
**                                          and DA2
**                                        - Manage max 16 additionals software
**                                          compatibility descriptors.
**                                        Corrupt Chunk CRC option for QA Test
**                                        (enable only in version DEBUG).
**
******************************************************************************/

#ifndef _GENIUS_IMAGES_DATA_H
#define _GENIUS_IMAGES_DATA_H


/******************************************************************************
* Includes
******************************************************************************/

#include <DataCarousel/ModuleData.h>


/******************************************************************************
* Defines
******************************************************************************/

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Typedefs
******************************************************************************/

/******************************************************************************
* Public Class
******************************************************************************/

/**
 *  @brief
 *    Genius Images Splitter:
 *
 *    1 - Load (Init() Method) the of images list defined in the input XML
 *        in tag GeniusImagesData.
 *
 *        XML format:
 *          <GeniusImagesData BlockName        = "<format label block>"
 *                            BlockSize        = "<size of block in bytes>"
 *                            Output           = "<output path>"
 *                            UpdateId         = "<update id>"
 *                            SignType         = "<0|1|2>"
 *                            SignKeyFilename  = "<RSA key path>"
 *                            SignFilename     = "<signature file path>"
 *                            LocationType     = "<0|1>"
 *                            NbImages         = "<N in decimal>"
 *                            ImageSize<I>     = "<size in byte>"
 *                            FlashOffset<I>   = "<offset in flash>"
 *                            FlashPartId<I>   = "<partition id>">
 *            <File value="<image 1 path>"/>
 *            ...
 *            <File value="<image N path>"/>
 *            <NO_File value=""/>
 *            ...
 *            <NO_File value=""/>
 *          </GeniusImagesData>
 *
 *        Notes:
 *        - The Blockname format is a string with the %d to set the block
 *          number (ex: 51670001.%d)
 *        - If the signature type SignType is equal to
 *          kTS_WRITER_SIGNATURE_NONE (0) the field SignKeyFilename and
 *          SignFilename are not significants.
 *        - If the signature type SignType is equal to
 *          kTS_WRITER_SIGNATURE_AUTOMATIC (1) the field SignFilename is not
 *          significant and SignKeyFilename contain path to the key file to
 *          use to generate the RSA signature.
 *        - If the signature type SignType is equal to
 *          kTS_WRITER_SIGNATURE_FROMFILE (2) the field SignKeyFilename is not
 *          significant and SignFilename contain path to signature file to set.
 *        - I is in [1..N] (N=8 if image basic, N=16 if images by zone ID)
 *        - When I > N (NbImages) The ImageSize<I>, FlashOffset<I> and
 *          FlashPartId<I> are not significant (=0)
 *        - If the location type LocationType is equal to
 *          kTS_WRITER_IMG_LOCATION_BY_OFFSET (0) all images partition id
 *          FlashPartId<I> are not significant (=0).
 *        - If the location type LocationType is equal to
 *          kTS_WRITER_IMG_LOCATION_BY_PARTID (0) all images partition offset
 *          FlashOffset<I> are not significant (=0).
 *        - Use SignType to generate error for QA validation: Sign type is a
 *          32 bits with: the 16 bits b15..b0 the real sigignature type
 *          (0,1 or 2) and the 16 bits b31..b16 is the code to generate error
 *          in carousel: Here if > 0 contain the indice of image to corrupt
 *          with a bad CRC (1: image1, 2: image2, 3: image 1 and 2...)
 *
 *    2 - Split all images (CompileBuffer() method) in a list of block
 *        (cGeniusImagesBlock) with the size defined in the input XML (BlockSize).
 *
*/
class cGIDi_ImageProps;
class cGIDi_DirectoryModule;
class cGIDi_PayloadChunk;

class cGeniusImagesData : public dmsModuleData
{
public:
   /**
    *  @brief
    *    Genius Images loader/splitter contructor.
    *
    *  @param[in] pxoParent
    *       Pointer on parent data (memory implementation of the parent XML node)
    *
   */
   cGeniusImagesData (dmsModuleData *pxoParent);

   /**
    *  @brief
    *    Genius Images loader/splitter destructor: Clean the images table.
    *
   */
   virtual ~cGeniusImagesData ();

   /**
    *  @brief
    *    Loading and check the images data from the XML node
    *    <GeniusImagesData ...>...</GeniusImagesData>.
    *
    *  @param[in] pxoNode
    *       XML node 'GeniusImagesData' to load.
    *
    *  @retval true
    *       Loading successful.
    *
    *  @retval false
    *       Loading failure.
    *
    *  @remarks
    *    -# When error is detected a log is generated.
   */
    bool Init (wxXmlNode *pxoNode);

   /**
    *  @brief
    *    Split all images in block and add all block on the block list.
    *
    *  @pre
    *    The image file is previously loaded and concatened in order of
    *    image declaration in the internal buffer m_oBuffer.
    *
    *  @retval true
    *       Split successful.
    *
    *  @retval false
    *       Split failure.
    *
    *  @remarks
    *    -# When error is detected a log is generated.
   */
    bool CompileBuffer ();

   /**
    *  @brief
    *    Build (Create, generate and export) the block Directory Module (chunk 0).
    *
    *  @retval true
    *       Build successful.
    *
    *  @retval false
    *       Build failure.
    *
    *  @remarks
    *    -# When error is detected a log is generated.
   */
   bool CompileDirectoryModule ();

   /**
    *  @brief
    *    Generate the chunk info list in the Directory Module chunk 0.
    *
    *  @param[in/out] pxoDirModule
    *       Directory module where to store the chunk info list
    *
    *  @retval true
    *       Compilation successful.
    *
    *  @retval false
    *       Compilation failure.
    *
    *  @remarks
    *    -# When error is detected a log is generated.
   */
   bool CompileChunkInfoList (cGIDi_DirectoryModule *pxoDirModule);

   /**
    *  @brief
    *    Signature compilation.
    *
    *  @param[out] xoSignBuffer
    *       Buffer with signature computed
    *
    *  @retval true
    *       Signature generation successful.
    *
    *  @retval false
    *       Signature generation failure.
    *
    *  @remarks
    *    -# When error is detected a log is generated.
   */
   bool CompileSignature (dmsBuffer &xoSignBuffer);

   /**
    *  @brief
    *    Private attributes values.
    */
   int BlockSize      () { return m_iBlockSize; }
   u32 UpdateId       () { return m_u32UpdateId; }
   int NumberOfImages () { return m_iNumImages; }
   int NumberOfBlocks () { return (int)m_oBlockList.GetCount(); }

   /**
    * Public data necessary to parent and childs to manage the build traces
    * (Data loaded from input XML attribute of tag GeniusImagesData)
   */
   wxString          m_oBlockName;        // BlockName        = "<format label block>"
   wxString          m_oOutputDir;        // Output           = "<output path>"

private:
   /**
    * Private Data loaded from input XML attribute of tag GeniusImagesData
   */
   int               m_iBlockSize;          // BlockSize        = "<size of block in bytes>"
   u32               m_u32UpdateId;         // UpdateId         = "<update id>"
   int               m_iSignType;           // SignType         = "<0|1|2>"
   wxString          m_oSignKeyFilename;    // SignKeyFilename  = "<RSA key path>"
   wxString          m_oSignFilename;       // SignFilename     = "<signature file path>"
   int               m_iLocationType;       // LocationType     = "<0|1>"
   int               m_iNumImages;          // NbImages         = "<N in decimal>"
   cGIDi_ImageProps *m_poImagesProps;       // Table of (ImageSize<I>,FlashOffset<I>,FlashPartId<I>)
#ifdef _DEBUG
   u16               m_u16Veroler;          // Volontary error generator
#endif

   /**
    * Evaluated data
   */
   u32               m_u32LowerFlashOffset; // Lower image address
    dmsModuleDataList m_oBlockList;          // Result of split
};


/**
 *  @brief
 *    Block of a Genius Images generated by the previous class in the splitter
 *    step.
 *
 *    This block are the implementation memory of the genius proprietary
 *    carousel generated the CompileBuffer and Export methods.
 *
 *    There 2 types of block (named Chunk in the FRS) with the following
 *    format:
 *
 *     - Chunk 0 carries the Directory Module:
 *       Syntax                           No. of bits
 *       CarouselDirectoryModule() {
 *         UpdateID                                32
 *         LowerFlashOffset                        32
 *         Version                                 32
 *         ChunkCount                              32
 *         for (i = 0; i < ChunkCount; i++) {
 *            PartitionID                          16
 *            ChunkID                              16
 *            ChunkSize                            32
 *         }
 *         SignatureSize                           16
 *         for (i = 0; i < SignatureSize; i++) {
 *            SignatureByte                         8
 *         }
 *         ChunkCRC32                              32
 *       }
 *
 *     - Chunk 1..N carry pieces of binary images:
 *       Syntax                           No. of bits
 *       CarouselPayloadChunk() {
 *         ChunkID                                 16
 *         PayloadSize                             32
 *         PayloadCRC32                            32
 *         PayloadFlashOffset                      32
 *         for (i = 0; i < PayloadSize; i++) {
 *            PayloadByte                           8
 *         }
 *         ChunkCRC32                              32
 *       }
 *
 *     Notes:
 *     - In this version the field Version = 0x00020000 (v 2.0.0)
 *     - All block (address + size) of the same image are contiguous and
 *       have the same partition ID
 *     - If the playload flash offset of the chunks are not defined
 *       (=0xFFFFFFFF), the partition ID defined in the
 *       CarouselDirectoryModule() is the ICS NASC id the input point
 *       of flash mapping.
 *     - If the playload flash offset of the chunks are defined,
 *       the partition ID defined in the CarouselDirectoryModule() is
 *       an image index of images sources
 *     - In Generation NASC mode, the signature is not calculated (stored in
 *       the input binary of the images) but the field is present in the
 *       CarouselDirectoryModule() with a size = 128 and all bytes = 0x20.
 *
*/
class cGeniusImagesBlock : public dmsModuleData
{
public:
   /**
    *  @brief
    *    Genius Images block contructor.
    *
    *  @param[in] pxoParent
    *       Pointer on parent data (the splitter)
    *
    *  @param[in] xiNumber
    *       Block number (0: Chunk0 = Directory Module,
    *       > 0: ChunkN = PayloadChunk)
    *
   */
   cGeniusImagesBlock  (cGeniusImagesData *pxoParent, int xiNumber);

   /**
    *  @brief
    *    Genius Images block  destructor: Clean the current chun
    k.
    *
   */
   virtual ~cGeniusImagesBlock ();


   /**
    *  @brief
    *    Set flash location of the block.
    *
    *  @param[in] xu32Offset
    *       Offset in flash of the block (undefined value 0xFFFFFFFF
    *       if location by partition ID).
    *
    *  @param[in] xu16PartID
    *       Partition ID of the block (Given by xml if location by
    *       partition ID else the image index).
    *
    *  @param[in] xu32Size
    *       Block size (Payload chunk size).
    *
    *  @remarks
    *    -# Not used on the first block (chunk 0).
   */
  void SetLocation (u32 xu32Offset, u16 xu16PartID, u32 xu32Size);

   /**
    *  @brief
    *    Generate the genius proprietary carousel chunk.
    *
    *    The generation of the Directory Module (block 0) is reported after
    *    the processing of all Payload Chunks (block 1..N) and it is managed
    *    by the image splitter in special class cGIDi_BlockDirModule.
    *
    *    Compilation Payload Chunk:
    *    - Creation chunk and set data
    *    - Build the corresponding buffer
    *    - Save it on disk
    *
    *  @retval true
    *       Compilation successful.
    *
    *  @retval false
    *       Compilation failure.
    *
    *  @remarks
    *    -# When error is detected a log is generated.
   */
   bool CompileBuffer ();

#ifdef _DEBUG
  /**
    *  @brief
    *    Set the flag to generate an error in CRC
    */
   void SetVirus () { m_bVerolCRC = true; }
#endif

   /**
    *  @brief
    *    Private attributes values.
    */
   int  Number      () { return m_iNumber; }
   u32  FlashOffset () { return m_u32FlashOffset; }
   u16  PartitionID () { return m_u16PartitionID; }
   u32  Size        () { return m_u32PayloadSize; }

protected:
   int                m_iNumber;
   u32                m_u32FlashOffset;
   u16                m_u16PartitionID;
   u32                m_u32PayloadSize;
    cGeniusImagesData *m_poImagesSplitter;
    dmsData           *m_poChunk;
#ifdef _DEBUG
   bool               m_bVerolCRC;
#endif

   /**
    *  @brief
    *    Save on disk the content of the block and create the
    *    corresponding trace file.
    *
    *  @retval true
    *       Export successful.
    *
    *  @retval false
    *       Export failure.
    *
   */
   bool Export ();

   /**
    *  @brief
    *    Return the name of trace file
    *
   */
    wxString TraceName ();
};

#endif /* _GENIUS_IMAGES_DATA_H */

/* GeniusImagesData.h */

