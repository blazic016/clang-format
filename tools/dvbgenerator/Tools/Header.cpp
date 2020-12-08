/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <stdio.h>
#include <stdlib.h>

#include <Tools/Header.h>
#include <Tools/Tools.h>
#include <Tools/Xml.h>
#include <Tools/CRC.h>

#include "File.h"


#define PREFIX_TXT_TAB ".   "
#define PREFIX_XML_TAB "    "

/* ########################################################################

   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsHeaderItemList);

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsDataListInternal);

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsHtoNIndexList);


void HtoNBufInit(HtoNBuf_t *buf)
{
    buf->Begin      = NULL;
    buf->HardEnd    = NULL;
    buf->End        = NULL;
    buf->Current    = NULL;
    buf->CurrentBit = 0;
    buf->Clear      = true;
}

void HtoNBufSet(HtoNBuf_t *buf, u8* membuf, size_t size, bool clear)
{
    buf->Begin      = membuf;
    buf->HardEnd    = buf->Begin+size;
    buf->End        = buf->Begin;
    buf->Current    = buf->Begin;
    buf->CurrentBit = 0;
    buf->Clear      = clear;

    if (buf->Clear) *(buf->Current)=0;
}

void HtoNBufAlloc(HtoNBuf_t *buf, size_t size)
{
    HtoNBufSet(buf, (u_8*) malloc(size), size, true);
}

void HtoNBufBitInc(HtoNBuf_t *buf, int step)
{
    if (step==0) return;

    buf->CurrentBit += step;

    while (buf->CurrentBit>7) {buf->Current++;buf->CurrentBit-=8;}

    if (buf->Current > buf->End)
    {
        buf->End = buf->Current;
        if (buf->Clear) *(buf->End)=0;
    }
}

void HtoNBufByteInc(HtoNBuf_t *buf, int step)
{
    if (step==0) return;

    buf->Current += step;

    if (buf->Current > buf->End)
    {
        buf->End = buf->Current;
        if (buf->Clear) *(buf->End)=0;
    }
}

void HtoNBufReset(HtoNBuf_t *buf)
{
    buf->Current = buf->Begin;
    buf->End     = buf->Begin;

    if (buf->Clear) *(buf->Current) = 0;
}




void Trace_DumpHexa(FILE *f, u_8* data, size_t size)
{
    size_t i;

    for (i=0; i<size; i++)
    {
        fprintf(f," %02X", data[i]);
    }
}


void Trace_DumpNAscii(FILE *f, u_8* data, size_t size)
{
    size_t i;

    for (i=0; i<size; i++)
    {
        if ((data[i]>='0' && data[i]<='9')
            || (data[i]>='A'&&data[i]<='Z')
            || (data[i]>='a'&&data[i]<='z'))
            fprintf(f,"%c", data[i]);
        else
            fprintf(f,"%c", '.');
        if (i%32==31) fprintf(f, " ");
    }
}


/* ########################################################################

   ######################################################################## */

u8* dmsHtoN::m_poSharedBuffer = NULL;
wxCriticalSection dmsHtoN::m_oCS;
bool dmsHtoN::m_bLocked=false;

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

#define HTON_BUF_MAX_SIZE 16*1024*1024

dmsHtoN::dmsHtoN() : dmsHtoNBuffer()
{
    Lock();

    if (m_poSharedBuffer == NULL)
    {
        HtoNBufAlloc(&m_oBuffer, HTON_BUF_MAX_SIZE);
        m_poSharedBuffer = m_oBuffer.Begin;
    }
    else
    {
        HtoNBufSet(&m_oBuffer, m_poSharedBuffer, HTON_BUF_MAX_SIZE, true);
    }

    m_oIndexList.DeleteContents(true);
}


dmsHtoN::~dmsHtoN()
{
    if (m_oIndexList.GetCount()>0)
    {
        LOG0(L"Bad usage of index in HtoN");
    }
    if (m_bLocked) Unlock();
}


/* ########################################################################

   ######################################################################## */


#define HDR_OBJ(_item, _size) (*((u##_size*)_item->m_pItem))

dmsHeaderItem::dmsHeaderItem(dmsData* parent, void *pt, const wxString &name, u8 bitSize, u8 byteSize)
{
    m_pItem            = pt;
    m_oDeclareName     = name;
    m_oLoadName        = name;
    m_iBitSize         = bitSize;
    m_iByteSize        = byteSize;
    m_bPush            = false;
    m_bLoad            = false;
    m_bLoadOpt         = false;
    m_poLoadValues     = NULL;
    m_iHtoNOffset      = 0;
    m_pListToCount     = NULL;
    m_pLenData         = NULL;
    m_bImplemented     = true;
    m_bOk              = true;
    m_poContext        = NULL;
    m_poParent         = parent;
}

dmsHeaderItem::~dmsHeaderItem()
{
}



bool dmsHeaderItem::SetValue(u64 value)
{
    LOG_AF(m_iBitSize,  LOGE(L"Error, set int value to pointer"));

    u64 max = 0xFFFFFFFFFFFFFFFF >> (64-m_iBitSize);

    LOG_AF(value <= max, LOGE(L"Can't set value [%llx] in [%s:%d bits]", value, m_oOutputName, m_iBitSize));

    switch(m_iByteSize)
    {
    case 1: HDR_OBJ(this,  8) = value; break;
    case 2: HDR_OBJ(this, 16) = value; break;
    case 4: HDR_OBJ(this, 32) = value; break;
    case 8: HDR_OBJ(this, 64) = value; break;
    }

    return true;
}

u64 dmsHeaderItem::GetValue()
{
    switch(m_iByteSize)
    {
    case 1: return HDR_OBJ(this,  8); break;
    case 2: return HDR_OBJ(this, 16); break;
    case 4: return HDR_OBJ(this, 32); break;
    case 8: return HDR_OBJ(this, 64); break;
    }
    return -1;
}

void dmsHeaderItem::SetName()
{
    if (m_oOutputName.IsEmpty())
    {
        if (m_oLoadName.Len())
            m_oOutputName = m_oLoadName;
        else
            m_oOutputName = m_oDeclareName;
    }

    if (m_iBitSize==0 && HDR_OBJ(this, 0))
    {
        dmsData* data = (dmsData*)HDR_OBJ(this, 0);

        if (data->m_poParent==NULL)
            data->m_poParent = m_poParent;

        if (data->m_oOutputName.IsEmpty())
            data->m_oOutputName = m_oOutputName;
        else
            m_oOutputName = data->m_oOutputName;
    }

    if (m_oOutputName != m_oDeclareName && m_oOutputName.Find('(')==-1)
    {
        m_oOutputName = STR("%s (%s)", m_oDeclareName, m_oOutputName);
    }

    if (m_oOutputName.IsEmpty())
    {
        TODO();
    }
}

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsData::dmsData()
{
    m_bGenerated       = false;
    m_poItem           = NULL;
    m_bOk              = true;
    m_poContextManager = NULL;
    m_bUpdateComplete  = false;
    m_poParent         = NULL;

    m_oItemList.DeleteContents(true);
}


dmsData::~dmsData()
{
    if (! m_bUpdateComplete)
    {
        ;// LOG0("Dev Error : Data [%s] has never been updated", GetLongName()); // REPUT
    }

    dmsHeaderItem *item;

    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        if (item->m_iBitSize==0 && HDR_OBJ(item, 0)) delete ((dmsData*)HDR_OBJ(item, 0));
    }

    DELNUL(m_poContextManager);
}


/* ========================================================================

   ======================================================================== */

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

wxString dmsData::GetLongName()
{
    if (m_poParent)
        return m_poParent->GetLongName()+"/"+m_oOutputName;
    else
        return m_oOutputName;
}

/* ------------------------------------------------------------------------
   Recherche d'un item
   ------------------------------------------------------------------------ */

dmsHeaderItem *dmsData::Find(void *pt)
{
    dmsHeaderItem *item;

    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        if (item->m_pItem == pt) return item;
    }


#ifdef _DEBUG
   // TODO: Ca serait bien de voir pourquoi cette erreur
    LOG0(L"DATA [%p] not found", pt);
#endif

    return NULL;
}

void dmsData::SetName(void *pt, const wxString &name)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_oOutputName = name;
        item->m_oLoadName   = name;
    }
}

void dmsData::SetData(dmsData **pt, dmsData *data)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        (*pt) = data;

        if (data)
        {
            data->m_poItem   = item;
            data->m_poParent = this;
        }
    }
}

void dmsData::Generate1()
{
    dmsHtoN hton;

    UpdateRec();
    Generate(hton);
    ShowUpdateErrors();

    hton.Copy(m_oBuffer);
    hton.Unlock();
}

void dmsData::Generate1(dmsHtoN &hton)
{
    UpdateRec();
    Generate(hton);
    ShowUpdateErrors();
}

bool dmsData::Update()
{
    return true;
}

void dmsData::ShowUpdateErrors()
{
    if (m_bUpdateComplete) return;

    dmsHeaderItem *item;

    int nb=0;
    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        if (item->m_iBitSize==0)
        {
            dmsData* data = HDR_OBJ(item, 0);

            if (data && !data->m_bUpdateComplete)
            {
                data->ShowUpdateErrors();
                nb++;
            }
        }
    }

    if (nb==0)
    {
        //LOG0("Uncomplete update on %s", GetLongName());
        Update();
    }
}

bool dmsData::UpdateRec()
{
    if (m_bUpdateComplete) return true;

    SetName();

    m_bUpdateComplete = Update();

    dmsHeaderItem *item;

    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        item->SetName();

        if (item->m_iBitSize==0 && HDR_OBJ(item, 0))
        {
            if (! HDR_OBJ(item, 0)->UpdateRec())
                m_bUpdateComplete = false;
        }
    }

    return m_bUpdateComplete;
}


void dmsData::SetName(const wxString &name)
{
    m_oOutputName = name;

    if (m_poItem)
        m_poItem->m_oOutputName = name;
}

bool dmsData::GeneratedError()
{
    if (m_bGenerated)
    {
        LOGE(L"Initialisation after generation on [%s]", m_oTraceFilename);
    }

    return m_bGenerated;
}


void dmsData::Trace(const wxString &traceDirname, bool onlyTxt)
{
    wxString name;

    if (m_oTraceFilename.Len()==0)
        m_oTraceFilename = m_oOutputName;

    name = traceDirname+"/"+m_oTraceFilename;

    if (! onlyTxt)
        m_oBuffer.Save(name+".bin");
    else
        dmsAssumeDir(traceDirname);

    // Version trace

    wxFFile file;

    if (! file.Open(name+".txt", "w"))
    {
        LOGE(L"Error opening file [%s]", name+".txt");
        return;
    }
    TraceTxt(file.fp(),"");
    file.Close();
}




void dmsData::Add(void *pt, const wxString &name, u8 bitSize, u8 byteSize)
{
    dmsHeaderItem *item;

    //item = Find(pt);
    //if (item) return;

    item = new dmsHeaderItem(this, pt, name, bitSize, byteSize);

    if (item->m_iBitSize==0)
    {
        HDR_OBJ(item, 0) = NULL;
    }
    else
    {
        switch(item->m_iByteSize)
        {
        case 1: HDR_OBJ(item, 8) = 0; break;
        case 2: HDR_OBJ(item,16) = 0; break;
        case 4: HDR_OBJ(item,32) = 0; break;
        case 8: HDR_OBJ(item,64) = 0; break;
        }
    }

    m_oItemList.Append(item);
}

void dmsData::SetLenLimit(void *begin, void *end)
{
    dmsHeaderItem *item1 = Find(begin);
    dmsHeaderItem *item2 = Find(end);

    if (item1 && item2)
    {
        item1->m_bPush = true;
        item2->m_oPopList.Insert((size_t)0, item1);
    }
}

void dmsData::SetDataLen(void* lenpt, dmsData* data)
{
    dmsHeaderItem *item = Find(lenpt);

    if (item)
    {
        item->m_pLenData = data;
    }
}

void dmsData::SetListCount(void *count, dmsDataList* list)
{
    dmsHeaderItem *item = Find(count);

    if (item)
    {
        item->m_pListToCount = list;
    }
}



void dmsData::SetLoad(void *pt, bool opt)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_bLoad    = true;
        item->m_bLoadOpt = opt;
    }
}

void dmsData::SetLoad(void *pt, const char *tag)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_bLoad     = true;
        item->m_oLoadName = tag;
    }
}

void dmsData::SetLoad(void *pt, const char *tag, bool opt)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_bLoad     = true;
        item->m_oLoadName = tag;
        item->m_bLoadOpt  = opt;
    }
}



void dmsData::SetLoad(dmsData **pt, const char* tag, dmsData *data)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_bLoad          = true;
        item->m_oLoadName      = tag;
        SetData(pt, data);
    }
}



void dmsData::SetUnimplemented(dmsData **pt)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_bImplemented = false;
    }
}


void dmsData::SetValues(void *pt, dmsNameRank* values)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_bLoad        = true;
        item->m_poLoadValues = values;
    }
}

void dmsData::SetValues(void *pt, const char *tag, dmsNameRank* values)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_bLoad        = true;
        item->m_poLoadValues = values;
        item->m_oLoadName    = tag;
    }
}

void dmsData::SetMapping(void *pt, dmsvNumeric *map)
{
    dmsHeaderItem *item = Find(pt);

    if (item)
    {
        item->m_bLoad     = true;

        map->dmsValidator::SetMapping(pt);
        map->m_iBitMemSize = item->m_iByteSize*8;
        map->m_iBitSize    = item->m_iBitSize;

        if (m_poContextManager==NULL)
            m_poContextManager = new dmsXmlContextManager(NULL);

        item->m_poContext = m_poContextManager->Add(item->m_oLoadName, map);
    }
}


bool dmsData::Load(wxXmlNode *node)
{
    if (node==NULL)
    {
        m_bOk = false;
        return false;
    }

    if (m_poContextManager)
        m_poContextManager->m_poNode = node;

    dmsHeaderItem *item;

    m_oLoadedName = node->GetName();

    node->Used();

    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        if (! item->m_bLoad) continue;

        wxString name = item->m_oLoadName;
        if (item->m_bLoadOpt) name << "?";

        if (item->m_iBitSize==0)
        {
            if (HDR_OBJ(item, 0)==NULL)
            {
                dmsData *res = NULL;
                wxXmlNode *child;

                for (child=node->GetChildren();child;child=child->GetNext())
                {
                    res = Create(&(HDR_OBJ(item, 0)), child->GetName());
                    if (res)
                    {
                        SetData(&HDR_OBJ(item, 0), res);
                        item->m_bOk = res->Load(child);
                        break;
                    }
                }
            }

            if (HDR_OBJ(item, 0))
            {
                wxXmlNode *child = node;

                if (item->m_oLoadName.Len())
                    child = node->Find(name);

                if (child)
                {
                    item->m_bOk = HDR_OBJ(item, 0)->Load(child);
                }
                else
                {
                    item->m_bOk = item->m_bLoadOpt;
                }
            }
        }
        else
        {
            u64 value=0;
            bool res;

            if (item->m_poContext)
            {
                item->m_bOk = item->m_poContext->Set();

                if (!item->m_bOk)
                {
                    item->m_poContext->ShowError();
                }
                continue;
            }
            else if (item->m_poLoadValues)
                res = node->Read(name, &value, item->m_poLoadValues);
            else
                res = node->Read(name, &value);

            if (res)
            {
                item->m_bOk = item->SetValue(value);
            }
            else
            {
                item->m_bOk = item->m_bLoadOpt;
            }
        }

        if (! item->m_bOk)
        {
            LOGE(L"Fatal error in tag [%s]", node->GetLongName(item->m_oLoadName));
            m_bOk = false;
        }
    }

    return m_bOk;
}

void dmsData::SetName()
{
    if (m_oOutputName.IsEmpty())
    {
        if (m_poItem)
        {
            m_poItem->SetName();
            m_oOutputName = m_poItem->m_oOutputName;
        }
        else
        {
            if (m_oLoadedName.Len())
                m_oOutputName = m_oLoadedName;
            else
                m_oOutputName = "NO_NAME";
        }
    }
}

void dmsData::Generate(dmsHtoN &hton)
{
    dmsHeaderItem *item;

    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        if (item->m_pListToCount)
        {
            item->SetValue(item->m_pListToCount->m_oList.GetCount());
        }
        if (item->m_pLenData)
        {
            item->SetValue(item->m_pLenData->m_oBuffer.Len());
        }
        if (item->m_bPush)
        {
            item->m_iHtoNOffset = hton.m_oBuffer.Current-hton.m_oBuffer.Begin;
            hton.PushLen(item->m_pItem, item->m_iByteSize, item->m_iBitSize);
        }
        else if (item->m_iBitSize==0)
        {
            if (HDR_OBJ(item, 0))
            {
                HDR_OBJ(item, 0)->Generate(hton);
                if (! HDR_OBJ(item, 0)->m_bOk)
                {
                    item->m_bOk = false;
                    m_bOk = false;
                }
            }
        }
        else
        {
            item->m_iHtoNOffset = hton.m_oBuffer.Current-hton.m_oBuffer.Begin;
            hton.Write(item->m_pItem, item->m_iByteSize, item->m_iBitSize);
        }

        dmsHeaderItem *itemPushed;

        FOREACH2(dmsHeaderItemList, item->m_oPopList, itemPushed)
        {
            hton.PopLen(itemPushed->m_pItem, itemPushed->m_iByteSize, itemPushed->m_iBitSize);
        }
    }
    if (m_oItemList.GetCount()==0 && m_oBuffer.Len())
    {
        hton.Write(m_oBuffer);
    }

    m_bGenerated = true;
}


dmsData* dmsData::Create(void *pt, const char *tag)
{
    LOGE(L"NOT IMPLEMETED (Create tag [%s] in [%s])", tag, m_oOutputName);
    return NULL;
}


int dmsData::GetMaxItemNameLen()
{
    dmsHeaderItem *item;
    unsigned int max = 0;

    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        if (item->m_oOutputName.Len()>max) max = item->m_oOutputName.Len();
    }

    return max;
}



void dmsData::TraceXml(FILE *f, const wxString &prefix)
{
    dmsHeaderItem *item;

    if (m_oItemList.GetCount()==0 && m_oBuffer.Len()==0)
    {
        fprintf(f, "%s<%s/>\n", (const char *)prefix, (const char *)m_oOutputName);
        return;
    }

    fprintf(f, "%s<%s>\n", (const char *)prefix, (const char *)m_oOutputName);

    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        if (item->m_iBitSize==0)
        {
            if (HDR_OBJ(item, 0))
                ((dmsData*)HDR_OBJ(item, 0))->TraceXml(f, prefix+PREFIX_XML_TAB);
            else if (item->m_bImplemented)
                fprintf(f, "(null)");
            else
                fprintf(f, "(not implemented)");
        }
        else
        {
            u64 value = item->GetValue();

            if (item->m_bPush && value==0)
                fprintf(f, "%s%s<%s value=\"**EMPTY**\"/>\n", (const char *)prefix,PREFIX_XML_TAB,(const char *)item->m_oOutputName);
            else
                fprintf(f, "%s%s<%s value=\"0x%0*llX\"/>\n", (const char *)prefix,PREFIX_XML_TAB,(const char *)item->m_oOutputName, item->m_iByteSize*2, value);
        }
    }

    if (m_oItemList.GetCount()==0)
    {
        if (m_oBuffer.m_iBufferLen > 40 && f == stdout)
            fprintf(f, "*** BIG (%d Bytes) ***", m_oBuffer.m_iBufferLen);
        else
            Trace_DumpNAscii(f, m_oBuffer.m_poBuffer, m_oBuffer.m_iBufferLen);
    }

    fprintf(f, "%s</%s>\n", (const char *)prefix, (const char *)m_oOutputName);
}



void dmsData::TraceTxt(FILE *f, const wxString &prefix)
{
    dmsHeaderItem *item;
    wxString longPrefix;
    wxString name;

    int max = GetMaxItemNameLen();

    if (m_oItemList.GetCount()==0)
    {
        if (m_oBuffer.m_iBufferLen > 40 && f == stdout)
            fprintf(f, "*** BIG (%d Bytes) ***", m_oBuffer.m_iBufferLen);
        else
            Trace_DumpNAscii(f, m_oBuffer.m_poBuffer, m_oBuffer.m_iBufferLen);
        fprintf(f, "\n");

        return;
    }

    if (prefix.Len())
        fprintf(f, "\n");
    else
        fprintf(f, "[0000] %s%s\n", (const char *)prefix, (const char *)m_oOutputName);

    FOREACH(dmsHeaderItemList, m_oItemList, item)
    {
        wxString size;

        if (item->m_iBitSize)
            size = STR("%2d", item->m_iBitSize);
        else if (HDR_OBJ(item, 0))
            size = STR("N(%d)", ((dmsData *)HDR_OBJ(item, 0))->m_oBuffer.m_iBufferLen*8);
        else
            size = "N";

        longPrefix = STR("[%s] %s%s%-*s %s = %s",
            item->m_iHtoNOffset?STR("%4X", item->m_iHtoNOffset):"    ",
            prefix, PREFIX_TXT_TAB, max,
            item->m_oOutputName,
            size,
            item->m_bOk?"":"##################### ERROR");

        if (item->m_iBitSize==0)
        {
            fprintf(f, "%s ", (const char *)longPrefix);
            if (HDR_OBJ(item, 0))
                ((dmsData *)HDR_OBJ(item, 0))->TraceTxt(f, prefix+PREFIX_TXT_TAB);
            else if (item->m_bImplemented)
                fprintf(f, "(null)\n");
            else
                fprintf(f, "(not implemented)\n");
        }
        else
        {
            u64 value = item->GetValue();

            fprintf(f, "%s 0x%0*llX (%llu)", (const char *)longPrefix, item->m_iBitSize/4, value, value);
            if (item->m_poLoadValues)
            {
                fprintf(f, " \"%s\"", dmsGetName(value, item->m_poLoadValues));
            }
            fprintf(f, "\n");
        }
    }
}



/* ========================================================================

   ======================================================================== */


dmsDataList::dmsDataList() : dmsData()
{
    m_oList.DeleteContents(true);
}


dmsDataList::~dmsDataList()
{
    return;
}

void dmsDataList::Init()
{
    return;
}



bool dmsDataList::Load(wxXmlNode *node)
{
    wxXmlNode* child;
    dmsData*   data;

    m_oLoadedName = node->GetName();

    node->Used();

    for (child=node->GetChildren();child;child=child->GetNext())
    {
        data = Create(NULL, child->GetName());

        if (data)
        {
            Append(data);
            data->Load(child);
            m_bOk = m_bOk && data->m_bOk;
        }
        else
        {
            LOGE(L"Bad child type [%s] in [%s]", child->GetName(), node->GetLongName());
        }
    }
    return m_bOk;
}




bool dmsDataList::Update()
{
    if (m_bUpdateComplete) return true;

    dmsData *data;

    m_bUpdateComplete = true;

    FOREACH(dmsDataListInternal, m_oList, data)
    {
        if (! data->UpdateRec()) m_bUpdateComplete = false;
    }

    return m_bUpdateComplete;
}



void dmsDataList::ShowUpdateErrors()
{
    if (m_bUpdateComplete) return;

    dmsData *data;

    int nb=0;
    FOREACH(dmsDataListInternal, m_oList, data)
    {
        if (! data->m_bUpdateComplete)
        {
            nb++;
            data->ShowUpdateErrors();
        }
    }

    if (nb==0)
    {
        LOG0(L"Uncomplete update on %s", GetLongName());
    }
}


void dmsDataList::Generate(dmsHtoN &hton)
{
    dmsData *data;

    FOREACH(dmsDataListInternal, m_oList, data)
    {
        data->Generate(hton);
        if (! data->m_bOk) m_bOk = false;
    }

    m_bGenerated = true;
}

void dmsDataList::TraceXml(FILE *f, const wxString &prefix)
{
    dmsData *data;

    fprintf(f, "%s<%s>\n", (const char *)prefix, (const char *)m_oOutputName);

    FOREACH(dmsDataListInternal, m_oList, data)
    {
        data->TraceXml(f, prefix+PREFIX_XML_TAB);
    }

    fprintf(f, "%s</%s>\n", (const char *)prefix, (const char *)m_oOutputName);
}

void dmsDataList::TraceTxt(FILE *f,const wxString &prefix)
{
    dmsData *data;

    if (m_oList.GetCount())
    {
        fprintf(f, "LIST Count=%lu\n", m_oList.GetCount());
        int i=1;
        FOREACH(dmsDataListInternal, m_oList, data)
        {
            fprintf(f, "[    ] %s%s%s (%d/%lu)", (const char *)prefix, PREFIX_TXT_TAB,
                    (const char *)data->m_oOutputName,
                i++, m_oList.GetCount());
            data->TraceTxt(f, prefix+PREFIX_TXT_TAB);
        }
    }
    else
    {
        fprintf(f, "** EMPTY LIST **\n");
    }
}





void dmsDataList::Append(dmsData* data)
{
    m_oList.Append(data);

    data->m_poParent = this;
}




bool dmsDataAscii::Load(wxXmlNode *node)
{
    node->ReadAscii("", &m_oBuffer, m_bWithZero);
    return true;
}


bool dmsDataHexa::Load(wxXmlNode *node)
{
    node->Read("", &m_oBuffer);
    return true;
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsHtoNBuffer::dmsHtoNBuffer()
{
    HtoNBufInit(&m_oBuffer);
}

dmsHtoNBuffer::dmsHtoNBuffer(u8* buffer, size_t size, bool clear)
{
    HtoNBufSet(&m_oBuffer, buffer, size, clear);
}

dmsHtoNBuffer::~dmsHtoNBuffer()
{

}


/* ========================================================================

   ======================================================================== */

#if 0
#define DEBUG_HTON_WRITE(_format)\
    LOG0("[%3d:%d] %sWrite_%2d 0x%0*"_format, \
    m_oBuffer.Current-m_oBuffer.Begin, m_oBuffer.CurrentBit,\
    m_oBuffer.Current==m_oBuffer.End?"":"+ ",\
    len, sizeof(val), val);
#define DEBUG_HTON_WRITE_DETAIL\
    LOG0("[%3d:%d] -> %sWrite_%2d 0x%0*X => |= 0x%X (0x%X << %d)", \
    m_oBuffer.Current-m_oBuffer.Begin, m_oBuffer.CurrentBit,\
    m_oBuffer.Current==m_oBuffer.End?"":"+ ",\
    len, sizeof(val), val, (val<<(8-len-m_oBuffer.CurrentBit)), val, 8-len-m_oBuffer.CurrentBit);
#else
#define DEBUG_HTON_WRITE(_format)
#define DEBUG_HTON_WRITE_DETAIL
#endif


void dmsHtoNBuffer::Write(void *pt, u_8 byteLen, u_8 bitLen)
{
    switch (byteLen)
    {
    case 1: Write(*((u_8*)pt), bitLen); break;
    case 2: Write(*((u16*)pt), bitLen); break;
    case 4: Write(*((u32*)pt), bitLen); break;
    default:
        LOGE(L"BAD SIZE");
        break;
    }
}


/* ------------------------------------------------------------------------
   Ecrit une nombre "x" de taille "len" bits (<=8) sur un octet de buffer "hton"
   NB : rest = 8 - CurrentBit

   2 cas :

   cas 1) L'ecriture se fait sur un seul octet "hton"

             CurrentBit
             |len
             .|||||rest-len
             ......||
             ########
              #####
     => On ecrit avec un decalage de (Rest-Len)

   cas 2) L'ecriture se fait sur deux octets "hton"

             CurrentBit
             |len
              ||||||| |
             ######## ########
              ####### #
     => Fait deux écritures de "cas 1"
        - Une écriture de "x>>CurrentBit" de taille "rest"
        - Une écriture de "x" de taille "len-rest"

   ------------------------------------------------------------------------ */

void dmsHtoNBuffer::WriteByte(u_8 val,u_8 len)
{
    val=val&(0xFF>>(8-len)); // On s'assure que "val" tient bien sur "len" bits (pour les "ou")

    u8 rest = 8-m_oBuffer.CurrentBit;

    if (len>rest)
    {
        WriteByte(val>>m_oBuffer.CurrentBit, rest);
        WriteByte(val, len-rest);
    }
    else
    {
        *(m_oBuffer.Current) |= (val<<(rest-len));

        DEBUG_HTON_WRITE_DETAIL;

        HtoNBufBitInc(&m_oBuffer, len);
    }
}

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */


void dmsHtoNBuffer::Write(u_8 val,u_8 len)
{
    DEBUG_HTON_WRITE("X");

    if (len>0)  {WriteByte(val, len);}
}

void dmsHtoNBuffer::Write(u16 val,u_8 len)
{
    DEBUG_HTON_WRITE("X");

    u_8 *buf = (u_8*)&val;
    if (len>8)  {WriteByte(buf[1], len-8); len=8;}
    if (len>0)  {WriteByte(buf[0], len);}
}

void dmsHtoNBuffer::Write(u32 val,u_8 len)
{
    DEBUG_HTON_WRITE("X");

    u_8 *buf = (u_8*)&val;
    if (len>24) {WriteByte(buf[3], len-24); len=24;}
    if (len>16) {WriteByte(buf[2], len-16); len=16;}
    if (len>8)  {WriteByte(buf[1], len- 8); len= 8;}
    if (len>0)  {WriteByte(buf[0], len);}
}

void dmsHtoNBuffer::Write(u64 val,u_8 len)
{
    DEBUG_HTON_WRITE("I64X");

    u_8 *buf = (u_8*)&val;
    if (len>56) {WriteByte(buf[7], len-56); len=56;}
    if (len>48) {WriteByte(buf[4], len-48); len=48;}
    if (len>40) {WriteByte(buf[5], len-40); len=40;}
    if (len>32) {WriteByte(buf[4], len-32); len=32;}
    if (len>24) {WriteByte(buf[3], len-24); len=24;}
    if (len>16) {WriteByte(buf[2], len-16); len=16;}
    if (len>8)  {WriteByte(buf[1], len- 8); len= 8;}
    if (len>0)  {WriteByte(buf[0], len);}
}



void dmsHtoNBuffer::Write(const dmsBuffer &buf)
{
    memcpy(m_oBuffer.Current, buf.m_poBuffer, buf.m_iBufferLen);

    HtoNBufByteInc(&m_oBuffer, buf.m_iBufferLen);
}


void dmsHtoNBuffer::Copy(dmsBuffer &Buffer)
{
    Buffer.Set(GetBegin(), GetSize());
}

void dmsHtoNBuffer::PushIndex()
{
    m_oIndexList.Append(new dmsHtoNIndex(m_oBuffer.Current, m_oBuffer.CurrentBit));
}
void dmsHtoNBuffer::PopIndex(int &diff)
{
    u_8 *current = m_oBuffer.Current;

    if (m_oIndexList.GetCount()==0)
    {
        LOGE(L"Bad usage of Push/Pop index in HtoN");
        return;
    }
    m_oIndexList[m_oIndexList.GetCount()-1]->Get(m_oBuffer.Current, m_oBuffer.CurrentBit);
    m_oIndexList.DeleteNode(m_oIndexList.GetLast());

    diff = current-m_oBuffer.Current;
}
void dmsHtoNBuffer::GotoEnd()
{
    m_oBuffer.Current    = m_oBuffer.End;
    m_oBuffer.CurrentBit = 0;
}

void dmsHtoNBuffer::Backward(int nbBytes)
{
    m_oBuffer.Current -= nbBytes;
}


void dmsHtoNBuffer::PushLen(u_8 val,u_8 len)
{
    PushIndex();Write(val,len);PushIndex();
}
void dmsHtoNBuffer::PushLen(u16 val,u_8 len)
{
    PushIndex();Write(val,len);PushIndex();
}
void dmsHtoNBuffer::PushLen(u32 val,u_8 len)
{
    PushIndex();Write(val,len);PushIndex();
}
void dmsHtoNBuffer::PushLen(void *pt, u_8 byteLen, u_8 bitLen)
{
    switch (byteLen)
    {
    case 1: PushLen(*((u_8*)pt), bitLen); break;
    case 2: PushLen(*((u16*)pt), bitLen); break;
    case 4: PushLen(*((u32*)pt), bitLen); break;
    }
}
void dmsHtoNBuffer::PopLen(u_8 &val,u_8 len)
{
    int diff, tmp;
    PopIndex(diff);PopIndex(tmp);val=diff;Write(val,len);val=diff;GotoEnd();
}
void dmsHtoNBuffer::PopLen(u16 &val,u_8 len)
{
    int diff, tmp;
    PopIndex(diff);PopIndex(tmp);val=diff;Write(val,len);val=diff;GotoEnd();
}
void dmsHtoNBuffer::PopLen(u32 &val,u_8 len)
{
    int diff, tmp;
    PopIndex(diff);PopIndex(tmp);val=diff;Write(val,len);val=diff;GotoEnd();
}
void dmsHtoNBuffer::PopLen(void *pt, u_8 byteLen, u_8 bitLen)
{
    switch (byteLen)
    {
    case 1: PopLen(*((u_8*)pt), bitLen); break;
    case 2: PopLen(*((u16*)pt), bitLen); break;
    case 4: PopLen(*((u32*)pt), bitLen); break;
    }
}



void dmsHtoNBuffer::UpdateCRC32(unsigned int &CRC)
{
    Backward(4);

    CRC = CalculateCRC32(m_oBuffer.Begin, m_oBuffer.Current-m_oBuffer.Begin);

    Write(CRC);
}

void dmsHtoNBuffer::UpdateCRC16(unsigned short &CRC)
{
    Backward(2);

    CRC = CalculateCRC16(m_oBuffer.Begin, m_oBuffer.Current-m_oBuffer.Begin);

    Write(CRC);
}

void dmsHtoNBuffer::Skip(int byteLen, int bitLen)
{
    HtoNBufByteInc(&m_oBuffer, byteLen);
    HtoNBufBitInc (&m_oBuffer, bitLen);
}
