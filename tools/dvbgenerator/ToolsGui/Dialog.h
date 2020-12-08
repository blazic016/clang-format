/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _DMS_TOOLS_DIALOG_H_
#define _DMS_TOOLS_DIALOG_H_


#include <wx/wx.h>
#include <wx/calctrl.h>

#include <Tools/Validator.h>

class wxXmlNode;
class dmsValidator;

class dmsDialogContext;
class dmsDialogContextManager;
class wxNotebookEvent;
class dmsDepends;

WX_DECLARE_LIST(dmsDialogContext, dmsDialogContextList);
WX_DECLARE_LIST(dmsDepends, dmsDependsList);

/* ------------------------------------------------------------------------
   Class dmsDialog
   ------------------------------------------------------------------------ */

   /**
   \class dmsDialog
   \brief Outils de génération de boîtes de dialog

   Génère une boite de dialogue à partir d'une description XML.
   Permet :
   - La création automatique de composants graphiques à partir de tags xml
   - La validation des champs xml à partir des outils "dmsValidator"
   - Le mapping (optionnel) avec la conf à partir des outils "dmsValidator"
   */

class dmsDialog: public wxDialog
{
public:
    dmsDialogContextManager* m_poManager;

public:
    dmsDialog(wxWindow *parent, const wxString &title,
        wxWindowID id = -1,
        long style = /*wxDIALOG_MODELESS |*/ wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxNO_FULL_REPAINT_ON_RESIZE,
        const wxString& name = "dialogBox");

    virtual ~dmsDialog();

    virtual void OnEvent(dmsDialogContext* ctx) = 0;

    DECLARE_EVENT_TABLE();

    void OnClose(wxCloseEvent& event);
};

/* ------------------------------------------------------------------------
   dmsDialogContextManager
   ------------------------------------------------------------------------ */

   /**
   \class dmsDialogContextManager
   \brief Gestionnaire de validation pour une IHM de type \c dmsDialogContext
   */

class dmsDialogContextManager : public dmsvContextManager
{
public:
    wxWindow*         m_poWindow;
    wxSizer*          m_poSizer;

    dmsDialogContext* m_poErrorCxt;
    dmsDialogContext* m_poGetCxt;
    dmsDialogContext* m_poUpdated;

public:
    dmsDialogContextManager(wxWindow *parent);
    virtual ~dmsDialogContextManager();

    bool Load(wxXmlNode *node);
    bool Load(const wxString &xmlFilename);
    bool LoadResource(const wxString &resourceName);
    bool LoadText(const wxString &text);

    //dmsDialogContext* Find(wxWindow* win);

    bool DoAction(dmsDialogContext* ctx);
    void OnEvent(dmsDialogContext* ctx);
};


class dmsCliContextManager : public dmsvContextManager
{
public:
	dmsCliContextManager();
	virtual ~dmsCliContextManager();

    bool Load(wxXmlNode *node);
    bool Load(const wxString &xmlFilename);
    bool LoadResource(const wxString &resourceName);
    bool LoadText(const wxString &text);
};


/* ------------------------------------------------------------------------
   dmsDialogRootPanel
   ------------------------------------------------------------------------ */

   /**
   \class dmsDialogRootPanel
   \brief Gestionnaire d'évenements pour une IHM de type \c dmsDialogContext
   */

class dmsDialogRootPanel : public wxPanel
{
public:
    dmsDialogContextManager *m_poManager;


public:
    dmsDialogRootPanel(dmsDialogContextManager *manager, wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "panel");
    virtual ~dmsDialogRootPanel();

    DECLARE_EVENT_TABLE();

    void OnTextUpdated(wxCommandEvent& event);
    void OnButton(wxCommandEvent& event);
    void OnChoice(wxCommandEvent& event);
    void OnCheckBox(wxCommandEvent& event);
    void OnRadioBox(wxCommandEvent& event);
    void OnListBoxSelected(wxCommandEvent& event);
    void OnListBoxValidated(wxCommandEvent& event);
    void OnComboBox(wxCommandEvent& event);
    void OnNotebookPageChanged(wxNotebookEvent& event);
    void OnChildFocus(wxChildFocusEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnCalendarSelChanged(wxCalendarEvent& event);

    bool DoFocus();
};

/* ------------------------------------------------------------------------
   dmsDepends
   ------------------------------------------------------------------------ */

/**
   \class dmsDepends
   \brief Gestion des dépendances entre objets \c dmsDialogContext
*/


class dmsDepends
{
public:
    int     m_iChoice;
    bool    m_bNegation;

public:
    dmsDepends() { m_iChoice = 0; m_bNegation = false; }
};


/**
   \class dmsDialogContext
   \brief Validation d'un composant graphique
*/


class dmsDialogContext : public dmsvContext
{
public:
    dmsDialogContext*        m_poParent;
    wxWindow*                m_poWin;
    wxWindow*                m_poParentWin;
    wxSizer*                 m_poSizer;
    wxSizer*                 m_poParentSizer;
    wxString                 m_oType;
    wxString*                m_poChoiceList;
    wxString                 m_oAction;
    wxString                 m_oRef;

    wxString              m_oDepends;
    dmsDialogContext*     m_poDependsContext;
    dmsDependsList*       m_poDependsList;
    int                   m_iDependsState;
    dmsDialogContextList* m_poDependingList;
    dmsDialogContextList* m_poChildList;

public:
    dmsDialogContext(dmsvContextManager* manager, dmsValidator *validator, dmsDialogContext* parent, wxWindow *parentWin, wxSizer* parentSizer);
    virtual ~dmsDialogContext();

    bool Load(wxXmlNode *node);

    bool GetValue(wxString &value);
    bool SetValue(const wxString &value);

    void InitComboBox(const wxArrayString &values, int index);

    //void Enable(int mode, bool value);
    bool IsEnabled();

    void InitDepends();
    void ApplyDepends(int choice);

    void ShowError();
};


class dmsCliContext : public dmsvContext
{
public:
    wxString          		 m_oType;
    wxString                 m_oAction;
    wxString                 m_oRef;
    wxString                 m_oDepends;
    wxString*                m_poChoiceList;
public:
	dmsCliContext(dmsvContextManager* manager, dmsValidator* validator);
	virtual ~dmsCliContext();

	bool Load(wxXmlNode *node);
    bool GetValue(wxString &value);
    bool SetValue(const wxString &value);
};



/**
   \class wxMySizer
   \brief Gestion des dépendances entre objets \c dmsDialogContext

   Code récupéré sur internet
*/


class WXDLLEXPORT wxMySizer: public wxSizer
{
public:
    wxMySizer();

    void RecalcSizes();
    wxSize CalcMin();

protected:
    int m_stretchable;
    int m_minWidth;
    int m_minHeight;
    int m_fixedWidth;
    int m_fixedHeight;

private:
    DECLARE_CLASS(wxBoxSizer);
};




#endif /* _DMS_TOOLS_DIALOG_H_ */
