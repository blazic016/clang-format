/* ************************************************************************
   SmarDTV

   Description : Outils Standards

   Historique :
   - COF   - Iwedia  - v 0    - 05/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/file.h>

#include "Tools.h"
#include "Conversion.h"
#include "File.h"
#include "CRC.h"

#include <errno.h>

#ifdef __UNIX__
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#endif


/* ========================================================================
   Liste d'entiers
   ======================================================================== */


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(local_intList);


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsBufferList);



intList::intList() : local_intList()
{
    DeleteContents(true);
}




void intList::Add(const int i)
{
    Append(new int(i));
}


void intList::Insert(size_t index, const int i)
{
    wxListBase::Insert(index, new int(i));
}

intList::Node *intList::Find(const int i) const
{
    int *x;

    FOREACH(intList, (*this), x)
    {
        if (i==*x) return _node;
    }

    return NULL;
}

bool intList::Delete(const int i)
{
    intList::Node *pti;

    if ((pti=Find(i)))
        return DeleteNode(pti);

    return false;
}


bool intList::Member(const int i) const
{
    return (Find(i) != NULL);
}




bool intList::operator==(const intList& other) const
{
    size_t count = GetCount();

    if (count != other.GetCount()) return false;

    for (size_t n=0; n<count; n++)
    {
        if ((*Item(n)->GetData()) != (*other.Item(n)->GetData())) return false;
    }

    return true;
}




intList& intList::operator=(const intList& other)
{
    Clear();

    DoCopy(other);

    return *this;
}






int intList::operator[](const int &n) const
{
    if (n >= (int) GetCount()) return -1;

    return *(Item(n)->GetData());
}






void intList::DoCopy(const intList& other)
{
    wxASSERT(GetCount() == 0);

    size_t count = other.GetCount();
    for (size_t n=0; n<count; n++)
    {
        Add(*(other.Item(n)->GetData()));
    }
}



int intList::Shift()
{
    if (GetCount() == 0) return -1;

    intList::Node *node = Item(0);
    int result = *(node->GetData());

    DeleteNode(node);

    return result;
}





void intList::Unique()
{
    int *i;

    FOREACH(intList, (*this), i)
    {
        if (Find(*i) != _node) DeleteNode(_node);
    }
}


int intListSortFunction(const int **arg1, const int **arg2)
{
    if (**arg1 == **arg2) return 0;
    if (**arg1 >  **arg2) return 1;
    return -1;
}



void intList::Sort()
{
    local_intList::Sort(intListSortFunction);
}



void intList::Join(char separator, wxString &result, bool reset)
{
    int *i;
    bool empty;

    if (reset) result ="";

    empty = (result.Len() == 0);

    FOREACH(intList, (*this), i)
    {
        if (empty)
        {
            result << *i;
            empty = false;
        }
        else
            result << separator << *i;
    }
}


void intList::Split(const wxString &str, bool reset)
{
    const char *x;
    int i;

    if (reset) Clear();

    x = str.GetData();

    while (*x)
    {
        while (*x && (*x < '0' || *x > '9') && *x != '-') x++;
        if (*x && (sscanf(x, "%d", &i) > 0))
        {
            Add(i);
            if (*x == '-') x++;
            while (*x >= '0' && *x <= '9') x++;
        }
    }
}


void intList::Intersection(const intList& other)
{
    int *i;

    FOREACH(intList, (*this), i)
    {
        if (! other.Member(*i)) DeleteNode(_node);
    }
}




/* ========================================================================

   ======================================================================== */


bool tooEqual(const wxStringList &list1, const wxStringList &list2)
{
    size_t size = list1.GetCount();

    if (size != list2.GetCount()) return false;

    for (size_t i=0; i<size; i++)
    {
        if (wcscmp(list1[i], list2[i]) != 0) return false;
    }

    return true;
}


void tooSet(wxStringList &list1, const wxStringList &list2)
{
    list1.Clear();

    wxChar *s;

    FOREACH(wxStringList, list2, s)
        list1.Add(s);
}



/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsSleepCpt::dmsSleepCpt(int maxcount, int delay)
{
    m_iMaxCount = maxcount;
    m_iDelay    = delay;
    m_iCount    = 0;
}


dmsSleepCpt::~dmsSleepCpt()
{

}


/* ========================================================================

   ======================================================================== */


dmsSleepCpt& dmsSleepCpt::operator++(int i)
{
    m_iCount++;

    if (m_iCount >= m_iMaxCount)
    {
        m_iCount = 0;
        wxThread::Sleep(m_iDelay);
    }
    return *this;
}
/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsBuffer::dmsBuffer()
{
    m_poBuffer   = NULL;
    m_poCurrent  = NULL;
    m_iBufferLen = 0;
}


dmsBuffer::~dmsBuffer()
{
    Clear();
}


/* ========================================================================

   ======================================================================== */

void dmsBuffer::Clear()
{
    DELNULARR(m_poBuffer);
    m_iBufferLen = 0;
    m_poCurrent  = 0;
}

void dmsBuffer::Set(uchar* Buffer, uint Size)
{
    Alloc(Size);

    memcpy(m_poBuffer, Buffer, Size);
}

void dmsBuffer::SetText(char* Buffer, uint Size)
{
    Alloc(Size+1);

    memcpy(m_poBuffer, Buffer, Size);

    m_poBuffer[Size]=0;
}

void dmsBuffer::Set(const dmsBuffer &other, uint offset, uint len)
{
    if (offset+len>other.m_iBufferLen) len=other.m_iBufferLen-offset;

    if (len >= 0)
    {
        Alloc(len);
        memcpy(m_poBuffer, other.m_poBuffer+offset, len);
    }
    else
    {
        Clear();
    }
}


void dmsBuffer::Set(const char *value, bool withZero)
{
    int len = withZero?strlen(value)+1:strlen(value);

    Set((uchar*)value, len);
}


void dmsBuffer::Set(unsigned char value, int repeat)
{
    if (repeat > 0)
    {
        Alloc(repeat);
        memset(m_poBuffer, value, repeat);
    }
    else
    {
        Clear();
    }
}


bool dmsBuffer::Save(const wxString &filename)
{
    wxFile file;

    dmsAssumeDir(wxPathOnly(filename));

    LOG_AF(file.Open(filename, wxFile::write), LOGE(L"Error opening file [%s]", filename));


    if (m_poBuffer)
    {
        LOG_AF(file.Write(m_poBuffer, m_iBufferLen) == m_iBufferLen,
            LOGE(L"Error writing [%d] bytes in [%s]", m_iBufferLen, filename));
    }

    file.Close();

    return true;
}

bool dmsBuffer::Save(const wxString &filename, int offset, int len)
{
    wxFile file;

    dmsAssumeDir(wxPathOnly(filename));

    LOG_AF(file.Open(filename, wxFile::write), LOGE(L"Error opening file [%s]", filename));

    if (m_poBuffer)
    {
        if (offset+len > (int) m_iBufferLen)
        {
            LOGE(L"Error in saving buffer : offset+len(%d/%d) > len(%d)", offset, len, m_iBufferLen);
            return false;
        }
        LOG_AF(file.Write(m_poBuffer+offset, len) == (size_t) len,
            LOGE(L"Error writing [%d] bytes in [%s]", len, filename));
    }

    file.Close();

    return true;
}

bool dmsBuffer::Load(const wxString &filename)
{
    wxFile file;

    LOG_AF(file.Open(filename, wxFile::read), LOGE(L"Error opening file [%s]", filename));

    Alloc(file.Length());

    LOG_AF(file.Read(m_poBuffer, m_iBufferLen) == (off_t) m_iBufferLen,
        LOGE(L"Error reading [%d] bytes in [%s]", m_iBufferLen, filename));

    file.Close();

    return true;
}


bool dmsBuffer::Load(const wxString &filename, int offset, int maxLength)
{
    wxFile file;

    LOG_AF(file.Open(filename, wxFile::read), LOGE(L"Error opening file [%s]", filename));

    int length = file.Length()-offset;

    if (length > maxLength)
        length = maxLength;

    Alloc(length);

    file.Seek(offset);

    LOG_AF(file.Read(m_poBuffer, m_iBufferLen) == (off_t) length,
        LOGE(L"Error reading [%d] bytes in [%s]", m_iBufferLen, filename));

    file.Close();

    return true;
}

/* ------------------------------------------------------------------------
   Bouge une zone mémoire dans le sens suivant

   other <- this
   ------------------------------------------------------------------------ */

void dmsBuffer::Move(dmsBuffer &other)
{
    other.Clear();

    other.m_poBuffer   = m_poBuffer;
    other.m_iBufferLen = m_iBufferLen;
    other.Reset();

    m_poBuffer   = NULL;
    m_iBufferLen = 0;
}

void dmsBuffer::Alloc(uint Size)
{
    Clear();

    //LOG0("Alloc %d bytes", Size);

    m_poBuffer   = new uchar[Size];
    m_iBufferLen = Size;

    Reset();
}


void dmsBuffer::ReAlloc(uint Size)
{
    uchar* tmp = new uchar[Size];

    if (m_poBuffer)
    {
        memcpy(tmp, m_poBuffer, MIN(Size, m_iBufferLen));

        m_poCurrent = tmp+(m_poCurrent-m_poBuffer);

        delete [] m_poBuffer;
    }

    m_poBuffer   = tmp;
    m_iBufferLen = Size;

    if (m_poCurrent == NULL) Reset();
}

void dmsBuffer::Write(const dmsBuffer &other)
{
    if (other.Len() > Remaining())
    {
        ReAlloc(Len()-Remaining()+other.Len());
    }

    memcpy(m_poCurrent, other.m_poBuffer, other.m_iBufferLen);

    m_poCurrent += other.m_iBufferLen;
}


off_t dmsBuffer::Read(u8* other, uint len)
{
    if (len<Remaining()) len = Remaining();

    memcpy(other, m_poCurrent, len);

    m_poCurrent += len;

    return len;
}


void dmsBuffer::Append(const dmsBuffer &other)
{
    if (other.Len()==0) return;

    uint oldLen = m_iBufferLen;

    ReAlloc(Len()+other.Len());

    memcpy(m_poBuffer+oldLen, other.Begin(), other.Len());
}

bool dmsBuffer::SetFromDump(const wxString &dump)
{
    Clear();

    wxString other = dump;

    if (other.StartsWith("0x")) other.Replace("0x","",false);

    Alloc(other.Len()/2);

    return StrHexaToBuffer(other, m_poBuffer, m_iBufferLen);
}


unsigned int dmsBuffer::CRC32()
{
    return CalculateCRC32(Begin(), Len());
}


bool dmsBuffer::LoadResource(const wxString& resourceName)
{
#ifndef __UNIX__
    HRSRC hResource = ::FindResource(wxGetInstance(), resourceName, resourceType);
    if ( hResource == 0 )
    {
        LOGE(L"No resource [%s:%s]", resourceName, resourceType);
        return false;
    }

    HGLOBAL hData = ::LoadResource(wxGetInstance(), hResource);
    if ( hData == 0 )
        return false;

    void* resourceBuffer = ::LockResource(hData);
    if ( !resourceBuffer )
        return false;

    int len = ::SizeofResource(wxGetInstance(), hResource);

    if (resourceType=="TEXT")
        SetText((char*)resourceBuffer, len);
    else
        Set((u8*)resourceBuffer, len);

#ifndef __WIN32__
    UnlockResource(hData);
#endif
#else
    struct stat st; /*declare stat variable*/
    if (::stat(resourceName, &st) != 0) {
        LOGE(L"stat error (%d): %s", errno, strerror(errno));
        return false;
    }

    int fd = ::open(resourceName, O_RDWR);
    if (fd < 0) {
        return false;
    }

    char *buf = new char[st.st_size];

    if (::read(fd, buf, st.st_size) < 0) {
        delete [] buf;
        return false;
    }
    SetText(buf, st.st_size);

    delete [] buf;
#endif
    return true;
}
