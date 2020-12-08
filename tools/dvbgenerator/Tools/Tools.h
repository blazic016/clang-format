/* ************************************************************************
   SmarDTV

   Description : Outils Standards

   Historique :
   - COF   - Iwedia  - v 0    - 05/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _TOOLS_TOOLS_H_
#define _TOOLS_TOOLS_H_

#include <wx/wx.h>

#include "Log.h"

/* ------------------------------------------------------------------------
   Constantes
   ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------
   Types
   ------------------------------------------------------------------------ */

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long  ulong;
typedef wxLongLong_t int64;

typedef unsigned char       u1;
typedef unsigned char       u2;
typedef unsigned char       u3;
typedef unsigned char       u4;
typedef unsigned char       u5;
typedef unsigned char       u6;
typedef unsigned char       u8;
typedef unsigned short      u12;
typedef unsigned short      u13;
typedef unsigned short      u16;
typedef unsigned int        u24;
typedef unsigned int        u28;
typedef unsigned int        u32;
typedef wxULongLong_t       u64;


/* ------------------------------------------------------------------------
   Macros
   ------------------------------------------------------------------------ */


#define STR wxString::Format

#define FOREACHX(_VAR, _VAR_NEXT, _TYPE, _LIST, _ELT) \
    for (_TYPE::Node *_VAR_NEXT, *_VAR = (_LIST).GetFirst(); \
    _VAR ? (_VAR_NEXT=_VAR->GetNext(), _ELT = _VAR->GetData()) : (_VAR_NEXT=NULL,_ELT=NULL); \
         _VAR = _VAR_NEXT)

#define AFOREACHX(_INDEX, _LIST, _VAR) \
    for (size_t _INDEX=0;\
    _INDEX<_LIST.GetCount()?(_VAR=_LIST[_INDEX],true):(false);\
    _INDEX++)

#define FOREACH(_TYPE, _LIST, _ELT) FOREACHX(_node,  _node_next,  _TYPE, _LIST, _ELT)
#define FOREACH2(_TYPE, _LIST, _ELT) FOREACHX(_node2, _node2_next, _TYPE, _LIST, _ELT)
#define FOREACH3(_TYPE, _LIST, _ELT) FOREACHX(_node3, _node3_next, _TYPE, _LIST, _ELT)
#define FOREACH4(_TYPE, _LIST, _ELT) FOREACHX(_node4, _node4_next, _TYPE, _LIST, _ELT)
#define FOREACH5(_TYPE, _LIST, _ELT) FOREACHX(_node5, _node5_next, _TYPE, _LIST, _ELT)

#define AFOREACH(_LIST, _ELT) AFOREACHX(_index,  _LIST, _ELT)
#define AFOREACH2(_LIST, _ELT) AFOREACHX(_index2, _LIST, _ELT)
#define AFOREACH3(_LIST, _ELT) AFOREACHX(_index3, _LIST, _ELT)
#define AFOREACH4(_LIST, _ELT) AFOREACHX(_index4, _LIST, _ELT)
#define AFOREACH5(_LIST, _ELT) AFOREACHX(_index5, _LIST, _ELT)


#define DELNUL(_pointer) if (_pointer) {delete _pointer; _pointer = NULL;}
#define DELNULARR(_pointer) if (_pointer) {delete [] _pointer; _pointer = NULL;}

#define MIN(_a,_b)((_a)<(_b)?(_a):(_b))
#define MAX(_a,_b)((_a)>(_b)?(_a):(_b))

/* ------------------------------------------------------------------------
   Type "liste d'entiers"
   ------------------------------------------------------------------------ */

WX_DECLARE_LIST(int, local_intList);

class intList : public local_intList
{
public:
    intList();
    void Add(const int i);
    void Insert(size_t index, const int i);
    intList::Node *Find(const int i) const;
    bool Delete(const int i);
    bool Member(const int i) const;
    bool operator==(const intList& other) const;
    intList& operator=(const intList& other);
    int operator[](const int &n) const;
    int Shift();
    void Sort();
    void Unique();
    void Join(char separator, wxString &result, bool reset=true);
    void Split(const wxString &str, bool reset=true);
    void Intersection(const intList& other);
private:
    void DoCopy(const intList&);
};



bool tooEqual(const wxStringList &list1, const wxStringList &list2);
void tooSet(wxStringList &list1, const wxStringList &list2);

#define dmsInitConsoleAppli() \
{wxInitializer initializer;if (! initializer){\
LOGE("Failed to initialize the wxWindows library, aborting.");\
getchar();exit(-1);}}

/* ------------------------------------------------------------------------
   Classe qui applique une pause cycliquement (après x appels à
   "operator++")
   ------------------------------------------------------------------------ */

class dmsSleepCpt
{
public:
    int m_iCount;
    int m_iMaxCount;
    int m_iDelay;

public:
    dmsSleepCpt(int maxcount, int delay);
    virtual ~dmsSleepCpt();

    dmsSleepCpt& operator++(int);
};



class dmsBuffer
{
public:
    uchar* m_poBuffer;
    uint   m_iBufferLen;
    uchar* m_poCurrent;

public:
    dmsBuffer();
    virtual ~dmsBuffer();

    void Clear();
    void Set(uchar* Buffer, uint Size);
    void SetText(char* Buffer, uint Size);
    void Set(const dmsBuffer &other){Set(other.Begin(),other.Len());}
    void Set(const dmsBuffer &other, uint offset, uint len);
    void Set(const char *value, bool withZero);
    void Set(unsigned char value, int repeat);
    bool Save(const wxString &filename);
    bool Save(const wxString &filename, int offset, int len);
    bool Load(const wxString &filename);
    bool Load(const wxString &filename, int offset, int maxLength);

    uint Len()const{return m_iBufferLen;}
    bool IsEmpty(){return m_iBufferLen==0;}
    uchar* Begin()const{return m_poBuffer;}
    uchar* Current()const{return m_poCurrent;}
    void Move(dmsBuffer &other);
    void Reset(){m_poCurrent = m_poBuffer;}
    void Alloc(uint Size);
    void ReAlloc(uint Size);
    void Write(const dmsBuffer &other);
    off_t Read(u8* other, uint len);
    void Append(const dmsBuffer &other);
    uint Remaining() {return m_iBufferLen-(m_poCurrent-m_poBuffer);}

    bool SetFromDump(const wxString &dump);
    unsigned int CRC32();

    bool LoadResource(const wxString& resourceName);

};

WX_DECLARE_LIST(dmsBuffer, dmsBufferList);


#endif /* _TOOLS_TOOLS_H_ */
