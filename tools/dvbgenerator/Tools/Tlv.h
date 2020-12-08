/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 09/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _TOOLS_TLV_H_
#define _TOOLS_TLV_H_

#include <wx/wx.h>

#include "Tools.h"


class dmsTlvGroup;

WX_DECLARE_LIST(dmsTlvGroup, dmsTlvGroupList);


class dmsTlv
{
private:
    wxInputStream*  m_poInput;
    wxOutputStream* m_poOutput;
    dmsTlvGroupList m_oGroupList;

public:
    typedef enum {read, write} Mode;

public:
    wxString m_oFilename;
    Mode     m_eMode;
    bool     m_bOk;

private:
    bool BeginGroupRead(short id);
    void EndGroupRead();
    bool EndOfGroupRead();

    bool BeginGroupWrite(short id);
    void EndGroupWrite();
    bool EndOfGroupWrite();

    void PushGroup();
    void PopGroup();

    long &GetGroupBeginning();
    long &GetGroupEnd();
    long GetIndex();
    bool GotoAbs(long pos);
    bool GotoRel(long dep);

public:
    dmsTlv(const wxString &filename, Mode mode = read);
    dmsTlv();
    ~dmsTlv();

    bool Open(const wxString &filename, Mode mode = read);

    bool Read(short &value);
    bool Read(long  &value);
    bool Read(int64 &value);
    bool Read(wxString &value, long length);
    bool ReadTL(short &id, long &length);
    bool ReadTL();
    bool ReadTL(short &id);
    bool ReadTL(long &length);
    bool ReadForwardTL(short &id, long &length);

    bool Read(short id, int &value);
    bool Read(short id, int64 &value);
    bool Read(short id, wxString &value);
    bool Read(short id, wxDateTime &value);

    void Write(short value);
    void Write(long  value);
    void Write(int64 value);
    void Write(const wxString &value);
    void WriteTL(short id, long length);

    void Write(short id, int value);
    void Write(short id, int64 value);
    void Write(short id, const wxString &value);
    void Write(short id, const wxDateTime &value);

    bool BeginGroup(short id);
    void EndGroup();
    bool EndOfGroup();
};


class dmsTlvGroup
{
public:
    long m_iBeginning;
    long m_iEnd;

public:
    dmsTlvGroup();
    virtual ~dmsTlvGroup();
};


#endif /* _TOOLS_TLV_H_ */
