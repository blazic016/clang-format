/****************************************************************************
** @file MainFrame.h
**
** @brief
**   Management user input: Read, check and XML conversion in TS Writer format.
**
** @ingroup GENIUS USER INTERFACE
**
** @see MainFrame.xml and TS_Writer.xml
**
** @version $Rev: 62361 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/soft/ISSU_Generator/MainFrame.h $
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
**   LIPPA - SmarDTV - v 2.10 - 01/2012 - Add Usage ID in PMT in Protocol ID 3
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
**
******************************************************************************/

#ifndef _MAIN_FRAME_H_
#define _MAIN_FRAME_H_


#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/file.h>

#include <Tools/Event.h>

#include <ToolsGui/Dialog.h>

/* ========================================================================
   Declarations
   ======================================================================== */



class dmsMain
{
protected:
	bool shouldUseWait = true;

	virtual void printGenerationMsg(const wxString& msg, int icon) const = 0;
public:
	/* Who inherits or use this class it's up to him to initialize m_poManager*/
	dmsvContextManager* m_poManager;
    bool AssumeProfilSaved (const wxString &name="");
    bool CreateStream      ();
    dmsMain();
private:
    wxString m_oConf;

   bool ReplaceConf                  ();
   bool ReplaceDefinedImagesBasic    (int numImages, int blockSize);
   bool ReplaceDefinedImagesByZoneId (int numImages, int blockSize);
   void ReplaceUndefinedImages       (int numImages);
   bool ReplaceDefinedAddStreams     (int numStreams);
   void ReplaceUndefinedAddStreams   (int numStreams);
   int  GetPartitionSize             (int partitionId);
   int  GetPartitionLogicalAddr      (int partitionId);
   bool ReplaceItem                  (const wxString &name, const char* value);
   bool ReplaceItem                  (const wxString &name, bool  value);
   bool ReplaceItem                  (const wxString &name, int   value);

};




/**
    \brief Fenetre principale. Lance la génération xml, réalise le mapping, lance les actions (génération flux, ...)
*/

class dmsMainFrame: public dmsDialog, public dmsMain
{
	void printGenerationMsg(const wxString& msg, int icon) const override;
public:
	using dmsDialog::m_poManager;
    dmsMainFrame(const wxString& title);
    virtual ~dmsMainFrame();

    void OnEvent           (dmsDialogContext* ctx);

};


class dmsMainCli : public dmsMain
{
	void printGenerationMsg(const wxString& msg, int icon) const override;
public:
    dmsCliContextManager* m_poManager;
    dmsMainCli(const wxString& title);

};


extern dmsMainFrame *g_poMainFrame; // Fenetre principale (Appli accessible par wxGetApp())





#endif /* _MAIN_FRAME_H_ */

/* MainFrame.h */
