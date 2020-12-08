/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DMS_HEADER_TYPES_H_
#define _DMS_HEADER_TYPES_H_

#include <wx/wx.h>
#include <wx/ffile.h>

#include <stdio.h>

#include <Tools/Tools.h>

class dmsData;
class dmsvContext;
class dmsXmlContextManager;
class dmsvNumeric;

#include "Conversion.h"

typedef dmsData*            u0;
typedef unsigned char       u_1;
typedef unsigned char       u_2;
typedef unsigned char       u_3;
typedef unsigned char       u_4;
typedef unsigned char       u_5;
typedef unsigned char       u_8;

typedef struct
{
    u_8 *Begin;
    u_8 *End;
    u_8 *HardEnd;
    u_8 *Current;
    u_8 CurrentBit;
    bool Clear;
} HtoNBuf_t;


void HtoNBufInit(HtoNBuf_t *buf);
void HtoNBufSet(HtoNBuf_t *buf, u8* membuf, size_t size, bool clear);
void HtoNBufAlloc(HtoNBuf_t *buf, size_t size);
void HtoNBufReset(HtoNBuf_t *buf);
void HtoNBufBitInc(HtoNBuf_t *buf, int step);


#define  ASSERT(x) if(!x) printf("Assertion in file %s (line %u)\n", __FILE__, __LINE__)
/*
#define  SWAP16(x) { u_8 Byte = x & 0xFF; \
                     x >>= 8; \
                     x |= Byte <<8;}
#define  SWAP32(x) { u32 Long = x; \
                     x >>= 24; \
                     x |= (Long >> 8) & 0xFF00; \
                     x |= (Long << 8) & 0xFF0000; \
                     x |= (Long << 24) & 0xFF000000;}
*/
#define HtoN_8Set(x,buf) {(buf)[0]=(x);}
#define HtoN16Set(x,buf) {(buf)[0]=(u_8)((x)>>8);(buf)[1]=(u_8)(x);}
#define HtoN24Set(x,buf) {(buf)[0]=(u_8)((x)>>16);(buf)[1]=(u_8)((x)>>8);(buf)[2]=(u_8)(x);}
#define HtoN32Set(x,buf) {(buf)[0]=(u_8)((x)>>24);(buf)[1]=(u_8)((x)>>16);(buf)[2]=(u_8)((x)>>8);(buf)[3]=(u_8)(x);}

#define HtoN_8Or(x,buf) {(buf)[0]|=(x);}
#define HtoN16Or(x,buf) {(buf)[0]|=(u_8)((x)>>8);(buf)[1]|=(u_8)(x);}
#define HtoN32Or(x,buf) {(buf)[0]|=(u_8)((x)>>24);(buf)[1]|=(u_8)((x)>>16);(buf)[2]|=(u_8)((x)>>8);(buf)[3]|=(u_8)(x);}

#define HtoN_8(x,buf) {HtoN_8Set(x,(buf)->Current);HtoNBufBitInc(buf, 8);}
#define HtoN16(x,buf) {HtoN16Set(x,(buf)->Current);HtoNBufBitInc(buf,16);}
#define HtoN24(x,buf) {HtoN24Set(x,(buf)->Current);HtoNBufBitInc(buf,24);}
#define HtoN32(x,buf) {HtoN32Set(x,(buf)->Current);HtoNBufBitInc(buf,32);}
#define HtoNxx(_x,_size,_buf) {memcpy(_buf->Current,_x,_size);(_buf->Current)+=_size;}
#define HtoNSS(_x, _buf) HtoNxx(_x, sizeof(_x), _buf)

#define HtoN_8_SizeInit(_x, _size_buf, _buf) {(_size_buf)=_buf->Current; HtoN_8(_x, _buf);}
#define HtoN16_SizeInit(_x, _size_buf, _buf) {(_size_buf)=_buf->Current; HtoN16(_x, _buf);}
#define HtoN32_SizeInit(_x, _size_buf, _buf) {(_size_buf)=_buf->Current; HtoN32(_x, _buf);}

#define HtoN_8_SizeSet(_x, _size_buf, _buf) {_x=_buf->Current-_size_buf-1;HtoN_8Set(_x,_size_buf);}
#define HtoN16_SizeSet(_x, _size_buf, _buf) {_x=_buf->Current-_size_buf-2;HtoN16Set(_x,_size_buf);}
#define HtoN32_SizeSet(_x, _size_buf, _buf) {_x=_buf->Current-_size_buf-4;HtoN32Set(_x,_size_buf);}



#define HtoN_B_8(buf)       {(buf->Current)[0]=0;}
#define HtoN_S_8(x,buf,i,j) HtoN_8Or((x)<<(8-i-j),(buf)->Current);
#define HtoN_E_8(buf)       {HtoNBufBitInc(buf, 8);}

#define HtoN_B16(buf)       {(buf->Current)[0]=0;(buf->Current)[1]=0;}
#define HtoN_S16(x,buf,i,j) HtoN16Or((x)<<(16-i-j),(buf)->Current);
#define HtoN_E16(buf)       {HtoNBufBitInc(buf, 16);}
#define HtoN_S16Set(x,buf,i,j) HtoN16Or((x)<<(16-i-j),buf);

#define TRACE_TAG_OPEN(_f, _text) {fprintf(_f, "<%s>", _text);}
#define TRACE_TAG_CLOSE(_f, _text) {fprintf(_f, "</%s>\n", _text);}
#define TRACE_TAG(_f, _tag, _content) {TRACE_TAG_OPEN(_f, _tag);_content;TRACE_TAG_CLOSE(_f, _tag);}

#define TRACE_ASCII(_f,_text) {fprintf(_f, "%s\n", _text);}
#define TRACE_HEXA1(_f,_tag,_value) {fprintf(_f, "<%s value=\"0x%0*X\"/>\n", _tag, sizeof(_value)*2,_value);}
#define TRACE_HEXAN(_f,_tag,_value,_len) {fprintf(_f, "<%s value=\"0x%0*X\"/>\n", _tag, _len*2,_value);}
#if 0
#define TRACE_DECI1(_f,_tag,_value) {fprintf(_f, "<%s value=\"%d\"/>\n", _tag, _value);}
#else
#define TRACE_DECI1(_f,_tag,_value) {fprintf(_f, "<%s value=\"%d (0x%0*X)\"/>\n", _tag, _value, sizeof(_value)*2,_value, _tag);}
#endif

void Trace_DumpHexa(FILE *f, u_8* data, size_t size);
void Trace_DumpNAscii(FILE *f, u_8* data, size_t size);

#define TRACE_DUMP1(_f,_data,_size)  Trace_DumpHexa(_f, _data, _size);
#define TRACE_DUMPT(_f,_tag,_data,_size) {TRACE_TAG(_f, _tag, TRACE_DUMP1(_f, _data, _size));}
#define TRACE_DUMPAS(_f,_data,_size) Trace_DumpNAscii(_f, _data, _size));
#define TRACE_DUMPAT(_f,_tag, _data) {TRACE_TAG(_f, _tag, Trace_DumpNAscii(_f, _data, sizeof(_data)));}
#define TRACE_DUMPATS(_f,_tag, _data,_size) {TRACE_TAG(_f, _tag, Trace_DumpNAscii(_f, _data, _size));}
#define TRACE_DUMPTS(_f,_tag, _data) TRACE_DUMPT(_f, _tag, _data, sizeof(_data))



#define HDR_DEFINE_LIST(_type)\
class _type##List : public dmsDataList\
{public:_type##List():dmsDataList(){;}\
dmsData *Create(void *pt, const char *tag){return new _type();}};


#define FREE_NULL(pt) { free(pt); (pt)=NULL; }

#ifndef BOOL
#define BOOL  unsigned char
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

// Alloue et initialise une zone mémoire

#define MEM_CPY_NEW(_dst, _src, _len) (_dst) = new u_8[_len]; memcpy(_dst,_src,_len);

// Ajout d'un item "_ITEM" dans une liste d'éléments de type "_TYPE"
// - La liste commence par l'item "_FIRST"
// - L'élément suivant est représenté par l'attribut "Next"

#define LIST_ADD(_TYPE, _FIRST, _ITEM)\
{if (_FIRST==NULL){_FIRST=_ITEM;}\
else {_TYPE* Last=_FIRST;while(Last->Next)Last=Last->Next;Last->Next=_ITEM;_ITEM->Next=NULL;}}


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */


#include <wx/wx.h>

class dmsHtoN;
class wxXmlNode;


class dmsHtoNIndex
{
public:
    u_8 *CurrentByte;
    u_8 CurrentBit;

public:
    dmsHtoNIndex(){;}
    dmsHtoNIndex(u_8 *B, u_8 b){CurrentByte=B;CurrentBit=b;}
    virtual ~dmsHtoNIndex(){;}

    void Get(u_8 *&B, u_8 &b){B=CurrentByte;b=CurrentBit;}
};

WX_DECLARE_LIST(dmsHtoNIndex, dmsHtoNIndexList);



class dmsHtoNBuffer
{
public:
    HtoNBuf_t m_oBuffer;

    dmsHtoNIndexList m_oIndexList;

public:
    dmsHtoNBuffer();
    dmsHtoNBuffer(u8* buffer, size_t size, bool clear);
    virtual ~dmsHtoNBuffer();

    HtoNBuf_t *GetHtoNBuffer(){return &m_oBuffer;}
    u8 *GetBegin(){return m_oBuffer.Begin;}
    size_t GetSize(){return MAX(m_oBuffer.End,m_oBuffer.Current)-m_oBuffer.Begin;}

    void Copy(dmsBuffer &Buffer);

private:
    void PushIndex();
    void PopIndex(int &diff);

public:
    void Write(void *pt, u_8 byteLen, u_8 bitLen);
    void WriteByte(u_8 val,u_8 len=8);
    void Write(u_8 val,u_8 len=8);
    void Write(u16 val,u_8 len=16);
    void Write(u32 val,u_8 len=32);
    void Write(u64 val,u_8 len=64);
    void Write(const dmsBuffer &buf);

    void Skip(int byteLen, int bitLen);

    void PushLen(u_8 val,u_8 len=8);
    void PushLen(u16 val,u_8 len=16);
    void PushLen(u32 val,u_8 len=32);
    void PushLen(void *pt, u_8 byteLen, u_8 bitLen);
    void PopLen(u_8 &val,u_8 len=8);
    void PopLen(u16 &val,u_8 len=16);
    void PopLen(u32 &val,u_8 len=32);
    void PopLen(void *pt, u_8 byteLen, u_8 bitLen);

    void Backward(int nbBytes);
    void UpdateCRC32(unsigned int &CRC);
    void UpdateCRC16(unsigned short &CRC);

    void GotoEnd();
};



class dmsHtoN : public dmsHtoNBuffer
{
private:
    static u8* m_poSharedBuffer;
    static wxCriticalSection m_oCS;
    static bool m_bLocked;

public:
    dmsHtoN();
    virtual ~dmsHtoN();

    void Lock() {m_oCS.Enter();m_bLocked=true;}
    void Unlock() {m_oCS.Leave(); m_bLocked=false;}
};



#define HDR_INIT(_name, _length) Add(&_name, #_name, _length, sizeof(_name));

class dmsDataList;
class dmsHeaderItem;
WX_DECLARE_LIST(dmsHeaderItem, dmsHeaderItemList);






class dmsHeaderItem
{
public:
    void*                 m_pItem;

    wxString              m_oDeclareName;
    wxString              m_oLoadName;
    wxString              m_oOutputName;

    u_8                   m_iBitSize;
    u_8                   m_iByteSize;
    bool                  m_bPush;
    dmsHeaderItemList     m_oPopList;
    bool                  m_bLoad;
    bool                  m_bLoadOpt;
    int                   m_iHtoNOffset;
    dmsNameRank*          m_poLoadValues;
    dmsDataList*          m_pListToCount;
    dmsData*              m_pLenData;
    dmsData*              m_poParent;
    bool                  m_bImplemented;
    bool                  m_bOk;
    dmsvContext*          m_poContext;

public:
    dmsHeaderItem(dmsData* parent, void *pt, const wxString &name, u8 bitSize, u8 byteSize);
    virtual ~dmsHeaderItem();

    bool SetValue(u64 value);
    u64  GetValue();
    void SetName();
};




class dmsData
{
    friend dmsDataList;

public:
    dmsHeaderItemList      m_oItemList;
    dmsHeaderItem*         m_poItem;
    dmsXmlContextManager*  m_poContextManager;

    void Add(void *pt, const wxString &name, u8 bitSize, u8 byteSize);
    void SetLenLimit(void *begin, void *end);
    void SetDataLen(void* lenpt, dmsData* data);
    void SetListCount(void *count, dmsDataList* list);
    void SetName(void *pt, const wxString &name);
    void SetData(dmsData **pt, dmsData *data);
    void SetLoad(void *pt, bool opt);
    void SetLoad(void *pt, const char *tag);
    void SetLoad(void *pt, const char *tag, bool opt);
    void SetLoad(dmsData **pt, const char *tag, dmsData *data);
    void SetUnimplemented(dmsData **pt);
    void SetValues(void *pt, dmsNameRank* values);
    void SetValues(void *pt, const char *tag, dmsNameRank* values);
    void SetMapping(void *pt, dmsvNumeric *map);

    dmsHeaderItem *Find(void *pt);

public:
    wxString  m_oLoadedName;
    wxString  m_oOutputName;
    wxString  m_oTraceFilename;
    bool      m_bOk;
    dmsData*  m_poParent;

    void SetName();
    void SetName(const wxString &name);
    int  GetMaxItemNameLen();
    wxString GetLongName();

    dmsBuffer m_oBuffer;
    bool      m_bGenerated;
    bool      m_bUpdateComplete;

public:
    dmsData();
    virtual ~dmsData();

    void Generate1();
    void Generate1(dmsHtoN &hton);
    bool UpdateRec();
    void Trace(const wxString &traceDirname, bool onlyTxt=false);
    bool GeneratedError();

    virtual void    ShowUpdateErrors();
    virtual bool Load(wxXmlNode *node);
    virtual bool Update();
    virtual dmsData* Create(void *pt, const char *tag);
    virtual void TraceXml(FILE *f, const wxString &prefix);
    virtual void TraceTxt(FILE *f, const wxString &prefix);

protected:
    virtual void Generate(dmsHtoN &hton);
};

WX_DECLARE_LIST(dmsData, dmsDataListInternal);

class dmsDataList : public dmsData
{
friend dmsData;

protected:
    dmsDataListInternal m_oList;

public:
    dmsDataList();
    virtual ~dmsDataList();
    void Init();
    bool Load(wxXmlNode *node);
    void Generate(dmsHtoN &hton);
    void TraceXml(FILE *f, const wxString &prefix);
    void TraceTxt(FILE *f, const wxString &prefix);
    bool Update();
    void Append(dmsData* data);
    virtual void    ShowUpdateErrors();
};


class dmsDataAscii : public dmsData
{
public:
    bool m_bWithZero;
public:
    dmsDataAscii(bool withZero=true) : dmsData(){m_bWithZero=withZero;}
    bool Load(wxXmlNode *node);
};

class dmsDataHexa : public dmsData
{
public:
    dmsDataHexa() : dmsData(){;}
    bool Load(wxXmlNode *node);
};

#endif /* _DMS_HEADER_TYPES_H_ */

