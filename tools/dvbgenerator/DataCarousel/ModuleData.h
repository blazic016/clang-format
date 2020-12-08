/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 01/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Rename object 'cIwediaXXX' to
                                          'cGeniusXXX'
   ************************************************************************ */


#ifndef _DATA_CAROUSEL_MODULE_DATA_H_
#define _DATA_CAROUSEL_MODULE_DATA_H_

#include <wx/wx.h>

#include "Tools/Tools.h"

#include "Tools/Header.h"


class dmsModule;
class dmsModuleData;
class dmsDataCarousel;
class wxXmlNode;
class dmsBiopMessage;
class dmsFile;

typedef enum
{
    MODULE_DATA_TYPE_DATA=0,
    MODULE_DATA_TYPE_GROUP,
    MODULE_DATA_TYPE_REFERENCE,
    MODULE_DATA_TYPE_FILE,
    MODULE_DATA_TYPE_DIR,
    MODULE_DATA_TYPE_MODULE_DATA,
    MODULE_DATA_TYPE_COMPRESS,
    MODULE_DATA_TYPE_MD5,
    MODULE_DATA_TYPE_SIGNATURE,
    MODULE_DATA_TYPE_BIOP_DATA,
    MODULE_DATA_TYPE_BIOP_FILE_MESSAGE,
    MODULE_DATA_TYPE_BIOP_DIR_MESSAGE,
    MODULE_DATA_TYPE_BIOP_SERVICE_GATEWAY_MESSAGE,
    MODULE_DATA_TYPE_GENIUS_IMAGES_DATA,
   MODULE_DATA_TYPE_GENIUS_IMAGES_BLOCK
} EnumModuleDataType;


extern const char *DmsModuleDataTypeName[];

WX_DECLARE_LIST(dmsModuleData, dmsModuleDataList);

/* ------------------------------------------------------------------------
   dmsModuleData

   Permet d'enchainer des actions définies de manière hierarchique
   (avec init xml)
   ------------------------------------------------------------------------ */

class dmsModuleData : public wxObject
{
protected:
    dmsModuleData*     m_poParent;

public:
    dmsDataCarousel*   m_poDataCarousel;
    dmsModule*         m_poModule;
    EnumModuleDataType m_eType;

    dmsBuffer          m_oBuffer;

    int      m_iOriginalSize;   // Taille du buffer avant traitement
    int      m_iSize;           // Taille du buffer après traitement
    bool     m_bCompiling;      // Indique si déjà en compilation (éviter les boucles)
    bool     m_bKeepData;       // Indique si le buffer de data doit être libéré après usage
    bool     m_bAllowAppend;    // Autorise la concaténation des fils
    bool     m_bUpdated;        // Indique si le contenu est à jour par rapport aux références (ex: module)
    bool     m_bCompiled;       // Indique si la compilation a déjà été faite
    bool     m_bCompileEnable;  // Indique si le noeud et ses fils peut être compilés
    int      m_iCompileLoop;    // Indique le nombre de fois ou la compil a ete executee
    wxString m_oDirTrace;       // Debug
    bool     m_bTrace;          // Debug pour afficher l'arbre après compilation
    wxString m_oOutputFilename; // Nom en cas de sauvegarde sur disque

    // Création de nouveaux noeuds au cours de la compilation

    dmsModuleDataList  m_oChildList;    // Fils
    dmsModuleDataList  m_oOldChildList; // Fils présents avant la compilation et plus après

protected:
    dmsModuleDataList  m_oCompiledList; // Liste temporaire de noeuds créés dynamiquement par les "CompileBuffer".

public:
    dmsModuleData(dmsModuleData* parent, EnumModuleDataType type);
    virtual ~dmsModuleData();

    void MoveTo(dmsModuleData *newParent);
    void Remove();
    void GetNotEmpty(dmsModuleDataList &list, bool clear=true);

    virtual void V_Concat();

    bool Compile();
    virtual bool CompileBuffer(){return true;}

    void ClearBuffer();


    bool Load(wxXmlNode *node);

    wxString TraceInfo();
    wxString TraceFullName();
    wxString TraceLongName();

    void TraceRec(FILE* f, const wxString &prefix="");

    virtual wxString TraceName(){return "";}
    virtual void FileTrace(){LOGW(L"Dev FileTrace Method Missing");}

    dmsModuleData* FindType(EnumModuleDataType type);

    wxString FindDirTrace();

    virtual bool Init(wxXmlNode *node);

    bool AffectRec(dmsModule *module);

    void GetChildrenOrSubCompiled(dmsModuleDataList &List);
    void GetChildrenAndCompiled(dmsModuleDataList &List);

private:
    virtual bool Affect(dmsModule *module);
};


/* ------------------------------------------------------------------------
   Append "pure". Seulement pour permettre une instanciation de la
   classe virtuelle
   ------------------------------------------------------------------------ */

class dmsModuleDataGroup : public dmsModuleData
{
private:
    int m_iMaxSize;        // Taille max dans la concatenation des fils
public:
    dmsModuleDataGroup(dmsModuleData* parent);

    virtual bool CompileBuffer();
    virtual wxString TraceName();
    virtual bool Init(wxXmlNode *node);
};


class dmsModuleDataReference : public dmsModuleData
{
public:
    dmsModuleDataReference(dmsModuleData* parent):dmsModuleData(parent,MODULE_DATA_TYPE_REFERENCE){;}
    virtual bool CompileBuffer(){return true;}
};


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */


class dmsModuleDataDir : public dmsModuleData
{
public:
    //wxString m_oFilename;

public:
    dmsModuleDataDir(dmsModuleData* parent, const wxString &filename);
    virtual ~dmsModuleDataDir();

    bool CompileBuffer();
    virtual wxString TraceName();
};


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */


class dmsModuleDataFile : public dmsModuleData
{
public:
    //wxString m_oFilename;

public:
    dmsModuleDataFile(dmsModuleData* parent, const wxString &filename);
    virtual ~dmsModuleDataFile();

    bool CompileBuffer();
    virtual wxString TraceName();
    bool Affect(dmsModule *module);
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsModuleDataModule : public dmsModuleData
{
public:
    int m_iId;

public:
    dmsModuleDataModule(dmsModuleData* parent, int id);
    virtual ~dmsModuleDataModule();

    bool CompileBuffer();
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsModuleDataCompress : public dmsModuleData
{
public:
    dmsModuleDataCompress(dmsModuleData* parent);
    virtual ~dmsModuleDataCompress();

    virtual wxString TraceName();
    bool CompileBuffer();
    bool Compress();
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsModuleDataMD5 : public dmsModuleData
{
public:
    dmsModuleDataMD5(dmsModuleData* parent);
    virtual ~dmsModuleDataMD5();

    bool CompileBuffer();
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsModuleDataSignature : public dmsModuleData
{
public:
    dmsModuleDataSignature(dmsModuleData* parent);
    virtual ~dmsModuleDataSignature();

    bool CompileBuffer();
};


#endif /* _DATA_CAROUSEL_MODULE_DATA_H_ */
