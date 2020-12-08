/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 01/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Rename object 'cIwediaXXX' to
                                          'cGeniusXXX'
   ************************************************************************ */


#include "ModuleData.h"

#include <zlib.h>

#include <openssl/md5.h>
#include <openssl/rsa.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#include "LibCriptoCompat/libcrypto-compat.h"
#endif

#include <Tools/Conversion.h>
#include <Tools/Xml.h>
#include <Tools/File.h>
#include <Tools/Header.h>
#include <Tools/Log.h>

#include <ObjectCarousel/ObjectCarousel.h>
#include <ObjectCarousel/ModuleData.h>
#include <GeniusCarousel/GeniusImagesData.h>


#include "Module.h"
#include "Group.h"
#include "DataCarousel.h"

const char *DmsModuleDataTypeName[] = {
    "Data",
    "Group",                      //MODULE_DATA_TYPE_GROUP
    "Reference",                  //MODULE_DATA_TYPE_REFERENCE
    "File",                       //MODULE_DATA_TYPE_FILE
    "Dir",                        //MODULE_DATA_TYPE_DIR
    "ModuleData",                 //MODULE_DATA_TYPE_MODULE_DATA
    "Compress",                   //MODULE_DATA_TYPE_COMPRESS
    "MD5",                        //MODULE_DATA_TYPE_MD5
    "Signature",                  //MODULE_DATA_TYPE_SIGNATURE
    "BiopData",                   //MODULE_DATA_TYPE_BIOP_DATA
    "BiopFileMessage",            //MODULE_DATA_TYPE_BIOP_FILE_MESSAGE
    "BiopDirMessage",             //MODULE_DATA_TYPE_BIOP_DIR_MESSAGE
    "BiopServiceGatewayMessage",  //MODULE_DATA_TYPE_BIOP_SERVICE_GATEWAY_MESSAGE
    "GeniusImagesData",           //MODULE_DATA_TYPE_GENIUS_IMAGES_DATA
    "GeniusImagesBlock",          //MODULE_DATA_TYPE_GENIUS_IMAGES_BLOCK
    NULL};

const uchar GENIUS_RSA_PUBLIC[128] =
{
    0xBF,0x7F,0x17,0xA2,0x19,0x9A,0x13,0x5D,0x0D,0x4A,0x0B,0xA0,0x60,0xF8,0xAD,0x84,
    0x3E,0x73,0x27,0xF5,0x44,0xF1,0x5F,0x14,0xDB,0x25,0xB9,0x8E,0xEE,0xD6,0x8A,0x0B,
    0xDA,0x92,0xE6,0xF5,0xB2,0x07,0x2F,0xC4,0x8B,0x29,0xB7,0x1E,0xA4,0x97,0xA6,0xE0,
    0xD2,0xDE,0x03,0x51,0xD6,0xA1,0x00,0x9A,0x23,0x7B,0x71,0xB2,0x6E,0xFE,0x55,0x18,
    0x1C,0x63,0xC3,0x08,0xDE,0xA6,0xC1,0xB0,0x75,0x53,0x27,0xA1,0x56,0xB8,0x59,0x38,
    0xD6,0x4E,0x10,0x5C,0x97,0x9B,0xEC,0x0D,0xA7,0x15,0x6E,0xF8,0x70,0x75,0x0B,0xD7,
    0x5F,0x5D,0xF4,0xF7,0x77,0x79,0x1C,0x8B,0x9E,0x73,0x71,0x58,0xAA,0xFD,0x7E,0xB0,
    0x12,0xF2,0x36,0x99,0xF4,0x8F,0x70,0x9E,0x45,0x0C,0xCD,0x9F,0x25,0xD9,0xCF,0xFB
};
const uchar GENIUS_RSA_PUBLIC_EXPOSANT[3] =
{
    0x01,0x00,0x01
};
const uchar GENIUS_RSA_PRIVATE_EXPOSANT[128] =
{
    0x84,0xDD,0x4A,0x69,0x76,0xF7,0xF7,0x1D,0x65,0xDE,0x0D,0x29,0x0E,0x7F,0x1B,0xED,
    0x18,0x63,0x4F,0xC3,0x16,0x3D,0x9E,0x69,0xDB,0x0F,0x56,0xAA,0xBC,0x3D,0xF0,0x73,
    0xFF,0x08,0x53,0xE4,0xFE,0xCB,0x8B,0xB2,0x03,0x98,0x42,0xEC,0xA8,0xE8,0x6B,0xA9,
    0xAB,0xC4,0xCC,0x6A,0xD8,0xCE,0xC6,0x9B,0x2B,0xA9,0x37,0x9E,0xC4,0xF2,0x03,0xD9,
    0x4C,0xCE,0x35,0x4D,0x7C,0x1C,0x3B,0xF0,0xF5,0x55,0x28,0xC4,0xEF,0xB3,0xE7,0x0D,
    0x86,0xD7,0xD0,0xB6,0x87,0x15,0x91,0x8B,0x52,0x1C,0x5C,0x4C,0x06,0x9A,0xB6,0x6A,
    0xE7,0x14,0xB7,0x7D,0x5B,0xFE,0x4B,0x0C,0xED,0x81,0x20,0xD4,0x77,0x1A,0x46,0x87,
    0x45,0x26,0xB1,0x9E,0xA4,0xB2,0x57,0x5F,0x96,0x1C,0x52,0xA1,0x03,0xE5,0x5E,0x01
};



#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsModuleDataList);


/* ########################################################################
   dmsModuleData

   Chaque noeud peut effectuer trois actions

   - Concatenation des buffers des sous-noeuds

   - Transformation sur le buffer concaténé

   - Création dynamique de nouveaux noeuds

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsModuleData::dmsModuleData(dmsModuleData* parent, EnumModuleDataType type)
{
    m_eType          = type;
    m_poParent       = parent;
    m_poModule       = NULL;
    m_bCompiling     = false;
    m_iOriginalSize  = 0;
    m_iSize          = 0;
    m_bKeepData      = false;
    m_bAllowAppend   = true;
    m_poDataCarousel = parent?parent->m_poDataCarousel:NULL;
    m_bUpdated       = false;
    m_iCompileLoop   = 0;
    m_bCompileEnable = true;
    m_bCompiled      = false;
    m_bTrace         = false;

    m_oChildList.DeleteContents(false);
}

dmsModuleData::~dmsModuleData()
{
    dmsModuleData* child;

    FOREACH(dmsModuleDataList, m_oChildList, child)
    {
        delete child;
    }

    FOREACH2(dmsModuleDataList, m_oOldChildList, child)
    {
        delete child;
    }
}

void dmsModuleData::MoveTo(dmsModuleData *newParent)
{
    // Ancien pere
    if (m_poParent)
        m_poParent->m_oChildList.DeleteObject(this);

    // Nouveau pere
    m_poParent = newParent;

    m_poParent->m_oChildList.Append(this);

    m_poParent->m_bUpdated  = false;
    m_poParent->m_bCompiled = false;
}


void dmsModuleData::Remove()
{
    if (m_poParent)
    {
        if (! m_poParent->m_oChildList.DeleteObject(this))
        {
            LOGE(L"DEV Bad Link on [%s], not child of [%s]", TraceLongName(), m_poParent->TraceLongName());
        }
    }

    m_poParent = NULL;
}



void dmsModuleData::GetNotEmpty(dmsModuleDataList &list, bool clear)
{
    dmsModuleData* child;

    if (clear) list.Clear();

    if (m_iSize) {list.Append(this); return;}

    FOREACH(dmsModuleDataList, m_oChildList, child)
    {
        child->GetNotEmpty(list, false);
    }
}

wxString dmsModuleData::FindDirTrace()
{
    dmsModuleData* child;

    if (m_oDirTrace.Len()) return m_oDirTrace;

    FOREACH(dmsModuleDataList, m_oChildList, child)
    {
        wxString res = child->FindDirTrace();

        if (res.Len()) return res;
    }

    return "";
}

void dmsModuleData::ClearBuffer()
{
    m_oBuffer.Clear();
    m_iSize = 0;
}

/* ========================================================================

   ======================================================================== */

bool dmsModuleData::Init(wxXmlNode *WXUNUSED(node))
{
    return true;
}

bool dmsModuleData::Load(wxXmlNode *node)
{
   dmsModuleData     *child;
   wxXmlNode         *childNode;
   dmsObjectCarousel *p_oc;
   cGeniusImagesData *p_gimgd;
   int                num;

   if (! Init(node)) return false;

   for (childNode = node->GetChildren(); childNode; childNode=childNode->GetNext())
   {
      child = NULL;

      switch (dmsGetStrRank(childNode->GetName(), DmsModuleDataTypeName))
      {
      case MODULE_DATA_TYPE_FILE:
         child = new dmsModuleDataFile(this, childNode->GetValue());
         break;
      case MODULE_DATA_TYPE_DIR:
         child = new dmsModuleDataDir(this, childNode->GetValue());
         break;
      case MODULE_DATA_TYPE_MODULE_DATA:
         child = new dmsModuleDataModule(this, childNode->GetValue(0));
         break;
      case MODULE_DATA_TYPE_GROUP:
         child = new dmsModuleDataGroup(this);
         break;
      case MODULE_DATA_TYPE_COMPRESS:
         child = new dmsModuleDataCompress(this);
         break;
      case MODULE_DATA_TYPE_MD5:
         child = new dmsModuleDataMD5(this);
         break;
      case MODULE_DATA_TYPE_SIGNATURE:
         child = new dmsModuleDataSignature(this);
         break;
      case MODULE_DATA_TYPE_BIOP_DATA:
         p_oc = m_poDataCarousel->m_poOC;
         LOG_AF(p_oc, LOGE(L"BiopData not allowed : no object carousel defined"));
         child = new dmsModuleDataBiopData(this,p_oc);
         break;
      case MODULE_DATA_TYPE_BIOP_DIR_MESSAGE:
         p_oc = m_poDataCarousel->m_poOC;
         LOG_AF(p_oc, LOGE(L"BiopDirMessage not allowed : no object carousel defined"));

         if (childNode->Read("num?", &num))
         {
            child = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_DIR_MESSAGE,
                                             p_oc->m_oDirMessageList[num], p_oc);
         }
         else
         {
            child = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_DIR_MESSAGE,
                                             NULL, p_oc);
         }
         break;
      case MODULE_DATA_TYPE_BIOP_FILE_MESSAGE:
         p_oc = m_poDataCarousel->m_poOC;
         LOG_AF(p_oc, LOGE(L"BiopFile not allowed : no object carousel defined"));
         childNode->Read("", &num);
         child = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_FILE_MESSAGE,
                                          p_oc->m_oFileMessageList[num],p_oc);
         break;
      case MODULE_DATA_TYPE_BIOP_SERVICE_GATEWAY_MESSAGE:
         p_oc = m_poDataCarousel->m_poOC;
         LOG_AF(p_oc, LOGE(L"ServiceGatewayMessage not allowed : no object carousel defined"));
         child = new dmsModuleDataBiopRef(this, MODULE_DATA_TYPE_BIOP_SERVICE_GATEWAY_MESSAGE,
                                          NULL, p_oc);
         break;

      case MODULE_DATA_TYPE_GENIUS_IMAGES_DATA:
         p_gimgd = new cGeniusImagesData(this);
           if (!p_gimgd->Init(childNode)) return false;
           child = p_gimgd;
         break;

      default:
         LOGE(L"Unknown module data type [%s]", childNode->GetName());
         return false;
      }

      childNode->Used();

      if (child)
      {
         m_oChildList.Append(child);
         if (! child->Load(childNode)) return false;
      }
   }

   return true;
}

dmsModuleData* dmsModuleData::FindType(EnumModuleDataType type)
{
    dmsModuleData* child;
    dmsModuleData* result;

    if (m_eType==type) return this;

    FOREACH(dmsModuleDataList, m_oChildList, child)
    {
        result = child->FindType(type);

        if (result) return result;
    }

    return NULL;
}

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

void dmsModuleData::V_Concat()
{
    dmsModuleData* child;

    if (m_oChildList.GetCount()==0) return; // Si pas de fils, on ne vide pas le buffer

    if (m_oBuffer.Len())
    {
        // On vérifie que les fils peuvent encore donner leur contenu

        FOREACH(dmsModuleDataList, m_oChildList, child)
        {
            if (! child->m_bKeepData) return; // 1 Fils vide, rien faire
        }
    }

    m_oBuffer.Clear();

    bool updated = true;
    FOREACH(dmsModuleDataList, m_oChildList, child)
    {
        if (! child->m_bUpdated) updated = false;
    }

    FOREACH2(dmsModuleDataList, m_oChildList, child)
    {
        m_oBuffer.Append(child->m_oBuffer);

        if (! child->m_bKeepData && updated) child->m_oBuffer.Clear();
    }

    m_iSize = m_oBuffer.Len();
}

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

bool dmsModuleData::Compile()
{
    dmsModuleData* child;

    int lap=0;

    if (m_bCompiled) return true;

    m_bCompiled = true;

    m_iCompileLoop++;

    bool childDone = false;

    while (! childDone)
    {
        // Intro

        m_bUpdated = true;

        // Compilation des fils

        //LOG0("======> %d : %s", lap, TraceLongName()); // Bon log !

        FOREACH(dmsModuleDataList, m_oChildList, child)
        {
            if (! child->Compile()) return false;
            if (! child->m_bUpdated) m_bUpdated = false;
        }

        childDone = true;

        // Concatenation des fils

        if (m_bAllowAppend)
            V_Concat();
        else
            ClearBuffer();

        // Execution / Transformation du buffer
        if (m_bCompileEnable)
        {
            m_iOriginalSize = m_oBuffer.Len();

            if (!CompileBuffer()) return false;

            m_iSize = m_oBuffer.Len();
        }

        // Intégration des noeuds crées dynamiquement
        if (m_bCompileEnable && m_oCompiledList.GetCount())
        {
            m_bCompileEnable = false; // Ne pourra être fait qu'une seule fois

            FOREACH(dmsModuleDataList, m_oChildList, child)
                m_oOldChildList.Append(child);

            m_oChildList.Clear();

            FOREACH2(dmsModuleDataList, m_oCompiledList, child)
                m_oChildList.Append(child);

            m_oCompiledList.Clear();

            childDone = false; // Nouveaux fils --> On recommence

            if (m_bTrace)
            {
                LOG0(L"------------------------------------------");
                TraceRec(stdout, "    $ ");
                m_bTrace = false;
            }
        }

        lap++;
    }

    if (m_bUpdated && m_oDirTrace.Len()) FileTrace();

    return true;
}



void dmsModuleData::GetChildrenOrSubCompiled(dmsModuleDataList &List)
{
    dmsModuleData* child;

    FOREACH(dmsModuleDataList, m_oChildList, child)
    {
        if (child->m_oCompiledList.GetCount()==0)
        {
            List.Append(child);
        }
        else
        {
            dmsModuleData* subchild;
            FOREACH2(dmsModuleDataList, child->m_oCompiledList, subchild)
            {
                List.Append(subchild);
            }
        }
    }
}

void dmsModuleData::GetChildrenAndCompiled(dmsModuleDataList &List)
{
    dmsModuleData* child;

    FOREACH(dmsModuleDataList, m_oChildList, child)
    {
        List.Append(child);
    }

    FOREACH2(dmsModuleDataList, m_oCompiledList, child)
    {
        List.Append(child);
    }
}


bool dmsModuleData::Affect(dmsModule *module)
{
    m_poModule = module;
    return true;
}



bool dmsModuleData::AffectRec(dmsModule* module)
{
    dmsModuleData* child;

    if (m_poModule && module != m_poModule)
    {
        LOGE(L"Multiple affectation of module data");
        return false;
    }

    if (! Affect(module)) return false;

    if (m_bUpdated && m_oDirTrace.Len()) FileTrace();

    if (! m_bUpdated) m_bCompiled = false;

    FOREACH(dmsModuleDataList, m_oChildList, child)
    {
        if (! child->AffectRec(m_poModule)) return false;
    }

    return true;
}

wxString dmsModuleData::TraceInfo()
{
    return STR("%s%s%s, %s%dB%s",
        (m_oBuffer.m_iBufferLen==0)?"":"#",
        TraceFullName(),
        m_iCompileLoop>1?STR("*%d", m_iCompileLoop):"",
        m_iOriginalSize&&m_iOriginalSize!=m_iSize?STR("%dB=>", m_iOriginalSize):"",
        m_iSize,
        m_bUpdated?"":", To update !!!",
        m_bCompiled?"":", To compile !!!");
}

wxString dmsModuleData::TraceFullName()
{
    wxString name = TraceName();
    wxString res;

    if (name.Len())
        res.Printf("%s", name);
    else
        res.Printf("%s", DmsModuleDataTypeName[m_eType]);

    return res;
}

wxString dmsModuleData::TraceLongName()
{
    if (m_poParent)
        return m_poParent->TraceLongName()+"/"+TraceFullName();
    else
        return TraceFullName();
}
/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

void dmsModuleData::TraceRec(FILE* f, const wxString &prefix)
{
    dmsModuleData* child;

    fprintf(f, "%s%s\n", (const char *)prefix, (const char *)TraceInfo());

    FOREACH(dmsModuleDataList, m_oChildList, child)     child->TraceRec(f, prefix+"   ");
    FOREACH2(dmsModuleDataList, m_oCompiledList, child) child->TraceRec(f, prefix+"   (compiled)");
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModuleDataGroup::dmsModuleDataGroup(dmsModuleData* parent)
:dmsModuleData(parent,MODULE_DATA_TYPE_GROUP)
{
    m_bAllowAppend = false;
    m_iMaxSize     = 0;
}

/* ========================================================================

   ======================================================================== */

bool dmsModuleDataGroup::Init(wxXmlNode *node)
{
    dmsXmlContextManager m(node);

    m.Add("MaxSize", new dmsvNumeric(&m_iMaxSize, ":min:1"));

    return m.Validate();
}


bool dmsModuleDataGroup::CompileBuffer()
{
    dmsModuleData* group = NULL;
    dmsModuleData* child;

    //TraceRec(stdout, "xxxxxxxxxxxxxxxxx");

    dmsModuleDataList notEmptyList;

    GetNotEmpty(notEmptyList);

    if (false)
    {
        FOREACH2(dmsModuleDataList, m_oChildList, child)
        {
            if (child->m_oBuffer.Len() == 0)
            {
                dmsModuleData* child2;

                FOREACH(dmsModuleDataList, child->m_oChildList, child2)
                {
                    child2->MoveTo(this);
                }
            }
        }
    }

    size_t size = 0;

    FOREACH(dmsModuleDataList, notEmptyList, child)
    {
        // Taille atteinte : on passe à la concaténation suivante

        if (group==NULL || size+child->m_oBuffer.Len() > (size_t) m_iMaxSize)
        {
            if (group)
            {
                group->V_Concat();
                group->m_bUpdated = true;
            }

            group = new dmsModuleData(this, MODULE_DATA_TYPE_DATA);

            m_oCompiledList.Append(group);

            size=0;
        }

        child->MoveTo(group);

        size += child->m_oBuffer.Len();
    }

    if (group)
    {
        group->V_Concat();
        group->m_bUpdated = true;
    }

    //LOG0("--"); TraceRec(stdout, "*****************>");

    return true;
}

wxString dmsModuleDataGroup::TraceName()
{
    return STR("Group_%d_Bytes", m_iMaxSize);
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModuleDataDir::dmsModuleDataDir(dmsModuleData* parent, const wxString &filename)
: dmsModuleData(parent, MODULE_DATA_TYPE_DIR)
{
    m_oOutputFilename = filename;

    dmsStandardFileName(m_oOutputFilename);

    // Creation a partir du contenu d'un repertoire.
    // Les donnees des fichiers sont mises bout a bout et
    // regroupees dans un seul module.
    m_bAllowAppend = true;
}

dmsModuleDataDir::~dmsModuleDataDir()
{

}

wxString dmsModuleDataDir::TraceName()
{
    return wxFileNameFromPath(m_oOutputFilename);
}

/* ========================================================================

   ======================================================================== */

bool dmsModuleDataDir::CompileBuffer()
{
    dmsFileNode root(NULL, m_oOutputFilename);

    root.Init();

    LOG_AF(root.Expand(), LOGE(L"Error while expanding dir [%s]", m_oOutputFilename));

    dmsFileNode *node;

    FOREACH(dmsFileList, root.m_oChildList, node)
    {
        switch (node->m_oInfo.m_eType)
        {
        case DMS_FILE_TYPE_STANDARD:
            {
                dmsModuleDataFile *child;
                child  = new dmsModuleDataFile(this, node->m_oInfo.m_oFilename);
                m_oCompiledList.Append(child);
            }
            break;
        case DMS_FILE_TYPE_DIRECTORY:
            {
                dmsModuleDataDir *child;
                child = new dmsModuleDataDir(this, node->m_oInfo.m_oFilename);
                m_oCompiledList.Append(child);
            }
            break;
        default:
            break;
        }
    }

    LOG_AF(m_oCompiledList.GetCount(), LOGE(L"No file in directory [%s]", m_oOutputFilename));

    return true;
}



/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModuleDataFile::dmsModuleDataFile(dmsModuleData* parent, const wxString &filename)
: dmsModuleData(parent, MODULE_DATA_TYPE_FILE)
{
    m_oOutputFilename = filename;

    dmsStandardFileName(m_oOutputFilename);
}

dmsModuleDataFile::~dmsModuleDataFile()
{

}

wxString dmsModuleDataFile::TraceName()
{
    return wxFileNameFromPath(m_oOutputFilename);
}


bool dmsModuleDataFile::Affect(dmsModule *module)
{
    if (m_poModule &&
        m_poModule->m_oFirstFileName.IsEmpty())
        m_poModule->m_oFirstFileName = m_oOutputFilename;

    return true;
}

/* ========================================================================

   ======================================================================== */

bool dmsModuleDataFile::CompileBuffer()
{
    LOG_AF(m_oChildList.IsEmpty(), LOGE(L"No child allowed under File Tag"));

    return m_oBuffer.Load(m_oOutputFilename);
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModuleDataModule::dmsModuleDataModule(dmsModuleData* parent, int id)
: dmsModuleData(parent, MODULE_DATA_TYPE_MODULE_DATA)
{
    m_iId   = id;
}


dmsModuleDataModule::~dmsModuleDataModule()
{

}


/* ========================================================================

   ======================================================================== */

bool dmsModuleDataModule::CompileBuffer()
{
    LOG_AF(m_oChildList.IsEmpty(), LOGE(L"No child allowed under ModuleData Tag"));

    dmsModule* module = m_poModule->m_poGroup->FindModule(m_iId);

    LOG_AF(module, LOGE(L"No module [%d]", m_iId));

    if (! module->Compile()) return false;

    m_oBuffer.Set(module->m_poModuleData->m_oBuffer);

    return true;
}


/* ########################################################################

   ######################################################################## */
/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModuleDataCompress::dmsModuleDataCompress(dmsModuleData* parent) : dmsModuleData(parent, MODULE_DATA_TYPE_COMPRESS)
{
}


dmsModuleDataCompress::~dmsModuleDataCompress()
{

}


/* ========================================================================

   ======================================================================== */

bool dmsModuleDataCompress::CompileBuffer()
{
    return Compress();
}

bool dmsModuleDataCompress::Compress()
{
    dmsBuffer res;
    ulong len = m_oBuffer.Len()*1.1+12;

    res.Alloc(len);

    int code = compress(res.Begin(), &len, m_oBuffer.Begin(), m_oBuffer.Len());

    LOG_AF(code == Z_OK, LOGE(L"Error while zlib compression %d Bytes (zlib error code: %d)",
        m_oBuffer.Len(),
        code));

    res.ReAlloc(len);

    res.Move(m_oBuffer);

    return true;
}


wxString dmsModuleDataCompress::TraceName()
{
    return "zlib";
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModuleDataMD5::dmsModuleDataMD5(dmsModuleData* parent) : dmsModuleData(parent, MODULE_DATA_TYPE_MD5)
{
}


dmsModuleDataMD5::~dmsModuleDataMD5()
{

}


/* ========================================================================

   ======================================================================== */



bool dmsModuleDataMD5::CompileBuffer()
{
    MD5_CTX MD5;

    dmsBuffer res;

    res.Alloc(MD5_DIGEST_LENGTH);

    MD5_Init(&MD5);
    MD5_Update(&MD5, res.Begin(), res.Len());
    MD5_Final(res.Begin(), &MD5);

    res.Move(m_oBuffer);

    return true;
}



/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsModuleDataSignature::dmsModuleDataSignature(dmsModuleData* parent) : dmsModuleData(parent, MODULE_DATA_TYPE_SIGNATURE)
{
}


dmsModuleDataSignature::~dmsModuleDataSignature()
{

}


/* ========================================================================

   ======================================================================== */


bool dmsModuleDataSignature::CompileBuffer()
{
    MD5_CTX MD5;

    dmsBuffer res;

    res.Alloc(m_oBuffer.Len()+128); // Buffer augmenté de la taille de la clé
    res.Write(m_oBuffer);

    // Calcul MD5

    uchar md5[MD5_DIGEST_LENGTH];

    MD5_Init(&MD5);
    MD5_Update(&MD5, m_oBuffer.Begin(), m_oBuffer.Len());
    MD5_Final(md5, &MD5);

    // Signalture MD5

    RSA* RSA;

    RSA = RSA_new();

    BIGNUM *n = BN_bin2bn(GENIUS_RSA_PUBLIC,           128, NULL);
    BIGNUM *e = BN_bin2bn(GENIUS_RSA_PUBLIC_EXPOSANT,  3,   NULL);
    BIGNUM *d = BN_bin2bn(GENIUS_RSA_PRIVATE_EXPOSANT, 128, NULL);

    RSA_set0_key(RSA, n, e, d);

    if (RSA_private_encrypt(MD5_DIGEST_LENGTH, md5, res.Current(), RSA, RSA_PKCS1_PADDING) != 128)
    {
        LOGE(L"RSA encryption failure");
    }

    BN_free(n);
    BN_free(e);
    BN_free(d);
    RSA_free(RSA);

    // FIN

    res.Move(m_oBuffer);

    return true;
}
