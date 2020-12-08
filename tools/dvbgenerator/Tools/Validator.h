/* ************************************************************************
   SmarDTV

   Description : generic validator

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DMS_TOOLS_VALIDATOR_H_
#define _DMS_TOOLS_VALIDATOR_H_

#include <wx/wx.h>

#include "Log.h"

class wxRegEx;

class dmsValidator;
class dmsValidatorManager;
class dmsvContext;
class dmsvContextManager;


/* ========================================================================

   Gestion des "validateurs"

   Les validateurs :

   - Effectuent des tests de validation
   - Mettent au propre une valeur (Format)
   - (optionel) Associent la valeur "chaine de caractères" à une variable pour
     du mapping automatique

   ======================================================================== */


WX_DECLARE_LIST(dmsValidator, dmsValidatorList);


/* ------------------------------------------------------------------------

   Forme générique d'un "Validator"

   ------------------------------------------------------------------------ */

class dmsValidator
{
friend class dmsValidatorManager;
friend class dmsvContextManager;
friend class dmsvContext;

protected:
    // Gestion
    dmsValidatorManager* m_poManager;

    // Identification
    wxString m_eType;
    wxString m_oName;

    // Valeurs
    wxString  m_oStrValue;         // Valeur à valider
    wxString  m_oFirstValue;       // Historique : premiere valeur (pour du reset)
    wxString  m_oInitStr;          // Chaine d'initialisation
    wxString* m_poDefaultValue;    // Valeur par défaut (défini à l'initialisation)

    // Association
    void* m_poVar; // Variable associée

    // Validation générique
    wxString m_oTrim;       // Suppression des espaces
    bool     m_bAllowEmpty; // Autorise une valeur vide

    // Activation dans le manager
    //bool     m_bEnabled;     // Validateur activé ou non dans le manager
    bool     m_bAllowReset;  // Autorise le reset dans le manager
    bool     m_bInitialized; // Vérifie si le validateur est correctement initialisé (pour debug de nouveaux types de validateurs)
    bool     m_bOptional;    // Indique si la variable peut ne pas être affectée

private:
    wxString m_oError;       // Erreurs dans la validation
    bool     m_bErrorEnable; // Desactive la fonction "generation de message d'erreur"

public:
    // Constructeur / Desctructeur
    void ConstInit();
    dmsValidator(dmsValidatorManager* manager);
    dmsValidator(dmsValidatorManager* manager, void* var, const wxString &init);
    virtual ~dmsValidator();

    // Accesseurs
    void SetMapping(void *var);
    void SetName(const wxString &name){m_oName = name;}
    wxString GetName(){return m_oName;}
    void SetManager(dmsValidatorManager* manager);
    wxString GetError();
    void SetError(const wxChar *format, ...);
    //bool IsEnabled(){return m_bEnabled;}
    //void Enable(bool value){m_bEnabled=value;}

    // Initialisation
    bool Init(const wxString &info="");
    bool Init(const wxString &name, const wxString &value);

    // Methodes principales
    bool     Set(const char* value);
    bool     SetNoError(const char* value);
    wxString Get(){return m_oStrValue;}
    bool     Optional(); // Indique que la variable n'est pas affectée

    // Gestion
    void Clear(){m_oError.Clear();}

private:
    virtual wxString V_Default(){return "";}
    virtual bool V_Init(const wxString &name, const wxString &value)=0;
    virtual bool V_Validate(const wxString &value)=0;
    virtual void V_Format(){;}
    virtual void V_SetVar()=0;
    virtual void V_GetVar()=0;

    bool StartValidation();
    void SetDefault();

public:

    void InitReset(){Get();m_oFirstValue=m_oStrValue;}
    void Reset(){Set(m_oFirstValue);}
    bool IsModified(){return (m_oFirstValue!=m_oStrValue);}

    virtual void Trace();
};

/* ------------------------------------------------------------------------
    Manager des validateurs
   ------------------------------------------------------------------------ */

class dmsValidatorManager
{
public:
    dmsValidatorList m_oList;

public:
    dmsValidatorManager();
    virtual ~dmsValidatorManager();

    void Add(dmsValidator *mapping);

    dmsValidator* Add(dmsvContext* ctx, const wxString &type);
    dmsValidator* Find(const wxString &name);
    dmsValidator* FindNextError(dmsValidator* previous);

    void AddString(dmsvContext* ctx, wxString *var);
    void AddStringRegEx(dmsvContext* ctx, wxString *var, const wxString &regex);
    void AddEnum(dmsvContext* ctx, int* var, wxString values[]);
    void AddNumeric(dmsvContext* ctx, int* var, int bitSize=32, int base=10);

    void SetMapping(const wxString &name, void* var);
    void SetMapping(const wxString &name, int* var);

    void InitReset();
    void Reset();
    void ClearErrors();
    bool IsModified();

    void Trace();
};


/* ========================================================================

   Gestion des "contextes"

   Les "contextes" sont la partie "IHM" des validateurs :

   - Ils sont associés à un validateur
   - Ils ont une localisation (ex : erreur à la ligne x)

   Plusieurs contextes peuvent être associés à un même validateur. Exemple :

           contexte_fichier_conf <=> validateur <=> contexte_ihm_graphique

   ======================================================================== */

/* ------------------------------------------------------------------------
   Forme générique des contextes
   ------------------------------------------------------------------------ */

WX_DECLARE_LIST(dmsvContext, dmsvContextList);


class dmsvContext
{
public:
    // Gestion
    dmsvContextManager* m_poManager;

    // Identification
    wxString            m_oName;         // Nom pour accès
    wxString            m_oLocalization; // Nom pour l'affichage des erreurs
    void*               m_poData;        // Pour accès direct à l'objet de l'ihm associé (objet graphique, noeud xml, ...)

    // Associations
    dmsValidator*       m_poValidator;  // Validateur associé

    bool         m_bEnabled;
    dmsvContext* m_poEnableParent;


public:
    dmsvContext(dmsvContextManager* manager, dmsValidator* validator);
    virtual ~dmsvContext();

    bool Set();
    bool Set(const wxString& value);
    bool Get();
    bool Get(wxString& value);

    void Enable(bool value);
    bool Enabled();

    virtual bool GetValue(wxString &value)=0;
    virtual bool SetValue(const wxString &value)=0;
    virtual void ShowError();

};

/* ------------------------------------------------------------------------
   Manageur des contextes
   ------------------------------------------------------------------------ */

class dmsvContextManager
{
public:
    dmsvContextList      m_oList;
    dmsValidatorManager* m_poValidatorManager;

public:
    dmsvContextManager();
    virtual ~dmsvContextManager();

    dmsvContext* Find(const wxString &name);
    dmsvContext* FindData(void *data);

    bool Get();
    bool Set();

    bool Validate();
    bool ValidateAndLog();
};

/* ========================================================================

   ======================================================================== */

#include "ValidatorStd.h"

#endif /* _DMS_TOOLS_VALIDATOR_H_ */
