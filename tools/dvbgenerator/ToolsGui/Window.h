/* ************************************************************************
   SmarDTV

   Description : Extensions au systeme de fenetrage de wxWindows

   Historique :
   - COF   - Iwedia  - v 0    - 05/2003 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _TOOLS_WINDOW_H_
#define _TOOLS_WINDOW_H_

#include <wx/wx.h>
#include <wx/listctrl.h>

#include <Tools/Tools.h>

#include "Dialog.h"

void dmsLoadGeometry(wxWindow *win, const wxString &path, bool bOnlyPos = false);
void dmsSaveGeometry(wxWindow *win, const wxString &path);
void dmsLoadGeometry(wxFrame *frame, const wxString &path, bool bOnlyPos = false);
void dmsSaveGeometry(wxFrame *frame, const wxString &path);


void dmsConfigWriteInt(const wxString &path, int value);
void dmsConfigReadInt(const wxString &path, int &value, int defaultVal);
void dmsConfigWriteString(const wxString &path, const wxString &value);
void dmsConfigReadString(const wxString &path, wxString &value, const wxString &defaultVal);
void dmsConfigWriteBool(const wxString &path, bool value);
void dmsConfigReadBool(const wxString &path, bool &value, bool defaultVal);

void dmsMoveVisible(wxWindow *win, const wxPoint &point);

void dmsPostOnSize(wxWindow* win);

void AddIcoMenuItem(wxMenu* menu, int id, const wxString &name, const wxString &icoName);



/* ========================================================================
   Applications au look MDI Dms
   ======================================================================== */

/* ------------------------------------------------------------------------
   Fenetres MDI
   ------------------------------------------------------------------------ */

class dmsMDIChildFrame;
WX_DECLARE_LIST(dmsMDIChildFrame, dmsMDIChildFrameList);

// Fenetre mere

/**
    \class dmsMDIParentFrame
    \brief Outils de gestion des fenetres parentes MDI

     - Sauvegarde automatique de la liste des fenetre filles ouvertes
     - Gestion automatique du menu permettant l'ouverture des fenêtres filles
*/

class dmsMDIParentFrame : public wxMDIParentFrame
{
    friend dmsMDIChildFrame;
public:
    typedef enum {TILE_NONE, TILE_HORIZONTALY, TILE_VERTICALY} TileDir;
    wxSizer* m_poSizer;

public:
    dmsMDIChildFrameList m_oChildList;
    TileDir              m_eTileDir;

public:
    wxMenuBar *m_poMenuBar;

public:
    dmsMDIParentFrame(const wxString& title);
    virtual ~dmsMDIParentFrame();

    void GetIdList(intList &list);
    void LoadIdList(intList &list);
    void SaveIdList();

    void Load();
    void OpenChild(int id);

    void PostOpenChild(int id);
    void PostOpenChild(const intList &list);
    void SaveOpenChild(dmsMDIChildFrame *child);
    void SaveCloseChild(dmsMDIChildFrame *child);

    bool ChildActive(int id);
    dmsMDIChildFrame *FindChild(int id);

    virtual void SetSizer();

    DECLARE_EVENT_TABLE();

    void OnSize(wxSizeEvent& event);
};

// Fenetres filles

/**
    \class dmsMDIChildFrame
    \brief Outils de gestion des fenetres filles MDI
*/


class dmsMDIChildFrame : public wxMDIChildFrame
{
public:
    dmsMDIParentFrame *m_poParent;
    wxString          m_oTitle;
    int               m_iId;

  public:
    dmsMDIChildFrame(dmsMDIParentFrame* parent, const wxString &title, int id);
    virtual ~dmsMDIChildFrame();

    void *AddChild(const wxString &Name);

    DECLARE_EVENT_TABLE();

    void OnCloseWindow(wxCloseEvent& event);
};


/* ------------------------------------------------------------------------
   ListControl avec menu "add, copy, modify, delete"
   ------------------------------------------------------------------------ */

typedef enum
{
    ACMD_EVENT_NONE = 0,
    ACMD_EVENT_NEW,
    ACMD_EVENT_COPY,
    ACMD_EVENT_MODIFY,
    ACMD_EVENT_DELETE,
    ACMD_EVENT_USER,
} EnumAcmdEvent;



typedef void (*OnAcmdEventCallback)(void *element, EnumAcmdEvent event);

/**
    \class dmsAcmdListCtrl
    \brief ListControl avec menu contextuel "ajout, copy, modification, suppression"
*/

class dmsAcmdListCtrl : public wxListCtrl
{
protected:
    wxMenu *m_poMenu;
    bool   m_bInsertAtEnd;
    int    m_iMaxSize;

protected:
    wxList m_oSelectedDataList;

public:
    dmsAcmdListCtrl(wxWindow *parent);
    virtual ~dmsAcmdListCtrl();

    virtual void Update(int item) = 0;
    virtual bool DoCommand(void *elt, int cmd) = 0;
    virtual void SetMenu(void *elt);

    void SetSelectedDataList();

    void OnColClick(wxListEvent& event);
    void OnRightDown(wxMouseEvent &event);
    void OnActivated(wxListEvent& event);
    void OnMenu(wxCommandEvent& event);
    void OnChar(wxKeyEvent& event);

    void OnChanged(void *elt, EnumAcmdEvent event);
    static void CallbackEvent(void *elt, EnumAcmdEvent event);

    void *poGetFocused();
    void *poGetFirstSelected();
    wxListCtrlCompare m_poCompareCallback;
    void ApplyEvent(int id);

    void UpdateAll();
    void ClearAll();
    void ClearItem(int item);

    DECLARE_EVENT_TABLE();
};

/* ========================================================================

   ======================================================================== */


/**
    \class dmsTextDialog
    \brief Dialog composé uniquement d'un TextControl
*/


class dmsTextDialog: public dmsDialog
{
public:
    wxString m_oTitle;

public:
    dmsTextDialog(const wxString& title, const wxString &message, int icon);
    virtual ~dmsTextDialog();

    void OnEvent(dmsDialogContext* ctx);
};



#endif /* _TOOLS_WINDOW_H_ */
