/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 09/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DIALOG_STACK_H_
#define _DIALOG_STACK_H_


#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/regex.h>


class dmsWindowStackDisplay;
class wxXmlNode;

typedef enum
{
    WS_DISPLAY_TYPE_UNKNOWN = 0,
    WS_DISPLAY_TYPE_CHECKBOX,
    WS_DISPLAY_TYPE_CHOICE,
} EnumWSDisplayType;


typedef enum
{
    WS_DISPLAY_MODE_UNKNOWN = 0,
    WS_DISPLAY_MODE_ENABLE,
    WS_DISPLAY_MODE_SHOW,
} EnumWSDisplayMode;


/* ------------------------------------------------------------------------
   dmsWindowBoxSizer
   ------------------------------------------------------------------------ */

/**
   \deprecated remplacé par dmsDialog
   \class dmsWindowBoxSizer
   \brief (obsolete) Redimensionnement automatique des boîtes de dialogues
   */


class dmsWindowBoxSizer: public wxBoxSizer
{
public:
    dmsWindowBoxSizer(int orient = wxVERTICAL, wxWindow *win = NULL);

    void RecalcSizes();
    int  iSetWindow(wxWindow *win);

    wxWindow *GetWindow() { return m_Window; }
protected:
    wxWindow   *m_Window;
};





/* ------------------------------------------------------------------------
   dmsWindowStack
   ------------------------------------------------------------------------ */

/**
   \deprecated remplacé par dmsDialog
   \class dmsWindowStack
   \brief (obsolete) Outils de contruction de boites de dialogue
   */


class dmsWindowStack
{
  public:
      typedef enum {
          MODE_UNKNOWN = 0,
          MODE_DEFAULT,
          MODE_STATIC_BOX,
          MODE_SUNKEN_BORDER,
          MODE_RAISED_BORDER,
          MODE_SCROLLBAR,
          MODE_NOTEBOOK,
      } EnumMode;

      typedef enum {
          ID_BUTTON_OK = 1,
          ID_BUTTON_CANCEL,
          ID_BUTTON_USER_DEFINED,
      } EnumCtrlID;

  private:
      wxList                  m_oWindowList;
      wxList                  m_oButtonList;

  public:
    wxString                  m_oName;
    wxPoint                   pos;
    wxSize                    size;
    EnumMode m_eMode;

    wxWindow                  *m_poParent;
    wxWindow                  *m_poGlobalWin;
    wxWindow                  *m_poDataWin;
    wxWindow                  *m_poSubDataWin;
    wxWindow                  *m_poButtonWin;

    wxBoxSizer                *m_poParentSizer;
    wxBoxSizer                *m_poGlobalSizer;
    wxBoxSizer                *m_poDataSizer;
    wxBoxSizer                *m_poButtonSizer;
    wxBoxSizer                *m_poButtonHSizer;
    wxButton                  *m_poButtonOk;
    wxButton                  *m_poButtonCancel;

    bool                      m_bCurrentLabelResizable;
    bool                      m_bExpanded;
    bool                      m_bResizable;
    bool                      m_bHResizable;
    bool                      m_bCancel;
    bool                      m_bEnableClose;

    dmsWindowStackDisplay     *m_poDisplayRoot;

  public:
    dmsWindowStack();
    virtual ~dmsWindowStack();

    wxWindow *Win();

    // Modes
    void Init(wxWindow *parent);

    void SetMode(EnumMode mode);
    void SetStaticBoxMode();
    void SetSunkenBorderMode();
    void SetRaisedBorderMode();
    void SetScrollBarMode();
    void SetNoteBookMode();
    void SetPage(int page);
    void SetLineSpring(bool value);
    int  GetPage();

    void FitSizer(wxWindow *win, wxSizer *sizer);
    void AddButton(wxWindow *button);
    void AddButtonOkCancel();
    void FitWin();

    void Add(wxWindow *win, bool expand = false, bool hexpand = false);
    void Add(const wxString &label, wxWindow *win, bool expand = false, bool hexpand = false);
    wxStaticText* Add(const wxString &label);
    wxStaticText* Add(const wxString &label, const wxFont &font, const wxSize &size);
    void Add2(wxWindow *win1, const wxString &label, wxWindow *win2);
    void AddSizer(wxSizer *sizer, bool expand = false, bool hexpand = false);
    void AddLine();
    void NewLine(bool expand = false);

    wxBoxSizer *m_poUserLine;
    int        m_iUserLineLength;
    bool       m_bUserLineExpand;
    bool       m_bUserLineHExpand;

    void BeginLine();
    void AddInLine(wxWindow *win, const wxString label = " ", bool expand = false, bool hexpand = false);
    void EndLine();

    void EndOfPage();
    void AddPageStd(const wxString &label);
    wxPanel *AddPage(const wxString &label, bool expanded = true);

    void TracePositions();


    virtual bool Ok()                     { return true; }
    virtual bool Cancel()                 { return true; }
    virtual void Choice(int id)           { return; }
    virtual void CheckBox(int id)         { return; }
    virtual void RadioButton(int id)      { return; }
    virtual void ListBoxSelected(int id)  { return; }
    virtual bool ListBoxValidated(int id) { return true; }
    virtual void ExtraButton(int id)      { return; }
    virtual wxWindow *GetWindow(int id)   { return NULL; }
    virtual bool ValidateCtrl(wxTextCtrl *textctrl, const wxString &value) { return true; }

  public:
    void SetScrollHeight(int height);
    void SetResizable(int height);
    void EnableClose(bool value);

  private:

    void AddWin(wxWindow *win, bool expand = false, bool hexpand = false);

    int m_iScrollBarHeight;

  public:
      void AddDisplay(wxCheckBox *win, EnumWSDisplayMode mode);
      void AddDisplay(wxChoice *win, EnumWSDisplayMode mode);
      void AddDisplay(wxWindow *parent, int parentValue, wxWindow *child);
      void AddDisplay(wxWindow *parent, int parentValue, wxCheckBox *child);
      void UpdateDisplay(int windowId);

  public:
      wxTextCtrl *AddTextCtrl(const wxString &label, int size, const wxString &value, const wxValidator &validator = wxDefaultValidator);
      void AddMultiTextCtrl(const wxString &label, wxTextCtrl *&ctrl, int size, int hsize, const wxString &value, const wxValidator &validator = wxDefaultValidator);
      void AddTextCtrl(const wxString &label, wxTextCtrl *&ctrl, int size, int value, const wxString &unit = "");
      void AddStaticText(const wxString &label, const wxString &value);
      void AddChoice(const wxString &label, wxChoice *&ctrl, int size, int index, const char **values);
      void AddChoice(const wxString &label, wxChoice *&ctrl, int size, const wxString &value, const wxStringList &values);
      void AddComboBox(const wxString &label, wxComboBox *&ctrl, int size, const wxString &value, const wxStringList &values);
      void AddCheckBox(const wxString &label, wxCheckBox *&ctrl, bool value);
};



/* ------------------------------------------------------------------------
   dmsDialogStack
   ------------------------------------------------------------------------ */

/**
   \deprecated remplacé par dmsDialog
   \class dmsDialogStack
   \brief (obsolete) Boite de dialogue
   */


class dmsDialogStack: public wxDialog, public dmsWindowStack
{
public:
    dmsDialogStack(wxWindow *parent, const wxString &title,
        wxWindowID id = -1,
        long style = /*wxDIALOG_MODELESS |*/ wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
        const wxString& name = "dialogBox");

    ~dmsDialogStack();

    void OnCloseWindow(wxCloseEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnChoice(wxCommandEvent& event);
    void OnCheckBox(wxCommandEvent& event);
    void OnRadioButton(wxCommandEvent& event);
    void OnListBoxSelected(wxCommandEvent& event);
    void OnListBoxValidated(wxCommandEvent& event);
    void OnExtraButton(wxCommandEvent& event);
    wxWindow *GetWindow(int id);

    void PostOk();

public:
    DECLARE_EVENT_TABLE();
};



/**
   \deprecated remplacé par dmsDialog
   \class dmsFrameStack
   \brief (obsolete) Frame avec un contenu de type "Boite de dialogue"
   */


class dmsFrameStack: public wxFrame, public dmsWindowStack
{
public:
    dmsFrameStack(wxWindow *parent, const wxString &title, long style = wxDEFAULT_FRAME_STYLE);

    ~dmsFrameStack();

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnChoice(wxCommandEvent& event);
    void OnCheckBox(wxCommandEvent& event);
    void OnRadioButton(wxCommandEvent& event);
    void OnListBoxSelected(wxCommandEvent& event);
    void OnListBoxValidated(wxCommandEvent& event);
    wxWindow *GetWindow(int id);

public:
    DECLARE_EVENT_TABLE();
};


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

/**
   \deprecated remplacé par dmsDialog
   \class dmsWindowStackDisplay
   \brief (obsolete) Element d'une boite de dialogue
   */


WX_DECLARE_LIST(dmsWindowStackDisplay, dmsWindowStackDisplayList);

class dmsWindowStackDisplay
{
private:
    wxWindow                  *m_poWin;       // L'identifiant du control
    EnumWSDisplayType         m_eType;        // Le type du control
    EnumWSDisplayMode         m_eMode;        // Le mode d'affichage/masquage
    dmsWindowStackDisplayList m_oList;        // Les noeuds fils
    dmsWindowStackDisplay     *m_poParent;    // Le noeud père
    int                       m_iParentValue; // La valeur d'activation du père
    bool                      m_bActivated;
    int                       m_iValue;

public:
    dmsWindowStackDisplay(dmsWindowStackDisplay *parent, wxWindow *win);
    ~dmsWindowStackDisplay();

    dmsWindowStackDisplay *Find(wxWindow *win);

    void Add(wxWindow *win, EnumWSDisplayType type, EnumWSDisplayMode mode);
    void Add(wxWindow *parent, int parentValue, wxWindow *child, EnumWSDisplayType type=WS_DISPLAY_TYPE_UNKNOWN);

    void Update();
    void Update(wxWindow *win);
    void Display(bool activated);

    void Trace(const wxString &depth="");
};


/* ------------------------------------------------------------------------
   dmsValidator2
   ------------------------------------------------------------------------ */

/**
   \deprecated remplacé par dmsDialog
   \class dmsValidator2
   \brief (obsolete) Validation d'IHM

   Code inspiré de wxTextValidator

   Spécificités :
   - Control le champ après chaque touche appuyée
   - La méthode de validation peut être dérivé d'une dialogue stack
 */

class dmsValidator2: public wxValidator
{
public:
    dmsWindowStack *m_poWindowStack;
    wxColour       *m_poLastColour;

public:
    dmsValidator2();
    dmsValidator2(dmsWindowStack *window);
    dmsValidator2(const dmsValidator2& val);

    ~dmsValidator2();

    virtual wxObject *Clone() const { return new dmsValidator2(*this); }
    virtual bool Validate(wxWindow *parent);
    virtual bool ValidateHot(const wxString &value) {return true;}

    virtual bool TransferToWindow() {return true;}
    virtual bool TransferFromWindow() {return true;}

    bool Copy(const dmsValidator2& val);
    void OnChar(wxKeyEvent& event);

DECLARE_EVENT_TABLE()
};





/* ------------------------------------------------------------------------
   dmsRegExValidator
   ------------------------------------------------------------------------ */

/**
   \deprecated remplacé par dmsDialog
   \class dmsRegExValidator
   \brief (obsolete) Validation de type "expression rationnelle"

   Code inspiré de wxTextValidator
*/

class dmsRegExValidator: public dmsValidator2
{
protected:
    wxString m_oRegExStr;
    wxRegEx  m_oRegEx;

public:
    dmsRegExValidator(const wxString &regex);
    dmsRegExValidator(dmsWindowStack *window, const wxString &regex);
    dmsRegExValidator(const dmsRegExValidator& val);

    virtual ~dmsRegExValidator();

    virtual wxObject *Clone() const { return new dmsRegExValidator(*this); }
    bool Copy(const dmsRegExValidator& val);

    virtual bool ValidateHot(const wxString &value);
};



/* ------------------------------------------------------------------------
   dmsValidErrorDialog
   ------------------------------------------------------------------------ */


/**
   \deprecated remplacé par dmsDialog
   \class dmsValidErrorDialog
   \brief (obsolete) Boîte de dialogue affichant les messages d'erreur
*/

class dmsValidErrorDialog : public wxDialog
{
public:
    dmsValidErrorDialog(wxWindow *ctrl, const wxString &message);
    virtual ~dmsValidErrorDialog();

    void OnOk(wxCommandEvent& event);

    DECLARE_EVENT_TABLE();
};


#endif /* _DIALOG_STACK_H_ */

