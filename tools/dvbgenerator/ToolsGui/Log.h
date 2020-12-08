/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/05 - Creation
   ************************************************************************ */


#ifndef _TOOLS_GUI_LOG_H_
#define _TOOLS_GUI_LOG_H_

#include <wx/wx.h>

#include "Window.h"

/* ------------------------------------------------------------------------
   dmsLogListCtrl
   ------------------------------------------------------------------------ */

/**
    \class dmsLogListCtrl
    \brief ListControl d'affichage des logs
 */

class dmsLogListCtrl : public dmsAcmdListCtrl
{
public:
    dmsLogListCtrl(wxWindow *parent);
    virtual ~dmsLogListCtrl();

    void Update(int item);
    bool DoCommand(void *voidElt, int cmd);
    static int wxCALLBACK CallbackSort(long item1, long item2, long column);

    virtual void SetMenu(void *elt);
    int AddLogs();
};



#endif /* _TOOLS_GUI_LOG_H_ */
