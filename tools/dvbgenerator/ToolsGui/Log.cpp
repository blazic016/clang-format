/* ************************************************************************
   DMS HOSPITALITY

   Description :

   Historique :
   - COF v0 2004/05 - Creation
   ************************************************************************ */


#include "Log.h"

enum
{
    LOG_LIST_CTRL_DELETE_ALL = ACMD_EVENT_USER,
};


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsLogListCtrl::dmsLogListCtrl(wxWindow *parent) : dmsAcmdListCtrl(parent)
{
    InsertColumn( 0, "Time",     wxLIST_FORMAT_CENTER, 70);
    InsertColumn( 1, "Lv",       wxLIST_FORMAT_CENTER, 25);
    InsertColumn( 2, "Message",  wxLIST_FORMAT_LEFT, 600);

    SetBackgroundColour(wxColour(255,255,240));
    m_poCompareCallback = CallbackSort;

    m_bInsertAtEnd = true;
}


dmsLogListCtrl::~dmsLogListCtrl()
{
    ClearAll();
}


/* ========================================================================

   ======================================================================== */


void dmsLogListCtrl::Update(int item)
{
    dmsLog *elt = (dmsLog*) GetItemData(item);

    int i=0;
    SetItem(item, i++, elt->m_oDateTime.FormatTime());
    SetItem(item, i++, STR("%c", dmsLogLevelLetter[elt->m_eLevel]));
    SetItem(item, i++, elt->m_oMessage);


    wxFont font(8, wxNORMAL, wxNORMAL, wxNORMAL, FALSE, "courier new");
    wxListItem eltItem;
    eltItem.SetId(item);
    eltItem.SetFont(font);

    if (elt->m_eLevel == DMS_LOG_LEVEL_ERR)
    {
        eltItem.SetTextColour(*wxRED);
    }
    SetItem(eltItem);

    EnsureVisible(item);
}


int wxCALLBACK dmsLogListCtrl::CallbackSort(long item1, long item2, long column)
{
    return 0;
}



void dmsLogListCtrl::SetMenu(void *elt)
{
    m_poMenu->Append(LOG_LIST_CTRL_DELETE_ALL, "Clear");
}



bool dmsLogListCtrl::DoCommand(void *voidElt, int cmd)
{
    dmsLog *elt = (dmsLog*) voidElt;

    switch(cmd)
    {
    case ACMD_EVENT_NEW:
        break;

    case ACMD_EVENT_DELETE:
        DELNUL(elt);
        break;

    case LOG_LIST_CTRL_DELETE_ALL:
        ClearAll();
        break;
    }

    return true;
}


int dmsLogListCtrl::AddLogs()
{
    if (g_poLogManager == NULL) return 0;
    if (! g_poLogManager->GetCount()) return 0;

    int result = 0;

    wxCriticalSectionLocker lock(g_poLogManager->m_oKeepLogsCS);
    wxString line;

    dmsLog *log;

    while (1)
    {
        log = g_poLogManager->Shift();
        if (log == NULL) break;

        OnChanged(log, ACMD_EVENT_NEW);

        result++;
    }

    return result;
}
