/* ************************************************************************
   SmarDTV

   Description : !!!!!!!!!!!!!!!!!! Pas blindé, pas de test sur les datetime

   Historique :
   - COF   - Iwedia  - v 0    - 09/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/file.h>
#include <wx/wfstream.h>

#include "Tlv.h"


/* ########################################################################
   Déclarations
   ######################################################################## */

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsTlvGroupList);

/* ########################################################################
   Classe dmsTLV
   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsTlv::dmsTlv(const wxString &filename, Mode mode)
{
    m_poInput  = NULL;
    m_poOutput = NULL;

    Open(filename, mode);

    m_oGroupList.DeleteContents(true);
}


dmsTlv::~dmsTlv()
{

}


/* ========================================================================

   ======================================================================== */

bool dmsTlv::Open(const wxString &filename, Mode mode)
{
    m_eMode     = mode;
    m_oFilename = filename;

    DELNUL(m_poInput);
    DELNUL(m_poOutput);

    if (mode==read)
    {
        m_poInput = new wxFileInputStream(filename);
        m_bOk = m_poInput->IsOk();
    }
    else
    {
        m_poOutput = new wxFileOutputStream(filename);
        m_bOk = m_poOutput->IsOk();
    }

    return m_bOk;
}

/* ========================================================================

   ======================================================================== */
void dmsTlv::PushGroup()
{
    m_oGroupList.Insert((size_t)0, new dmsTlvGroup);
}
void dmsTlv::PopGroup()
{
    m_oGroupList.DeleteNode(m_oGroupList.GetFirst());
}
long &dmsTlv::GetGroupBeginning()
{
    return m_oGroupList[0]->m_iBeginning;
}
long &dmsTlv::GetGroupEnd()
{
    return m_oGroupList[0]->m_iEnd;
}
long dmsTlv::GetIndex()
{
    if (m_poInput)
    {
        if (m_poInput->IsOk())
            return m_poInput->TellI();
        else
            return -1;
    }
    else if (m_poOutput)
    {
        if (m_poOutput->IsOk())
            return m_poOutput->TellO();
        else
            return -1;
    }
    else
        return -1;
}
bool dmsTlv::GotoAbs(long pos)
{
    if (m_poInput)
    {
        if (m_poInput->IsOk())
            m_poInput->SeekI(pos);
        else return false;
    }
    else if (m_poOutput)
    {
        if (m_poOutput->IsOk())
            m_poOutput->SeekO(pos);
        else return false;
    }
    return true;
}
bool dmsTlv::GotoRel(long dep)
{
    return GotoAbs(GetIndex()+dep);
}

/* ========================================================================
   Lectures
   ======================================================================== */
/* ------------------------------------------------------------------------
   Lectures basiques
   ------------------------------------------------------------------------ */
bool dmsTlv::Read(short &value)
{
    if (! m_poInput->IsOk()) return false;

    m_poInput->Read(&value, sizeof(value));
    return true;
}
bool dmsTlv::Read(long  &value)
{
    if (! m_poInput->IsOk()) return false;

    m_poInput->Read(&value, sizeof(value));
    return true;
}
bool dmsTlv::Read(int64 &value)
{
    if (! m_poInput->IsOk()) return false;

    m_poInput->Read(&value, sizeof(value));
    return true;
}
bool dmsTlv::Read(wxString &value, long length)
{
    if (! m_poInput->IsOk()) return false;

    m_poInput->Read(wxStringBuffer(value, length), length);
    return true;
}
/* ------------------------------------------------------------------------
   Lecture du TL
   ------------------------------------------------------------------------ */
bool dmsTlv::ReadTL(short &id, long &length)
{
    return Read(id) && Read(length);
}
bool dmsTlv::ReadTL()
{
    return GotoRel(6);
}
bool dmsTlv::ReadTL(short &id)
{
    return Read(id) && GotoRel(4);
}
bool dmsTlv::ReadTL(long &length)
{
    return GotoRel(2) &&    Read(length);
}
bool dmsTlv::ReadForwardTL(short &id, long &length)
{
    int index = GetIndex();

    return ReadTL(id, length) && GotoAbs(index);
}
/* ------------------------------------------------------------------------
   Lectures de TL de base
   ------------------------------------------------------------------------ */
bool dmsTlv::Read(short id, int &value)
{
    return ReadTL() && Read((long&)value);
}
bool dmsTlv::Read(short id, int64 &value)
{
    return ReadTL() && Read(value);
}
bool dmsTlv::Read(short id, wxString &value)
{
    long length;

    return ReadTL(length) && Read(value, length);
}
bool dmsTlv::Read(short id, wxDateTime &value)
{
    long nb;
    bool ok = ReadTL() && Read(nb);
    value.Set((time_t)nb);
    return ok;
}
/* ========================================================================
   Ecritures
   ======================================================================== */
/* ------------------------------------------------------------------------
   Ecritures basiques
   ------------------------------------------------------------------------ */
void dmsTlv::Write(short value)
{
    m_poOutput->Write(&value, sizeof(value));
}
void dmsTlv::Write(long value)
{
    m_poOutput->Write(&value, sizeof(value));
}
void dmsTlv::Write(int64 value)
{
    m_poOutput->Write(&value, sizeof(value));
}
void dmsTlv::Write(const wxString &value)
{
    m_poOutput->Write(value.c_str(), value.Len());
}
/* ------------------------------------------------------------------------
   Ecriture du TL
   ------------------------------------------------------------------------ */
void dmsTlv::WriteTL(short id, long length)
{
    Write(id);
    Write(length);
}
/* ------------------------------------------------------------------------
   Ecritures de TL de base
   ------------------------------------------------------------------------ */
void dmsTlv::Write(short id, const wxString &value)
{
    WriteTL(id, value.Len());
    Write(value);
}
void dmsTlv::Write(short id, int value)
{
    WriteTL(id, sizeof(value));
    Write((long)value);
}
void dmsTlv::Write(short id, int64 value)
{
    WriteTL(id, sizeof(value));
    Write(value);
}
void dmsTlv::Write(short id, const wxDateTime &value)
{
    WriteTL(id, sizeof(time_t));
    Write(value.GetTicks());
}
/* ========================================================================
   Gestion des groupes
   ======================================================================== */
/* ------------------------------------------------------------------------
   Gestion des groupes en lecture
   ------------------------------------------------------------------------ */
bool dmsTlv::BeginGroupRead(short id)
{
    short id2;
    long length2;
    ReadForwardTL(id2, length2);
    GetGroupEnd() = GetGroupBeginning()+length2;
    ReadTL();
    return true;
}
void dmsTlv::EndGroupRead()
{
}
bool dmsTlv::EndOfGroupRead()
{
    return true;
}
/* ------------------------------------------------------------------------
   Gestion des groupes en écriture
   ------------------------------------------------------------------------ */
bool dmsTlv::BeginGroupWrite(short id)
{
    WriteTL(id|0x8000, 0);
    return true;
}
void dmsTlv::EndGroupWrite()
{
    long beginning = GetGroupBeginning();
    long len       = GetIndex() - beginning;

    GotoAbs(beginning);
    GotoRel(2);
    Write(len);
    GotoRel(len-6);
}
bool dmsTlv::EndOfGroupWrite()
{
    return true;
}
/* ------------------------------------------------------------------------
   Appels génériques
   ------------------------------------------------------------------------ */
bool dmsTlv::BeginGroup(short id)
{
    PushGroup();
    GetGroupBeginning() = GetIndex();

    if (m_eMode == read)
        return BeginGroupRead(id);
    else
        return BeginGroupWrite(id);
}
void dmsTlv::EndGroup()
{
    if (m_eMode == read)
        EndGroupRead();
    else
        EndGroupWrite();
    PopGroup();
}
bool dmsTlv::EndOfGroup()
{
    return GetIndex() >= GetGroupEnd();
}


/* ########################################################################
   Classe dmsTlvGroup
   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsTlvGroup::dmsTlvGroup()
{
    m_iBeginning=0;
    m_iEnd=0;
}


dmsTlvGroup::~dmsTlvGroup()
{

}


/* ========================================================================

   ======================================================================== */
