/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#include <wx/regex.h>

#include "Tools.h"
#include "File.h"
#include "Conversion.h"
#include "Validator.h"


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsvContextList);


/* ########################################################################

   Forme générique d'un "Validator"

   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsValidatorList);

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

void dmsValidator::ConstInit()
{
    //m_bEnabled          = true;
    m_poVar             = NULL;
    m_bAllowReset       = true;
    m_bAllowEmpty       = true;
    m_poDefaultValue    = NULL;
    m_bInitialized      = false;
    m_bOptional         = false;
    m_bErrorEnable      = true;
}


dmsValidator::dmsValidator(dmsValidatorManager* manager)
{
    ConstInit();

    SetManager(manager);
}

dmsValidator::dmsValidator(dmsValidatorManager* manager, void* var, const wxString &init)
{
    ConstInit();

    SetManager(manager);

    SetMapping(var);

    m_oInitStr = init;
}


dmsValidator::~dmsValidator()
{
    DELNUL(m_poDefaultValue);

    return;
}

/* ========================================================================

   ======================================================================== */

void dmsValidator::SetError(const wxChar *format, ...)
{
    if (! m_bErrorEnable) return;

    va_list varg;

    va_start(varg, format);
    m_oError.PrintfV(format, varg);
    va_end(varg);

#ifdef _DEBUG
    LOGE(L"Validation error in [%s] : [%s]", m_oName, m_oError);
#endif
}


/* ========================================================================

   ======================================================================== */



void dmsValidator::SetMapping(void *var)
{
    m_poVar = var;

    if (m_bInitialized) SetDefault();
}

void dmsValidator::SetDefault()
{
    if (m_poDefaultValue)
    {
        Set(*m_poDefaultValue); // Valeur donnée dans l'init
    }
    else if (m_poVar)
    {
        V_GetVar(); // Valeur de la variable
        V_Format();
    }
    else
    {
        SetNoError(V_Default());
    }
}

/* ========================================================================

   ======================================================================== */


bool dmsValidator::StartValidation()
{
    // Control sur une mauvaise utilisation du validateur
    if (! m_bInitialized)
    {
        if (! Init(m_oInitStr)) return false;
    }

    if (m_oError.Len())
    {
        LOGE(L"Dev Error : clean validator [%s] before reuse", m_oName);
        return false;
    }

    return true;
}


bool dmsValidator::Set(const char* value)
{
    wxString Value = value;

    if (! StartValidation()) return false;

    // Préparation de la donnée
    if (m_oTrim.Len())
        Value.Trim(true).Trim(false);

    if (m_oTrim=="full")
    {
        wxRegEx regex("[^[a-zA-Z_-]]+");

        regex.ReplaceAll(&Value, "");
    }

    // Validation générale
    if (! m_bAllowEmpty && Value.Len()==0)
    {
        m_oError = "Empty value";
        return false;
    }

    // Validation spécialisée
    bool ok = V_Validate(Value);

    // Validation OK
    m_oStrValue = Value;

    // Mise à jour du mapping
    if (m_poVar) V_SetVar();

    // Mise au propre de la valeur éventuelle
    V_Format();

    return ok;
}

bool dmsValidator::SetNoError(const char* value)
{
    m_bErrorEnable = false;
    bool res = Set(value);
    m_bErrorEnable = true;

    return res;
}



bool dmsValidator::Optional()
{
    if (! StartValidation()) return false;

    if (m_bOptional) return true;

    SetError(L"Not founded");

    return false;
}


bool dmsValidator::Init(const wxString &info)
{
    if (m_bInitialized)
    {
        LOGE(L"Dev Error : validator [%s] already initialized", m_oName);
        return false;
    }

    if (info.Len())
    {
        int count;
        wxString* list=NULL;

        dmsSplitChoices(info, count, list);

        LOG_AF(count%2==0, LOGE(L"Init mapping with odd parameters"));

        for (int i=0; i<count; i+=2)
        {
            bool ok = Init(list[i], list[i+1]);
            if (!ok)
            {
                delete[] list;
                return false;
            }
        }

        delete []list;
    }

    if (m_oName.Last()=='?')
        m_bOptional = true;

    // Initialisé

    m_bInitialized = true;

    // Gestion de l'initialisation

    SetDefault();

    return true;
}


bool dmsValidator::Init(const wxString &name, const wxString &value)
{
    //if (name=="enable")  {m_bEnabled = (value=="true");return true;}
    if (name=="trim")    {m_oTrim = value; return true;}
    if (name=="reset")   {m_bAllowReset = (value=="true"); return true;}
    if (name=="empty")   {m_bAllowEmpty = (value=="true"); return true;}
    if (name=="default") {m_poDefaultValue = new wxString(value); return true;}

    bool ok = V_Init(name, value);
    if (!ok)
    {
        LOGE(L"Error in arguments [%s=%s] of [%s]", name, value, m_oName);
        return false;
    }

    return true;
}



void dmsValidator::SetManager(dmsValidatorManager* manager)
{
    m_poManager = manager;

    if (m_poManager)
        m_poManager->m_oList.Append(this);
}

wxString dmsValidator::GetError()
{
    wxString res = m_oError;

    Clear();

    return res;
}


void dmsValidator::Trace()
{
    LOG0(L"- %-10s %-25s %p : [%s]", m_eType, m_oName, m_poVar, m_oError);
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsValidatorManager::dmsValidatorManager()
{
    m_oList.DeleteContents(true);
}


dmsValidatorManager::~dmsValidatorManager()
{
}

void dmsValidatorManager::Add(dmsValidator *mapping)
{
    m_oList.Append(mapping);
}

dmsValidator* dmsValidatorManager::Add(dmsvContext* ctx, const wxString &type)
{
    dmsValidator* res;

    if (type=="string")        res = new dmsvString(this);
    else if (type=="numeric")  res = new dmsvNumeric(this);
    else if (type=="enum")     res = new dmsvEnum(this);
    else if (type=="filename") res = new dmsvFilename(this);
    else if (type=="bool")     res = new dmsvBool(this);
    else if (type=="date")     res = new dmsvDate(this);
    else res = NULL;

    if (res)
    {
        ctx->m_poValidator = res;
        //m_oList.Append(res);
    }

    return res;
}

dmsValidator* dmsValidatorManager::Find(const wxString &name)
{
    dmsValidator* map;

    FOREACH(dmsValidatorList, m_oList, map)
    {
        if (map->m_oName==name) return map;
    }
    return NULL;
}

dmsValidator* dmsValidatorManager::FindNextError(dmsValidator* previous)
{
    dmsValidator* map;

    FOREACH(dmsValidatorList, m_oList, map)
    {
        if (map==previous)
        {
            previous=NULL;
        }
        else if (previous==NULL)
        {
            if (map->m_oError.Len()) return map;
        }
    }
    return NULL;

}


void dmsValidatorManager::SetMapping(const wxString &name, void* var)
{
    dmsValidator* map = Find(name);

    if (map==NULL)
        LOG0(L"No var named [%s]", name);
    else
        map->SetMapping(var);
}

void dmsValidatorManager::SetMapping(const wxString &name, int* var)
{
    dmsValidator* map = Find(name);

    if (map==NULL)
        LOG0(L"No var named [%s]", name);
    else
    {
        map->SetMapping(var);
        if (map->m_eType=="numeric")
            ((dmsvNumeric*)map)->m_iBitMemSize=32;
    }
}

void dmsValidatorManager::AddString(dmsvContext* ctx, wxString *var)
{
    dmsvString *Map = new dmsvString(this);

    Map->SetMapping(var);
    ctx->m_poValidator = Map;

    Add(Map);
}

void dmsValidatorManager::AddStringRegEx(dmsvContext* ctx, wxString *var, const wxString &regex)
{
    dmsvString *Map = new dmsvString(this);

    Map->SetMapping(var);
    ctx->m_poValidator = Map;
    Map->SetRegEx(regex);

    Add(Map);
}

void dmsValidatorManager::AddEnum(dmsvContext* ctx, int* var, wxString values[])
{
    if (values==NULL) return;

    dmsvEnum* Map = new dmsvEnum(this);

    Map->SetMapping(var);
    Map->SetValues(values);
    ctx->m_poValidator = Map;

    Add(Map);
}

void dmsValidatorManager::AddNumeric(dmsvContext* ctx, int* var, int bitSize, int base)
{
    dmsvNumeric* Map = new dmsvNumeric(this);

    Map->SetMapping(var);
    Map->m_iBitMemSize = 32;
    Map->m_iBitSize    = bitSize;
    Map->m_iBase       = base;
    ctx->m_poValidator = Map;

    Add(Map);
}

void dmsValidatorManager::InitReset()
{
    dmsValidator* mapping;

    FOREACH(dmsValidatorList, m_oList, mapping)
    {
        mapping->InitReset();
    }
}
void dmsValidatorManager::Reset()
{
    dmsValidator* mapping;

    FOREACH(dmsValidatorList, m_oList, mapping)
    {
        if (mapping->m_bAllowReset) mapping->Reset();
    }
}

void dmsValidatorManager::ClearErrors()
{
    dmsValidator* mapping;

    FOREACH(dmsValidatorList, m_oList, mapping)
    {
        mapping->Clear();
    }
}

bool dmsValidatorManager::IsModified()
{
    dmsValidator* mapping;

    FOREACH(dmsValidatorList, m_oList, mapping)
    {
        if (mapping->m_bAllowReset && mapping->IsModified())
            return true;
    }

    return false;
}

void dmsValidatorManager::Trace()
{
    dmsValidator* mapping;

    FOREACH(dmsValidatorList, m_oList, mapping)
    {
        mapping->Trace();
    }
}

/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsvContext::dmsvContext(dmsvContextManager* manager, dmsValidator* validator)
{
    m_poValidator = validator;
    m_poManager   = manager;
    m_poData      = NULL;

    m_bEnabled       = true;
    m_poEnableParent = NULL;


    /*
    if (validator==NULL)
        LOGE("No validator in context");
    */

    if (m_poManager)
        m_poManager->m_oList.Append(this);
}


dmsvContext::~dmsvContext()
{

}

/* ========================================================================

   ======================================================================== */

void dmsvContext::Enable(bool value)
{
    m_bEnabled=value;
}

bool dmsvContext::Set()
{
    if (m_poValidator==NULL || !Enabled()) return true;

    wxString value;

    if (GetValue(value))
        return m_poValidator->Set(value);
    else
        return m_poValidator->Optional();
}

bool dmsvContext::Set(const wxString& value)
{
    if (m_poValidator==NULL || !Enabled()) return false;

    return m_poValidator->Set(value);
}

bool dmsvContext::Get()
{
    if (m_poValidator == NULL) return false;

    return SetValue(m_poValidator->Get());
}

bool dmsvContext::Get(wxString& value)
{
    if (m_poValidator == NULL) return false;

    value = m_poValidator->Get();

    return true;
}

void dmsvContext::ShowError()
{
    LOGE(L"%s", m_poValidator->m_oError);
}


bool dmsvContext::Enabled()
{
    if (m_poEnableParent==NULL) return m_bEnabled;

    return m_bEnabled && m_poEnableParent->Enabled();
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsvContextManager::dmsvContextManager()
{
    m_oList.DeleteContents(true);

    m_poValidatorManager = NULL;
}


dmsvContextManager::~dmsvContextManager()
{

}


/* ========================================================================

   ======================================================================== */


dmsvContext* dmsvContextManager::Find(const wxString &name)
{
    if (name.IsEmpty()) return NULL;

    dmsvContext* ctx;

    FOREACH(dmsvContextList, m_oList, ctx)
    {
        if (ctx->m_oName==name) return ctx;
    }

    FOREACH2(dmsvContextList, m_oList, ctx)
    {
        if (ctx->m_poValidator && ctx->m_poValidator->m_oName==name) return ctx;
    }

    return NULL;
}



dmsvContext* dmsvContextManager::FindData(void *data)
{
    if (data==NULL) return NULL;

    dmsvContext* ctx;

    FOREACH(dmsvContextList, m_oList, ctx)
    {
        if (ctx->m_poData == data) return ctx;
    }

    return NULL;
}



bool dmsvContextManager::Get()
{
    dmsvContext* ctx;

    FOREACH(dmsvContextList, m_oList, ctx)
    {
        if (ctx->m_poValidator)
        {
            if (! ctx->Get()) return false;
        }
    }

    return true;
}


bool dmsvContextManager::Set()
{
    dmsvContext* ctx;

    FOREACH(dmsvContextList, m_oList, ctx)
    {
        if (ctx->m_poValidator)
        {
            if (! ctx->Set()) return false;
        }
    }

    return true;
}

bool dmsvContextManager::Validate()
{
    if (Set()) return true;

    //m_poManager->Trace();

    dmsValidator* map = m_poValidatorManager->FindNextError(NULL);

    if (map==NULL)
    {
        LOGE(L"Dev Error : unable to find existing error");
        return false;
    }

    dmsvContext* ctx = Find(map->m_oName);

    ctx->ShowError();

    map->Clear();

    return false;
}

