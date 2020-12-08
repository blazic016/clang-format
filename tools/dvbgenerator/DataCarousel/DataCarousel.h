/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 11/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Rename object 'cIwediaXXX' to
                                          'cGeniusXXX'
   ************************************************************************ */

#ifndef _LIB_DATA_CAROUSEL_MAIN_H_
#define _LIB_DATA_CAROUSEL_MAIN_H_

#include <wx/wx.h>

#include <Tools/Tools.h>


class dmsMuxStreamSection;
class dmsSuperGroup;
class dmsObjectCarousel;
class cGeniusCarousel;

class dmsDataCarousel
{
public:
    dmsMuxStreamSection *m_poMux;
    dmsSuperGroup       *m_poSuperGroup;
    dmsObjectCarousel   *m_poOC;
    cGeniusCarousel     *m_poGC;

    int      m_iPID;
    wxString m_oOutputDir;

public:
    dmsDataCarousel();
    virtual ~dmsDataCarousel();

    bool Load(wxXmlNode *node);

    bool Compile();
    void Generate();
};

#endif /* _LIB_DATA_CAROUSEL_MAIN_H_ */
