/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#include <wx/regex.h>


#include "Tools.h"
#include "File.h"
#include "Conversion.h"

#include "ValidatorStd.h"


/* ########################################################################

   ######################################################################## */


/* ========================================================================

   ======================================================================== */

void dmsvString::ConstInit()
{
    m_eType   = "string";

    m_poRegEx = NULL;
}


dmsvString::dmsvString(dmsValidatorManager* validator) : dmsValidator(validator)
{
    ConstInit();
}

dmsvString::dmsvString(wxString *var, const wxString &init) : dmsValidator(NULL, var, init)
{
    ConstInit();
}

dmsvString::~dmsvString()
{
    DELNUL(m_poRegEx);
}

/* ========================================================================

   ======================================================================== */

wxString dmsvString::V_Default()
{
    return m_oTemplate;
}


bool dmsvString::V_Init(const wxString &name, const wxString &value)
{
    if (name=="template")
    {
        m_oTemplate = value;

        //if (m_poDefaultValue==NULL) m_poDefaultValue = new wxString(m_oTemplate);

        wxString regex;
        size_t i, j;

        for (i=0; i<value.Len(); i++)
        {
            j=i;
            switch (char(value[i]))
            {
            case '0':
                {
                    while (i<value.Len() && value[i]=='0') i++;
                    regex << STR("[[:digit:]]{%lu}", i-j);
                    i--;
                }
                break;
            default:
                {
                    regex << value[i];
                }
                break;
            }
        }

        SetRegEx("^"+regex+"$");

        //LOG0("Regex '%s'", regex);
    }

    return true;
}


bool dmsvString::V_Validate(const wxString &value)
{
    if (m_poRegEx && (!m_poRegEx->Matches(value)))
    {
        SetError(STR("Expression [%s] must match template [%s]", value, m_oTemplate));
        return false;
    }
    return true;
}

void dmsvString::V_Format()
{
    return;
}

void dmsvString::V_SetVar()
{
    *((wxString*)(m_poVar)) = m_oStrValue;
}

void dmsvString::V_GetVar()
{
    m_oStrValue = *((wxString*)(m_poVar));
}

/* ========================================================================

   ======================================================================== */

void dmsvString::SetRegEx(const wxString &regex)
{
    DELNUL(m_poRegEx);

    m_poRegEx = new wxRegEx(regex);
}



/* ########################################################################

   ######################################################################## */

void dmsvBool::ConstInit()
{
    m_eType             = "bool";
    m_oTrim             = "true";
}

dmsvBool::dmsvBool(dmsValidatorManager* validator) : dmsValidator(validator)
{
    ConstInit();
}


dmsvBool::dmsvBool(bool *var, const wxString &init) : dmsValidator(NULL, var, init)
{
    ConstInit();
}


dmsvBool::~dmsvBool()
{
}

/* ========================================================================

   ======================================================================== */

wxString dmsvBool::V_Default()
{
    return "false";
}

bool dmsvBool::V_Init(const wxString &name, const wxString &value)
{
    return true;
}

bool dmsvBool::V_Validate(const wxString &value)
{
    static const char* trueValues[]  = {"true", "t", "vrai", "v", "1", NULL};
    static const char* falseValues[] = {"false", "f", "faux", "f", "0", NULL};

    wxString val = value;
    val.MakeLower();

    if (dmsGetStrRank(val, trueValues)>=0)
        m_bValue = true;
    else if (dmsGetStrRank(val, falseValues)>=0)
        m_bValue = false;
    else
        SetError(L"Expression [%s] is not a boolean value", value);

    return true;
}


void dmsvBool::V_Format()
{
    m_oStrValue = m_bValue?"true":"false";
}

void dmsvBool::V_SetVar()
{
    *((bool*)(m_poVar)) = m_bValue;
}

void dmsvBool::V_GetVar()
{
    m_bValue = *((bool*)(m_poVar));
}

/* ########################################################################

   ######################################################################## */

void dmsvNumeric::ConstInit()
{
    m_iBitMemSize = 0;
    m_iBitSize    = 0;
    m_iBase       = 0;
    m_iMin        = 0;
    m_iMax        = 0xFFFFFFFF;

    m_eType     = "numeric";
    m_oTrim     = "true";
}


dmsvNumeric::dmsvNumeric(dmsValidatorManager* validator) : dmsValidator(validator)
{
    ConstInit();
}


dmsvNumeric::dmsvNumeric(int *var, const wxString &init) : dmsValidator(NULL, var, init)
{
    ConstInit();

    m_iBitMemSize = 32;
    m_iBitSize    = 32;
}

dmsvNumeric::dmsvNumeric(const wxString &init) : dmsValidator(NULL, NULL, init)
{
    ConstInit();
}

/* ========================================================================

   ======================================================================== */

wxString dmsvNumeric::V_Default()
{
    return "0";
}


bool dmsvNumeric::V_Init(const wxString &name, const wxString &value)
{
    if (name=="base")
    {
        if (value=="bcd") {m_iBase=4; return true;}
        if (StrToInt(value, m_iBase)) return true;
    }
    if (name=="bit"  && (StrToInt(value, m_iBitSize)))
    {
        if (m_iBitSize < 32 && m_iMax==0xFFFFFFFF) m_iMax = (1 << m_iBitSize)-1;
        return true;
    }
    if (name=="min" && StrToInt(value, m_iMin))
        return true;

    u32 max;
    if (name=="max" && StrToInt(value, max) && (max>=m_iMin))
    {
        if (max < m_iMax) m_iMax = max;
        return true;
    }

    LOGE(L"Error while initializing mapping [numeric] with [%s,%s]", name, value);

    return true;
}

bool dmsvNumeric::V_Validate(const wxString &value)
{
    ulong tmp;
    if (m_iBase==4)
    {
        if (! SetBCD(value, m_iValue)) return false;
    }
    else
    {
        if (! value.ToULong(&tmp, m_iBase))
        {
            SetError(L"Bad number [%s] (base %s)", value, m_iBase==0?"10":m_iBase==4?"BCD":STR("%d", m_iBase));
            return false;
        }
        m_iValue = (u32)tmp;
        wxString head = value;
        ulong nbhead;
        head.RemoveLast();
        head.ToULong(&nbhead, m_iBase);

        if (m_iValue>m_iMax || (m_iValue==0xFFFFFFFF && nbhead>=m_iValue))
        {
            SetError(L"Number [%s] must be lower than [%s]", value, Format(m_iMax));
            return false;
        }
        if (m_iValue<m_iMin)
        {
            SetError(L"Number [%s] must be greater than [%s]", value, Format(m_iMin));
            return false;
        }
    }

    return true;
}


wxString dmsvNumeric::Format(u32 val)
{
    if (m_iBase==16)
        return STR("0x%0*X", ((m_iBitSize-1)/4)+1, val);
    else
        return STR("%u", val);
}


void dmsvNumeric::V_Format()
{
    m_oStrValue = Format(m_iValue);
}


void dmsvNumeric::V_SetVar()
{
    switch (m_iBitMemSize)
    {
    case 8:  (*((u8* )m_poVar))=m_iValue; break;
    case 16: (*((u16*)m_poVar))=m_iValue; break;
    case 32: (*((u32*)m_poVar))=m_iValue; break;
    }
}

void dmsvNumeric::V_GetVar()
{
    switch (m_iBitMemSize)
    {
    case 8:  m_iValue=(*((u8* )m_poVar)); break;
    case 16: m_iValue=(*((u16*)m_poVar)); break;
    case 32: m_iValue=(*((u32*)m_poVar)); break;
    }
}

/* ========================================================================

   ======================================================================== */


bool dmsvNumeric::SetBCD(const wxString& value, u32 &nb)
{
    nb=0;

    if (m_iBitSize % 4 != 0)
    {
        SetError(L"BitSize must be multiple of 4 in BCD conversion");
        return false;
    }

    size_t j = 0;

    for (int i = (int)value.Len(); i>=0; i--)
    {
        char c = value[i];

        if (c<'0'||c>'9') continue;

        u8* current = ((u8*)(&nb))+(j/2);
        bool high=(j%2==1);


        if (j*4+(high?0:4) > m_iBitSize)
        {
            SetError(L"Too much digits in [%s]. Mem Size is [%d] bits.", value, m_iBitSize);
            return false;
        }

        c = c-'0';

        if (high)
            *current |= c << 4;
        else
            *current = c;

        j++;
    }

    return true;
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

void dmsvEnum::ConstInit()
{
    m_eType = "enum";
}


dmsvEnum::dmsvEnum(dmsValidatorManager* validator) : dmsValidator(validator)
{
    ConstInit();
}


dmsvEnum::dmsvEnum(void *var, const wxString &values, const wxString &init) : dmsValidator(NULL, var, init)
{
    ConstInit();

    SetValues(values);
}


dmsvEnum::~dmsvEnum()
{
}

/* ========================================================================

   ======================================================================== */

wxString dmsvEnum::V_Default()
{
    if (m_oValueNames.GetCount()==0) return "";

    return m_oValueNames[0];
}

bool dmsvEnum::V_Init(const wxString &name, const wxString &value)
{
    if (name=="ifempty") {m_oValueIfEmpty = value; return true;}

    return false;
}


bool dmsvEnum::V_Validate(const wxString &value)
{
    m_iIndex = m_oValueNames.Index(value);

    if (m_iIndex == wxNOT_FOUND)
    {
        SetError(L"Bad value [%s]. Must be either ", value);
        for (size_t i=0; i<m_oValueNames.GetCount(); i++)
            SetError(L"%s%s", (i==0)?"[":", ", m_oValueNames[i]);
        SetError(L"]");

        m_iIndex = 0;

        return false;
    }

    return true;
}

void dmsvEnum::AssumeValid(int &index)
{
    if (m_oValueNames.GetCount()==0 && m_oValueIfEmpty.Len())
    {
        AddUniqValue(m_oValueIfEmpty);
    }

    if (index >= (int) m_oValueNames.GetCount()) index = 0;
}

void dmsvEnum::V_Format()
{
    AssumeValid(m_iIndex);

    if (m_iIndex < (int) m_oValueNames.GetCount())
        m_oStrValue = m_oValueNames[m_iIndex];
    else
        m_oStrValue.Clear();
}


void dmsvEnum::V_SetVar()
{
    *((int*)(m_poVar)) = m_iIndex;
}

void dmsvEnum::V_GetVar()
{
    m_iIndex = *((int*)(m_poVar));
}

/* ========================================================================

   ======================================================================== */

void dmsvEnum::SetValues(char *firstValue, ...)
{
    char** item;

    for (item = &firstValue; *item; item++)
    {
        m_oValueNames.Add(*item);
    }
}


void dmsvEnum::SetValues(wxString values[])
{
    wxString* item;

    for (item = values; item->Len(); item++)
    {
        m_oValueNames.Add(*item);
    }
}


void dmsvEnum::SetValues(const wxString &values)
{
    int count;
    wxString *tab;

    dmsSplitChoices(values, count, tab);

    for (int i=0; i<count; i++)
    {
        m_oValueNames.Add(tab[i]);
    }

    delete [] tab;
}


bool dmsvEnum::SetIndex(int index)
{
    if (index<0 || index >= (int)m_oValueNames.GetCount())
    {
        SetError(L"Index [%d] out of bounds [0..%d]", index, m_oValueNames.GetCount());
        return false;
    }
    else
    {
        return Set(m_oValueNames[index]);
    }
}


/* ========================================================================

   ======================================================================== */


bool dmsvEnum::AddUniqValue(const char* name)
{
    wxString value = name;

    if (value.Len()==0)
    {
        SetError(L"Null value");
        return false;
    }

    if (m_oValueNames.Index(value, false)!=wxNOT_FOUND)
    {
        SetError(L"Value [%s] already exists", value);
        return false;
    }

    m_oValueNames.Add(value);

    m_iIndex = m_oValueNames.GetCount()-1;

    V_Format();

    return true;
}


bool dmsvEnum::DelValue(const char* name)
{
    if (strlen(name)==0)
    {
        SetError(L"Null value", name);
        return false;
    }

    if (m_oValueNames.Index(name, false)==wxNOT_FOUND)
    {
        SetError(L"Unknown value [%s]", name);
        return false;
    }

    m_oValueNames.Remove(name);

    V_Format();

    return true;
}

/* ########################################################################

   ######################################################################## */


void dmsvFilename::ConstInit()
{
    m_eType = "filename";
}


dmsvFilename::dmsvFilename(dmsValidatorManager* validator) : dmsValidator(validator)
{
    ConstInit();
}

dmsvFilename::dmsvFilename(wxString* var, const wxString &init) : dmsValidator(NULL, var, init)
{
    ConstInit();
}

dmsvFilename::~dmsvFilename()
{
}


/* ========================================================================

   ======================================================================== */


bool dmsvFilename::V_Init(const wxString &name, const wxString &value)
{
    if (name=="type") {m_oType = value; return true;}
    if (name=="mode") {m_oMode = value; return true;}

    return false;
}


bool dmsvFilename::V_Validate(const wxString &value)
{
    wxString val = value;

    if (m_oMode=="r")
    {
        if (val.IsEmpty())
        {
            SetError(L"Empty filename");
            return false;
        }
        if (! dmsFile::Access(val, dmsFile::read))
        {
            SetError(L"Error opening file [%s] in read mode", val);
            return false;
        }
    }

    return true;
}

void dmsvFilename::V_Format()
{
    return;
}

void dmsvFilename::V_SetVar()
{
    *((wxString*)(m_poVar)) = m_oStrValue;
}

void dmsvFilename::V_GetVar()
{
    m_oStrValue = *((wxString*)(m_poVar));
}

/* ########################################################################

   ######################################################################## */

void dmsvDate::ConstInit()
{
    m_eType             = "date";
    m_oValue            = wxDateTime::Now();
}

dmsvDate::dmsvDate(dmsValidatorManager* validator) : dmsValidator(validator)
{
    ConstInit();
}


dmsvDate::dmsvDate(wxDateTime *var, const wxString &init) : dmsValidator(NULL, var, init)
{
    ConstInit();
}


dmsvDate::~dmsvDate()
{
}

/* ========================================================================

   ======================================================================== */

bool dmsvDate::V_Init(const wxString &name, const wxString &value)
{
    if (name=="mode") {m_oMode = value; return true;}

    return false;
}

bool dmsvDate::V_Validate(const wxString &value)
{
    if (value.Len() > 0)
    {
        if (m_oMode=="older")
        {
            if (StrToDateTime(value, m_oValue) &&
                m_oValue.IsEarlierThan(wxDateTime::Today()))
            {
                SetError(L"Date in the past not allowed");
                return false;
            }
        }
    }
    return true;
}

void dmsvDate::V_Format()
{
    m_oStrValue = m_oValue.Format("%Y/%m/%d");
}

void dmsvDate::V_SetVar()
{
    *((wxDateTime*)(m_poVar)) = m_oValue;
}

void dmsvDate::V_GetVar()
{
    m_oValue = *((wxDateTime*)(m_poVar));
}
