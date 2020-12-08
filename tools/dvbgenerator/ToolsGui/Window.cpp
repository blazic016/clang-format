/* ************************************************************************
   SmarDTV

   Description : Extensions au systeme de fenetrage de wxWindows

   Historique :
   - COF   - Iwedia  - v 0    - 05/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/mstream.h>
#include <wx/image.h>

#include <Tools/Tools.h>
#include <Resources/iWedia.xpm>

#include "Window.h"

/* ========================================================================
   Sauvegarde de la geometry (coordonnees) de la fenetre.

   Le "path" est utilise pour distinguer les differentes frames
   d'une application.


   Ne pas oublier de mettre le "VendorName" et le "AppName" au debut
   du Application::OnInit();

   SetVendorName(DMS_VENDOR_NAME);
   SetAppName("DmsExample");
   ======================================================================== */



void dmsLoadGeometry(wxWindow *win, const wxString &path, bool bOnlyPos)
{
    int x, y, w, h;
    int x1, y1, w1, h1;

    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    win->GetClientSize(&w1, &h1);
    win->GetPosition(&x1, &y1);

    config->SetPath(STR("%s/Geometry", path.GetData()));
    x = config->Read("x", x1);
    y = config->Read("y", y1);
    if (! bOnlyPos)
    {
        w = config->Read("w", w1);
        h = config->Read("h", h1);
    }

    if (x+w < 0) x=0;
    if (y+h < 0) y=0;

    win->Move(x,y);
    win->SetSize(x,y,-1,-1);

    // On vérifie si la position a bien été prise en compte
    // (le pb arrive dans le cas d'appli MDI avec Toolbar)
    win->GetPosition(&x1, &y1);
    bool badPlace = false;
    if (x1 != x) {badPlace = true; x = 2*x - x1;}
    if (y1 != y) {badPlace = true; y = 2*y - y1;}
    if (badPlace)
    {
        win->Move(x,y);
        win->GetPosition(&x, &y);
    }


    if (! bOnlyPos) win->SetClientSize(w, h);
}








void dmsSaveGeometry(wxWindow *win, const wxString &path)
{
    int x, y, w, h;

    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    win->GetClientSize(&w, &h);
    win->GetPosition(&x, &y);

    config->SetPath(STR("%s/Geometry",path.GetData()));
    config->Write("x", (long) x);
    config->Write("y", (long) y);
    config->Write("w", (long) w);
    config->Write("h", (long) h);

    config->Flush();
}




void dmsLoadGeometry(wxFrame *frame, const wxString &path, bool bOnlyPos)
{
    bool maximize;

    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    dmsLoadGeometry((wxWindow*) frame, path, bOnlyPos);

    config->Read("Maximize", &maximize, frame->IsMaximized());

    if (maximize != frame->IsMaximized())
        frame->Maximize(maximize);
}




void dmsSaveGeometry(wxFrame *frame, const wxString &path)
{
    wxConfigBase *config = wxConfigBase::Get();
    if ( config == NULL ) return;

    dmsSaveGeometry((wxWindow*) frame, path);

    config->Write("Maximize", frame->IsMaximized());

    config->Flush();
}




/* ========================================================================

   ======================================================================== */



void dmsConfigWriteInt(const wxString &path, int value)
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    config->Write(path, (long) value);
}




void dmsConfigReadInt(const wxString &path, int &value, int defaultVal)
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    config->Read(path, &value, defaultVal);

}



void dmsConfigWriteString(const wxString &path, const wxString &value)
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    config->Write(path, value);
}




void dmsConfigReadString(const wxString &path, wxString &value, const wxString &defaultVal)
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    config->Read(path, &value, defaultVal);
}




void dmsConfigWriteBool(const wxString &path, bool value)
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    config->Write(path, value);
}




void dmsConfigReadBool(const wxString &path, bool &value, bool defaultVal)
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    config->Read(path, &value, defaultVal);

}


/* ========================================================================

   ======================================================================== */


void dmsMoveVisible(wxWindow *win, const wxPoint &point)
{
    wxRect screen = wxGetClientDisplayRect();
    wxRect rect   = win->GetRect();
    wxRect old = rect;

    rect.x = point.x;
    rect.y = point.y;

    if (rect.width > screen.width)
        rect.x = 0;
    else if (rect.x+rect.width > screen.width)
        rect.x = screen.width - rect.width;

    if (rect.height > screen.height)
        rect.y = 0;
    else if (rect.y+rect.height > screen.height)
        rect.y = screen.height - rect.height;

    if (rect.x < 0) rect.x = 0;
    if (rect.y < 0) rect.y = 0;

    if (old.x != rect.x || old.y != rect.y)
        win->Move(rect.x, rect.y);
}


void dmsPostOnSize(wxWindow* win)
{
    wxSizeEvent SizeEvent(win->GetSize());
    win->GetEventHandler()->AddPendingEvent(SizeEvent);
}




void AddIcoMenuItem(wxMenu* menu, int id, const wxString &name, const wxString &icoName)
{
    wxMenuItem *item = new wxMenuItem(menu, id, name);

    item->SetBitmap(wxIcon(icoName, wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));

    menu->Append(item);
}




/* ########################################################################

   ######################################################################## */


BEGIN_EVENT_TABLE(dmsMDIParentFrame, wxMDIParentFrame)
    EVT_SIZE(dmsMDIParentFrame::OnSize)
END_EVENT_TABLE();


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsMDIParentFrame::dmsMDIParentFrame(const wxString& title)
: wxMDIParentFrame(NULL, -1, title, wxDefaultPosition, wxSize(800, 600))
{
    m_poMenuBar = new wxMenuBar;
    m_eTileDir  = TILE_NONE;
    m_poSizer   = NULL;
}


dmsMDIParentFrame::~dmsMDIParentFrame()
{
}



/* ========================================================================

   ======================================================================== */


void dmsMDIParentFrame::OnSize(wxSizeEvent& event)
{
    wxMDIParentFrame::OnSize(event);

    if (m_eTileDir == TILE_NONE) return;

    int w, h;

    GetClientWindow()->GetClientSize(&w, &h);
    //w -= GetClientAreaOrigin().x;
    //h -= GetClientAreaOrigin().y;

    SetSizer();

    if (m_poSizer)
    {
        m_poSizer->SetDimension(GetClientAreaOrigin().x,GetClientAreaOrigin().y,w,h);
        m_poSizer->Layout();
        DELNUL(m_poSizer);
    }
}


void dmsMDIParentFrame::SetSizer()
{
    dmsMDIChildFrame *child;

    wxBoxSizer* sizer = new wxBoxSizer(m_eTileDir==TILE_HORIZONTALY?wxVERTICAL:wxHORIZONTAL);

    FOREACH(dmsMDIChildFrameList, m_oChildList, child)
    {
        sizer->Add(child,1,wxEXPAND);
    }

    m_poSizer = sizer;
}

/* ========================================================================
   Gestion des fenêtres filles
   ======================================================================== */


void dmsMDIParentFrame::Load()
{
    intList list;

    LoadIdList(list);
    PostOpenChild(list);
}

void dmsMDIParentFrame::OpenChild(int id)
{
    if (ChildActive(id)) return;

    PostOpenChild(id);
}

void dmsMDIParentFrame::GetIdList(intList &list)
{
    list.Clear();

    dmsMDIChildFrame* child;

    FOREACH(dmsMDIChildFrameList, m_oChildList, child)
    {
        list.Add(child->m_iId);
    }
}


void dmsMDIParentFrame::SaveOpenChild(dmsMDIChildFrame *child)
{
    wxMenuBar *mbar = GetMenuBar();

    if (mbar == NULL) return;

    mbar->Enable(child->m_iId, false);

    m_oChildList.Append(child);

    SaveIdList();
}


void dmsMDIParentFrame::SaveCloseChild(dmsMDIChildFrame *child)
{
    wxMenuBar *mbar = GetMenuBar();

    if (mbar == NULL) return;

    mbar->Enable(child->m_iId, true);

    m_oChildList.DeleteObject(child);

    SaveIdList();
}



void dmsMDIParentFrame::PostOpenChild(int id)
{
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, id);

    wxPostEvent(this, event);
}


void dmsMDIParentFrame::PostOpenChild(const intList &list)
{
    int *id;

    FOREACH(intList, list, id)
    {
        PostOpenChild(*id);
    }
}


dmsMDIChildFrame *dmsMDIParentFrame::FindChild(int id)
{
    dmsMDIChildFrame* child;

    FOREACH(dmsMDIChildFrameList, m_oChildList, child)
    {
        if (child->m_iId == id) return child;
    }
    return NULL;
}


bool dmsMDIParentFrame::ChildActive(int id)
{
    return FindChild(id)!=NULL;
}



void dmsMDIParentFrame::SaveIdList()
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    intList list;
    wxString value;

    GetIdList(list);
    list.Join(' ', value);

    config->Write("/Window/MdiChildList", value);
}


void dmsMDIParentFrame::LoadIdList(intList &list)
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    wxString value;

    config->Read("/Window/MdiChildList", &value, "");

    list.Split(value);
}



/* ########################################################################
   Frame
   ######################################################################## */


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsMDIChildFrameList);

BEGIN_EVENT_TABLE(dmsMDIChildFrame, wxMDIChildFrame)
    EVT_CLOSE(dmsMDIChildFrame::OnCloseWindow)
END_EVENT_TABLE();



/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsMDIChildFrame::dmsMDIChildFrame(dmsMDIParentFrame *parent, const wxString &title, int id)
: wxMDIChildFrame(parent, -1, title)
{
    SetIcon(wxICON(iWedia));

    m_oTitle = title;

    m_poParent = parent;
    m_iId = id;

    m_poParent->SaveOpenChild(this);

    dmsLoadGeometry(this, "/Window/" + m_oTitle);

    wxSizeEvent event;
    wxPostEvent(m_poParent, event);
}



dmsMDIChildFrame::~dmsMDIChildFrame()
{
    dmsSaveGeometry(this, "/Window/" + m_oTitle);
}



/* ========================================================================
   Evenement
   ======================================================================== */

void dmsMDIChildFrame::OnCloseWindow(wxCloseEvent& event)
{
    m_poParent->SaveCloseChild(this);

    dmsPostOnSize(m_poParent);

    event.Skip();
}



/* ########################################################################
   ListControl avec menu "add, copy, modify, delete"
   ######################################################################## */

/* ========================================================================
   Tables d'évenements
   ======================================================================== */


BEGIN_EVENT_TABLE(dmsAcmdListCtrl, wxListCtrl)
    EVT_LIST_COL_CLICK(-1, dmsAcmdListCtrl::OnColClick)
    EVT_RIGHT_DOWN(dmsAcmdListCtrl::OnRightDown)
    EVT_LIST_ITEM_ACTIVATED(-1, dmsAcmdListCtrl::OnActivated)
    EVT_MENU(-1, dmsAcmdListCtrl::OnMenu)
    EVT_CHAR(dmsAcmdListCtrl::OnChar)
END_EVENT_TABLE();

/* ========================================================================
   Constructeurs / Desctructeur
   ======================================================================== */


dmsAcmdListCtrl::dmsAcmdListCtrl(wxWindow *parent) : wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT)
{
    m_poCompareCallback = NULL;
    m_poMenu            = NULL;
    m_bInsertAtEnd      = true;
    m_iMaxSize          = 1000;
}


dmsAcmdListCtrl::~dmsAcmdListCtrl()
{
    DELNUL(m_poMenu);
}


/* ========================================================================
   Evenements
   ======================================================================== */



void dmsAcmdListCtrl::OnColClick(wxListEvent& event)
{
    if (m_poCompareCallback)
        SortItems(m_poCompareCallback, event.GetColumn());
}


void dmsAcmdListCtrl::SetSelectedDataList()
{
    long item = -1;

    m_oSelectedDataList.Clear();
    while (1)
    {
        item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1) break;
        m_oSelectedDataList.Append((wxObject*)GetItemData(item));
    }
}


void dmsAcmdListCtrl::OnRightDown(wxMouseEvent &event)
{
    int flags;
    int item;

    item = HitTest(wxPoint(event.GetX(), event.GetY()), flags);

    if (item==-1 || GetItemState(item, wxLIST_STATE_SELECTED)==0)
    {
        // Si l'item courant n'est pas selectionné
        // On déselectionne tout
        for (int i = GetItemCount()-1; i >=0; i--)
        {
            if (i != item)
                SetItemState(i, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        }
    }

    if (item >= 0)
    {
        // On sélectionne l'item courant
        SetItemState(item,
            wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
            wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    }

    DELNUL(m_poMenu);

    m_poMenu = new wxMenu;

    SetSelectedDataList();

    if (m_oSelectedDataList.GetCount() == 1)
        SetMenu(m_oSelectedDataList[0]);
    else
        SetMenu(NULL);

    PopupMenu(m_poMenu, event.GetX(),event.GetY());
}


void dmsAcmdListCtrl::SetMenu(void *elt)
{
    m_poMenu->Append(ACMD_EVENT_NEW,    "New");
    if (elt)
    {
        m_poMenu->Append(ACMD_EVENT_COPY,   "Copy");
        m_poMenu->Append(ACMD_EVENT_MODIFY, "Modify");
        m_poMenu->Append(ACMD_EVENT_DELETE, "Delete");
    }
}


void dmsAcmdListCtrl::OnActivated(wxListEvent& event)
{
    wxPostEvent(this, wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ACMD_EVENT_MODIFY));
}



void dmsAcmdListCtrl::ApplyEvent(int id)
{
    SetSelectedDataList();

    if (m_oSelectedDataList.GetCount())
    {
        void *elt;

        FOREACH(wxList, m_oSelectedDataList, elt)
        {
            if (! DoCommand(elt, id)) return;
        }
    }
    else
    {
        DoCommand(NULL, id);
    }
}


void dmsAcmdListCtrl::OnMenu(wxCommandEvent& event)
{
    ApplyEvent(event.GetId());
}



void dmsAcmdListCtrl::OnChar(wxKeyEvent& event)
{
    long keycode = event.GetKeyCode();

    switch (keycode)
    {
    case WXK_DELETE: ApplyEvent(ACMD_EVENT_DELETE); break;
    }

    event.Skip();
}




/* ========================================================================

   ======================================================================== */


void dmsAcmdListCtrl::OnChanged(void *elt, EnumAcmdEvent event)
{
    long item;

    switch (event)
    {
        case ACMD_EVENT_NEW:
            {
                if (GetItemCount() > m_iMaxSize)
                {
                    Freeze();
                    while (GetItemCount() > m_iMaxSize/2)
                    {
                        if (m_bInsertAtEnd)
                            ClearItem(0);
                        else
                            ClearItem(GetItemCount()-1);
                    }
                    Thaw();
                }
                item = InsertItem(m_bInsertAtEnd ? GetItemCount() : 0, "");
                SetItemData(item, (long) elt);
                Update(item);
            }
            break;
        case ACMD_EVENT_MODIFY:
            item = FindItem(-1, (long) elt);
            if (item != -1) Update(item);
            break;
        case ACMD_EVENT_DELETE:
            item = FindItem(-1, (long) elt);
            if (item != -1) DeleteItem(item);
            break;
        default:
            break;
    }
}



void *dmsAcmdListCtrl::poGetFocused()
{
    int item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);

    if (item == -1) return NULL;

    return (void*) GetItemData(item);
}



void *dmsAcmdListCtrl::poGetFirstSelected()
{
    int item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (item == -1) return NULL;

    return (void*) GetItemData(item);
}



void dmsAcmdListCtrl::CallbackEvent(void *elt, EnumAcmdEvent event)
{
}



void dmsAcmdListCtrl::UpdateAll()
{
    long item = -1;
    while (1)
    {
        item = GetNextItem(item);
        if (item == -1) break;
        Update(item);
    }
}


void dmsAcmdListCtrl::ClearAll()
{
    Freeze();
    while (GetItemCount()) ClearItem(0);
    Thaw();
}


void dmsAcmdListCtrl::ClearItem(int item)
{
    DoCommand((wxObject*)GetItemData(item), ACMD_EVENT_DELETE);
    DeleteItem(item);
}


/* ########################################################################

   ######################################################################## */


dmsTextDialog::dmsTextDialog(const wxString& title, const wxString &message, int icon) : dmsDialog(NULL, title)
{
    wxString text =
        "<?xml version='1.0' encoding='ISO-8859-1'?>"
        "<RootPanel border='sunken' expand='true' str='1'>"
        "<Margin space='10'>"
        "<HBox expand='true' str='1'>"
        "<VBox>"
        "  <Spacer space='10'/>"
        "  <StaticBitmap label='Hello' resource='$ICON$'/>"
        "</VBox>"
        "<Spacer space='10'/>"
        "<VBox expand='true' str='1'>"
        "  <Spacer space='10'/>"
        "  <TextCtrl name='Text' expand='true' str='1' read-only='true' multiline='true'"
        "            hscroll='true' vscroll='true' width='400' height='200' fontname='courier new'"
        "            fontsize='8'/>"
        "  <Spacer space='10'/>"
        "  <HBox expand='true'>"
        "    <Spring/>"
        "    <Button name='Quit' label='Ok'/>"
        "    <Spring/>"
        "  </HBox>"
        "</VBox>"
        "</HBox>"
        "</Margin>"
        "</RootPanel>"
        ;


    switch (icon)
    {
    case wxICON_ERROR: text.Replace("$ICON$", "Resources/error.png"); break;
    default:           text.Replace("$ICON$", "Resources/info.png"); break;
    }

    if (! m_poManager->LoadText(text)) return;

    dmsDialogContext* ctx  = (dmsDialogContext*)(m_poManager->Find("Text"));
    wxTextCtrl*       ctrl = (wxTextCtrl*)(ctx->m_poWin);

    ctrl->SetValue(message);
    ctrl->ShowPosition(0);

    m_oTitle = title;

    dmsLoadGeometry(this, STR("/Window/%s", m_oTitle));

    Show(true);
}


dmsTextDialog::~dmsTextDialog()
{
    dmsSaveGeometry(this, STR("/Window/%s", m_oTitle));
}

void dmsTextDialog::OnEvent(dmsDialogContext* ctx)
{
    wxString name;

    if (ctx)
        name = ctx->m_oName;
    else
        name = "Quit";

    if (name=="Quit")
    {
        EndModal(wxID_OK);
        return;
    }

}
