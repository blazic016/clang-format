/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 09/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/config.h>
#include <wx/statline.h>

#include <Tools/Log.h>
#include <Tools/Validator.h>
#include <Tools/Xml.h>

#include "Window.h"
#include "DialogStack.h"


/* ========================================================================

   ======================================================================== */



/* ========================================================================

   ======================================================================== */





/* ########################################################################

   Un sizer qui peut changer la taille d'une fenetre lorsqu'il est
   redimensionné.

   ######################################################################## */




dmsWindowBoxSizer::dmsWindowBoxSizer(int orient, wxWindow *win) : wxBoxSizer(orient)
{
    m_Window        = win;
}




void dmsWindowBoxSizer::RecalcSizes()
{
    if (m_Window)
        m_Window->SetSize(m_position.x, m_position.y, m_size.x, m_size.y);

    wxBoxSizer::RecalcSizes();
}


int dmsWindowBoxSizer::iSetWindow(wxWindow *win)
{
    m_Window = win;

    return 0;
}



/* ########################################################################

   ######################################################################## */





/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsWindowStack::dmsWindowStack()
{
    m_eMode       = MODE_UNKNOWN;

    m_bExpanded   = false;
    m_bResizable  = false;
    m_bHResizable = false;
    m_poUserLine  = NULL;

    pos  = wxDefaultPosition;
    size = wxDefaultSize;

    m_poDataWin      = NULL;
    m_poSubDataWin   = NULL;
    m_poButtonOk     = NULL;
    m_poButtonCancel = NULL;

    m_bCancel = true;
    m_bEnableClose = true;

    m_poDisplayRoot = new dmsWindowStackDisplay(NULL, NULL);
}




dmsWindowStack::~dmsWindowStack()
{
    wxConfigBase *config = wxConfigBase::Get();
    if (config == NULL) return;

    if (m_eMode == MODE_NOTEBOOK)
        config->Write(STR("/Window/%s/NoteBookPage", m_oName), GetPage());

    dmsSaveGeometry(m_poParent, STR("/Window/%s", m_oName));

    DELNUL(m_poDisplayRoot);
}



/* ========================================================================
   Init
   ======================================================================== */


void dmsWindowStack::Init(wxWindow *parent)
{
    m_poParent = parent;

    m_poGlobalWin = new wxPanel(parent, -1);
    m_poButtonWin = new wxPanel(m_poGlobalWin, -1);

    m_poParentSizer  = new dmsWindowBoxSizer(wxVERTICAL, m_poGlobalWin);
    m_poDataSizer    = new wxBoxSizer(wxVERTICAL);
    m_poGlobalSizer  = new wxBoxSizer(wxVERTICAL);
    m_poButtonSizer  = new wxBoxSizer(wxVERTICAL);
    m_poButtonHSizer = new wxBoxSizer(wxHORIZONTAL);

    m_poGlobalWin->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
}




/* ========================================================================
   Fixe les modes
   ======================================================================== */





void dmsWindowStack::SetMode(EnumMode mode)
{
    m_eMode = mode;

    switch (mode)
    {
    case MODE_DEFAULT:
        m_poDataWin = new wxPanel(m_poGlobalWin, -1);
        NewLine();
        break;
    case MODE_STATIC_BOX:
        m_poDataWin = new wxPanel(m_poGlobalWin, -1, pos, size, wxSIMPLE_BORDER);
        NewLine();
        break;
    case MODE_SUNKEN_BORDER:
        m_poDataWin = new wxPanel(m_poGlobalWin, -1, pos, size, wxSUNKEN_BORDER);
        NewLine();
        break;
    case MODE_RAISED_BORDER:
        m_poDataWin = new wxPanel(m_poGlobalWin, -1, pos, size, wxRAISED_BORDER);
        NewLine();
        break;
    case MODE_SCROLLBAR:
        m_poDataWin = new wxScrolledWindow(m_poGlobalWin, -1, pos, size, wxVSCROLL | wxSUNKEN_BORDER);
        m_iScrollBarHeight = 200;
        NewLine();
        break;
    case MODE_NOTEBOOK:
        m_poDataWin       = new wxNotebook(m_poGlobalWin, -1);
        m_bExpanded       = true;
        DELNUL(m_poDataSizer);
        break;
    default:
        return;
    }

    m_oWindowList.Append(m_poDataWin);

    m_poDataWin->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
}




void dmsWindowStack::SetStaticBoxMode()    {SetMode(MODE_STATIC_BOX);   }
void dmsWindowStack::SetSunkenBorderMode() {SetMode(MODE_SUNKEN_BORDER);}
void dmsWindowStack::SetRaisedBorderMode() {SetMode(MODE_RAISED_BORDER);}
void dmsWindowStack::SetScrollBarMode()    {SetMode(MODE_SCROLLBAR);    }
void dmsWindowStack::SetNoteBookMode()     {SetMode(MODE_NOTEBOOK);     }



void dmsWindowStack::SetPage(int page)
{
    if (m_eMode == MODE_NOTEBOOK)
    {
        ((wxNotebook*)m_poDataWin)->SetSelection(page);
    }
}

int dmsWindowStack::GetPage()
{
    if (m_eMode == MODE_NOTEBOOK)
    {
        return ((wxNotebook*)m_poDataWin)->GetSelection();
    }
    return 0;
}

/* ========================================================================
   Fit
   ======================================================================== */




void dmsWindowStack::FitSizer(wxWindow *win, wxSizer *sizer)
{
    if (win==NULL || sizer==NULL) return;

    win->SetAutoLayout(true);
    win->SetSizer(sizer);

    sizer->Fit(win);
    sizer->SetSizeHints(win);
}








void dmsWindowStack::FitWin()
{
    if (m_poDataWin==NULL) return;

    AddButtonOkCancel();

    if (m_eMode == MODE_NOTEBOOK)
    {
        EndOfPage();

        wxConfigBase *config = wxConfigBase::Get();
        if (config == NULL) return;
        int page;
        config->Read(STR("/%s/NoteBookPage", m_oName), &page, 0);
        if (page >= 0 && page < (int)((wxNotebook*) m_poDataWin)->GetPageCount())
            SetPage(page);
    }

    if (m_poDataSizer)
        FitSizer(m_poDataWin, m_poDataSizer);

    FitSizer(m_poButtonWin, m_poButtonSizer);

    if (m_eMode == MODE_SCROLLBAR)
    {
        wxSize size = m_poDataWin->GetSize();

        m_poDataWin->SetSize(size.GetWidth()+20, m_iScrollBarHeight);
        ((wxScrolledWindow*)m_poDataWin)->SetScrollbars(1,1,1,size.GetHeight());
    }

    m_poGlobalSizer->Add(m_poDataWin,   1, wxEXPAND | wxALL, 10);

    m_poGlobalSizer->Add(m_poButtonWin, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    FitSizer(m_poGlobalWin, m_poGlobalSizer);

    m_poParentSizer->Add(m_poGlobalWin, 1, wxEXPAND);

    FitSizer(m_poParent, m_poParentSizer);

    int w, h;

    w = m_poParent->GetSize().GetWidth();
    h = m_poParent->GetSize().GetHeight();

    m_poParent->SetSizeHints(
        w,
        h,
        m_bHResizable ? -1 : w,
        m_bResizable  ? -1 : h);

    dmsLoadGeometry(m_poParent, STR("/Window/%s", m_oName));

    m_poDisplayRoot->Update();
}




/* ========================================================================
   Ajouts d'éléments
   ======================================================================== */

void dmsWindowStack::AddButton(wxWindow *button)
{
    m_oButtonList.Append(button);
    m_oWindowList.Append(button);
}


void dmsWindowStack::AddButtonOkCancel()
{
    void *win;

    if (m_eMode == MODE_SUNKEN_BORDER || m_eMode == MODE_RAISED_BORDER)
    {
        NewLine();
    }

    if (m_eMode == MODE_DEFAULT)
    {
        NewLine();
        AddLine();
    }

    if (m_oButtonList.IsEmpty())
    {
        m_poButtonOk = new wxButton(m_poButtonWin, ID_BUTTON_OK, "&Ok");
        AddButton(m_poButtonOk);
        if (m_bCancel)
        {
            m_poButtonCancel = new wxButton(m_poButtonWin, ID_BUTTON_CANCEL, "&Cancel");
            AddButton(m_poButtonCancel);
        }
    }

    FOREACH(wxList, m_oButtonList, win)
    {
        m_poButtonHSizer->Add((wxWindow*) win, 0, wxRIGHT, _node->GetNext()?10:0);
    }

    m_poButtonSizer->Add(m_poButtonHSizer, 0, wxALIGN_RIGHT);

    ((wxWindow*)(m_oButtonList.GetFirst()->GetData()))->SetFocus();

    //m_poOkCtrl->SetDefault();
}



void dmsWindowStack::AddWin(wxWindow *win, bool expand, bool hexpand)
{
    m_oWindowList.Append(win);

    m_poDataSizer->Add(win, expand ? 1 : 0, (wxEXPAND * hexpand) | wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);
}

void dmsWindowStack::AddSizer(wxSizer *sizer, bool expand, bool hexpand)
{
    m_poDataSizer->Add(sizer, expand ? 1 : 0, (wxEXPAND * hexpand) | wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);
}




void dmsWindowStack::Add(wxWindow *win, bool expand, bool hexpand)
{
    if (m_poUserLine)
    {
        AddInLine(win, " ", expand, hexpand);
    }
    else
    {
        AddWin(win, expand, hexpand);

        if (expand) m_bResizable = true;
        if (hexpand) m_bHResizable = true;
    }
}







void dmsWindowStack::Add(const wxString &label, wxWindow *win, bool expand, bool hexpand)
{
    Add(label);
    Add(win, expand, hexpand);
    NewLine();
}



wxStaticText* dmsWindowStack::Add(const wxString &label)
{
    wxStaticText* win= new wxStaticText(Win(), -1, label);

    AddWin(win);

    return win;
}



wxStaticText* dmsWindowStack::Add(const wxString &label, const wxFont &font, const wxSize &size)
{
    wxStaticText *win = new wxStaticText(Win(), -1, label, wxDefaultPosition, size);

    win->SetFont(font);

    AddWin(win);

    return win;
}



void dmsWindowStack::Add2(wxWindow *win1, const wxString &label, wxWindow *win2)
{
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *txt;

    sizer->Add(win1, 0, wxALIGN_RIGHT | wxALL, 0);
    sizer->Add(txt = new wxStaticText(Win(), -1, label), 0, wxALIGN_RIGHT | wxALL, 0);
    sizer->Add(win2, 0, wxALIGN_RIGHT | wxLEFT, 0);

    m_oWindowList.Append(win1);
    m_oWindowList.Append(txt);
    m_oWindowList.Append(win2);

    AddSizer(sizer);
}

void dmsWindowStack::BeginLine()
{
    if (m_poUserLine)
    {
        LOGE(L"Already construct line");
        return;
    }

    m_bUserLineExpand = false;
    m_bUserLineHExpand = false;

    m_poUserLine = new wxBoxSizer(wxHORIZONTAL);
    m_iUserLineLength = 0;
}

void dmsWindowStack::AddInLine(wxWindow *win, const wxString label, bool expand, bool hexpand)
{
    if (m_poUserLine==NULL)
    {
        LOGE(L"No current line");
        return;
    }

    wxStaticText *txt;

    if (expand) { m_bUserLineExpand = true; m_bResizable = true; }
    if (hexpand) { m_bUserLineHExpand = true; m_bHResizable = true; }

    if (m_iUserLineLength == 0)
    {
        m_poUserLine->Add(win, hexpand, wxALIGN_RIGHT|wxALL|(wxEXPAND*expand), 0);
    }
    else
    {
        m_poUserLine->Add(txt = new wxStaticText(Win(), -1, label), 0, wxALIGN_RIGHT | wxALL, 0);
        m_poUserLine->Add(win, hexpand, wxALIGN_RIGHT|wxLEFT|(wxEXPAND*expand), 0);
    }

    m_iUserLineLength++;

    m_oWindowList.Append(win);
}

void dmsWindowStack::EndLine()
{
    if (m_poUserLine==NULL)
    {
        LOGE(L"No current line");
        return;
    }

    AddSizer(m_poUserLine, m_bUserLineExpand, m_bUserLineHExpand);

    m_poUserLine = NULL;
}



void dmsWindowStack::AddLine()
{
    AddWin(new wxStaticLine(Win(), -1), false, true);
}



void dmsWindowStack::NewLine(bool expand)
{
    AddWin(new wxStaticText(Win(), -1, ""), expand);
}


/* ========================================================================

   ======================================================================== */




wxWindow *dmsWindowStack::Win()
{
    if (m_poSubDataWin) return m_poSubDataWin;

    if (m_poDataWin == NULL) SetMode(MODE_DEFAULT);

    return m_poDataWin;
}



/* ========================================================================

   ======================================================================== */



void dmsWindowStack::SetScrollHeight(int height)
{
    m_iScrollBarHeight = height;
}



void dmsWindowStack::EnableClose(bool value)
{
    m_bEnableClose = value;

    if (m_bEnableClose)
    {
        if (m_poButtonOk) m_poButtonOk->Enable(true);
        if (m_poButtonCancel) m_poButtonCancel->Enable(true);
    }
    else
    {
        if (m_poButtonOk) m_poButtonOk->Enable(false);
        if (m_poButtonCancel) m_poButtonCancel->Enable(false);
    }
}



void dmsWindowStack::EndOfPage()
{
    if (m_poSubDataWin)
    {
        m_poDataSizer->RecalcSizes();
        m_poDataSizer->Fit(m_poSubDataWin);
        m_poSubDataWin->SetAutoLayout(true);
        m_poSubDataWin->SetSizer(m_poDataSizer);
        m_poDataSizer  = NULL;
        m_poSubDataWin = NULL;
    }
}


void dmsWindowStack::AddPageStd(const wxString &label)
{
    wxFont font = Win()->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    NewLine();
    Add(label, font, size);
    AddLine();
    NewLine();
}


wxPanel *dmsWindowStack::AddPage(const wxString &label, bool expanded)
{
    if (m_eMode != MODE_NOTEBOOK)
    {
        AddPageStd(label);
        return NULL;
    }

    EndOfPage();

    m_poSubDataWin = new wxPanel(m_poDataWin, -1);
    m_poSubDataWin->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    ((wxNotebook*) m_poDataWin)->AddPage((wxPanel*) m_poSubDataWin, label);
    m_poDataSizer = new wxBoxSizer(wxVERTICAL);
    m_bCurrentLabelResizable = expanded;
    NewLine();

    return (wxPanel*) m_poSubDataWin;
}



/* ========================================================================
   DEBUG
   ======================================================================== */


void dmsWindowStack::TracePositions()
{
    wxObject *obj=NULL;
    wxWindow *win;
    wxRect R;
    int i=0;
    wxSize size1 = m_poDataSizer->GetSize();
    wxSize size2 = m_poButtonSizer->GetSize();

    wxLogDebug("========> Positions <============");
    wxLogDebug("==> Data   Sizer (%3d %3d)", size1.GetWidth(), size1.GetHeight());
    wxLogDebug("==> Button Sizer (%3d %3d)", size2.GetWidth(), size2.GetHeight());
    FOREACH(wxList, m_oWindowList, obj)
    {
        win = ((wxWindow*)obj);
        R = win->GetRect();
        wxLogDebug("Position of %3d %p-%p [%-10s] (%3d,%3d,%3d,%3d) %s",
            i++,
            win,
            win->GetParent(),
            win->GetName(),
            R.width, R.height, R.x, R.y,
            win->GetLabel());
    }
}


/* ========================================================================
   L'affichage
   ======================================================================== */


void dmsWindowStack::AddDisplay(wxCheckBox *win, EnumWSDisplayMode mode)
{
    m_poDisplayRoot->Add(win, WS_DISPLAY_TYPE_CHECKBOX, mode);
}

void dmsWindowStack::AddDisplay(wxChoice *win, EnumWSDisplayMode mode)
{
    m_poDisplayRoot->Add(win, WS_DISPLAY_TYPE_CHOICE, mode);
}

void dmsWindowStack::AddDisplay(wxWindow *parent, int parentValue, wxWindow *child)
{
    m_poDisplayRoot->Add(parent, parentValue, child, WS_DISPLAY_TYPE_UNKNOWN);
}

void dmsWindowStack::AddDisplay(wxWindow *parent, int parentValue, wxCheckBox *child)
{
    m_poDisplayRoot->Add(parent, parentValue, child, WS_DISPLAY_TYPE_CHECKBOX);
}

void dmsWindowStack::UpdateDisplay(int windowId)
{
    wxWindow *win = GetWindow(windowId);

    if (win)
    {
        m_poDisplayRoot->Update(win);
    }
}


/* ========================================================================
   Macros
   ======================================================================== */


wxTextCtrl *dmsWindowStack::AddTextCtrl(const wxString &label, int size, const wxString &value, const wxValidator &validator)
{
    wxTextCtrl* ctrl = new wxTextCtrl(Win(), -1, value, wxDefaultPosition, wxSize(size==0?-1:abs(size), -1),
        0, validator);

    wxString cleanedLabel = label;
    cleanedLabel.Replace(":", "");
    ctrl->SetName(cleanedLabel);

    Add(label, ctrl, 0, (size<0));

    return ctrl;
}

void dmsWindowStack::AddMultiTextCtrl(const wxString &label, wxTextCtrl *&ctrl, int size, int hsize, const wxString &value, const wxValidator &validator)
{
    ctrl = new wxTextCtrl(Win(), -1, value, wxDefaultPosition, wxSize(abs(size), abs(hsize)), wxTE_MULTILINE,
        validator);

    Add(label, ctrl, (hsize<0), (size<0));
}


void dmsWindowStack::AddTextCtrl(const wxString &label, wxTextCtrl *&ctrl, int size, int value, const wxString &unit)
{
    ctrl = new wxTextCtrl(Win(), -1, STR("%d", value), wxDefaultPosition, wxSize(size, -1),
        wxTE_RIGHT,
        dmsRegExValidator("^[[:digit:]]{1,7}$"));

    Add(label, ctrl);
}

void dmsWindowStack::AddStaticText(const wxString &label, const wxString &value)
{
    Add(label + "  " + value);
    NewLine();
}


void dmsWindowStack::AddChoice(const wxString &label, wxChoice *&ctrl, int size, int index, const char **values)
{
    ctrl = new wxChoice(Win(), -1, wxDefaultPosition, wxSize(size, -1));

    for (const char **value = values; *value; value++)
    {
        ctrl->Append(*value);
    }

    ctrl->SetSelection(index);

    Add(label, ctrl);
}

void dmsWindowStack::AddChoice(const wxString &label, wxChoice *&ctrl, int size, const wxString &value, const wxStringList &values)
{
    ctrl = new wxChoice(Win(), -1, wxDefaultPosition, wxSize(size, -1));
    int index = 0, i;
    wxChar *str;

    i=0;
    FOREACH(wxStringList, values, str)
    {
        ctrl->Append(str);
        if (value.CmpNoCase(str) == 0)
        {
            index = i;
        }
        i++;
    }

    ctrl->SetSelection(index);

    Add(label, ctrl);
}


void dmsWindowStack::AddComboBox(const wxString &label, wxComboBox *&ctrl, int size, const wxString &value, const wxStringList &values)
{
    ctrl = new wxComboBox(Win(), -1, "", wxDefaultPosition, wxSize(size, -1));
    int index = 0, i;
    wxChar *str;

    i=0;
    FOREACH(wxStringList, values, str)
    {
        ctrl->Append(str);
        if (value.CmpNoCase(str) == 0)
        {
            index = i;
        }
        i++;
    }

    ctrl->SetSelection(index);

    Add(label, ctrl);
}


void dmsWindowStack::AddCheckBox(const wxString &label, wxCheckBox *&ctrl, bool value)
{
    ctrl = new wxCheckBox(Win(), -1, label);

    ctrl->SetValue(value);

    Add(ctrl);
}



/* ########################################################################

   ######################################################################## */




BEGIN_EVENT_TABLE(dmsDialogStack, wxDialog)
    EVT_CLOSE(dmsDialogStack::OnCloseWindow)
    EVT_BUTTON(ID_BUTTON_OK, dmsDialogStack::OnOk)
    EVT_BUTTON(ID_BUTTON_CANCEL, dmsDialogStack::OnCancel)
    EVT_CHOICE(-1, dmsDialogStack::OnChoice)
    EVT_RADIOBUTTON(-1, dmsDialogStack::OnRadioButton)
    EVT_CHECKBOX(-1, dmsDialogStack::OnCheckBox)
    EVT_LISTBOX(-1, dmsDialogStack::OnListBoxSelected)
    EVT_LISTBOX_DCLICK(-1, dmsDialogStack::OnListBoxValidated)
    EVT_BUTTON(-1, dmsDialogStack::OnExtraButton)
END_EVENT_TABLE()





dmsDialogStack::dmsDialogStack(wxWindow *parent, const wxString &title,
                             wxWindowID id,
                             long style,
                             const wxString& name)
                             :
wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, style, name),
dmsWindowStack()
{
    dmsWindowStack::Init(this);

    m_oName = title;
    m_oName.Replace(" ", "", true);
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
}





dmsDialogStack::~dmsDialogStack()
{
}




/* ========================================================================
   Evenements
   ======================================================================== */


void dmsDialogStack::PostOk()
{
    if (Ok())
    {
        EndModal(wxID_OK);
    }
}



void dmsDialogStack::OnCloseWindow(wxCloseEvent& event)
{
    if (m_bEnableClose) EndModal(wxID_CANCEL);
}



void dmsDialogStack::OnOk(wxCommandEvent& event)
{
    if (Ok())
    {
        EndModal(wxID_OK);
    }
}


void dmsDialogStack::OnCancel(wxCommandEvent& event)
{
    if (Cancel()) EndModal(wxID_CANCEL);
}



void dmsDialogStack::OnChoice(wxCommandEvent& event)
{
    UpdateDisplay(event.GetId());
    Choice(event.GetId());
}

void dmsDialogStack::OnRadioButton(wxCommandEvent& event)
{
    RadioButton(event.GetId());
}

void dmsDialogStack::OnCheckBox(wxCommandEvent& event)
{
    UpdateDisplay(event.GetId());
    CheckBox(event.GetId());
}


void dmsDialogStack::OnListBoxSelected(wxCommandEvent& event)
{
    ListBoxSelected(event.GetId());
}

void dmsDialogStack::OnListBoxValidated(wxCommandEvent& event)
{
    if (ListBoxValidated(event.GetId())) OnOk(event);
}


void dmsDialogStack::OnExtraButton(wxCommandEvent& event)
{
    ExtraButton(event.GetId());
}


wxWindow *dmsDialogStack::GetWindow(int id)
{
    return FindWindow(id);
}

/* ########################################################################

   ######################################################################## */




BEGIN_EVENT_TABLE(dmsFrameStack, wxFrame)
    EVT_BUTTON(ID_BUTTON_OK, dmsFrameStack::OnOk)
    EVT_BUTTON(ID_BUTTON_CANCEL, dmsFrameStack::OnCancel)
    EVT_CHOICE(-1, dmsFrameStack::OnChoice)
    EVT_RADIOBUTTON(-1, dmsFrameStack::OnRadioButton)
    EVT_CHECKBOX(-1, dmsFrameStack::OnCheckBox)
    EVT_LISTBOX(-1, dmsFrameStack::OnListBoxSelected)
    EVT_LISTBOX_DCLICK(-1, dmsFrameStack::OnListBoxValidated)
END_EVENT_TABLE()





dmsFrameStack::dmsFrameStack(wxWindow *parent, const wxString &title, long style)
        : wxFrame(parent, -1, title, wxDefaultPosition, wxDefaultSize, style),
          dmsWindowStack()
{
    dmsWindowStack::Init(this);

    m_oName = title;
    m_oName.Replace(" ", "", true);
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
}





dmsFrameStack::~dmsFrameStack()
{
}




/* ========================================================================
   Evenements
   ======================================================================== */





void dmsFrameStack::OnOk(wxCommandEvent& event)
{
    Ok();
}



void dmsFrameStack::OnCancel(wxCommandEvent& event)
{
    Cancel();
}



void dmsFrameStack::OnChoice(wxCommandEvent& event)
{
    Choice(event.GetId());
}

void dmsFrameStack::OnRadioButton(wxCommandEvent& event)
{
    RadioButton(event.GetId());
}

void dmsFrameStack::OnCheckBox(wxCommandEvent& event)
{
    CheckBox(event.GetId());
}


void dmsFrameStack::OnListBoxSelected(wxCommandEvent& event)
{
    ListBoxSelected(event.GetId());
}

void dmsFrameStack::OnListBoxValidated(wxCommandEvent& event)
{
    if (ListBoxValidated(event.GetId())) Ok();
}

wxWindow *dmsFrameStack::GetWindow(int id)
{
    return FindWindow(id);
}


/* ########################################################################

   Classe dmsWindowStackDisplay

   Permet d'automatiser les actions

   - enable(true) enable(false)
   - show(true) show(false)

   A partir d'actions réalisées sur
   - des checkbox
   - des choices
   ######################################################################## */


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsWindowStackDisplayList);

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsWindowStackDisplay::dmsWindowStackDisplay(dmsWindowStackDisplay *parent, wxWindow *win)
{
    m_oList.DeleteContents(true);

    m_poParent = parent;
    m_poWin    = win;
    m_eType    = WS_DISPLAY_TYPE_UNKNOWN;
    m_eMode    = WS_DISPLAY_MODE_UNKNOWN;
    m_bActivated   = false;
    m_iParentValue = -1;
    m_iValue = -1;
}


dmsWindowStackDisplay::~dmsWindowStackDisplay()
{

}


/* ========================================================================

   ======================================================================== */


/* ------------------------------------------------------------------------
   Recherche récursive d'un noeud
   ------------------------------------------------------------------------ */

dmsWindowStackDisplay *dmsWindowStackDisplay::Find(wxWindow *win)
{
    dmsWindowStackDisplay *child, *result;

    if (win == m_poWin) return this;

    FOREACH(dmsWindowStackDisplayList, m_oList, child)
    {
        result = child->Find(win);
        if (result)
            return result;
    }

    return NULL;
}


/* ------------------------------------------------------------------------
   Ajout/Modification des valeurs "type" et "mode" d'un noeud
   ------------------------------------------------------------------------ */

void dmsWindowStackDisplay::Add(wxWindow *win, EnumWSDisplayType type, EnumWSDisplayMode mode)
{
    dmsWindowStackDisplay *node = Find(win);

    if (node == NULL)
    {
        node = new dmsWindowStackDisplay(this, win);
        m_oList.Append(node);
    }

    node->m_eType = type;
    node->m_eMode = mode;
}


/* ------------------------------------------------------------------------
   Ajout d'un noeud fils.
   ------------------------------------------------------------------------ */

void dmsWindowStackDisplay::Add(wxWindow *parent, int parentValue, wxWindow *child,  EnumWSDisplayType type)
{
    dmsWindowStackDisplay *node = Find(parent);
    dmsWindowStackDisplay *childnode;

    if (node == NULL) return;

    childnode = node->Find(child);

    if (childnode == NULL)
    {
        childnode = new dmsWindowStackDisplay(this, child);
        node->m_oList.Append(childnode);
    }

    childnode->m_iParentValue = parentValue;
    childnode->m_eType        = type;
    childnode->m_eMode        = node->m_eMode;
}



/* ------------------------------------------------------------------------
   Activation/Desactivation d'un noeud
   ------------------------------------------------------------------------ */


void dmsWindowStackDisplay::Update()
{
    dmsWindowStackDisplay *child;

    Update(m_poWin);

    FOREACH(dmsWindowStackDisplayList, m_oList, child)
    {
        child->Display(true);
    }
}

void dmsWindowStackDisplay::Update(wxWindow *win)
{
    dmsWindowStackDisplay *node = Find(win);

    if (node)
        node->Display(true);
}


void dmsWindowStackDisplay::Display(bool activated)
{
    m_bActivated = activated;

    switch (m_eType)
    {
    case WS_DISPLAY_TYPE_CHECKBOX:
        m_iValue = (int) ((wxCheckBox*)m_poWin)->GetValue();
        break;
    case WS_DISPLAY_TYPE_CHOICE:
        m_iValue = (int) ((wxChoice*)m_poWin)->GetSelection();
        break;
    default:
        m_iValue = -1;
        break;
    }

    switch (m_eMode)
    {
    case WS_DISPLAY_MODE_ENABLE:
        m_poWin->Enable(activated);
        break;
    case WS_DISPLAY_MODE_SHOW:
        m_poWin->Show(activated);
        break;
    default:
        break;
    }

    dmsWindowStackDisplay *child;

    FOREACH(dmsWindowStackDisplayList, m_oList, child)
    {
        child->Display(activated && (child->m_iParentValue == m_iValue));
    }
}


void dmsWindowStackDisplay::Trace(const wxString &depth)
{
    char *TypeTab[] = {(char *)"??", (char *)"Checkbox", (char *)"Choice"};
    char *ModeTab[] = {(char *)"??", (char *)"Enable", (char *)"Show"};
    char *ActivatedTab[] = {(char *)"off", (char *)"on"};

    wxLogDebug("%s- [%p] type=%s value=%d mode=%s activated=%s father_value=%d",
        depth, m_poWin, TypeTab[m_eType], m_iValue, ModeTab[m_eMode],
        ActivatedTab[m_bActivated],
        m_iParentValue);

    dmsWindowStackDisplay *child;

    FOREACH(dmsWindowStackDisplayList, m_oList, child)
    {
        child->Trace(depth + "  ");
    }
}




/* ########################################################################
   Validateur générique adapté aux "DialogStack"
   ######################################################################## */

/* ========================================================================
   Table des messages
   ======================================================================== */

BEGIN_EVENT_TABLE(dmsValidator2, wxValidator)
    EVT_CHAR(dmsValidator2::OnChar)
END_EVENT_TABLE()


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsValidator2::dmsValidator2()
{
    m_poWindowStack = NULL;
    m_poLastColour  = NULL;
}

dmsValidator2::dmsValidator2(dmsWindowStack *window)
{
    m_poWindowStack = window;
    m_poLastColour  = NULL;
}

dmsValidator2::dmsValidator2(const dmsValidator2& val)
    : wxValidator()
{
    Copy(val);
}

bool dmsValidator2::Copy(const dmsValidator2& val)
{
    wxValidator::Copy(val);

    m_poWindowStack   = val.m_poWindowStack;

    if (val.m_poLastColour)
    {
        DELNUL(m_poLastColour);
        m_poLastColour = new wxColor(*(val.m_poLastColour));
    }
    else
        m_poLastColour = NULL;

    return TRUE;
}

dmsValidator2::~dmsValidator2()
{
    DELNUL(m_poLastColour);
}

/* ========================================================================

   ======================================================================== */

bool dmsValidator2::Validate(wxWindow *parent)
{
    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    bool ok;

    if (!control->IsEnabled()) return true;

    wxString value(control->GetValue());

    g_poLogManager->Clear();

    ok = ValidateHot(value);

    if (ok && m_poWindowStack)
        ok = m_poWindowStack->ValidateCtrl(control, value);

    ok = ok && (g_poLogManager->GetCount() == 0);

    if (! ok)
    {
        if (parent->GetName() == "panel" &&
            parent->GetParent()->GetName() == "notebook")
        {
            wxNotebook *notebook = (wxNotebook*) parent->GetParent();
            for (unsigned int i=0; i<notebook->GetPageCount(); i++)
            {
                if (notebook->GetPage(i) == parent)
                {
                    notebook->SetSelection(i);
                    break;
                }
            }
        }

        if (g_poLogManager->GetCount() == 0)
        {
            if (value.Len() == 0)
                LOGE(L"Empty value");
            else
                LOGE(L"Bad value");
        }

        control->SetFocus();
        m_poLastColour = new wxColor(control->GetBackgroundColour());
        control->SetBackgroundColour(wxColour(255,240,240));
        control->Refresh(true);

        wxMessageDialog dialog(NULL,
            g_poLogManager->Format("%s\n"),
            "Error",
            wxOK | wxCENTRE | wxICON_ERROR);

        //dmsValidErrorDialog dialog(control, );
        dialog.ShowModal();

        return false;
    }
    else
    {
        if (m_poLastColour)
        {
            control->SetBackgroundColour(*m_poLastColour);
            control->Refresh();
            DELNUL(m_poLastColour);
        }

        return true;
    }
}



void dmsValidator2::OnChar(wxKeyEvent& event)
{
    if (m_validatorWindow)
    {
        int keyCode = (int)event.GetKeyCode();

        if (!(keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode > WXK_START))
        {
            bool ok;
            long selectBegin, selectEnd;

            wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;

            control->GetSelection(&selectBegin, &selectEnd);

            wxString value(control->GetValue());

            if (selectEnd > selectBegin)
            {
                value = STR("%s%c%s", value.Mid(0,selectBegin), keyCode, value.Mid(selectEnd));
            }
            else
            {
                int index = control->GetInsertionPoint();
                value = STR("%s%c%s", value.Mid(0, index), keyCode, value.Mid(index));
            }

            ok = ValidateHot(value);

            if (! ok)
            {
                if (!wxValidator::IsSilent()) wxBell();
                return;
            }
        }
        event.Skip();
    }
    else
    {
        event.Skip();
    }
}


/* ########################################################################
   Validateur basé sur expressions rationnelles
   ######################################################################## */


/* ========================================================================
   Constructeur / Desctructeur
   ======================================================================== */


dmsRegExValidator::dmsRegExValidator(const wxString &regex)
: dmsValidator2()
{
    m_oRegExStr = regex;
    m_oRegEx.Compile(m_oRegExStr);
}


dmsRegExValidator::dmsRegExValidator(dmsWindowStack *window, const wxString &regex)
: dmsValidator2(window)
{
    m_oRegExStr = regex;
    m_oRegEx.Compile(m_oRegExStr);
}



dmsRegExValidator::dmsRegExValidator(const dmsRegExValidator& val)
    : dmsValidator2()
{
    Copy(val);
}

bool dmsRegExValidator::Copy(const dmsRegExValidator& val)
{
    dmsValidator2::Copy(val);

    m_oRegExStr = val.m_oRegExStr;
    m_oRegEx.Compile(m_oRegExStr);

    return TRUE;
}

dmsRegExValidator::~dmsRegExValidator()
{
}

/* ========================================================================

   ======================================================================== */

bool dmsRegExValidator::ValidateHot(const wxString &value)
{
    return m_oRegEx.Matches(value);
}




/* ########################################################################

   ######################################################################## */




BEGIN_EVENT_TABLE(dmsValidErrorDialog, wxDialog)
    EVT_BUTTON(-1, dmsValidErrorDialog::OnOk)
END_EVENT_TABLE()



dmsValidErrorDialog::dmsValidErrorDialog(wxWindow *ctrl, const wxString &message)
: wxDialog(NULL, -1, "Validation Error",
           wxDefaultPosition,
           wxDefaultSize,
           wxRESIZE_BORDER)

{
    wxColor colour(255,255,0);

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText *title = new wxStaticText(this, -1, "Valid error",
        wxDefaultPosition, wxDefaultSize,
        wxALIGN_CENTRE);
    title->SetBackgroundColour(wxColour(192,192,0));
    sizer->Add(title,0,wxEXPAND);

    wxStaticText *text = new wxStaticText(this, -1, message);
    text->SetBackgroundColour(colour);
    sizer->Add(text,0,wxALL,25);

    text->SetEventHandler(this);

    wxButton     *btn = new wxButton(this, 1000, "Ok", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    btn->SetBackgroundColour(colour);
    sizer->Add(btn, 0, wxALIGN_CENTER);

    SetBackgroundColour(colour);

    wxPoint point(10, ctrl->GetSize().GetHeight()-10);

    ctrl->ClientToScreen(&(point.x), &(point.y));

    SetAutoLayout(true);
    SetSizer(sizer);

    sizer->Fit(this);
    sizer->SetSizeHints(this);

    dmsMoveVisible(this, point);
}



dmsValidErrorDialog::~dmsValidErrorDialog()
{
}



void dmsValidErrorDialog::OnOk(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}
