/****************************************************************************
** @file MainApp.h
**
** @brief
**   Entry point of windows application Genius.
**
** @ingroup GENIUS USER INTERFACE
**
** @see
**
** @version $Rev: 62361 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/soft/ISSU_Generator/MainApp.h $
**          $Date: 2013-03-11 16:06:08 +0100 (lun., 11 mars 2013) $
**
** @author  SmarDTV Rennes - LIPPA
**
** COPYRIGHT:
**   2011 SmarDTV
**
** @history
**   COF   - Iwedia  - v 0    - 05/2003 - Creation
**   LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
**   LIPPA - SmarDTV - v 2.00 - 10/2011 - New tab 'Additional Streams' to
**                                        inject streams audio and video in
**                                        stream generated.
**   LIPPA - SmarDTV - v 2.10 - 12/2011 - Add DSI Subdescriptor Update ID
**                                        and Usage ID
**   LIPPA - SmarDTV - v 2.11 - 02/2012 - Fix bug Mux sections modules
**                                        multiplexed with REPEAT shorty
**                                        modules.
**   LIPPA - SmarDTV - v 2.12 - 02/2012 - DSI repetition all seconds.
**   LIPPA - SmarDTV - v 2.13 - 03/2012 - Generate raw sections in a file and
**                                        choice generated files extensions.
**   LIPPA - SmarDTV - v 2.20 - 03/2012 - Merge Telefonica functionality:
**                                        - Manage 16 images max
**                                        - Manage Zone ID with config file
**                                          and DA2
**                                        - Manage max 16 additionals software
**                                          compatibility descriptors.
**                                        Corrupt Chunk CRC option for QA Test
**                                        (enable only in version DEBUG).
**   LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
**                                        Header in sections file
**   LIPPA - SmarDTV - v 2.30 - 03/2013 - Use DVB generic OUI 0x15A for all
**                                        OUIs in signaling table (PMT, NIT
**                                        and BAT).
**   LIPPA - SmarDTV - v 2.31 - 07/2013 - Manage Flash Location NASC 3.0
**                                        (modules NASC 3.0).
**   LIPPA - SmarDTV - v 2.32 - 09/2013 - Support Module Size 512 K and 1 M.
**   LIPPA - SmarDTV - v 2.33 - 01/2014 - Hardware and Product Versions became
**                                        a mask on target versions to support
**                                        several versions in same carousel.
**                                        Suppress the NASC 3 module ID 4
**                                        'application kernel'
**
******************************************************************************/

#ifndef _MAIN_APP_H_
#define _MAIN_APP_H_

#include <wx/wx.h>

#include "MainConf.h"
#include "MainFrame.h"

#define SOFT_VERSION "3.0.0"

class wxSingleInstanceChecker;
class dmsMainApp;

/**
    \brief Application principale. Lance l'IHM. Contient le numéro de version à mettre à jour !
*/

class dmsMainApp: public wxApp
{
public:
    // Lancement de l'application

    wxSingleInstanceChecker* m_poInstanceChecker;

public:
    bool OnInit();
    int  OnExit();
};


DECLARE_APP(dmsMainApp);


extern dmsMainApp   *g_poMainApp;



#endif /* _MAIN_APP_H_ */

/* MainApp.h */
