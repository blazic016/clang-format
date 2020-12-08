/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _DMS_TOOLS_VALIDATOR_STANDARDS_H_
#define _DMS_TOOLS_VALIDATOR_STANDARDS_H_


#include "Validator.h"

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsvString: public dmsValidator
{
public:
    wxString m_oTemplate;
    wxRegEx* m_poRegEx;

public:
    void ConstInit();
    dmsvString(dmsValidatorManager* validator=NULL);
    dmsvString(wxString *var, const wxString &init="");
    virtual ~dmsvString();

    virtual wxString V_Default();
    virtual bool V_Init(const wxString &name, const wxString &value);
    virtual bool V_Validate(const wxString &value);
    virtual void V_Format();
    virtual void V_SetVar();
    virtual void V_GetVar();

    void SetRegEx(const wxString &regex);

};


class dmsvBool: public dmsValidator
{
public:
    bool m_bValue;

public:
    void ConstInit();
    dmsvBool(dmsValidatorManager* validator=NULL);
    dmsvBool(bool *var, const wxString &init="");
    virtual ~dmsvBool();

    virtual wxString V_Default();
    virtual bool V_Init(const wxString &name, const wxString &value);
    virtual bool V_Validate(const wxString &value);
    virtual void V_Format();
    virtual void V_SetVar();
    virtual void V_GetVar();
};


class dmsvNumeric: public dmsValidator
{
public:
    u32 m_iValue;
    u8  m_iBitMemSize;
    u8  m_iBitSize;
    u8  m_iBase;
    u32 m_iMin;
    u32 m_iMax;

public:
    void ConstInit();
    dmsvNumeric(dmsValidatorManager* validator=NULL);
    dmsvNumeric(int *var, const wxString &init="");
    dmsvNumeric(const wxString &init);

    void SetVar(u8  *var){m_poVar = var;m_iBitMemSize=8;}
    void SetVar(u16 *var){m_poVar = var;m_iBitMemSize=16;}
    void SetVar(u32 *var){m_poVar = var;m_iBitMemSize=32;}
    void SetVar(int *var){m_poVar = var;m_iBitMemSize=32;}

    wxString Format(u32 val);

    virtual wxString V_Default();
    virtual bool V_Init(const wxString &name, const wxString &value);
    virtual bool V_Validate(const wxString &value);
    virtual void V_Format();
    virtual void V_SetVar();
    virtual void V_GetVar();

    bool SetBCD(const wxString& value, u32 &nb);
};


class dmsvEnum : public dmsValidator
{
public:
    int           m_iIndex;

    wxArrayString m_oValueNames;   // Liste des noms
    wxString      m_oValueIfEmpty; // Valeur à mettre par défaut si la liste vide

public:
    void ConstInit();
    dmsvEnum(dmsValidatorManager* validator);
    dmsvEnum(void *var, const wxString &values, const wxString &init);
    virtual ~dmsvEnum();

    void AssumeValid(int &index);

    void SetValues(char *firstValue, ...);
    void SetValues(wxString values[]);
    void SetValues(const wxString &values);
    bool SetIndex(int index);

    bool AddUniqValue(const char* name);
    bool DelValue(const char* name);

private:
    virtual wxString V_Default();
    virtual bool V_Init(const wxString &name, const wxString &value);
    virtual bool V_Validate(const wxString &value);
    virtual void V_Format();
    virtual void V_SetVar();
    virtual void V_GetVar();
};


class dmsvFilename: public dmsValidator
{
public:
    wxString m_oType;
    wxString m_oMode;

public:
    void ConstInit();
    dmsvFilename(dmsValidatorManager* validator);
    dmsvFilename(wxString *var, const wxString &init="");
    virtual ~dmsvFilename();

    virtual bool V_Init(const wxString &name, const wxString &value);
    virtual bool V_Validate(const wxString &value);
    virtual void V_Format();
    virtual void V_SetVar();
    virtual void V_GetVar();

    void SetRegEx(const wxString &regex);
};

class dmsvDate: public dmsValidator
{
public:
    wxDateTime  m_oValue;
    wxString m_oMode;

public:
    void ConstInit();
    dmsvDate(dmsValidatorManager* validator=NULL);
    dmsvDate(wxDateTime *var, const wxString &init="");
    virtual ~dmsvDate();

    virtual bool V_Init(const wxString &name, const wxString &value);
    virtual bool V_Validate(const wxString &value);
    virtual void V_Format();
    virtual void V_SetVar();
    virtual void V_GetVar();
};



#endif /* _DMS_TOOLS_VALIDATOR_STANDARDS_H_ */
