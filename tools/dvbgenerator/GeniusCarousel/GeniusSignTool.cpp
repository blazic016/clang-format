/****************************************************************************
** @file GeniusSignTool.h
**
** @brief
**   Signature Generation (RSA + MD5).
**
**
** @ingroup GENIUS CAROUSEL
**
** @version $Rev: 61905 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/lib/GeniusCarousel/GeniusSignTool.cpp $
**          $Date: 2011-06-09 18:52:08 +0200 (jeu., 09 juin 2011) $
**
** @author  SmarDTV Rennes - LIPPA
**
** COPYRIGHT:
**   2011 SmarDTV
**
** @history
**  - COF   - Iwedia  - v 0    - 05/2005 - Creation
**  - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
**
******************************************************************************/


#include <Tools/Header.h>

#include "GeniusSignTool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#include "LibCriptoCompat/libcrypto-compat.h"
#endif


#define _kSZ_BLOCK_READ  16384          /* Size of block readed              */


#define _kSZ_BIG_DATA  1024  // Size of data block used for AES key and IV
#define _kSZ_AES_KEY     16  // Size of the AES keys


/* ------------------------------------------------------------------------
   File key header
   ------------------------------------------------------------------------ */

typedef struct
{
    int      iDate;                     // Date/hour of generation
    char     zId[kKT_SZ_FNAME+1];       // Key identifier
    int      iSizeN;                    // Size of public modulus (in bytes)
    int      iSizeE;                    // Size of public exponent (in bytes)
    int      iSizeD;                    // Size of private exponent (in bytes)
} _tFileKeyHeader;

/* ------------------------------------------------------------------------
   Data block used for to make the AES key and the initial vector
   ------------------------------------------------------------------------ */

static unsigned char _tucTheBigData[_kSZ_BIG_DATA] =
{
   0xBF,0x7F,0x17,0xA2,0x19,0x9A,0x13,0x5D,0x0D,0x4A,0x0B,0xA0,0x60,0xF8,0xAD,0x84,
   0x3E,0x73,0x27,0xF5,0x44,0xF1,0x5F,0x14,0xDB,0x25,0xB9,0x8E,0xEE,0xD6,0x8A,0x0B,
   0xDA,0x92,0xE6,0xF5,0xB2,0x07,0x2F,0xC4,0x8B,0x29,0xB7,0x1E,0xA4,0x97,0xA6,0xE0,
   0xCD,0xBF,0xE7,0x54,0xF0,0xB5,0xF5,0x0E,0xEB,0xE1,0xCB,0x43,0x08,0x29,0x69,0x4C,
   0x20,0xB3,0x8B,0x84,0x90,0x77,0x8C,0x78,0xA3,0x10,0x86,0x5A,0x26,0x0A,0xCB,0xEB,
   0xD6,0x4E,0x10,0x5C,0x97,0x9B,0xEC,0x0D,0xA7,0x15,0x6E,0xF8,0x70,0x75,0x0B,0xD7,
   0x5F,0x5D,0xF4,0xF7,0x77,0x79,0x1C,0x8B,0x9E,0x73,0x71,0x58,0xAA,0xFD,0x7E,0xB0,
   0x12,0xF2,0x36,0x99,0xF4,0x8F,0x70,0x9E,0x45,0x0C,0xCD,0x9F,0x25,0xD9,0xCF,0xFB,
   0xE4,0x57,0xAE,0xE7,0xA2,0xE0,0x75,0x28,0x8D,0x37,0x0B,0x98,0x97,0xB2,0x25,0x3B,
   0x9B,0x22,0x89,0x74,0xBB,0x80,0x32,0xBA,0x56,0xD8,0x16,0x8B,0xC2,0x4A,0x2F,0x2B,
   0xB1,0xCC,0x93,0xAF,0xC4,0xF8,0x99,0x91,0x25,0x3D,0xAD,0x09,0x4E,0xF5,0xEC,0x85,
   0x9B,0x14,0x17,0xD2,0x58,0xA0,0x0B,0x8C,0x52,0xB0,0xD3,0x8C,0xF0,0x3D,0xF6,0xC4,
   0xB9,0xBA,0xBD,0xC4,0xA3,0xC7,0x95,0x1C,0xAB,0xB1,0x12,0x55,0x61,0xF7,0x7D,0x01,
   0x66,0x8B,0x48,0x63,0xC2,0xDB,0x50,0x38,0xF8,0x90,0xA5,0x21,0xE8,0x3B,0x28,0x7D,
   0xDC,0x90,0xE7,0x49,0x79,0xB5,0xD2,0x2F,0xD3,0x6C,0x24,0x77,0x9C,0xCE,0x28,0x85,
   0xB7,0xF6,0xFA,0xAE,0x49,0xFF,0x40,0x7A,0xC0,0xFC,0xA8,0x62,0xA8,0x3F,0xE7,0x11,
   0x84,0xDD,0x4A,0x69,0x76,0xF7,0xF7,0x1D,0x65,0xDE,0x0D,0x29,0x0E,0x7F,0x1B,0xED,
   0x18,0x63,0x4F,0xC3,0x16,0x3D,0x9E,0x69,0xDB,0x0F,0x56,0xAA,0xBC,0x3D,0xF0,0x73,
   0xFF,0x08,0x53,0xE4,0xFE,0xCB,0x8B,0xB2,0x03,0x98,0x42,0xEC,0xA8,0xE8,0x6B,0xA9,
   0xAB,0xC4,0xCC,0x6A,0xD8,0xCE,0xC6,0x9B,0x2B,0xA9,0x37,0x9E,0xC4,0xF2,0x03,0xD9,
   0x4C,0xCE,0x35,0x4D,0x7C,0x1C,0x3B,0xF0,0xF5,0x55,0x28,0xC4,0xEF,0xB3,0xE7,0x0D,
   0x86,0xD7,0xD0,0xB6,0x87,0x15,0x91,0x8B,0x52,0x1C,0x5C,0x4C,0x06,0x9A,0xB6,0x6A,
   0xE7,0x14,0xB7,0x7D,0x5B,0xFE,0x4B,0x0C,0xED,0x81,0x20,0xD4,0x77,0x1A,0x46,0x87,
   0x44,0x6B,0x35,0x80,0x38,0xA0,0x92,0xDA,0x4C,0xBD,0xE2,0x9E,0x9D,0x4D,0x86,0xB4,
   0x7E,0x7E,0x71,0xBB,0x65,0x77,0x3B,0x3B,0xF4,0x4B,0x99,0x2D,0xD5,0x50,0x8F,0x23,
   0xC3,0x89,0xAD,0xF2,0xEB,0xCD,0x25,0x8E,0x85,0x1F,0x1E,0xDA,0xFC,0x2A,0xD9,0xED,
   0x0A,0x93,0x91,0xB3,0xB8,0x54,0x74,0x41,0x5E,0x81,0x97,0x51,0xD2,0xA6,0x6E,0x6A,
   0xE0,0xBB,0x78,0x40,0xAD,0x59,0x43,0x3F,0x16,0x09,0xE5,0x33,0x6D,0xA9,0xB4,0x6C,
   0xFD,0x5C,0xAA,0xD1,0x95,0xEE,0xD0,0x2A,0xCB,0xD0,0xAE,0xDA,0x92,0x87,0x52,0x3B,
   0x19,0x10,0x81,0xF2,0x39,0x32,0x87,0xD2,0x0C,0x9B,0xCD,0x19,0x07,0xE5,0x16,0x09,
   0xCA,0x8F,0xE5,0x0E,0x68,0x4D,0xC3,0xCD,0x46,0x75,0x6B,0x3E,0x78,0x1B,0x89,0x4A,
   0xC7,0x0B,0x5C,0xBB,0xCC,0x1E,0x46,0xB3,0x05,0x88,0xAB,0xBF,0xA3,0x83,0x86,0x89,
   0xBD,0xF8,0x1F,0x53,0xDD,0x2A,0x5F,0x8D,0xAF,0x1E,0xD0,0x40,0x9C,0x64,0x07,0xF3,
   0x48,0xA3,0x2F,0x09,0x1C,0xC3,0x3C,0xEE,0xFE,0xE8,0x44,0x6C,0xBC,0x74,0x4F,0xA5,
   0x0B,0x18,0x45,0x7E,0x85,0x3C,0x93,0x87,0xCF,0x45,0xA9,0x81,0x53,0x05,0x90,0x29,
   0x50,0x4E,0x80,0x70,0x36,0xEC,0x4C,0x25,0xB3,0x11,0x24,0xF3,0x6C,0xE8,0x10,0xCB,
   0x81,0x92,0x72,0x5C,0xC1,0xB9,0x0B,0x28,0x5C,0xF0,0xD8,0x60,0x8C,0x83,0x02,0xFD,
   0x92,0xCE,0x0D,0x21,0x25,0xA3,0xB1,0x4E,0x53,0x77,0x92,0x72,0xE4,0x84,0x9D,0x18,
   0xED,0x2A,0xFD,0x72,0x6D,0x89,0x0F,0xF3,0xA0,0xC4,0x1A,0x6F,0xDB,0x43,0xE2,0xC7,
   0x90,0x2F,0xAF,0x07,0xE0,0x0B,0xC4,0x7F,0x6E,0x52,0x14,0xC4,0xB8,0x5F,0x7F,0x6F,
   0xB2,0xD8,0x16,0xE8,0xB3,0x5F,0x4E,0xF1,0xE6,0x0F,0x27,0xF8,0xDB,0xC1,0x44,0xF2,
   0x10,0x5A,0x41,0x4B,0xD9,0x1D,0x3B,0xC2,0x35,0xB6,0x56,0x2B,0x8F,0x7B,0x3E,0x1F,
   0x23,0xB0,0x9A,0x22,0x15,0x83,0xFE,0xA8,0xF2,0x95,0x2A,0x18,0x8B,0x8E,0x68,0x07,
   0x66,0x67,0x7A,0x0F,0x35,0x91,0x30,0x6D,0xA0,0xF4,0xC2,0x63,0x6E,0x38,0x4F,0xEC,
   0x37,0xCE,0xC7,0x3C,0x46,0xA6,0xA0,0x0C,0x0D,0xEB,0x98,0xAF,0xAE,0x6B,0xF6,0xF6,
   0x30,0xAC,0x9E,0x7C,0x7B,0x0D,0x98,0x27,0xF3,0x34,0xDE,0xC4,0x73,0xAE,0xE0,0xB5,
   0x45,0x26,0xB1,0x9E,0xA4,0xB2,0x57,0x5F,0x96,0x1C,0x52,0xA1,0x03,0xE5,0x5E,0x01,
   0x02,0x60,0xB5,0x71,0x1C,0x63,0x29,0xF1,0x43,0x99,0x5E,0x45,0xD6,0x61,0x44,0x11,
   0xBB,0xE4,0xA2,0xCD,0x7D,0x10,0x32,0x7B,0x71,0x10,0xD6,0xC0,0x25,0x87,0x85,0x5A,
   0x09,0x0D,0x99,0x16,0xEA,0x76,0xB3,0xCF,0xC0,0xA2,0x06,0xF0,0xB7,0xFD,0x24,0xDF,
   0x85,0x96,0xAA,0x77,0x67,0xF2,0xB5,0x43,0x41,0x07,0x61,0x2D,0xF8,0xED,0x4E,0x84,
   0x13,0x22,0x29,0x2A,0x56,0x56,0x1A,0x39,0xAF,0x19,0xEB,0x5E,0x8A,0x43,0x64,0xCD,
   0xBB,0xEB,0xBE,0x46,0x64,0x36,0xAC,0x31,0x32,0xB7,0x9E,0x05,0x2D,0xC1,0x0E,0x83,
   0x6B,0x1B,0x8F,0xEB,0x61,0xC4,0x53,0x95,0xFE,0x20,0x4A,0x1D,0xD3,0xB6,0xD8,0x9A,
   0xD3,0xF3,0x56,0x51,0xCB,0xDD,0x13,0x50,0xBC,0x5C,0x55,0x79,0xFA,0x70,0xC1,0xEB,
   0x2A,0x78,0x1F,0xD1,0xB0,0x96,0x5E,0xAD,0x86,0x22,0xF3,0x91,0x87,0x39,0x7D,0x05,
   0x28,0xE1,0x56,0xF6,0xA4,0x96,0x48,0x95,0x89,0x05,0x23,0xE4,0x4C,0x86,0x0E,0xE2,
   0x87,0x03,0xB9,0x62,0x3E,0x03,0x33,0x9E,0x29,0x08,0x57,0xC0,0x3A,0x17,0xCE,0xD4,
   0x52,0x5A,0xEF,0x89,0x97,0xE4,0xB1,0xA5,0xDA,0xD1,0x04,0x1B,0x1F,0x53,0xE7,0xBA,
   0x5F,0x4E,0x9E,0x06,0xFE,0x29,0x97,0xA2,0x2C,0x4D,0xE2,0x7F,0x63,0x62,0x7B,0xA0,
   0x81,0x1A,0xB7,0xE1,0x21,0xAD,0xC3,0xD9,0x67,0x83,0x30,0xAE,0x3F,0x68,0x6B,0x5E,
   0x8C,0x7B,0x53,0x22,0xC9,0x3D,0xF1,0xBA,0x45,0xFB,0x82,0xD5,0xD6,0x42,0x68,0x4D,
   0x20,0x06,0xC8,0xAE,0x2F,0x62,0x8F,0x72,0x60,0xC3,0x56,0xFF,0xD2,0xA1,0xF8,0x8C,
   0xA1,0x85,0xC1,0xC8,0x7E,0x88,0x25,0xC9,0x49,0xC4,0x76,0xA2,0xDD,0xE5,0xB9,0x65
};

/* ========================================================================
   Fonctions locales
   ======================================================================== */


/* ------------------------------------------------------------------------
   Buffer allocation and checking
   ------------------------------------------------------------------------ */

static void *_pAlloc (int iSize, char *zLabel)
{
   void *p;

   p = malloc(iSize);

   if (p == NULL)
      LOGE(L"ERROR : %s allocation %d bytes failed",zLabel,iSize);

   return(p);
}

/* ------------------------------------------------------------------------
   Extract between a big number implementation, the binary value of a
   RSA key
   ------------------------------------------------------------------------ */

static bool _bKeyGetValue (const BIGNUM *pBN, int *piSize, unsigned char **pucBinVal,
                           char *zLabel)
{
   unsigned char *p  = NULL;
   int            len;

   if (*pucBinVal != NULL) free(*pucBinVal);
   *pucBinVal = NULL;
   *piSize    = 0;

   len = BN_num_bytes(pBN);

   if (len <= 0)
   {
      LOGE(L"ERROR : %s undefined (size=%d)\n",zLabel,len);
      return(false);
   }

   p = (unsigned char*)_pAlloc(len,zLabel);

   if (p == NULL) return(false);

   BN_bn2bin(pBN,p);

   *piSize    = len;
   *pucBinVal = p;

   return(true);
}

/* ------------------------------------------------------------------------
   Set the value binary of a key in the BIGNUMBER corresponding of the
   OpenSSL key context
   ------------------------------------------------------------------------ */

static bool _bKeySetValue (BIGNUM **pBN, int iSize, unsigned char *pucBinVal,
                           char *zLabel)
{
   *pBN = BN_bin2bn(pucBinVal,iSize,*pBN);

   if (*pBN == NULL)
   {
      LOGE(L"ERROR : Set %s binary value to RSA key failed (size=%d)", zLabel,iSize);
      return(false);
   }

   return(true);
}

/* ------------------------------------------------------------------------
   Read a binary value in a file
   ------------------------------------------------------------------------ */

static bool _bReadBinValue (FILE *pFile, int iSize, unsigned char **pucBinVal,
                            char *zLabel, char *zFileName)
{
   unsigned char *p = NULL;

   if (*pucBinVal != NULL) free(*pucBinVal);
   *pucBinVal = NULL;

   if (iSize <= 0)
   {
      LOGE(L"ERROR: %s readed undefined (size=%d)",zLabel,iSize);
      return(false);
   }

   p = (unsigned char*)_pAlloc(iSize,zLabel);
   if (p == NULL) return(false);

   if (fread(p,iSize,1,pFile) != 1)
   {
      LOGE(L"ERROR: Read %s in file key '%s' failed", zLabel,zFileName);
      return(false);
   }

   *pucBinVal = p;

   return(true);
}

/* ------------------------------------------------------------------------
   Write a binary value in a file
   ------------------------------------------------------------------------ */

static bool _bWriteBinValue (FILE *pFile, int iSize, unsigned char *pucBinVal,
                             char *zLabel, char *zFileName)
{
   if (iSize > 0)
   {
      if (fwrite(pucBinVal,iSize,1,pFile) != 1)
      {
         LOGE(L"ERROR: Write %s on file key '%s' failed", zLabel,zFileName);
         return(false);
      }
   }

   return(true);
}

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

static void _GetBigData (unsigned char *pDst, int iPos, int iNum)
{
   int i,p,j;

   for (i = 0, p = iPos; i < iNum; i++, p++)
   {
      j = p % _kSZ_BIG_DATA;
      pDst[i] = _tucTheBigData[j];
   }

}

/* ------------------------------------------------------------------------
   Make the AES Key and the AES initial vector
   ------------------------------------------------------------------------ */

static void _MakeAESKey (tRSAKeyCtx *pCtx, unsigned char *pAesKey,
                         unsigned char *pAesIV)
{
   unsigned char md[MD5_DIGEST_LENGTH]; // 16 bytes
   MD5_CTX       md5_ctx;
   unsigned char tmp[4];

   /*
    * Build the message digest of :
    * - Generation date/time
    * - Key identifier
    * - Public key
    * - 16 bytes of the Big data
    */
   tmp[0] = (unsigned char)((((unsigned int)pCtx->iDateGen) >> 24) & 0x000000FF);
   tmp[1] = (unsigned char)((((unsigned int)pCtx->iDateGen) >> 16) & 0x000000FF);
   tmp[2] = (unsigned char)((((unsigned int)pCtx->iDateGen) >>  8) & 0x000000FF);
   tmp[3] = (unsigned char)( ((unsigned int)pCtx->iDateGen)        & 0x000000FF);

   MD5_Init(&md5_ctx);
   MD5_Update(&md5_ctx,tmp,4);
   MD5_Update(&md5_ctx,pCtx->zIdent,strlen(pCtx->zIdent));
   MD5_Update(&md5_ctx,pCtx->pucN,pCtx->iSizeN);
   MD5_Update(&md5_ctx,_tucTheBigData+tmp[2],4);
   MD5_Update(&md5_ctx,_tucTheBigData+tmp[0],4);
   MD5_Update(&md5_ctx,_tucTheBigData+tmp[3],4);
   MD5_Update(&md5_ctx,_tucTheBigData+tmp[1],4);
   MD5_Final(md,&md5_ctx);

   // Make AES Key
   _GetBigData(pAesKey,   ((int)md[ 7]*256 + (int)md[10]),4);
   _GetBigData(pAesKey+4, ((int)md[ 0]*256 + (int)md[ 4]),4);
   _GetBigData(pAesKey+8, ((int)md[13]*256 + (int)md[ 6]),4);
   _GetBigData(pAesKey+12,((int)md[ 3]*256 + (int)md[15]),4);

   // Make AES initial vector
   _GetBigData(pAesIV,   ((int)md[ 9]*256 + (int)md[ 8]),4);
   _GetBigData(pAesIV+4, ((int)md[12]*256 + (int)md[ 1]),4);
   _GetBigData(pAesIV+8, ((int)md[ 5]*256 + (int)md[ 2]),4);
   _GetBigData(pAesIV+12,((int)md[11]*256 + (int)md[14]),4);
}

/* ------------------------------------------------------------------------
   Private key (private exponent D) encryption
   ------------------------------------------------------------------------ */

static void _EncryptPrivateExponent (tRSAKeyCtx *pCtx)
{
   unsigned char key[_kSZ_AES_KEY];
   unsigned char iv[_kSZ_AES_KEY];
   AES_KEY       aes_key;
   int           sz,m;

   m = pCtx->iSizeD % _kSZ_AES_KEY;
   if (m == 0) sz = pCtx->iSizeD;
   else sz = pCtx->iSizeD + _kSZ_AES_KEY - m;

   _MakeAESKey(pCtx,key,iv);
   AES_set_encrypt_key(key,_kSZ_AES_KEY*8,&aes_key);
   AES_cbc_encrypt(pCtx->pucD,pCtx->pucD,sz,&aes_key,iv,AES_ENCRYPT);
}

/* ------------------------------------------------------------------------
   Private key (private exponent D) encryption
   ------------------------------------------------------------------------ */

static void _DecryptPrivateExponent (tRSAKeyCtx *pCtx)
{
   unsigned char key[_kSZ_AES_KEY];
   unsigned char iv[_kSZ_AES_KEY];
   AES_KEY       aes_key;
   int           sz,m;

   m = pCtx->iSizeD % _kSZ_AES_KEY;
   if (m == 0) sz = pCtx->iSizeD;
   else sz = pCtx->iSizeD + _kSZ_AES_KEY - m;

   _MakeAESKey(pCtx,key,iv);
   AES_set_decrypt_key(key,_kSZ_AES_KEY*8,&aes_key);
   AES_cbc_encrypt(pCtx->pucD,pCtx->pucD,sz,&aes_key,iv,AES_DECRYPT);
}



/* ========================================================================

   ======================================================================== */

/* ------------------------------------------------------------------------
   RSA Key context initialization
   ------------------------------------------------------------------------ */

void gst_RSAKeyCtx_Init (tRSAKeyCtx *pCtx)
{
    pCtx->iDateGen    = 0;
    pCtx->zIdent[0]   = 0;
    pCtx->zFileKey[0] = 0;
    pCtx->zFileBL[0]  = 0;
    pCtx->iSizeN      = 0;
    pCtx->pucN        = NULL;
    pCtx->iSizeE      = 0;
    pCtx->pucE        = NULL;
    pCtx->iSizeD      = 0;
    pCtx->pucD        = NULL;
    pCtx->pKey        = NULL;

}

/* ------------------------------------------------------------------------
   RSA Key context cleaning
   ------------------------------------------------------------------------ */

void gst_RSAKeyCtx_Cleanup (tRSAKeyCtx *pCtx)
{
   if (pCtx->pucN != NULL) free(pCtx->pucN);
   if (pCtx->pucE != NULL) free(pCtx->pucE);
   if (pCtx->pucD != NULL) free(pCtx->pucD);
   if (pCtx->pKey != NULL) RSA_free(pCtx->pKey);
}

/* ------------------------------------------------------------------------
   Set the name (identifier) of the key and format the files
   where to save or to load the RSA key :
   - Key file for tkuiSign :
     <name>.key
   - Public key source for boot loader :
     <name>_loal_crypt.c

   Note : If the name already contain the extension ".key", this
          extension is deleted before.
   ------------------------------------------------------------------------ */

void gst_RSAKeyCtx_SetName (tRSAKeyCtx *pCtx, const char *pName)
{
   int i,p;

   // Find if extension 'key' present in name
   i = strlen(pName);
   p = -1;

   while ((i > 0) && (p < 0))
   {
      i--;
      if (pName[i] == '.') p = i;
   }

   // Set identifier
   strcpy(pCtx->zIdent,pName);
   if (p > 0)
   {
      if (strcmp(pName+p+1,kKT_FILE_KEY_EXT) == 0) pCtx->zIdent[p] = 0;
   }

   // Format Key File name
   sprintf(pCtx->zFileKey,"%s.%s",pCtx->zIdent,kKT_FILE_KEY_EXT);

   // Format public key source file name
   sprintf(pCtx->zFileBL,"%s%s",pCtx->zIdent,kKT_FILE_BL_SUFF);
}

/* ------------------------------------------------------------------------
   Extract the binary values of the keys implemented in the
   OpenSSL key context
   ------------------------------------------------------------------------ */

bool gst_RSAKeyCtx_KeyToBin (tRSAKeyCtx *pCtx)
{
   bool ok = true;
   const BIGNUM *n, *e, *d;

   RSA_get0_key(pCtx->pKey, &n, &e, &d);

   if (!n || !e || !d) {
       return false;
   }

   ok = _bKeyGetValue(n,&pCtx->iSizeN,&pCtx->pucN,
              (char *)"Public modulus N");
   if (ok) {
      ok = _bKeyGetValue(e,&pCtx->iSizeE,&pCtx->pucE,
              (char *)"Public exponent E");
   }
   if (ok) {
      ok = _bKeyGetValue(d,&pCtx->iSizeD,&pCtx->pucD,
              (char *)"Private exponent E");
   }

   if ((ok) && (pCtx->iDateGen <= 0)) pCtx->iDateGen = time(NULL);

   return(ok);
}

/* ------------------------------------------------------------------------
   Format the RSA OpenSSL key context with the binary values of
   the keys
   ------------------------------------------------------------------------ */

bool gst_RSAKeyCtx_BinToKey (tRSAKeyCtx *pCtx)
{
   bool ok = true;

   if (pCtx->pKey != NULL) RSA_free(pCtx->pKey);
   pCtx->pKey = RSA_new();

   if (pCtx->pKey == NULL)
   {
      LOGE(L"ERROR: Create new RSA key failed");
      ok = false;
   }

   BIGNUM *n, *e, *d;

   if (ok) {
      ok = _bKeySetValue(&n,pCtx->iSizeN,pCtx->pucN,
              (char *)"Public modulus N");
   }
   if (ok) {
      ok = _bKeySetValue(&e,pCtx->iSizeE,pCtx->pucE,
              (char *)"Public exponent E");
   }
   if (ok) {
      ok = _bKeySetValue(&d,pCtx->iSizeD,pCtx->pucD,
              (char *)"Private exponent E");
   }

   if (ok) {
       ok = (bool)RSA_set0_key(pCtx->pKey, n, e, d);
   }

   return(ok);
}

/* ------------------------------------------------------------------------
   Save the RSA key
   ------------------------------------------------------------------------ */

bool gst_RSAKeyCtx_SaveKey (tRSAKeyCtx *pCtx)
{
   bool             ok = true;
   FILE            *f;
   _tFileKeyHeader  head;

   if (pCtx->iDateGen <= 0) pCtx->iDateGen = time(NULL);

   // Private key encryption
   _EncryptPrivateExponent(pCtx);

   // Save on disk
   f = fopen(pCtx->zFileKey,"wb");

   if (f == NULL)
   {
      LOGE(L"ERROR: Create file key '%s' failed", pCtx->zFileKey);
      return(false);
   }

   // Write header
   strcpy(head.zId,pCtx->zIdent);
   head.iDate  = pCtx->iDateGen;
   head.iSizeN = pCtx->iSizeN;
   head.iSizeE = pCtx->iSizeE;
   head.iSizeD = pCtx->iSizeD;

   if (fwrite(&head,sizeof(_tFileKeyHeader),1,f) != 1)
   {
      LOGE(L"ERROR: Write header of file key '%s' failed", pCtx->zFileKey);
      ok = false;
   }

   // Write the binary values of the key
   if (ok)
      ok = _bWriteBinValue(f,pCtx->iSizeN,pCtx->pucN,
              (char *)"Public modulus N",pCtx->zFileKey);
   if (ok)
      ok = _bWriteBinValue(f,pCtx->iSizeE,pCtx->pucE,
              (char *)"Public exponent E",pCtx->zFileKey);
   if (ok)
      ok = _bWriteBinValue(f,pCtx->iSizeD,pCtx->pucD,
              (char *)"Private exponent D",pCtx->zFileKey);

   fclose(f);

   return(ok);
}

/* ------------------------------------------------------------------------
   Create the boot loader source file containing the RSA public key
   ------------------------------------------------------------------------ */

bool gst_RSAKeyCtx_CreatePubKeySrc (tRSAKeyCtx *pCtx)
{
   FILE *f;
   int   i;

   if (pCtx->iDateGen <= 0) pCtx->iDateGen = time(NULL);

   f = fopen(pCtx->zFileBL,"wt");

   if (f == NULL)
   {
      LOGE(L"ERROR: Create file source '%s' for boot loader failed", pCtx->zFileBL);
      return(false);
   }

   fprintf(f,"/******************************************************************************\n");
   fprintf(f," *                     COPYRIGHT 2011 SMARDTV                                 *\n");
   fprintf(f," ******************************************************************************\n");
   fprintf(f," *\n");
   fprintf(f," * MODULE NAME: LOAL_FLASH\n");
   fprintf(f," *\n");
   fprintf(f," * FILE NAME: $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/lib/GeniusCarousel/GeniusSignTool.cpp $\n");
   fprintf(f," *            $Rev: 61905 $\n");
   fprintf(f," *            $Date: 2011-06-09 18:52:08 +0200 (jeu., 09 juin 2011) $\n");
   fprintf(f," *\n");
   fprintf(f," * PRIVATE\n");
   fprintf(f," *\n");
   fprintf(f," * FILE:        loal_cypt.c\n");
   fprintf(f," *\n");
   fprintf(f," * DESCRIPTION: RSA public key to check the signature.\n");
   fprintf(f," *\n");
   fprintf(f," *****************************************************************************/\n\n");

   fprintf(f,"/*****************************************************************************/\n");
   fprintf(f,"/*          INCLUDES                                                         */\n");
   fprintf(f,"/*****************************************************************************/\n\n");
   fprintf(f,"#include \"crules.h\"\n\n\n");

   fprintf(f,"/*****************************************************************************/\n");
   fprintf(f,"/*          LOCAL MODULES VARIABLES                                          */\n");
   fprintf(f,"/*****************************************************************************/\n\n");
   fprintf(f,"/*\n");
   fprintf(f," * %s RSA Prublic Key\n",pCtx->zIdent);
   fprintf(f," */\n");
   fprintf(f,"MODULE uint8_t LOAL_CRYPT_RsaPublicKey[] =\n{\n");
   fprintf(f,"   0x00");
   for (i = 0; i < pCtx->iSizeN; i++)
   {
      fprintf(f,", ");
      if ((i % 16) == 0) fprintf(f,"\n   ");
      fprintf(f,"0x%02X",pCtx->pucN[i]);
   }
   fprintf(f,"\n};\n\n");

   fprintf(f,"/*****************************************************************************\n");
   fprintf(f," * END loal_cypt.c\n");
   fprintf(f," *****************************************************************************/\n");

   fclose(f);

   return(true);
}

/* ------------------------------------------------------------------------
   Load the RSA key since the key file
   ------------------------------------------------------------------------ */

bool gst_RSAKeyCtx_LoadKey (tRSAKeyCtx *pCtx)
{
   bool             ok = true;
   FILE            *f;
   _tFileKeyHeader  head;

   f = fopen(pCtx->zFileKey,"rb");

   if (f == NULL)
   {
      LOGE(L"ERROR: Open file key '%s' failed", pCtx->zFileKey);
      return(false);
   }

   // Read header
   if (fread(&head,sizeof(_tFileKeyHeader),1,f) != 1)
   {
      LOGE(L"ERROR: Read header of file key '%s' failed\n", pCtx->zFileKey);
      ok = false;
   }
   else
   {
      strcpy(pCtx->zIdent,head.zId);
      pCtx->iDateGen = head.iDate;
      pCtx->iSizeN   = head.iSizeN;
      pCtx->iSizeE   = head.iSizeE;
      pCtx->iSizeD   = head.iSizeD;
   }

   // Read the binary values of the RSA key
   if (ok)
      ok = _bReadBinValue(f,pCtx->iSizeN,&pCtx->pucN,
              (char *)"Public modulus N",pCtx->zFileKey);
   if (ok)
      ok = _bReadBinValue(f,pCtx->iSizeE,&pCtx->pucE,
              (char *)"Public exponent E",pCtx->zFileKey);
   if (ok)
      ok = _bReadBinValue(f,pCtx->iSizeD,&pCtx->pucD,
              (char *)"Private exponent E",pCtx->zFileKey);

   fclose(f);

   // Private key decryption
   if (ok) _DecryptPrivateExponent(pCtx);

   return(ok);
}

/* ------------------------------------------------------------------------
   Trace the values of the RSA Key
   ------------------------------------------------------------------------ */

void gst_RSAKeyCtx_Dump (tRSAKeyCtx *pCtx, bool bWithTitle)
{
   if (bWithTitle) printf("\n* RSA Key '%s' :\n",pCtx->zIdent);
   gst_DumpBin(pCtx->pucN,pCtx->iSizeN,(char *)"- Public modulus N");
   gst_DumpBin(pCtx->pucE,pCtx->iSizeE,(char *)"- Public exponent E");
   gst_DumpBin(pCtx->pucD,pCtx->iSizeD,(char *)"- Private exponent D");
}

/* ------------------------------------------------------------------------
   Print a bytes table for the debug trace
   ------------------------------------------------------------------------ */

void gst_DumpBin (unsigned char *pucBin, int iSize, char *zLabel)
{
   int i;

   printf("%s (%d bytes) :",zLabel,iSize);

   for (i = 0; i < iSize; i++)
   {
      if (((i % 16) == 0) && (iSize > 16)) printf("\n  %04d -",i);
      printf(" %02X",pucBin[i]);
   }

   printf("\n");
}




static bool _RSA_EncryptSign (tRSAKeyCtx *pKey, unsigned char *pucClearSign,
                              unsigned char **ppucEncSign)
{
   bool           ok      = true;
   unsigned char *p_res   = NULL;
   unsigned char *p_check = NULL;
   int            len;

   /* Results allocation with the size of key modulus */
   if (pKey->iSizeN <= 0)
   {
      LOGE(L"ERROR : RSA Key undefined");
      return(false);
   }

   p_res = (u8*) malloc(pKey->iSizeN);
   if (p_res == NULL)
   {
      LOGE(L"ERROR : Allocation %d bytes failed",pKey->iSizeN);
      ok = false;
   }
   else
   {
      p_check = (u8*) malloc(pKey->iSizeN);
      if (p_check == NULL)
      {
         LOGE(L"ERROR : Allocation %d bytes failed",pKey->iSizeN);
         ok = false;
      }
   }

   /* RSA encryption with the private key */
   if (ok)
   {
      len = RSA_private_encrypt(MD5_DIGEST_LENGTH,pucClearSign,p_res,
                                pKey->pKey,RSA_PKCS1_PADDING);

      if (len != pKey->iSizeN)
      {
         LOGE(L"ERROR : RSA encryption failed "
                "(result on %d bytes, %d wait)",
                len,pKey->iSizeN);
         ok = false;
      }
    }

   /* RSA Decryption with public key for checking */
   if (ok)
   {
      len = RSA_public_decrypt(pKey->iSizeN,p_res,p_check,
                               pKey->pKey,RSA_PKCS1_PADDING);

      if (len != MD5_DIGEST_LENGTH)
      {
         LOGE(L"ERROR : RSA decryption for checking failed "
                "(result on %d bytes, %d wait)",
                len,MD5_DIGEST_LENGTH);
         ok = false;
      }
      else if (memcmp(pucClearSign,p_check,MD5_DIGEST_LENGTH) != 0)
      {
         LOGE(L"ERROR : RSA decryption for checking failed "
                "(result not equal to source)");
         ok = false;
      }
    }

   /* Done, return result and free temporary buffer */
   if (ok) *ppucEncSign = p_res;
   else if (p_res != NULL) free(p_res);
   if (p_check != NULL) free(p_check);

   return(ok);
}



bool gst_RSAKeySign(const dmsBuffer &Data, const wxString& KeyFilename, dmsBuffer &Signature)
{
   tRSAKeyCtx the_key;
   bool       ok;
   unsigned char *p_enc_sign = NULL;
   unsigned char  clear_sign[MD5_DIGEST_LENGTH];

   // Load RSA Key

   gst_RSAKeyCtx_Init(&the_key);
   gst_RSAKeyCtx_SetName(&the_key, KeyFilename.c_str());

   ok = gst_RSAKeyCtx_LoadKey(&the_key);

   if (ok)
   {
      time_t date;
      localtime(&date);

      the_key.iDateGen = (int)date;

      ok = gst_RSAKeyCtx_BinToKey(&the_key);
   }

   // Make the signature

   if (ok)
   {
       MD5_CTX md5_ctx;

       MD5_Init(&md5_ctx);
       MD5_Update(&md5_ctx,Data.Begin(),Data.Len());
       MD5_Final(clear_sign,&md5_ctx);
   }

   // Signature encryption

   if (ok)
   {
      ok = _RSA_EncryptSign(&the_key,clear_sign,&p_enc_sign);
   }

   if (ok)
   {
       Signature.Set(p_enc_sign, the_key.iSizeN);
   }

   // Signature Generation done

   gst_RSAKeyCtx_Cleanup(&the_key);

   if (p_enc_sign != NULL) free(p_enc_sign);

   return ok;
}

/**
 * Return a signature with all char at ' ' (0x20)
 *
*/
bool gst_GetEmptySignature (dmsBuffer &EmptySign)
{
   unsigned char empty[kKT_DFT_SIGN_SIZE];
   int           i;

   for (i = 0; i < kKT_DFT_SIGN_SIZE; i++) empty[i] = (unsigned char)' ';

   EmptySign.Set(empty,kKT_DFT_SIGN_SIZE);

   return true;
}

/* GeniusSignTool.cpp */
