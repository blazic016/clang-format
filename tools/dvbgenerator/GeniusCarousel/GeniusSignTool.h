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
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/lib/GeniusCarousel/GeniusSignTool.h $
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


#ifndef _GENIUS_SIGN_TOOL_H_
#define _GENIUS_SIGN_TOOL_H_

#include <wx/wx.h>

#include <openssl/rsa.h>

class dmsBuffer;

#define kKT_DFT_KEY_LENGTH     1024 // Default keys size in bits (128 bytes)
#define kKT_DFT_PUBLIC_EXP    65537 // Default public exponent (01 00 01)
#define kKT_SZ_FNAME            255 // Maximum size of a file name

#define kKT_DFT_SIGN_SIZE       128 // Default signature size


#define kKT_FILE_KEY_EXT   "key"           // File key extension
#define kKT_FILE_BL_SUFF   "_loal_crypt.c" // Boot loader src public key suffixe

/* ------------------------------------------------------------------------
   RSA Key context
   ------------------------------------------------------------------------ */

typedef struct
{
    int      iDateGen;                 // Date/hour of generation
    char     zIdent[kKT_SZ_FNAME+1];   // Identifier (original name)
    char     zFileKey[kKT_SZ_FNAME+1]; // Key file name
    char     zFileBL[kKT_SZ_FNAME+1];  // Public key source name
    int      iSizeN;                   // Size of public modulo (in bytes)
    unsigned char *pucN;               // The public modulo
    int      iSizeE;                   // Size of public exponent (in bytes)
    unsigned char *pucE;               // The public exponent
    int      iSizeD;                   // Size of private exponent (in bytes)
    unsigned char *pucD;               // The private exponent
    RSA      *pKey;                    // OpenSSL Key
} tRSAKeyCtx;

/* ------------------------------------------------------------------------
   Fonctions
   ------------------------------------------------------------------------ */

void gst_RSAKeyCtx_Init            (tRSAKeyCtx *pCtx);
void gst_RSAKeyCtx_Cleanup         (tRSAKeyCtx *pCtx);
void gst_RSAKeyCtx_SetName         (tRSAKeyCtx *pCtx, const char *pName);
bool gst_RSAKeyCtx_KeyToBin        (tRSAKeyCtx *pCtx);
bool gst_RSAKeyCtx_BinToKey        (tRSAKeyCtx *pCtx);
bool gst_RSAKeyCtx_SaveKey         (tRSAKeyCtx *pCtx);
bool gst_RSAKeyCtx_CreatePubKeySrc (tRSAKeyCtx *pCtx);
bool gst_RSAKeyCtx_LoadKey         (tRSAKeyCtx *pCtx);
void gst_RSAKeyCtx_Dump            (tRSAKeyCtx *pCtx, bool bWithTitle);
void gst_DumpBin                   (unsigned char *pucBin, int iSize, char *zLabel);
bool gst_RSAKeySign                (const dmsBuffer &Data,
                                    const wxString& KeyFilename,
                                    dmsBuffer &Signature);
bool gst_GetEmptySignature         (dmsBuffer &EmptySign);


#endif /* _GENIUS_SIGN_TOOL_H_ */

/* GeniusSignTool.h */
