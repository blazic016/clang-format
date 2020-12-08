/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/image.h>
#include <wx/cshelp.h>
#include <wx/calctrl.h>

#include <Tools/Xml.h>
#include <Tools/Validator.h>
#include <Tools/File.h>

#include "Dialog.h"
#include "Window.h"


/* ########################################################################

   ######################################################################## */

/* ========================================================================

   ======================================================================== */

BEGIN_EVENT_TABLE(dmsDialog, wxDialog)
    EVT_CLOSE(dmsDialog::OnClose)
END_EVENT_TABLE()

/* ========================================================================

   ======================================================================== */

dmsDialog::dmsDialog(wxWindow *parent, const wxString &title,
                     wxWindowID id,
                     long style,
                     const wxString& name) :
wxDialog()
{
    m_poManager = new dmsDialogContextManager(this);

#ifdef __WXMSW__
    //SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);
#endif

    wxDialog::Create(parent, id, title, wxDefaultPosition, wxDefaultSize, style, name);

    //wxSimpleHelpProvider* provider = new wxSimpleHelpProvider;
    //wxHelpProvider::Set(provider);
}

dmsDialog::~dmsDialog()
{
    DELNUL(m_poManager);
}

/* ========================================================================

   ======================================================================== */

void dmsDialog::OnClose(wxCloseEvent& event)
{
    OnEvent(NULL);
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsDialogContextManager::dmsDialogContextManager(wxWindow *parent) : dmsvContextManager()
{
    m_poWindow           = parent;
    m_poSizer            = new wxBoxSizer(wxVERTICAL);
    m_poValidatorManager = new dmsValidatorManager();

    m_poErrorCxt = NULL;
    m_poGetCxt   = NULL;
    m_poUpdated  = NULL;
}

dmsCliContextManager::dmsCliContextManager() : dmsvContextManager()
{
	m_poValidatorManager = new dmsValidatorManager();
}




dmsDialogContextManager::~dmsDialogContextManager()
{
    DELNUL(m_poValidatorManager);
    //DELNUL(m_poSizer);
}

dmsCliContextManager::~dmsCliContextManager()
{
    DELNUL(m_poValidatorManager);
    //DELNUL(m_poSizer);
}


/* ========================================================================

   ======================================================================== */

bool dmsDialogContextManager::Load(wxXmlNode *node)
{
    bool res;

    if (node==NULL) return false;

    dmsDialogContext* root = new dmsDialogContext(this, NULL, NULL, m_poWindow, m_poSizer);

    res = root->Load(node);

    if (! res) { wxMessageBox("Error while loading IHM file"); exit(1); }

    dmsvContext* ctx;

    FOREACH(dmsvContextList, m_oList, ctx)
    {
        ((dmsDialogContext*)ctx)->InitDepends();
    }

    m_poWindow->SetSizer(m_poSizer);
    m_poWindow->SetAutoLayout(true);
    m_poSizer->SetSizeHints(m_poWindow);
    m_poWindow->SetSize(0,0);

    return res;
}
#include <iostream>
bool dmsCliContextManager::Load(wxXmlNode *node)
{
    bool res;

    if (node==NULL) return false;

    dmsCliContext* root = new dmsCliContext(this, NULL);

    res = root->Load(node);

    if (! res) {std::cout << ("Error while loading IHM file") << std::endl; exit(1); }

    /*
    dmsvContext* ctx;

    FOREACH(dmsvContextList, m_oList, ctx)
    {
        ((dmsDialogContext*)ctx)->InitDepends();
    }

    m_poWindow->SetSizer(m_poSizer);
    m_poWindow->SetAutoLayout(true);
    m_poSizer->SetSizeHints(m_poWindow);
    m_poWindow->SetSize(0,0);
*/
    return res;
}


bool dmsDialogContextManager::Load(const wxString &name)
{
    wxXmlDocument doc;

    doc.Load(name);

    return Load(doc.GetRoot());
}

bool dmsCliContextManager::Load(const wxString &name)
{
    wxXmlDocument doc;

    doc.Load(name);

    return Load(doc.GetRoot());
}


bool dmsDialogContextManager::LoadResource(const wxString &resourceName)
{
    wxXmlDocument doc;
    dmsBuffer     buf;

    if (! buf.LoadResource(resourceName)) return false;

    doc.LoadText((char*) buf.m_poBuffer);

    return Load(doc.GetRoot());
}

bool dmsCliContextManager::LoadResource(const wxString &resourceName)
{
    wxXmlDocument doc;
    dmsBuffer     buf;

    if (! buf.LoadResource(resourceName)) return false;

    doc.LoadText((char*) buf.m_poBuffer);

    return Load(doc.GetRoot());
}





bool dmsDialogContextManager::LoadText(const wxString &text)
{
    wxXmlDocument doc;

    doc.LoadText(text);

    return Load(doc.GetRoot());
}

bool dmsCliContextManager::LoadText(const wxString &text)
{
    wxXmlDocument doc;

    doc.LoadText(text);

    return Load(doc.GetRoot());
}



/* ========================================================================

   ======================================================================== */

void dmsDialogContextManager::OnEvent(dmsDialogContext* ctx)
{
    if (ctx->m_oAction.Len() && ! DoAction(ctx)) return;

    ((dmsDialog*)m_poWindow)->OnEvent(ctx);
}


bool dmsDialogContextManager::DoAction(dmsDialogContext* ctx)
{
    dmsDialogContext* ref = NULL;
    wxString value;

    if (ctx->m_oRef.Len()) ref = (dmsDialogContext*) Find(ctx->m_oRef);

    LOG_AF(ref, LOGE(L"No reference for dir-chooser"));

    ref->GetValue(value);

    if (ctx->m_oAction=="file-chooser")
    {
        wxFileDialog dlg(m_poWindow, "Choose a file", "", value);

        if (dlg.ShowModal()==wxID_OK)
        {
            ref->Set(dlg.GetPath());
            ref->Get();
        }
        return true;
    }

    if (ctx->m_oAction=="dir-chooser")
    {
        wxDirDialog dlg(m_poWindow, "Choose a directory", value, wxDD_NEW_DIR_BUTTON);

        if (dlg.ShowModal()==wxID_OK)
        {
            ref->Set(dlg.GetPath());
            ref->Get();
        }
        return true;
    }

    if (ctx->m_oAction=="file-open")
    {
        dmsFileOpenWithDefaultApp(value);

        return true;
    }

    if (ctx->m_oAction=="combo-add")
    {
        dmsvEnum* map   = (dmsvEnum*) ref->m_poValidator;

        if (! map->AddUniqValue(value))
        {
            wxMessageDialog dialog(NULL,
                map->GetError(),
                "Input error",
                wxOK | wxCENTRE | wxICON_ERROR);

            dialog.ShowModal();

            return false;
        }

        map->Get();

        return true;
    }

    if (ctx->m_oAction=="combo-del")
    {
        wxComboBox* win = (wxComboBox*)(ref->m_poWin);
        dmsvEnum* map   = (dmsvEnum*) ref->m_poValidator;

        if (value.Len())
        {
            wxMessageDialog dialog(NULL,
                STR("Do you confirm suppression of profil '%s'", value),
                "Delete profile",
                wxNO_DEFAULT|wxYES_NO|wxICON_WARNING);

            if (dialog.ShowModal()!=wxID_YES) return true;
        }

        if (! map->DelValue(value))
        {
            wxMessageDialog dialog(NULL,
                map->GetError(),
                "Input error",
                wxOK | wxCENTRE | wxICON_ERROR);

            dialog.ShowModal();

            return false;
        }

        int i = win->FindString(value);

        win->Delete(i);
        win->SetSelection(0);

        map->Get();
        ref->Get();

        return true;
    }

    //return ref->Set();
    //ref->Get(value);

    LOGE(L"Unrecognized action [%s]", ctx->m_oAction);

    return false;
}



/* ########################################################################

   ######################################################################## */


BEGIN_EVENT_TABLE(dmsDialogRootPanel, wxPanel)
    EVT_BUTTON                (-1, dmsDialogRootPanel::OnButton)
    EVT_CHOICE                (-1, dmsDialogRootPanel::OnChoice)
    EVT_RADIOBOX              (-1, dmsDialogRootPanel::OnRadioBox)
    EVT_CHECKBOX              (-1, dmsDialogRootPanel::OnCheckBox)
    EVT_LISTBOX               (-1, dmsDialogRootPanel::OnListBoxSelected)
    EVT_LISTBOX_DCLICK        (-1, dmsDialogRootPanel::OnListBoxValidated)
    EVT_COMBOBOX              (-1, dmsDialogRootPanel::OnComboBox)
    EVT_COMMAND               (-1, wxEVT_COMMAND_TEXT_UPDATED, dmsDialogRootPanel::OnTextUpdated)
    EVT_NOTEBOOK_PAGE_CHANGED (-1, dmsDialogRootPanel::OnNotebookPageChanged)
    EVT_CALENDAR_SEL_CHANGED  (-1, dmsDialogRootPanel::OnCalendarSelChanged)
    EVT_CHILD_FOCUS           (dmsDialogRootPanel::OnChildFocus)
    EVT_IDLE                  (dmsDialogRootPanel::OnIdle)
END_EVENT_TABLE()


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsDialogRootPanel::dmsDialogRootPanel(dmsDialogContextManager *manager, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: wxPanel(parent, id, pos, size, style, name)
{
    m_poManager  = manager;
}


dmsDialogRootPanel::~dmsDialogRootPanel()
{

}


/* ========================================================================

   ======================================================================== */


void dmsDialogRootPanel::OnTextUpdated(wxCommandEvent& event)
{
    m_poManager->m_poUpdated = (dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject());

    //LOG0("[%s|%s]", m_poManager->m_poUpdated->m_oName, event.GetString());
}


void dmsDialogRootPanel::OnButton(wxCommandEvent& event)
{
    m_poManager->OnEvent((dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject()));
}

void dmsDialogRootPanel::OnChildFocus(wxChildFocusEvent& event)
{
    if (! DoFocus()) event.Skip();
}

bool dmsDialogRootPanel::DoFocus()
{
    bool ok = true;

    if (m_poManager->m_poUpdated &&
        m_poManager->m_poUpdated->m_poValidator &&
        m_poManager->m_poUpdated->IsEnabled())
    {
        if (! m_poManager->m_poUpdated->Set())
        {
            m_poManager->m_poErrorCxt = m_poManager->m_poUpdated;
            ok = false;
        }
        else
            m_poManager->m_poUpdated->Get();
    }
    m_poManager->m_poUpdated = NULL;

    return ok;
}

void dmsDialogRootPanel::OnIdle(wxIdleEvent& event)
{
    if (m_poManager->m_poErrorCxt)
    {
        m_poManager->m_poErrorCxt->ShowError();
        m_poManager->m_poErrorCxt = NULL;
    }
    if (m_poManager->m_poGetCxt)
    {
        m_poManager->m_poGetCxt->Get();
        m_poManager->m_poGetCxt = NULL;
    }
}


void dmsDialogRootPanel::OnChoice(wxCommandEvent& event)
{
    if (! DoFocus()) return;

    m_poManager->OnEvent((dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject()));
}

void dmsDialogRootPanel::OnRadioBox(wxCommandEvent& event)
{
    if (! DoFocus()) return;

    dmsDialogContext* ctx = (dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject());

    wxRadioBox* win = (wxRadioBox*) ctx->m_poWin;

    ctx->ApplyDepends(win->GetSelection());
    ctx->Set();

    m_poManager->OnEvent(ctx);
}

void dmsDialogRootPanel::OnCheckBox(wxCommandEvent& event)
{
    if (! DoFocus()) return;

    dmsDialogContext* ctx = (dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject());

    wxCheckBox* win = (wxCheckBox*) ctx->m_poWin;

    ctx->ApplyDepends(win->GetValue()?1:0);
    ctx->Set();

    m_poManager->OnEvent(ctx);
}

void dmsDialogRootPanel::OnListBoxSelected(wxCommandEvent& event)
{
    if (! DoFocus()) return;

    m_poManager->OnEvent((dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject()));
}

void dmsDialogRootPanel::OnListBoxValidated(wxCommandEvent& event)
{
    if (! DoFocus()) return;

    m_poManager->OnEvent((dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject()));
}

void dmsDialogRootPanel::OnComboBox(wxCommandEvent& event)
{
    if (! DoFocus()) return;

    dmsDialogContext* ctx = (dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject());

    wxComboBox* win = (wxComboBox*) ctx->m_poWin;

    ctx->ApplyDepends(win->GetSelection());
    ctx->Set();

    m_poManager->OnEvent(ctx);
}

void dmsDialogRootPanel::OnNotebookPageChanged(wxNotebookEvent& event)
{
    if (! DoFocus()) return;
}

void dmsDialogRootPanel::OnCalendarSelChanged(wxCalendarEvent& event)
{
    m_poManager->m_poUpdated = (dmsDialogContext*)m_poManager->FindData((wxWindow*)event.GetEventObject());
    if (! DoFocus()) return;
}


/* ########################################################################

   ######################################################################## */


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(dmsDialogContextList);
WX_DEFINE_LIST(dmsDependsList);



dmsCliContext::dmsCliContext(dmsvContextManager* manager, dmsValidator *validator) : dmsvContext(manager, validator)
{
    m_poChoiceList     = NULL;


}

dmsDialogContext::dmsDialogContext(dmsvContextManager* manager, dmsValidator *validator, dmsDialogContext* parent, wxWindow *parentWin, wxSizer* parentSizer) : dmsvContext(manager, validator)
{
    m_poParent         = parent;
    m_poEnableParent   = parent;
    m_poParentWin      = parentWin;
    m_poParentSizer    = parentSizer;
    m_poWin            = NULL;
    m_poSizer          = NULL;
    m_poChoiceList     = NULL;
    m_iDependsState    = 0;
    m_poDependsList    = NULL;
    m_poDependingList  = NULL;
    m_poValidator      = NULL;
    m_poDependsContext = NULL;

    m_poChildList      = new dmsDialogContextList();
}

dmsCliContext::~dmsCliContext()
{
}
dmsDialogContext::~dmsDialogContext()
{
    if (m_poChoiceList) delete [] m_poChoiceList;

    if (m_poDependsList)
    {
        dmsDepends *poDepends;
        FOREACH(dmsDependsList, *(m_poDependsList), poDepends) delete(poDepends);
        DELNUL(m_poDependsList);
    }

    DELNUL(m_poDependingList);
    DELNUL(m_poChildList);

    if (m_oType=="Notebook" && !!m_oName)
    {
        dmsConfigWriteInt(STR("/Window/Notebook/%s_Page", m_oName), ((wxNotebook*)m_poWin)->GetSelection());
    }
}


/* ========================================================================

   ======================================================================== */
bool dmsCliContext::Load(wxXmlNode* node)
{
    m_oName = node->GetPropVal("name", "");
    m_oType = node->GetName();

    wxString type      = node->GetPropVal("type", "");
    wxString validinfo = node->GetPropVal("valid", "");
    wxString choices   = node->GetPropVal("choices", "");
    wxString readonly  = node->GetPropVal("read-only", "");
    wxString multiline = node->GetPropVal("multiline", "false");
    wxString helptext  = node->GetPropVal("helptext", "");



    m_oDepends = node->GetPropVal("depends", "");
    m_oAction  = node->GetPropVal("action", "");
    m_oRef     = node->GetPropVal("ref", "");


    int choiceCount = 0;

    if (choices.Len()) ::dmsSplitChoices(choices, choiceCount, m_poChoiceList);

    if (m_oType == "Window")
    {
    }
    else if (m_oType == "Panel")
    {
    }
    else if (m_oType == "RootPanel")
    {
    }
    else if (m_oType == "ContextHelpButton")
    {
    }
    else if (m_oType == "StaticText")
    {
    }
    else if (m_oType == "StaticBitmap")
    {
    }
    else if (m_oType == "Button")
    {
    }
    else if (m_oType == "TextCtrl")
    {
    }
    else if (m_oType == "CheckBox")
    {
    }
    else if (m_oType == "StaticBox")
    {
    }
    else if (m_oType == "RadioBox")
    {
        if (m_poChoiceList)
        {
            type = "enum";
        }
    }
    else if (m_oType == "ListBox")
    {
    }
    else if (m_oType == "ComboBox")
    {
    	type = "enum";
    }
    else if (m_oType == "Notebook")
    {
    }
    else if (m_oType == "Page")
    {
    }
    else if (m_oType == "Sizer")
    {
    }
    else if (m_oType == "BoxSizer")
    {
    }
    else if (m_oType == "HBox")
    {
    }
    else if (m_oType == "VBox")
    {
    }
    else if (m_oType == "GridSizer")
    {
    }
    else if (m_oType== "Spacer" )
    {
    }
    else if (m_oType == "Margin")
    {
    }
    else if (m_oType == "Spring")
    {
    }
    else if (m_oType == "comment")
    {
        return true;
    }
    else if (m_oType == "CalendarCtrl")
    {
    }
    else
    {
        return false;
    }


     if (type.Len())
     {
         m_poValidator = m_poManager->m_poValidatorManager->Add(this, type);

         m_poValidator->SetName(m_oName);

         if (choices.Len())
         {
             ((dmsvEnum*)m_poValidator)->SetValues(m_poChoiceList);
         }

         bool ok = m_poValidator->Init(validinfo);
         if (!ok) {
        	 return false;
         }

         m_poValidator->Clear();
     }


     // Appel récursif

     wxXmlNode *child;
     for (child=node->GetChildren();child;child=child->GetNext())
     {
         dmsCliContext* item = new dmsCliContext(
             m_poManager,
             NULL);

         item->m_poManager = m_poManager;

         if (! item->Load(child)){
        	 return false;
         }
     }



     return true;

}
#include <iostream>
bool dmsDialogContext::Load(wxXmlNode *node)
{
    dmsvEnum valid(NULL);

    m_oName = node->GetPropVal("name", "");
    m_oType = node->GetName();

    // Variables

    wxPoint  pos       = wxDefaultPosition;
    wxSize   size      = wxDefaultSize;
    wxString type      = node->GetPropVal("type", "");
    wxString validinfo = node->GetPropVal("valid", "");
    wxString label     = node->GetPropVal("label", "");
    wxString text      = node->GetPropVal("text", "");
    wxString width     = node->GetPropVal("width", "");
    wxString height    = node->GetPropVal("height", "");
    wxString orient    = node->GetPropVal("orient", "V");
    wxString expand    = node->GetPropVal("expand", "");
    wxString bgcolor   = node->GetPropVal("bgcolor", "");
    wxString color     = node->GetPropVal("color", "");
    wxString choices   = node->GetPropVal("choices", "");
    wxString readonly  = node->GetPropVal("read-only", "");
    wxString multiline = node->GetPropVal("multiline", "false");
    wxString hscroll   = node->GetPropVal("hscroll", "false");
    wxString vscroll   = node->GetPropVal("vscroll", "false");
    wxString resource  = node->GetPropVal("resource", "");
    wxString fontname  = node->GetPropVal("fontname", "");
    wxString fontsize  = node->GetPropVal("fontsize", "");
    wxString helptext  = node->GetPropVal("helptext", "");

    m_oDepends = node->GetPropVal("depends", "");
    m_oAction  = node->GetPropVal("action", "");
    m_oRef     = node->GetPropVal("ref", "");

    const dmsNameRank borderNames[] = {
        {wxBORDER_NONE,   (char*)"none"},
        {wxBORDER_STATIC, (char*)"static"},
        {wxBORDER_SIMPLE, (char*)"simple"},
        {wxBORDER_RAISED, (char*)"raised"},
        {wxBORDER_SUNKEN, (char*)"sunken"},
        {wxBORDER_DOUBLE, (char*)"double"},
        {0,(char*)NULL}};

    int border      = dmsGetStrRank(node->GetPropVal("border", "none"), borderNames);
    int space       = StrToInt(node->GetPropVal("space", "0"));
    int proportion  = StrToInt(node->GetPropVal("str",   ""));
    int margin      = StrToInt(node->GetPropVal("margin", "0"));

    int choiceCount=0;

    if (choices.Len()) ::dmsSplitChoices(choices, choiceCount, m_poChoiceList);

    if (width.Len())  size.x = StrToInt(width);
    if (height.Len()) size.y = StrToInt(height);

    int flag  = 0;
    int style = 0;

    if (expand=="true") { flag |= wxEXPAND|wxALL; }

    style |= border;

    // Construction

    if (m_oType == "Window")
    {
        m_poWin = new wxWindow(m_poParentWin, -1, pos, size);
    }
    else if (m_oType == "Panel")
    {
        m_poWin = new wxPanel(m_poParentWin, -1, pos, size, style|wxTAB_TRAVERSAL);
        m_poSizer = new wxBoxSizer(wxVERTICAL);
    }
    else if (m_oType == "RootPanel")
    {
        m_poWin = new dmsDialogRootPanel((dmsDialogContextManager*)m_poManager, m_poParentWin, -1, pos, size, style|wxTAB_TRAVERSAL);
        m_poSizer = new wxBoxSizer(wxVERTICAL);
    }
    else if (m_oType == "ContextHelpButton")
    {
        m_poWin = new wxContextHelpButton(m_poParentWin, wxID_CONTEXT_HELP, pos, size);
    }
    else if (m_oType == "StaticText")
    {
        m_poWin = new wxStaticText(m_poParentWin, -1, label, pos, size);
    }
    else if (m_oType == "StaticBitmap")
    {
        if (resource.Len())
        {
            wxImage   image(resource, wxBITMAP_TYPE_ANY);
            m_poWin = new wxStaticBitmap(m_poParentWin, -1, wxBitmap(image), pos, size);
        }
    }
    else if (m_oType == "Button")
    {
        m_poWin = new wxButton(m_poParentWin, -1, label, pos, size);
    }
    else if (m_oType == "TextCtrl")
    {
        int style=0;

        if (readonly  == "true") style |= wxTE_READONLY;
        if (multiline == "true") style |= wxTE_MULTILINE;
        if (hscroll   == "true") style |= wxHSCROLL;
        if (vscroll   == "true") style |= wxTE_AUTO_SCROLL;


        m_poWin = new wxTextCtrl(m_poParentWin, -1, "", pos, size, style);
    }
    else if (m_oType == "CheckBox")
    {
        m_poWin = new wxCheckBox(m_poParentWin, -1, label, pos, size);
    }
    else if (m_oType == "StaticBox")
    {
        m_poSizer = new wxStaticBoxSizer(new wxStaticBox(m_poParentWin, -1, label), orient=="V"?wxVERTICAL:wxHORIZONTAL);
    }
    else if (m_oType == "RadioBox")
    {
        if (m_poChoiceList)
        {
            m_poWin = new wxRadioBox(m_poParentWin, -1, label, pos, size, choiceCount, m_poChoiceList);
            type = "enum";
        }
    }
    else if (m_oType == "ListBox")
    {
        m_poWin = new wxListBox(m_poParentWin, -1, pos, size);
    }
    else if (m_oType == "ComboBox")
    {
        int style=0;

        if (readonly != "false")    style |= wxCB_READONLY;

        m_poWin = new wxComboBox(m_poParentWin, -1, "", pos, size, choiceCount, m_poChoiceList, style);

        type = "enum";

        wxComboBox* win = (wxComboBox*)(m_poWin);

        unsigned int i=0;
        while (i<win->GetCount())
        {
            if (win->GetString(i).IsEmpty())
                win->Delete(i);
            else
                i++;
        }
    }
    else if (m_oType == "Notebook")
    {
        m_poWin = new wxNotebook(m_poParentWin, -1, pos, size, style);
    }
    else if (m_oType == "Page")
    {
        m_poWin = new wxPanel(m_poParentWin, -1, pos, size);
        m_poSizer = new wxBoxSizer(wxVERTICAL);

        wxNotebook *notebook = (wxNotebook*)m_poParentWin;
        wxPanel    *page     = (wxPanel*)m_poWin;
        page->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
        notebook->AddPage(page, text);
    }
    else if (m_oType == "Sizer")
    {
        m_poSizer = new wxMySizer();
    }
    else if (m_oType == "BoxSizer")
    {
        m_poSizer = new wxBoxSizer(orient=="V"?wxVERTICAL:wxHORIZONTAL);
    }
    else if (m_oType == "HBox")
    {
        m_poSizer = new wxBoxSizer(wxHORIZONTAL);
    }
    else if (m_oType == "VBox")
    {
        m_poSizer = new wxBoxSizer(wxVERTICAL);
    }
    else if (m_oType == "GridSizer")
    {
        int rows    = StrToInt(node->GetPropVal("rows", "0"));
        int cols    = StrToInt(node->GetPropVal("cols", "0"));
        int vgap    = StrToInt(node->GetPropVal("vgap", "0"));
        int hgap    = StrToInt(node->GetPropVal("hgap", "0"));
        int growcol = StrToInt(node->GetPropVal("growcol", "-1"));

        m_poSizer = new wxFlexGridSizer(rows, cols, vgap, hgap);

        wxFlexGridSizer* grid = (wxFlexGridSizer*) m_poSizer;

        if (growcol>=0) grid->AddGrowableCol(growcol);
    }
    else if (m_oType== "Spacer" && m_poParentSizer)
    {
        m_poParentSizer->Add(space,space,proportion,flag);
    }
    else if (m_oType == "Margin" && m_poParentSizer)
    {
        m_poSizer = new wxBoxSizer(wxVERTICAL);

        m_poParentSizer->Add(m_poSizer, 1, wxEXPAND|wxALL, space);

        m_poParentSizer = NULL;
    }
    else if (m_oType == "Spring" && m_poParentSizer)
    {
        m_poParentSizer->Add(0,0,1);
        m_poParentSizer = NULL;
    }
    else if (m_oType == "comment")
    {
        return true;
    }
    else if (m_oType == "CalendarCtrl")
    {
        m_poWin = new wxCalendarCtrl(m_poParentWin, -1);
    }
    else
    {
        wxMessageBox(STR("Unkown Type %s", m_oType));
        return false;
    }

    // // // // // // // // // // // // // // // // // // // // // //

    if (bgcolor.Len() && m_poWin)
    {
        int c;
        sscanf(bgcolor.c_str()+1, "%X", &c);
        m_poWin->SetBackgroundColour(wxColour(c>>16,c>>8&0xFF,c&0xFF));
    }

    if (color.Len() && m_poWin)
    {
        int c;
        sscanf(color.c_str()+1, "%X", &c);
        m_poWin->SetForegroundColour(wxColour(c>>16,c>>8&0xFF,c&0xFF));
    }

    if (type.Len())
    {
        m_poValidator = m_poManager->m_poValidatorManager->Add(this, type);

        m_poValidator->SetName(m_oName);

        if (choices.Len())
        {
            ((dmsvEnum*)m_poValidator)->SetValues(m_poChoiceList);
        }

        bool ok = m_poValidator->Init(validinfo);
        if (!ok) return false;

        m_poValidator->Clear();
    }


    if (helptext.Len())
    {
        m_poWin->SetHelpText(helptext);
    }


    m_poData = m_poWin;

    //

    if (fontname.Len())
    {
        wxFont font = m_poWin->GetFont();
        font.SetFaceName(fontname);
        m_poWin->SetFont(font);
    }

    if (fontsize.Len())
    {
        wxFont font = m_poWin->GetFont();
        font.SetPointSize(StrToInt(fontsize));
        m_poWin->SetFont(font);
    }

    // Appel récursif

    wxXmlNode *child;
    for (child=node->GetChildren();child;child=child->GetNext())
    {
        dmsDialogContext* item = new dmsDialogContext(
            m_poManager,
            NULL,
            this,
            m_poWin?m_poWin:m_poParentWin,
            m_poSizer);

        item->m_poManager = m_poManager;

        m_poChildList->Append(item);

        if (! item->Load(child)) return false;
    }

    // Post initialisation

    // // // // // // // // // // // // // // // // // // // // // //

    if (m_poWin && m_poSizer)
    {
        m_poWin->SetSizerAndFit(m_poSizer);
        m_poWin->SetAutoLayout(true);
    }

    if (m_poParentSizer && m_poWin)
    {
        m_poParentSizer->Add(m_poWin, proportion, flag, margin);
    }
    else if (m_poParentSizer && m_poSizer)
    {
        m_poParentSizer->Add(m_poSizer, proportion, flag, margin);
    }

    if (m_oType=="Notebook" && !!m_oName)
    {
        int page;

        dmsConfigReadInt(STR("/Window/Notebook/%s_Page", m_oName), page, 0);

        ((wxNotebook*)m_poWin)->SetSelection(page);
    }

    return true;
}

bool dmsCliContext::SetValue(const wxString &value)
{
	if(m_poValidator)
	{
		m_poValidator->Set(value.fn_str());
		return true;
	}
	return false;
}


bool dmsDialogContext::SetValue(const wxString &value)
{
    if (m_oType == "TextCtrl")
    {
        wxTextCtrl* win = (wxTextCtrl*) m_poWin;
        win->SetValue(value);
    }
    else if (m_oType == "CheckBox")
    {
        wxCheckBox* win = (wxCheckBox*) m_poWin;
        win->SetValue(value=="true");
        ApplyDepends(win->GetValue()?1:0);
    }
    else if (m_oType == "ListBox")
    {
        TODO();
    }
    else if (m_oType == "ComboBox")
    {
        wxComboBox* win = (wxComboBox*) m_poWin;
        if (win->FindString(value)==-1) win->Append(value);
        win->SetValue(value);
        ApplyDepends(win->GetSelection());
    }
    else if (m_oType == "RadioBox")
    {
        wxRadioBox* win = (wxRadioBox*) m_poWin;
        win->SetStringSelection(value);
        ApplyDepends(win->GetSelection());
    }
    else if (m_oType == "CalendarCtrl")
    {
        wxCalendarCtrl* win = (wxCalendarCtrl*) m_poWin;
        wxDateTime oDiffusionDate;
        StrToDateTime(value, oDiffusionDate);
        win->SetDate(oDiffusionDate);
    }
    else
        return false;

    return true;
}


bool dmsCliContext::GetValue(wxString &value)
{
	if(m_poValidator)
	{
		value = m_poValidator->Get();
		return true;
	}
	return false;
}

bool dmsDialogContext::GetValue(wxString &value)
{
    if (m_oType == "TextCtrl")
    {
        wxTextCtrl* win = (wxTextCtrl*) m_poWin;
        value = win->GetValue();
    }
    else if (m_oType == "CheckBox")
    {
        wxCheckBox* win = (wxCheckBox*) m_poWin;
        value = win->GetValue()?"true":"false";
    }
    else if (m_oType == "ListBox")
    {
        TODO();
    }
    else if (m_oType == "ComboBox")
    {
        wxComboBox* win = (wxComboBox*) m_poWin;
        value = win->GetValue();
    }
    else if (m_oType == "RadioBox")
    {
        wxRadioBox* win = (wxRadioBox*) m_poWin;
        value = win->GetStringSelection();
    }
    else if (m_oType == "CalendarCtrl")
    {
        wxCalendarCtrl* win = (wxCalendarCtrl*) m_poWin;
        value = win->GetDate().Format("%Y/%m/%d");
    }
    else return false;

    return true;
}



void dmsDialogContext::InitComboBox(const wxArrayString &values, int index)
{
    LOG_AV(m_oType=="ComboBox", LOGE(L"Dialog Context type error. Not ComboBox"));

    wxComboBox* win = (wxComboBox*) m_poWin;

    win->Clear();

    for (size_t i=0; i<values.GetCount(); i++)
    {
        win->Append(values[i]);
    }

    win->SetSelection(index);
}

void dmsDialogContext::InitDepends()
{
    if (m_oDepends.Len()==0) return;

    int       count,count2;
    wxString* values ;
    wxString* values2;
    int k=0;

    ::dmsSplitChoices(m_oDepends, count, values);

    m_poDependsContext = (dmsDialogContext*) m_poManager->Find(values[0]);

    for (int i = 0; i < count; i++)
    {
        if(m_poDependsContext==NULL|| k!=0)
        {
            ::dmsSplitChoices(values[i], count2, values2);
            m_poDependsContext = (dmsDialogContext*) m_poManager->Find(values2[0]);
            k=1;
        }
        else
        {
            count2  = count;
            values2 = values;
        }

    if (m_poDependsContext==NULL)
    {
        wxMessageBox(STR("IHM spec error : no dependance on [%s]", values[0]));
        return;
    }

    if (m_poDependsContext->m_poDependingList==NULL)
        m_poDependsContext->m_poDependingList = new dmsDialogContextList;

    m_poDependsContext->m_poDependingList->Append(this);
        for (int j = 1; j < count2; j++)
        {
                    if (m_poDependsList == NULL) m_poDependsList = new dmsDependsList;


                    dmsDepends *poDepends = new dmsDepends;
                    m_poDependsList->Append(poDepends);

                    wxString tmp = values2[j];
                    if (tmp[0]=='!') { poDepends->m_bNegation=true; tmp.Remove(0,1); }
                    if (!StrToInt(tmp, poDepends->m_iChoice)) poDepends->m_iChoice = 0;
        }


    }
    k=0;

    delete [] values;
}
/*
void dmsDialogContext::Enable(int mode, bool value)
{
    if (mode==0) // Show
    {
        if (m_poWin) m_poWin->Show(value);
    }
    else // Enable
    {
        if (m_poWin) m_poWin->Enable(value);
    }

    dmsDialogContext* ctx;

    FOREACH(dmsDialogContextList, *m_poChildList, ctx)
    {
        ctx->Enable(mode, value);
    }
}
*/
bool dmsDialogContext::IsEnabled()
{
    if (m_poWin==NULL) return true;

    return m_poWin->IsShown() && m_poWin->IsEnabled();
}

void dmsDialogContext::ApplyDepends(int choice)
{
    if (m_poDependingList==NULL) return;

    m_iDependsState = choice;

    dmsDialogContext* ctx;

    FOREACH(dmsDialogContextList, *m_poDependingList, ctx)
    {
        dmsDepends* poDepends;
        bool on = true;
        FOREACH(dmsDependsList, *(ctx->m_poDependsList), poDepends)
        {
            on = (poDepends->m_iChoice==m_iDependsState);
            if (poDepends->m_bNegation) on = !on;
            if (!on) break;
        }

        if (ctx->m_oType=="StaticBox")
        {
            wxStaticBoxSizer* sizer = (wxStaticBoxSizer*) ctx->m_poSizer;
            wxStaticBox*      box   = sizer->GetStaticBox();

            ctx->m_poParentSizer->Show(sizer, on);
            ctx->Enable(on);
            box->Show(on);

            //ctx->m_poParentWin->Fit();
            //ctx->m_poParentSizer->Layout();
        }
        else
        {
            ctx->m_poWin->Enable(on);
            ctx->Enable(on);
            //ctx->m_poWin->Show(on);
        }
    }

    if (m_oType == "RadioBox")
    {
        wxRadioBox* win = (wxRadioBox*) m_poWin;

        win->SetSelection(choice);
    }

}


void dmsDialogContext::ShowError()
{
    dmsDialogContext *node = this;

    while (node)
    {
        if (node->m_oType=="Page")
        {
            wxNotebook* notebook = (wxNotebook*) node->m_poParent->m_poWin;

            for (unsigned int i=0; i<notebook->GetPageCount(); i++)
            {
                if (notebook->GetPage(i) == node->m_poWin)
                {
                    notebook->SetSelection(i);
                    break;
                }
            }
        }
        else if (node->m_poDependsContext)
        {
            dmsDepends* poDepends;

            FOREACH(dmsDependsList, *(node->m_poDependsList), poDepends)
            {
                if (!poDepends->m_bNegation)
                    node->m_poDependsContext->ApplyDepends(poDepends->m_iChoice);
            }
        }
        node = node->m_poParent;
    }

    wxColour old = m_poWin->GetBackgroundColour();

    m_poWin->SetBackgroundColour(wxColour(255,255,0));
    m_poWin->Refresh(true);

    wxMessageDialog dialog(NULL,
        m_poValidator->GetError(),
        "Input error",
        wxOK | wxCENTRE | wxICON_ERROR);

    dialog.ShowModal();

    m_poWin->SetBackgroundColour(old);
    m_poWin->Refresh(true);
    m_poWin->SetFocus();
}

/* ########################################################################

   ######################################################################## */

wxMySizer::wxMySizer()
{
}

void wxMySizer::RecalcSizes()
{
    if (m_children.GetCount() == 0)
        return;

    int delta = 0;
    int extra = 0;
    if (m_stretchable)
    {
        if (false)
        {
            delta = (m_size.x - m_fixedWidth) / m_stretchable;
            extra = (m_size.x - m_fixedWidth) % m_stretchable;
        }
        else
        {
            delta = (m_size.y - m_fixedHeight) / m_stretchable;
            extra = (m_size.y - m_fixedHeight) % m_stretchable;
        }
    }

    wxPoint pt( m_position );

    wxwxSizerItemListNode *node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem *item = (wxSizerItem*) node->GetData();
        if (item->IsShown())
        {
            int weight = 1;
            if (item->GetProportion())
                weight = item->GetProportion();

            wxSize size( item->CalcMin() );

            if (true)
            {
                wxCoord height = size.y;
                if (item->GetProportion())
                {
                    height = (delta * weight) + extra;
                    extra = 0; // only the first item will get the remainder as extra size
                }

                wxPoint child_pos( pt );
                wxSize  child_size( wxSize( size.x, height) );

                if (item->GetFlag() & (wxEXPAND | wxSHAPED))
                    child_size.x = m_size.x;
                else if (item->GetFlag() & wxALIGN_RIGHT)
                    child_pos.x += m_size.x - size.x;
                else if (item->GetFlag() & (wxCENTER | wxALIGN_CENTER_HORIZONTAL))
                // XXX wxCENTER is added for backward compatibility;
                //     wxALIGN_CENTER should be used in new code
                    child_pos.x += (m_size.x - size.x) / 2;

                item->SetDimension( pt, child_size );

                pt.y += 0;
            }
            else
            {
                wxCoord width = size.x;
                if (item->GetProportion())
                {
                    width = (delta * weight) + extra;
                    extra = 0; // only the first item will get the remainder as extra size
                }

                wxPoint child_pos( pt );
                wxSize  child_size( wxSize(width, size.y) );

                if (item->GetFlag() & (wxEXPAND | wxSHAPED))
                    child_size.y = m_size.y;
                else if (item->GetFlag() & wxALIGN_BOTTOM)
                    child_pos.y += m_size.y - size.y;
                else if (item->GetFlag() & (wxCENTER | wxALIGN_CENTER_VERTICAL))
                // XXX wxCENTER is added for backward compatibility;
                //     wxALIGN_CENTER should be used in new code
                    child_pos.y += (m_size.y - size.y) / 2;

                item->SetDimension( child_pos, child_size );

                pt.x += width;
            }
        }

        node = node->GetNext();
    }
}

wxSize wxMySizer::CalcMin()
{
    if (m_children.GetCount() == 0)
        return wxSize(10,10);

    m_stretchable = 0;
    m_minWidth = 0;
    m_minHeight = 0;
    m_fixedWidth = 0;
    m_fixedHeight = 0;

    // Find how long each stretch unit needs to be
    int stretchSize = 1;
    wxwxSizerItemListNode *node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem *item = (wxSizerItem*) node->GetData();
        if (item->IsShown() && item->GetProportion() != 0)
        {
            int stretch = item->GetProportion();
            wxSize size( item->CalcMin() );
            int sizePerStretch;
            // Integer division rounded up is (a + b - 1) / b
            if (false)
                sizePerStretch = ( size.x + stretch - 1 ) / stretch;
            else
                sizePerStretch = ( size.y + stretch - 1 ) / stretch;
            if (sizePerStretch > stretchSize)
                stretchSize = sizePerStretch;
        }
        node = node->GetNext();
    }
    // Calculate overall minimum size
    node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem *item = (wxSizerItem*) node->GetData();
        if (item->IsShown())
        {
            m_stretchable += item->GetProportion();

            wxSize size( item->CalcMin() );
            if (item->GetProportion() != 0)
            {
                size.x = stretchSize * item->GetProportion();
                size.y = stretchSize * item->GetProportion();
            }

            if (false)
            {
                m_minWidth += size.x;
                m_minHeight = wxMax( m_minHeight, size.y );
            }
            else
            {
                m_minHeight = wxMax( m_minHeight, size.y );
                m_minWidth = wxMax( m_minWidth, size.x );
            }

            if (item->GetProportion() == 0)
            {
                if (true)
                {
                    m_fixedHeight = wxMax( m_fixedHeight, size.y );
                    m_fixedWidth = wxMax( m_fixedWidth, size.x );
                }
                else
                {
                    m_fixedWidth += size.x;
                    m_fixedHeight = wxMax( m_fixedHeight, size.y );
                }
            }
        }
        node = node->GetNext();
    }

    return wxSize( m_minWidth, m_minHeight );
}

IMPLEMENT_CLASS(wxMySizer, wxSizer)
