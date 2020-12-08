/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 11/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode.
   - LIPPA - SmarDTV - v 2.00 - 10/2011 - Support 2 PMT and 2 data Carousel
   - LIPPA - SmarDTV - v 2.12 - 02/2012 - DSI repetition all seconds.
   - LIPPA - SmarDTV - v 2.13 - 03/2012 - Generate raw sections in a file and
                                          choice generated files extensions.
   - LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
                                          Header in sections file
   ************************************************************************ */

#ifndef _DVB_LIB_TS_WRITER_H_
#define _DVB_LIB_TS_WRITER_H_

#include <wx/wx.h>

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class wxXmlNode;
class wxXmlDocument;

class dmsMultiplexer;
class dmsDataCarousel;
class dmsTS_WriterConf;

class dmsTS_Writer
{
public:
    dmsMultiplexer*   m_poMux;
    dmsDataCarousel*  m_poCarousel;
    dmsDataCarousel*  m_poCarousel2;
    dmsTS_WriterConf* m_poConfig;

    wxString       m_oOutputDir;
    wxString       m_oConfigFile;
    wxXmlDocument* m_poDoc;


public:
    dmsTS_Writer();
    virtual ~dmsTS_Writer();

    int DeleteDirectory(wxString sDirectoryName);

    bool Load(const wxString &filename, bool bClean = false);
    bool Load(wxXmlNode* root, bool bClean = false);

    int  OnRun();
    void Run(int argc, char *argv[]);
    bool Generate();
};


#endif /* _DVB_LIB_TS_WRITER_H_ */
