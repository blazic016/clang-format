/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/regex.h>

#ifdef WIN32
#include <Winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


#include "Conversion.h"



char *g_StrBool[] = {(char *)"false", (char *)"true"};


bool StrToBool(const wxString &str, bool *value)
{
    if (str.CmpNoCase("true") == 0 ||
        str.CmpNoCase("t")    == 0 ||
        str.CmpNoCase("vrai") == 0 ||
        str.CmpNoCase("1")    == 0)
    {
        *value = true;
        return true;
    }
    else if (str.CmpNoCase("false") == 0 ||
        str.CmpNoCase("f")    == 0 ||
        str.CmpNoCase("faux") == 0 ||
        str.CmpNoCase("0")    == 0)
    {
        *value = false;
        return true;
    }
    else
    {
        return false;
    }
}



long StrToLong(const wxString &str)
{
    long res;

    if (str.ToLong(&res)) return res;

    return 0;
}


int StrToInt(const wxString &str)
{
    return (int) StrToLong(str);
}

bool StrToInt(const wxString &str, int &result)
{
    long l;
    if (str.ToLong(&l)) {
        result = (int)l;
        return true;
    }

    if (str.Len() > 2 && str[0] == '0')
    {
        wxString value = str.Mid(2);
        switch(char(str[1]))
        {
            case 'x': {
                if (value.ToULong((unsigned long*)&l, 16)) {
                    result = (int)l;
                    return true;
                }
                break;
            }
            case 'b': {
                if (value.ToULong((unsigned long*)&l, 2)) {
                    result = (int)l;
                    return true;
                }
                break;
            }
        }
    }

    return false;
}

bool StrToInt(const wxString &str, unsigned char &result)
{
    u64 i = 0;

    if (! StrToInt(str, i)) return false;

    if (i < 0 || i > UCHAR_MAX) return false;

    result = (unsigned char)i;

    return true;
}

bool StrToInt(const wxString &str, unsigned short &result)
{
    u64 i;

    if (! StrToInt(str, i)) return false;

    if (i < 0 || i > USHRT_MAX) return false;

    result = (unsigned short)i;

    return true;
}

bool StrToInt(const wxString &str, u64 &result)
{
    if (str.ToULongLong(&result)) {
        return true;
    }

    if (str.Len() > 2 && str[0] == '0')
    {
        wxString value = str.Mid(2);
        switch(char(str[1]))
        {
        case 'x': if (value.ToULongLong(&result, 16)) return true; break;
        case 'b': if (value.ToULongLong(&result, 2))  return true; break;
        }
    }

    return false;
}


bool StrToInt(const wxString &str, unsigned int &result)
{
    u64 i;

    if (! StrToInt(str, i)) return false;

    if (i < 0 || i > UINT_MAX) return false;

    result = (unsigned int)i;

    return true;
}

bool StrToInt(const wxString &str, int &result, const wxString &labelName, int labelValue)
{
    if (StrToInt(str, result)) return true;

    if (dmsStrFlexibleEquals(str, labelName))
    {
        result = labelValue;
        return true;
    }

    return false;
}


bool StrToInt(const wxString &str, int &result, int min, int max)
{
    long l;
    if (!str.ToLong(&l)) return false;

    if (l < min || l > max) return false;

    result = (int)l;
    return true;
}


void StrConvertToStrList(const wxString &str, wxStringList &list, bool removeBlanks)
{
    wxRegEx exp;
    wxString tail;

    if (removeBlanks)
        exp.Compile("\\s*([^[:space:]]+)\\s*(.*)");
    else
        exp.Compile("^(.*)$");

    list.Clear();
    tail = str;

    while (exp.Matches(tail))
    {
        list.Add(exp.GetMatch(tail, 1));
        tail  = exp.GetMatch(tail, 2);
    }
}








/* ------------------------------------------------------------------------
   Conversion binaire -> hexa
   ------------------------------------------------------------------------ */

void StrHexaFromBuffer(const unsigned char *buffer, size_t bufferLength, wxString &result)
{
    result = "";

    for (size_t i = 0; i < bufferLength; i++)
    {
        result << STR("%02X", buffer[i]);
    }
}


/* ------------------------------------------------------------------------
   Conversion hexa -> binaire
   ------------------------------------------------------------------------ */

bool StrHexaToBuffer(wxString &str, unsigned char *buffer, size_t bufferMaxLength)
{
    if (str.Len() % 2 == 1)
    {
        LOGE(L"Hexa string has not odd length");
        return false;
    }

    const char *x = str.c_str();

    while (bufferMaxLength != 0 && *x != 0)
    {
        long y;
        if (sscanf(x, "%2lx", &y) == 1)
        {
            *buffer = (unsigned char) y;
            x += 2;
            buffer++;
        }
        else
        {
            LOGE(L"Bad Hexa char '%c'", *x);
            return false;
        }
        bufferMaxLength--;
    }

    if (*x)
    {
        LOGE(L"Too small buffer");
        return false;
    }
    return true;
}


/* ------------------------------------------------------------------------
   Représentation hexadécimale d'un buffer

   Découpé en lignes de LINE_WIDTH caractères.

   Une ligne = un compteur d'octets + vue hexadécimale + vue ascii

   Parametres :
   - buffer avec sa taille
   - le résultat sous forme de chaine de caractères
   - deux listes pour ajouter des reperes dans l'affichage (optionnel)
   ------------------------------------------------------------------------ */

void StrLongHexaFromBuffer(const unsigned char *buffer, size_t bufferLength, wxString &result,
                           intList *bookmarks1, intList *bookmarks2)
{
#define LINE_WIDTH 16

    char hexa[LINE_WIDTH*3+1];
    char ascii[LINE_WIDTH+1];

    result = "";

    for (size_t i = 0; i < bufferLength; i++)
    {
        sprintf(ascii + i%LINE_WIDTH, "%c", (buffer[i]<33||buffer[i]>126) ? '.' : buffer[i]);
        sprintf(hexa  + i%LINE_WIDTH*3, "%c%02X",
            bookmarks1 && bookmarks1->Member(i) ? '#' :
            bookmarks2 && bookmarks2->Member(i) ? '!' : ' ',
            buffer[i]);

        if (i%16==15 || i==bufferLength-1)
        {
            while (i%16<15) // bourrage de la derniere ligne
            {
                strcat(ascii, " ");
                strcat(hexa, "   ");
                i++;
            }
            result.Append(STR("[%08d|%s|%s]\n", i-i%LINE_WIDTH, hexa, ascii));
        }
    }
}




void StrJoinIntList(intList &ilist, const char *format, wxString &result, int count)
{
    int *i;

    result = "";

    FOREACH(intList, ilist, i)
    {
        if (count >= 0)
            result.Append(STR(format, count++, *i));
        else
            result.Append(STR(format, *i));
    }
}



/* ----------------------------------------------------------------
   Conversion d'un entier en chaine 'décimale pointée'.
   ---------------------------------------------------------------- */


wxString StrFromIP(uint i)
{
    struct in_addr in;

#ifdef WIN32
    in.S_un.S_addr = htonl(i);
#else
    in.s_addr = htonl(i);
#endif

    return inet_ntoa(in);
}


uint StrToIp(const wxString &str)
{
    uint result;

    result = inet_addr(str.c_str());

    return ntohl(result);
}



/* ========================================================================
   Outils sur regexp
   ======================================================================== */

wxString RegExReplaceTags(const wxString &exp)
{
    wxString result = exp;

    result.Replace(L"§d", L"[[:digit:]]");
    result.Replace(L"§s", L"[[:space:]]");

    return result;
}

bool RegExMatches(wxRegEx &exp, const wxString &expInit, const wxString &str)
{
    wxString validExp(RegExReplaceTags(expInit));

    if (! exp.Compile(validExp))
    {
        LOGE(L"[DEBUG] Regular expression error: %s", str);
        return false;
    }

    return exp.Matches(str);
}

bool RegExMatchesUnit(wxRegEx &exp, const wxString &unit, long &value, wxString &str)
{
    if (! RegExMatches(exp, STR(L"^§s*(§d{1,7})§s*%ss?(.*)", unit), str)) return false;

    exp.GetMatch(str, 1).ToLong(&value);

    str = exp.GetMatch(str, 2);

    return true;
}

/* ========================================================================
   Outils DateTime
   ======================================================================== */

wxDateTime DateTimeAdd(const wxDateTime &ref, const wxDateTime &span)
{
    return wxDateTime::Now();
}

/* ------------------------------------------------------------------------
   Conversion d'une chaine de caractères en date

   Modes
   0 :
   ------------------------------------------------------------------------ */


bool StrToDateTimeSpan(const wxString &str, dmsDateTimeSpan &date)
{
    wxRegEx exp;
    wxString rest = str;
    long year=0, month=0, week=0, day=0, hour=0, minute=0, second=0, value=0;
    bool result = true;

    // Récupération d'une valeur entière + unité

    if (RegExMatchesUnit(exp, "sec", value, rest))         second = value;
    else if (RegExMatchesUnit(exp, "second", value, rest)) second = value;
    else if (RegExMatchesUnit(exp, "min", value, rest))    minute = value;
    else if (RegExMatchesUnit(exp, "minute", value, rest)) minute = value;
    else if (RegExMatchesUnit(exp, "hour", value, rest))   hour   = value;
    else if (RegExMatchesUnit(exp, "day", value, rest))    day    = value;
    else if (RegExMatchesUnit(exp, "week", value, rest))   week   = value;
    else if (RegExMatchesUnit(exp, "month", value, rest))  month  = value;
    else if (RegExMatchesUnit(exp, "year", value, rest))   year   = value;
    else if (RegExMatchesUnit(exp, "", value, rest))       second = value;
    else {LOGE(L"Date span invalid (bad unit in '%s')", str);result=false;}

    // Vérification des valeurs lues

    date.Set(year, month, week, day, hour, minute, second);

    return result;
}


bool StrToDateTime(const wxString &str, wxDateTime &datetime)
{
    wxRegEx exp;
    wxString rest = str;
    long year=0, month=1, day=1, hour=0, minute=0, second=0;
    bool result = true;
    bool withDate = false, withTime=false;

    // Traitement du cas 'now'

    if (RegExMatches(exp, L"^§s*now§s*\\+§s*(.*)", rest))
    {
        dmsDateTimeSpan date;

        rest = exp.GetMatch(rest, 1);

        result = StrToDateTimeSpan(rest, date);

        if (result)
        {
            datetime.SetToCurrent();
            date.AddTo(datetime);
        }
    }
    else
    {
        // Récupération de la partie YYYY/MM/DD

        if (RegExMatches(exp, L"^§s*(§d{4})/(§d{1,2})/(§d{1,2})§s*(.*)", rest)) // YYYY/MM/DD
        {
            exp.GetMatch(rest, 1).ToLong(&year);
            exp.GetMatch(rest, 2).ToLong(&month);
            exp.GetMatch(rest, 3).ToLong(&day);
            rest = exp.GetMatch(rest, 4);
            withDate = true;
        }
        else if (RegExMatches(exp, L"^§s*(§d{4})/(§d{1,2})§s*(.*)", rest)) // YYYY/MM
        {
            exp.GetMatch(rest, 1).ToLong(&year);
            exp.GetMatch(rest, 2).ToLong(&month);
            rest = exp.GetMatch(rest, 3);
            withDate = true;
        }

        // Récupération de la partie HH:MM:SS

        if (RegExMatches(exp, L"^§s*(§d{1,2}):(§d{1,2}):(§d{1,2})§s*(.*)", rest)) // HH:MM:SS
        {
            exp.GetMatch(rest, 1).ToLong(&hour);
            exp.GetMatch(rest, 2).ToLong(&minute);
            exp.GetMatch(rest, 3).ToLong(&second);
            rest = exp.GetMatch(rest, 4);
            withTime = true;
        }
        else if (RegExMatches(exp, L"^§s*(§d{1,2}):(§d{1,2})§s*(.*)", rest)) // HH:MM
        {
            exp.GetMatch(rest, 1).ToLong(&hour);
            exp.GetMatch(rest, 2).ToLong(&minute);
            rest = exp.GetMatch(rest, 3);
            withTime = true;
        }

        // Vérification des valeurs lues

        if (month<1||month>12) {LOGE(L"Date month=%d",  month); result=false;}
        if (day<1||day>31)     {LOGE(L"Date day=%d",    day);   result=false;}
        if (hour>23)           {LOGE(L"Date hour=%d",   hour);  result=false;}
        if (minute>59)         {LOGE(L"Date minute=%d", minute);result=false;}
        if (second>59)         {LOGE(L"Date second=%d", second);result=false;}
        if (!withDate && !withTime) result=false;

        // Construction du résultat

        if (result)
            datetime.Set(day,wxDateTime::Month(month-1),year, hour,minute,second,0);
    }

    if (! result)
    {
        LOGE(L"Invalid Date '%s'", str);
    }

    return result;
}




/* ------------------------------------------------------------------------
   Converti une chaine de caractère en son index dans un tableau de chaines
   ------------------------------------------------------------------------ */

int StrConvertToRank(const char *str, char **tab)
{
    for (int index=0; tab[index]; index++)
    {
        if (strcmp(tab[index], str) == 0) return index;
    }

    return 0;
}







/* ========================================================================

   ======================================================================== */


bool dmsConv(wxChar Dest[], const wxString &Src, size_t size)
{
    size_t max = Src.Len()+1;

    if (size < max) max = size;

    wcsncpy(Dest, Src.GetData(), max);
    Dest[max-1] = 0;

    return (size == max);
}



bool dmsConv(ulong &Dest, const wxString &Src, int base)
{
    return Src.ToULong(&Dest, base);
}


bool dmsConv(int &Dest, const wxString &Src, int base)
{
    long l;
    if (! Src.ToLong(&l, base)) return false;

    Dest = (int)l;
    return true;
}


bool dmsConv(unsigned short &Dest, const wxString &Src, int base)
{
    ulong res;

    if (! Src.ToULong(&res, base)) return false;

    Dest = (unsigned short)res;
    return true;
}




bool dmsChkEthernetCard(const wxString &str)
{
    wxRegEx exp("^([[:xdigit:]]{2}[:-][[:xdigit:]]{2}[:-][[:xdigit:]]{2}[:-][[:xdigit:]]{2}[:-][[:xdigit:]]{2}[:-][[:xdigit:]]{2})$");

    if (!exp.Matches(str))
    {
        LOGE(L"Bad Mac Ethernet Address: %s", str);
        return false;
    }

    return true;
}


/* ========================================================================

   ======================================================================== */

int dmsGetRatio(wxLongLong_t done, wxLongLong_t total)
{
    if (total == 0) return 100;

    return done * 100 / total;
}



int dmsDelayBeforeNow(const wxDateTime &date)
{
    return wxDateTime::Now().GetTicks() - date.GetTicks();
}

wxString dmsShorterDateFormat(const wxDateTime &date)
{
    if (date.FormatDate() == wxDateTime::Now().FormatDate())
        return date.Format("%H:%M");
    else
        return date.Format("%x %H:%M");
}


/* ########################################################################

   ######################################################################## */

bool dmsDateTimeAfterThan(const wxDateTime &date, int delay)
{
    return date.IsLaterThan(wxDateTime::Now()+wxTimeSpan(0,0,delay,0));
}

bool dmsDateTimeBeforeThan(const wxDateTime &date, int delay)
{
    return date.IsEarlierThan(wxDateTime::Now()+wxTimeSpan(0,0,delay,0));
}




void dmsStandardFileName(wxString &name)
{
    name.Replace("\\", "/");
}


wxString dmsIntToStringWithBlancks(int value)
{
    int tmp = value;
    wxString result;

    while (tmp>0)
    {
        if (tmp>=1000)
            result = STR("%03d %s", tmp%1000, result);
        else
            result = STR("%d %s", tmp%1000, result);
        tmp=tmp/1000;
    }

    return result;
}

wxString dmsFileSizeWithUnit(int64 size)
{
    if (size < 1024*1024)
        return dmsIntToStringWithBlancks(size) + " B";

    if (size < 1024*1024*1024)
        return dmsIntToStringWithBlancks(size/1024) + " KB";

    return dmsIntToStringWithBlancks(size/1024/1024) + " MB";
}

/* ------------------------------------------------------------------------
   Test l'égalité entre deux chaînes de la manière la plus flexible possible

   Compare que la séquence de lettre alphabétique en ignorant la casse.
   ------------------------------------------------------------------------ */

bool dmsStrFlexibleEquals(const wxString &a, const wxString &b)
{
    int  i=0, j=0;
    char x, y;

    while (a[i] || b[j]) // il faut consommer entierement les deux chaines
    {
        x=a[i]; y=b[j];

        // On ignore la casse et on ignore tout ce qui n'est pas alphanum
        if (x>='a' && x<='z') x=x-'a'+'A';
        else if (! ((x>='0'&&x<='9')||(x>='A'&&x<='Z')||x==0)) {i++; continue;}

        if (y>='a' && y<='z') y=y-'a'+'A';
        else if (! ((y>='0'&&y<='9')||(y>='A'&&y<='Z')||y==0)) {j++; continue;}

        // Cas d'échec
        if (x!=y) return false;

        // Progression
        i++; j++;
    }

    return true;
}


int dmsGetStrRank(const char *value, const char *list[])
{
    int i=0;

    while (list[i])
    {
        if (dmsStrFlexibleEquals(list[i], value))
        {
            return i;
        }
        i++;
    }
    return -1;
}

bool dmsGetStrRank(const wxString &value, const dmsNameRank list[], int &rank)
{
    int i=0;

    while (list[i].name)
    {
        if (dmsStrFlexibleEquals(list[i].name, value))
        {
            rank = list[i].rank;
            return true;
        }
        i++;
    }
    return false;
}

bool dmsGetStrRank(const wxString &value, const dmsNameRank list[], u64 &rank)
{
    int i=0;

    while (list[i].name)
    {
        if (dmsStrFlexibleEquals(list[i].name, value))
        {
            rank = list[i].rank;
            return true;
        }
        i++;
    }
    return false;
}

int dmsGetStrRank(const wxString &value, const dmsNameRank list[])
{
    int rank;

    if (dmsGetStrRank(value, list, rank))
        return rank;
    else
        return -1;
}

char *dmsGetName(int rank, const dmsNameRank list[])
{
    int i=0;

    while ((list[i].name) && (list[i].rank!=rank)) i++;

    return list[i].name;
}



void dmsSplitChoices(const wxString &choices, int &choiceCount, wxString* &choiceList)
{
    char sep = choices[0];

    choiceCount = choices.Freq(sep);
    choiceList  = new wxString[choiceCount+1]; // On met au bout une chaine vide

    int rank=0;
    int first=1;

    for (size_t i=0; i<choices.Len(); i++)
    {
        if (choices[i+1]==sep || i+1==choices.Len())
        {
            choiceList[rank] = choices.Mid(first, i+1-first);
            first = i+2;
            rank++;
        }
    }
}

