/****************************************************************************
** @file MainApp.cpp
**
** @brief
**   Entry point of windows application Genius.
**
** @ingroup GENIUS USER INTERFACE
**
** @see
**
** @version $Rev: 62361 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/soft/ISSU_Generator/MainApp.cpp $
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
**   LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
**                                        Header in sections file
**   LIPPA - SmarDTV - v 2.30 - 03/2013 - Use DVB generic OUI 0x15A for all
**                                        OUIs in signaling table (PMT, NIT
**                                        and BAT).
**   LIPPA - SmarDTV - v 2.31 - 07/2013 - Manage Flash Location NASC 3.0
**                                        (modules NASC 3.0).
**
******************************************************************************/

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/splash.h>
#include <wx/image.h>
#include <wx/snglinst.h>

#include <Tools/Tools.h>
#include <ToolsGui/Window.h>

#include "MainApp.h"


/* ########################################################################
   Declarations
   ######################################################################## */

IMPLEMENT_APP(dmsMainApp)

dmsMainApp   *g_poMainApp   = NULL;


/* ########################################################################
   Application
   ######################################################################## */

/* ========================================================================
   Initialisation
   ======================================================================== */

bool dmsMainApp::OnInit()
{
    wxString instance;

    wxImage::AddHandler(new wxJPEGHandler());
    wxImage::AddHandler(new wxPNGHandler());

    g_poLogManager = new dmsLogManager();

    g_poMainApp = this;

    SetVendorName("SmarDTV");
    SetAppName("Genius");

    g_poMainConf  = new dmsConf;
    g_poMainConf->Load();

    g_poMainFrame = new dmsMainFrame("Genius");

#ifndef _DEBUG
    int sleep = 1000;

    {
        wxImage image("Resources/iWedia.png", wxBITMAP_TYPE_PNG);
        wxSplashScreen* splash = new wxSplashScreen(image,
            wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
            sleep, NULL, -1, wxDefaultPosition, wxDefaultSize,
            wxNO_BORDER|wxSTAY_ON_TOP);
        wxYield();
    }

    wxThread::Sleep(sleep);
#endif

    m_poInstanceChecker = new wxSingleInstanceChecker();
//        STR("%s/%s", GetVendorName().c_str(), GetAppName().c_str()));
    if ( m_poInstanceChecker->IsAnotherRunning() )
    {
        wxThread::Sleep(1000);
        wxLogError(_("Another ISSU Generator is running"));

        DELNUL(g_poMainConf);

        return FALSE;
    }

    g_poMainFrame->SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);



    if (g_poMainFrame->m_poManager->Find("ROOT"))
    {
        dmsvEnum* map = (dmsvEnum*) g_poMainFrame->m_poManager->m_poValidatorManager->Find("Profile");

        g_poMainConf->GetGroupList(map->m_oValueNames);

        map->AssumeValid(g_poMainConf->m_iProfile);

        ((dmsDialogContext*)(g_poMainFrame->m_poManager->Find("Profile")))->InitComboBox(map->m_oValueNames, g_poMainConf->m_iProfile);

        map->SetIndex(g_poMainConf->m_iProfile);

        g_poMainConf->Load(map->Get(), *g_poMainFrame->m_poManager->m_poValidatorManager);
        g_poMainFrame->m_poManager->m_poValidatorManager->InitReset();

        g_poMainFrame->m_poManager->Get();
    }

    g_poMainFrame->Show(true);

    return TRUE;
}



int dmsMainApp::OnExit()
{
    if (g_poMainConf) g_poMainConf->Save();

    DELNUL(g_poMainConf);
    DELNUL(m_poInstanceChecker);

    delete wxConfigBase::Set(NULL);

    DELNUL(g_poLogManager);

    return 0;
}


/* MainApp.cpp */
