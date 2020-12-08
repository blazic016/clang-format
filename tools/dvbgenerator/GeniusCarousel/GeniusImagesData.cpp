/****************************************************************************
** @file GeniusImagesData.cpp
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
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/lib/GeniusCarousel/GeniusImagesData.cpp $
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


/******************************************************************************
* Includes
******************************************************************************/

#include <wx/filename.h>

#include <Tools/Xml.h>
#include <TS_Writer/TS_WriterVersion.h>
#include <DataCarousel/Module.h>
#include <DataCarousel/DataCarousel.h>

#include "GeniusImagesData.h"
#include "GeniusSignTool.h"


/******************************************************************************
* Defines
******************************************************************************/

#define kGIDi_OFFSET_UNDEF    0xFFFFFFFF
#define kGIDi_PARTID_UNDEF        0xFFFF


/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Typedefs
******************************************************************************/

/******************************************************************************
* Private Class
******************************************************************************/

/**
 *  @brief
 *    Images property (size, offset, partition id) to manage the dynamic table
 *    of images in the images splitter cGeniusImagesData.
 *
*/
class cGIDi_ImageProps
{
public:
   cGIDi_ImageProps ()
   {
      Set(kGIDi_OFFSET_UNDEF,kGIDi_PARTID_UNDEF,0);
   }
   cGIDi_ImageProps (u32 xu32Offset, u16 xu16PartID, u32 xu32Size)
   {
      Set(xu32Offset,xu16PartID,xu32Size);
   }

   void Set (u32 xu32Offset, u16 xu16PartID, u32 xu32Size)
   {
      m_u32FlashOffset = xu32Offset;
      m_u16PartitionID = xu16PartID;
      m_u32Size        = xu32Size;
   }

   u32 FlashOffset () { return m_u32FlashOffset; }
   u16 PartitionID () { return m_u16PartitionID; }
   u32 Size        () { return m_u32Size; }

private:
   u32 m_u32FlashOffset;
   u16 m_u16PartitionID;
   u32 m_u32Size;
};

/**
 *  @brief
 *    Memory implementation of Chunk 0 to carried the following
 *    Directory Module:
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
*/
// - The Chunk information
class cGIDi_ChunkInfo : public dmsData
{
public:
    u16 PartitionID;
    u16 ChunkID;
    u32 ChunkSize;

public:
   cGIDi_ChunkInfo()
    {
      HDR_INIT(PartitionID,16);
      HDR_INIT(ChunkID,    16);
      HDR_INIT(ChunkSize,  32);
      SetName("ChunkInfo");
   }
};

// - The Chunks informations table (
HDR_DEFINE_LIST(cGIDi_ChunkInfo);

// - The Directory module (chunk 0)
class cGIDi_DirectoryModule : public dmsData
{
public:
   u32                  UpdateID;
    u32                  LowerFlashOffset;
    u32                  Version;
    u32                  ChunkCount;
    cGIDi_ChunkInfoList *ChunkInfoList;
    u16                  SignatureSize;
    dmsData             *SignatureByte;
    u32                  ChunkCRC32;

public:
   cGIDi_DirectoryModule()
    {
      HDR_INIT(UpdateID,        32);
        HDR_INIT(LowerFlashOffset,32);
        HDR_INIT(Version,         32);
        HDR_INIT(ChunkCount,      32);
        HDR_INIT(ChunkInfoList,    0);
        HDR_INIT(SignatureSize,   16);
        HDR_INIT(SignatureByte,    0);
        HDR_INIT(ChunkCRC32,      32);

        ChunkInfoList = new cGIDi_ChunkInfoList();
        SignatureByte = new dmsData();
        SetListCount(&ChunkCount,ChunkInfoList);
        SetLenLimit(&SignatureSize,&SignatureByte);
    }
};

/**
 *  @brief
 *    Memory implementation of Chunk 1..N to carried the block (part of
 *    binary images splitted) with the followinf format:
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
*/
class cGIDi_PayloadChunk : public dmsData
{
public:
    u16      ChunkID;
    u32      PayloadSize;
    u32      PayloadCRC32;
    u32      PayloadFlashOffset;
    dmsData *PayloadByte;
    u32      ChunkCRC32;

public:
    cGIDi_PayloadChunk()
    {
        HDR_INIT(ChunkID,           16);
        HDR_INIT(PayloadSize,       32);
        HDR_INIT(PayloadCRC32,      32);
        HDR_INIT(PayloadFlashOffset,32);
        HDR_INIT(PayloadByte,        0);
        HDR_INIT(ChunkCRC32,        32);
        PayloadByte = new dmsData();
    }
};

/**
 *  @brief
 *    Special Block 0 of a Genius Images generated by the images
 *    splitter class in the splitter step to generate the chunk 0
 *    type CarouselDirectoryModule() at the end of generation of all
 *    all chunk 1..N  type CarouselPayloadChunk().
 *
*/
class cGIDi_BlockDirModule : public cGeniusImagesBlock
{
public:
   cGIDi_BlockDirModule (cGeniusImagesData *pxoParent) : cGeniusImagesBlock(pxoParent,0)
   {
   }

   bool CompileBuffer (u32 xu32UpdateId, u32 xu32LowerOffset);
};


/******************************************************************************
* Public Class Implementation
******************************************************************************/


/******************************************************************************
* Public Class Implementation
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

/**
 *  @brief
 *    Genius Images loader/splitter contructor.
 *
 *  @param[in] pxoParent
 *       Pointer on parent data (memory implementation of the parent XML node)
 *
*/
cGeniusImagesData::cGeniusImagesData (dmsModuleData *pxoParent): dmsModuleData(pxoParent,MODULE_DATA_TYPE_GENIUS_IMAGES_DATA)
{
   m_iNumImages    = 0;
   m_poImagesProps = NULL;
}

/**
 *  @brief
 *    Genius Images loader/splitter destructor: Clean the images table.
 *
*/
cGeniusImagesData::~cGeniusImagesData ()
{
#ifdef _DEBUG
   LOG0(L"[LIPPA-DBG] cGeniusImagesData: Delete the %d images splitter",
        m_iNumImages);
#endif
   if (m_poImagesProps != NULL) delete[] m_poImagesProps;
}

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
bool cGeniusImagesData::Init(wxXmlNode *pxoNode)
{
   u32  offset,size;
   u16  part_id;
   int  i;
   char label[50];
#ifdef _DEBUG
   u32  val32;
#endif

    pxoNode->Read("BlockName",      &m_oBlockName);
    pxoNode->Read("BlockSize",      &m_iBlockSize);
    pxoNode->Read("Output?",        &m_oOutputDir);
    pxoNode->Read("UpdateId",       &m_u32UpdateId);
#ifdef _DEBUG
   pxoNode->Read("SignType",       &val32);
   m_iSignType  = (int)(val32 & 0x0000FFFF);
   m_u16Veroler = (u16)((val32 & 0xFFFF0000) >> 16);
#else
   pxoNode->Read("SignType",       &m_iSignType);
#endif
    pxoNode->Read("SignKeyFilename",&m_oSignKeyFilename);
    pxoNode->Read("SignFilename?",  &m_oSignFilename);
   pxoNode->Read("LocationType",   &m_iLocationType);
    pxoNode->Read("NbImages",       &m_iNumImages);

   if (m_iNumImages < 1)
   {
      LOGE(L"ERROR: No images defined (images number=%d)\n",m_iNumImages);
      return false;
   }

   m_poImagesProps = new cGIDi_ImageProps[m_iNumImages];
   if (m_poImagesProps == NULL)
   {
      LOGE(L"ERROR: Create %d images failure\n",m_iNumImages);
      m_iNumImages = 0;
      return false;
   }

   // Load the images
   if (m_iLocationType == kTS_WRITER_IMG_LOCATION_BY_PARTID)
      m_u32LowerFlashOffset = 0;
   else
      m_u32LowerFlashOffset = 0xFFFFFFFF;

   for (i = 0; i < m_iNumImages; i++)
   {
      sprintf(label,"ImageSize%d",i+1);
      pxoNode->Read(label,&size);

      if (m_iLocationType == kTS_WRITER_IMG_LOCATION_BY_PARTID)
      {
         sprintf(label,"FlashPartId%d",i+1);
         pxoNode->Read(label,&part_id);
         offset = kGIDi_OFFSET_UNDEF;
      }
      else
      {
         sprintf(label,"FlashOffset%d",i+1);
         pxoNode->Read(label,&offset);
         part_id = kGIDi_PARTID_UNDEF;
         if (offset < m_u32LowerFlashOffset) m_u32LowerFlashOffset = offset;
      }

      m_poImagesProps[i].Set(offset,part_id,size);
   }

#ifdef _DEBUG_TROP_BAVARD
   if (m_iLocationType == kTS_WRITER_IMG_LOCATION_BY_PARTID)
      LOG0(L"[LIPPA-DBG] GID Init(): %d images loc by PartitionID (@lowerFlash=0x%08X)",
           m_iNumImages,m_u32LowerFlashOffset);
   else
      LOG0(L"[LIPPA-DBG] GID Init(): %d images loc by Flash Offset (@lowerFlash=0x%08X)",
           m_iNumImages,m_u32LowerFlashOffset);
   for (i = 0; i < m_iNumImages; i++)
   {
      LOG0(L"%2d - @offset=0x%08X part_id=0x%04X size=0x%08X (%lu bytes)",
           i,m_poImagesProps[i].FlashOffset(),
           m_poImagesProps[i].PartitionID(),
           m_poImagesProps[i].Size(),
           m_poImagesProps[i].Size());
   }
#endif

   return true;
}

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
bool cGeniusImagesData::CompileBuffer ()
{
   cGIDi_BlockDirModule *p_b0;
   cGeniusImagesBlock   *p_block;
   int                   n_block,i;
   u32                   buff_offset,buff_rest;
   u32                   image_rest,block_foff,block_size;
#ifdef _DEBUG
   u16                   bit_image = 1;
   bool                  set_virus;
#endif

   // Format block name format if not formated
    if ((m_oBlockName.Find("_NAME_") >=0) && (m_oChildList.GetCount()))
    {
        wxString name;
        wxFileName::SplitPath(m_oChildList[0]->TraceName(), NULL, &name, NULL);
        m_oBlockName.Replace("_NAME_", wxFileNameFromPath(name));
    }

#ifdef _DEBUG
   LOG0(L"[LIPPA-DBG] GID CompileBuffer: BufferLen=%lu BlockSize=%d ImagesNum=%d (BlockName=%s)",
        m_oBuffer.Len(),m_iBlockSize,m_iNumImages, (const char *)m_oBlockName);
#endif

   /**
    *  Create Block 0 to store the CarouselDirectoryModule()
    *  data necessary to generate the genius proprietary carousel
   */
   p_b0 = new cGIDi_BlockDirModule(this);
   m_oCompiledList.Append(p_b0);
   m_oBlockList.Append(p_b0);

   /**
    *  Split the images in N Blocks to store the CarouselPayloadChunk()
    *  data necessary to generate the genius proprietary carousel
    *  The payloads are extracted from the internal buffer where are
    *  concatened all images file content.
   */
   n_block     = 1;
   buff_offset = 0;
   buff_rest   = m_oBuffer.Len();

   for (i = 0; i < m_iNumImages; i++)
   {
      image_rest = m_poImagesProps[i].Size();
      if (m_iLocationType == kTS_WRITER_IMG_LOCATION_BY_PARTID)
         block_foff = 0;
      else
         block_foff = m_poImagesProps[i].FlashOffset();

      if (image_rest > buff_rest)
      {
         LOGE(L"Buffer Images size (%lu bytes) invalid: Image %d on %lu bytes overflow (rest %lu)",
              m_oBuffer.Len(),i,image_rest,buff_rest);
         return false;
      }

#ifdef _DEBUG
      if (m_u16Veroler & bit_image) set_virus = true;
      else set_virus = false;
      bit_image <<= 1;
#endif

      // Split the image in block
      while (image_rest > 0)
      {
         block_size = (u32)m_iBlockSize;
         if (block_size > image_rest) block_size = image_rest;

         // Create the block
         p_block = new cGeniusImagesBlock(this,n_block);

#ifdef _DEBUG
         // Set the virus only on first bloc
         if (set_virus)
         {
            p_block->SetVirus();
            set_virus = FALSE;
         }
#endif

         // Set the payload part of image
         p_block->m_oBuffer.Set(m_oBuffer,buff_offset,block_size);

         // Define the location in flash
         if (m_iLocationType == kTS_WRITER_IMG_LOCATION_BY_PARTID)
         {
#ifdef _DEBUG
            LOG0(L"[LIPPA-DBG] GID CompileBuffer: Image %d - "
                 "Block %3d size %6d from @0x%08X -> Part 0x%04X [0x%08X,0x%08X]",
                 i,n_block,block_size,buff_offset,
                 m_poImagesProps[i].PartitionID(),
                 block_foff,block_size);
#endif
            p_block->SetLocation(kGIDi_OFFSET_UNDEF,
                                 m_poImagesProps[i].PartitionID(),
                                 block_size);
         }
         else
         {
#ifdef _DEBUG
            LOG0(L"[LIPPA-DBG] GID CompileBuffer: Image %d - "
                 "Block %3d size %6d from @0x%08X -> Flash [0x%08X,0x%08X]",
                 i,n_block,block_size,buff_offset,block_foff,block_size);
#endif
            p_block->SetLocation(block_foff,
                                 (u16)(i+1),
                                 block_size);
         }

         m_oCompiledList.Append(p_block);
         m_oBlockList.Append(p_block);

         n_block++;
         buff_offset += (u32)block_size;
         buff_rest   -= (u32)block_size;
         block_foff  += (u32)block_size;
         image_rest  -= (u32)block_size;
      }
   }

   if (buff_rest > 0)
   {
      LOGE(L"Images size (%lu bytes) invalid: "
           "The buffer not contain the %d images (rest %lu bytes)",
           m_oBuffer.Len(),m_iNumImages,buff_rest);
      return false;
   }

   ClearBuffer(); // /!\ Lippa Note: In version 1.4 in comment

    return true;
}

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
bool cGeniusImagesData::CompileDirectoryModule ()
{
   bool                  ok       = true;
   cGIDi_BlockDirModule *p_block0 = (cGIDi_BlockDirModule*)m_oBlockList[0];

   ok = p_block0->CompileBuffer(m_u32UpdateId,m_u32LowerFlashOffset);

#ifdef _DEBUG
   LOG0(L"[LIPPA-DBG] GID CompileBuffer %d images in %d chunk %s",
        m_iNumImages,m_oBlockList.GetCount(),
        (ok)?"DONE OK":"failure");
#endif

   // Child concat forbidden
   m_bAllowAppend = false;

   return ok;
}

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
bool cGeniusImagesData::CompileChunkInfoList (cGIDi_DirectoryModule *pxoDirModule)
{
   dmsModuleData      *p_data;
   cGeniusImagesBlock *p_block;
   cGIDi_ChunkInfo    *p_chunkinfo;

   FOREACH(dmsModuleDataList,m_oBlockList,p_data)
   {
      p_block = (cGeniusImagesBlock*)p_data;
      if (p_block->Number() > 0)
      {
         p_chunkinfo = new cGIDi_ChunkInfo();

            pxoDirModule->ChunkInfoList->Append(p_chunkinfo);
            p_chunkinfo->PartitionID = p_block->PartitionID();
            p_chunkinfo->ChunkID     = (u16)p_block->Number();
            p_chunkinfo->ChunkSize   = p_block->m_oBuffer.Len();
        }
   }

   return true;
}

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
bool cGeniusImagesData::CompileSignature (dmsBuffer &xoSignBuffer)
{
   switch (m_iSignType)
   {
   case kTS_WRITER_SIGNATURE_NONE:
      // Signature already in image
      LOG0(L"NO Signature");
       break;

   case kTS_WRITER_SIGNATURE_AUTOMATIC:
      // Automatic signature => The key filename must be defined
      if (m_oSignKeyFilename.IsEmpty())
      {
         LOGE(L"Error in signature : Key file name undefined");
         return false;
      }

      // Compute the signature
      LOG0(L"Compute Signature signed by key '%s'",m_oSignKeyFilename);
      if (!gst_RSAKeySign(m_oBuffer,m_oSignKeyFilename,xoSignBuffer))
      {
         LOGE(L"Error signing buffer with key '%s'",m_oSignKeyFilename);
         return false;
      }
      break;

   case kTS_WRITER_SIGNATURE_FROMFILE:
      // Signature from a file
      if (m_oSignFilename.IsEmpty())
      {
         LOGE(L"Error in signature : Signature file name undefined");
         return false;
      }

      // Load the signature
      LOG0(L"Add Signature '%s'",m_oSignFilename);
      if (!xoSignBuffer.Load(m_oSignFilename))
      {
         LOGE(L"Error loading adding signature '%s'",m_oSignFilename);
         return false;
      }
      break;

   default:
      LOGE(L"Error in signature : UNSUPPORTED type %d",m_iSignType);
      return false;
   }

   return true;
}


/*****************************************************************************/


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
 *     - Chunk 1..N carry part of binary images:
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
cGeniusImagesBlock::cGeniusImagesBlock (cGeniusImagesData *pxoParent, int xiNumber) : dmsModuleData(pxoParent,MODULE_DATA_TYPE_GENIUS_IMAGES_BLOCK)
{
    m_poImagesSplitter = pxoParent;
    m_iNumber          = xiNumber;
    m_oOutputFilename  = STR(pxoParent->m_oBlockName,m_iNumber);
    m_poChunk          = NULL;
#ifdef _DEBUG
   m_bVerolCRC        = false;
#endif
}

/**
 *  @brief
 *    Genius Images block  destructor: Clean the current chunk.
 *
*/
cGeniusImagesBlock::~cGeniusImagesBlock ()
{
#ifdef _DEBUG
   LOG0(L"[LIPPA-DBG] cGeniusImagesBlock: Delete the chunk %d",m_iNumber);
#endif
    DELNUL(m_poChunk);
}

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
void cGeniusImagesBlock::SetLocation (u32 xu32Offset, u16 xu16PartID, u32 xu32Size)
{
   m_u32FlashOffset = xu32Offset;
   m_u16PartitionID = xu16PartID;
   m_u32PayloadSize = xu32Size;
}

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
bool cGeniusImagesBlock::CompileBuffer ()
{
   bool                ok = true;
    cGIDi_PayloadChunk *p_pc;
    dmsHtoN             hton;

   // Special block 0 managed by the splitter after the last block
   if (m_iNumber == 0) return true;

   // Very improbable but okazou
   if (m_u32PayloadSize != m_oBuffer.Len())
   {
      LOGE(L"Splitter in ze choux on block %d (offset 0x%08X, payload size %lu but wait %lu)",
            m_iNumber,m_u32FlashOffset,m_u32PayloadSize,m_oBuffer.Len());
   }

   // Create and define the payload chunk
   p_pc = new cGIDi_PayloadChunk();
   p_pc->SetName("PayloadChunk");
   m_oBuffer.Move(p_pc->PayloadByte->m_oBuffer);
   p_pc->ChunkID            = m_iNumber;
   p_pc->PayloadFlashOffset = m_u32FlashOffset;
   p_pc->PayloadSize        = m_u32PayloadSize;
    p_pc->PayloadCRC32       = p_pc->PayloadByte->m_oBuffer.CRC32();

#ifdef _DEBUG
   // Corrupt CRC for QA validation
   if (m_bVerolCRC)
   {
      LOGW(L"[QA-TEST] CRC Corruption in Chunk%02d of partID=0x%04x",
           m_iNumber,m_u16PartitionID);
      p_pc->ChunkCRC32 += 1;
   }
#endif

   // Generate the buffer
    p_pc->Generate1(hton);
    hton.UpdateCRC32(p_pc->ChunkCRC32);
    hton.Copy(m_oBuffer);

   // Save the pointer to delete it at the block destruction
   m_poChunk = p_pc;

   // Save on disk
   Export();

   LOG0(L"Chunk%02d: partID=0x%04x - @=0x%08x - size=0x%08x=%6u bytes "
        "- pCRC=0x%08x cCRC=0x%08x => %d bytes",
        m_iNumber,m_u16PartitionID,p_pc->PayloadFlashOffset,p_pc->PayloadSize,
        p_pc->PayloadSize,p_pc->PayloadCRC32,p_pc->ChunkCRC32,m_oBuffer.Len());

   // Is last block ?
    if (m_iNumber == (int) (m_poImagesSplitter->NumberOfBlocks()-1))
    {
      // Last block => Generate the Directory Module (chunk 0)
      ok = m_poImagesSplitter->CompileDirectoryModule();
   }

    return ok;
}

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
bool cGeniusImagesBlock::Export ()
{
    if (m_poImagesSplitter->m_oOutputDir.Len())
    {
        wxString name;
        wxFileName::SplitPath(m_oOutputFilename, NULL, &name, NULL);

        name = m_poImagesSplitter->m_oOutputDir + "/"+name;

        if (m_poDataCarousel && m_poDataCarousel->m_oOutputDir.Len())
        {
            m_poChunk->m_oTraceFilename = STR("%s", m_oOutputFilename);
            m_poChunk->Trace(name+"/Trace-Split", true);
        }

        return m_oBuffer.Save(name+"/"+m_oOutputFilename);
    }

    return true;
}

/**
 *  @brief
 *    Return the name of trace file
 *
*/
wxString cGeniusImagesBlock::TraceName ()
{
    return "Block_"+wxFileNameFromPath(m_oOutputFilename);
}



/******************************************************************************
* Private Class Implementation
******************************************************************************/

/**
 *  @brief
 *    Build the genius proprietary carousel Directory Module data
 *    (chunk 0):
 *    - Creation and set global data
 *    - Create the chunk list info
 *    - Generate the signature
 *    - Build the corresponding buffer
 *    - Save it on disk
 *
 *  @param[in] xu32UpdateId
 *       Update ID to store in header of Directory Module.
 *
 *  @param[in] xu32Address
 *       Lower flash addresse to store in header of Directory Module.
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
bool cGIDi_BlockDirModule::CompileBuffer (u32 xu32UpdateId, u32 xu32LowerOffset)
{
   cGIDi_DirectoryModule *p_dm;
   dmsHtoN                hton;

   // Create and define the directory module chunk
   p_dm = new cGIDi_DirectoryModule();
   p_dm->SetName("DirectoryModule");
   p_dm->UpdateID         = xu32UpdateId;
   p_dm->LowerFlashOffset = xu32LowerOffset;
   p_dm->Version          = kTS_WRITER_STVD_CARO_VERSION_VALUE;

   // Save the pointer to delete it at the block destruction
   m_poChunk = p_dm;

   // Set the chunk infos list
   if (!m_poImagesSplitter->CompileChunkInfoList(p_dm)) return false;

   // generate the signature
   if (!m_poImagesSplitter->CompileSignature(p_dm->SignatureByte->m_oBuffer)) return false;

   // Generate the buffer
   p_dm->Generate1(hton);
   hton.UpdateCRC32(p_dm->ChunkCRC32);
   hton.Copy(m_oBuffer);

   /**
    *  /!\ Remote compilation of chunk 0 (after the N chunk)
    *
    * ABSOLUTLY necessary to set the dmsModuleData property m_iSize with
    * the buffer size compiled here.
    *
    * In Normal case, automaticly set after the normal compilation by the class
    * dmsModuleData but in this case after the normal compilation
    * the size is debilous.
    */
   m_iSize = m_oBuffer.Len();

   // Save on disk
   Export();

   LOG0(L"Chunk%02d: UpdateID=0x%08x - Lower@=0x%08x - Version=0x%08x "
        "- NumChunks=%lu - SignSize=%lu - CRC=0x%08x => %d bytes",
        m_iNumber,p_dm->UpdateID,p_dm->LowerFlashOffset,
        p_dm->Version,p_dm->ChunkCount,
        p_dm->SignatureSize,p_dm->ChunkCRC32,m_oBuffer.Len());

   return true;
}

/* GeniusImagesData.cpp */
