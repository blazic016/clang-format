/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.12 - 02/2012 - DSI repetition all seconds.
   - LIPPA - SmarDTV - v 2.13 - 03/2012 - Generate raw sections in a file and
                                          choice generated files extensions.
   - LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
                                          Header in sections file
   ************************************************************************ */

#ifndef _LIB_DVB_TS_WRITER_CONF_H_
#define _LIB_DVB_TS_WRITER_CONF_H_

class wxXmlNode;

class dmsTS_WriterConf
{
public:
    wxString m_oOutputFile;
    wxString m_oOutputSectionsFile;
    wxString m_oOutputSigFile;
    bool     m_bTrace;
   int      m_iLoader;
   int      m_iModeSectionsFile;
   u32      m_u32UpdateId;
   u32      m_u32OUI;
   u16      m_u16PlateformModel;
   u16      m_u16PlateformVersion;
   u16      m_u16ProductModel;
   u16      m_u16ProductVersion;

public:
    dmsTS_WriterConf();
    virtual ~dmsTS_WriterConf();

public:
    bool Load(wxXmlNode *node);
};

#endif /* _LIB_DVB_TS_WRITER_CONF_H_ */
