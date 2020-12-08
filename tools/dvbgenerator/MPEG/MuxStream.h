/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   - LIPPA - SmarDTV - v 2.12 - 02/2012 - DSI repetition all seconds.
   - LIPPA - SmarDTV - v 2.13 - 03/2012 - Generate raw sections in a file and
                                          choice generated files extensions.
   - LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
                                          Header in sections file
   ************************************************************************ */

#ifndef _LIB_MPEG_MULTIPLEX_STREAM_H_
#define _LIB_MPEG_MULTIPLEX_STREAM_H_

#include <MPEG/MPEG.h>

class wxXmlNode;
class wxFile;
class dmsMultiplexer;
class dmsMuxStream;

/* ------------------------------------------------------------------------
   dmsMuxStream
   ------------------------------------------------------------------------ */


WX_DECLARE_LIST(dmsMuxStream, dmsMuxStreamList);


class dmsMuxStream
{
public:
    dmsMultiplexer* m_poMux;
    wxString        m_oLabel;
    int             m_iPID;
    int             m_iRate;

    wxString m_oGroupName;

    bool m_bCarryPCR;                     // Contient la PCR

    // Supervision du play
    int m_iPlayLap;                       // Nombre de fois ou le flux à bouclé
    u64 m_iPlayPcr;                       // Heure du prochain paquet
    u64 m_iPlayCount;                     // Nombre d'octets émis
    u8  m_iPlayContinuityCounter;         // Le "continuity counter" de mpeg

    bool m_bFromTemplate; // Indique si le noeud est crée à partir du template (pid=-1)


public:
    dmsMuxStream(dmsMultiplexer *mux);
    virtual ~dmsMuxStream();

    bool Load(wxXmlNode *node);
    virtual bool V_Load(wxXmlNode *node)=0;

    bool PlayStart();
    bool PlayNext(u8* buffer);
    void PlayStop();
    void PlayNextCount();

    virtual bool Open()=0;
    virtual void Close()=0;
    virtual bool ReadTS(u8* buffer)=0;
    virtual bool EofTS()=0;
    virtual void Reset()=0;
};


/* ------------------------------------------------------------------------
   dmsMuxStreamSection
   ------------------------------------------------------------------------ */

class dmsMuxStreamSectionGroupList;
class dmsMuxStreamSectionGroup;
class dmsMuxStreamSectionMuxedItemList;
class dmsMuxStreamSectionMuxedItem;
class dmsMuxStreamSectionItem;
class dmsMuxStreamSection;

WX_DECLARE_LIST(dmsMuxStreamSection, dmsMuxStreamSectionList);

class dmsMuxStreamSection : public dmsMuxStream
{
public:
    typedef enum {
        Asap,
        Repeat,
        Stretch,
    } MultiplexMode;

public:
   wxString                    m_oName;
    int                         m_iWriteCount;
    int                         m_iMaxGroupLen;
    MultiplexMode               m_eMultiplexMode;

public: // le Play
    void* m_poPlayNode;
    int   m_iPlayOffset;

public:
    dmsMuxStreamSection(dmsMultiplexer *mux);
    virtual ~dmsMuxStreamSection();

    dmsMuxStreamSectionGroupList*     m_poGroupList;
    dmsMuxStreamSectionMuxedItemList* m_poMuxedList;

    bool V_Load(wxXmlNode *node);

    void Clear();


    int GetNextGroup();
    int GetSectionCount();

    void Generate(wxFile &file, dmsBuffer &Buffer, int pid);
    void Generate(const wxString &Filename, dmsBuffer &Buffer, int pid);
    void Generate(const wxString &Filename, dmsMPEG_MuxSection &section);
    bool Generate(const wxString &Filename, const wxString &traceFileName);
    void GenerateFrequent(wxFile &file, bool trace, wxFFile &traceFile, int &frequentOffset, int &groupOffset, int &lineOffset);

    void Add(dmsMPEG_MuxSection &section, int position=-1);

    void TraceMuxedList(wxFFile &trace);

public:
    bool Open();
    void Close();
    bool ReadTS(u8* buffer);
    bool EofTS();
    void Reset();

private:
    void SetMaxGroupLen();
   bool MuxSectionsEmbeddedLoader();
   bool MuxSectionsBootLoader();
    void GenerateFrequent(int &frequentOffset, int &groupOffset, int &lineOffset);

   void SetLabel();
};

/* ------------------------------------------------------------------------
   dmsMuxStreamFile
   ------------------------------------------------------------------------ */

class dmsMuxStreamFile : public dmsMuxStream
{
public:
    wxString m_oName;
    wxFile*  m_poFile;

public:
    dmsMuxStreamFile(dmsMultiplexer *mux);
    virtual ~dmsMuxStreamFile();

    bool V_Load(wxXmlNode *node);

public:
    bool Open();
    void Close();
    bool ReadTS(u8* buffer);
    bool EofTS();
    void Reset();
};

/* ------------------------------------------------------------------------
   dmsMuxStreamStuffing
   ------------------------------------------------------------------------ */

class dmsMuxStreamStuffing : public dmsMuxStream
{
public:
    u64 m_iPlayNextCarriedPcr;            // Heure max du prochain PCR à mettre dans le flux
    int m_iPlayPcrStep;                   // Nombre de PCR (arrondi !) entre deux paquets

public:
    dmsMuxStreamStuffing(dmsMultiplexer *mux);
    virtual ~dmsMuxStreamStuffing();

public:
    bool V_Load(wxXmlNode *node);

    bool Open();
    void Close(){;}
    bool ReadTS(u8* buffer);
    bool EofTS();
    void Reset(){;}
};

#endif /* _LIB_MPEG_MULTIPLEX_STREAM_H_ */
