/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _TOOLS_CONVERSION_H_
#define _TOOLS_CONVERSION_H_


#include <wx/wx.h>
#include <wx/datetime.h>

#include "Tools.h"


class dmsDateTimeSpan;

typedef struct { int rank; char *name; } dmsNameRank;

extern char *g_StrBool[];

int dmsGetStrRank(const char *value, const char *list[]);
int dmsGetStrRank(const wxString &value, const dmsNameRank list[]);
char *dmsGetName(int rank, const dmsNameRank list[]);
bool dmsGetStrRank(const wxString &value, const dmsNameRank list[], u64 &rank);
bool dmsGetStrRank(const wxString &value, const dmsNameRank list[], int &rank);

bool StrToBool(const wxString &str, bool *value);

void StrConvertToStrList(const wxString &str, wxStringList &list, bool removeBlanks = true);

long StrToLong(const wxString &str);
void StrToLong(const wxString &str, long &result);
int  StrToInt(const wxString &str);
bool StrToInt(const wxString &str, int &result);
bool StrToInt(const wxString &str, unsigned char &result);
bool StrToInt(const wxString &str, unsigned short &result);
bool StrToInt(const wxString &str, unsigned int &result);
bool StrToInt(const wxString &str, u64 &result);
bool StrToInt(const wxString &str, int &result, const wxString &labelName, int labelValue);
bool StrToInt(const wxString &str, int &result, int min, int max);
void StrLongHexaFromBuffer(const unsigned char *buffer, size_t bufferLength, wxString &result,
                       intList *bookmarks1 = NULL, intList *bookmarks2 = NULL);
void StrHexaFromBuffer(const unsigned char *buffer, size_t bufferLength, wxString &result);
bool StrHexaToBuffer(wxString &str, unsigned char *buffer, size_t bufferMaxLength);

void StrJoinIntList(intList &ilist, const char *format, wxString &result, int count = -1);

bool dmsStrFlexibleEquals(const wxString &a, const wxString &b);
int  StrConvertToRank(const char *str, char **tab);

bool dmsChkEthernetCard(const wxString &str);

wxString StrFromIP(uint i);
uint     StrToIp(const wxString &str);

void dmsStandardFileName(wxString &name);



bool StrToDateTime(const wxString &str, wxDateTime &datetime);
bool StrToDateTimeSpan(const wxString &str, dmsDateTimeSpan &date);



bool dmsConv(wxChar Dest[], const wxString &Src, size_t size);
bool dmsConv(ulong &Dest, const wxString &Src, int base = 10);
bool dmsConv(int &Dest, const wxString &Src, int base = 10);
bool dmsConv(unsigned short &Dest, const wxString &Src, int base = 10);

int  dmsGetRatio(wxLongLong_t done, wxLongLong_t total);

int dmsDelayBeforeNow(const wxDateTime &date);
wxString dmsShorterDateFormat(const wxDateTime &date);

wxString dmsFileSizeWithUnit(int64 size);


/* ------------------------------------------------------------------------
   Classe qui réunit "wxDateSpan" et "wxTimeSpan"
   ------------------------------------------------------------------------ */

class dmsDateTimeSpan
{
public:
    wxDateSpan m_oDate;
    wxTimeSpan m_oTime;

public:
    dmsDateTimeSpan(){;}
    dmsDateTimeSpan(const wxDateSpan &date, const wxTimeSpan &time){m_oDate=date;m_oTime=time;}
    dmsDateTimeSpan(const wxDateSpan &date){m_oDate=date;}
    dmsDateTimeSpan(const wxTimeSpan &time){m_oTime=time;}
    virtual ~dmsDateTimeSpan(){;}

    void Set(const wxDateSpan &date, const wxTimeSpan &time){m_oDate=date;m_oTime=time;}
    void Set(const wxDateSpan &date){m_oDate=date;}
    void Set(const wxTimeSpan &time){m_oTime=time;}
    void Set(const dmsDateTimeSpan &span){m_oDate=span.m_oDate;m_oTime=span.m_oTime;}

    void Set(int years, int months, int weeks, int days, int hours, int minutes, int seconds)
    {
        m_oDate = wxDateSpan(years, months, weeks, days);
        m_oTime = wxTimeSpan(hours, minutes, seconds, 0);
    }

    dmsDateTimeSpan& operator=(const dmsDateTimeSpan &span){Set(span);return *this;}

    void AddTo(wxDateTime &date) {date.Add(m_oDate);date.Add(m_oTime);}

};


bool dmsDateTimeAfterThan(const wxDateTime &date, int delay);
bool dmsDateTimeBeforeThan(const wxDateTime &date, int delay);

void dmsSplitChoices(const wxString &choices, int &choiceCount, wxString* &choiceList);


#endif /* _TOOLS_CONVERSION_H_ */
