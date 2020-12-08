/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 11/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode.
   - LIPPA - SmarDTV - v 2.00 - 10/2011 - Support 2 PMT and 2 data Carousel
   - LIPPA - SmarDTV - v 2.12 - 02/2012 - DSI repetition all seconds.
   - LIPPA - SmarDTV - v 2.13 - 03/2012 - Generate raw sections in a file and
                                          choice generated files extensions.
   - LIPPA - SmarDTV - v 2.21 - 09/2012 - Add Signalisation tables or USB
                                          Header in sections file
   ************************************************************************ */

#include <wx/wx.h>
#include <wx/file.h>
#include <wx/cmdline.h>
#include <wx/msgout.h>
#include <wx/filename.h>

#ifdef __WIN32__
#include <windows.h>
#endif

#include <Tools/Tools.h>
#include <Tools/Conversion.h>
#include <Tools/Xml.h>
#include <Tools/File.h>

#include <DataCarousel/DataCarousel.h>

#include <MPEG/Multiplexer.h>
#include <MPEG/PMT.h>
#include <MPEG/PAT.h>

#include <DVB/NIT.h>
#include <DVB/BAT.h>
#include <DVB/SDT.h>
#include <DVB/TDT.h>

#include <SSU/UNT.h>


#include "TS_Writer.h"
#include "Config.h"


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsTS_Writer::dmsTS_Writer()
{
    m_poMux      = NULL;
    m_poCarousel = NULL;
    m_poCarousel2 = NULL;
    m_poConfig   = NULL;
    m_poDoc      = NULL;

    m_poMux = new dmsMultiplexer();
}


dmsTS_Writer::~dmsTS_Writer()
{
    DELNUL(m_poDoc);
    DELNUL(m_poMux);
    DELNUL(m_poCarousel);
    DELNUL(m_poCarousel2);
    DELNUL(m_poConfig);
}

/* ========================================================================

   ======================================================================== */

int dmsTS_Writer::DeleteDirectory(wxString sDirectoryName)
{
#ifdef __WIN32__
    wxString        sPattern;
    wxString        sFileName;
    HANDLE          hFile;
    WIN32_FIND_DATA FileInformation;
    DWORD           dwError;
    int             iRet;

    sPattern = sDirectoryName + "\\*.*";
    hFile = ::FindFirstFile(sPattern, &FileInformation);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (FileInformation.cFileName[0] != '.')
            {
                sFileName = sDirectoryName + "\\" + FileInformation.cFileName;
                if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // remove sub-directory recursively
                    if ((iRet = this->DeleteDirectory(sFileName)) != 0)
                    {
                        return iRet;
                    }
                }
                else
                {
                    // set file attributes
                    if (::SetFileAttributes(sFileName, FILE_ATTRIBUTE_NORMAL) == FALSE)
                    {
                        return ::GetLastError();
                    }
                    // delete file
                    if (::DeleteFile(sFileName) == FALSE)
                    {
                        return ::GetLastError();
                    }
                }
            }
        }
        while (::FindNextFile(hFile, &FileInformation));

        // close handle
        ::FindClose(hFile);

        if ((dwError = ::GetLastError()) != ERROR_NO_MORE_FILES)
        {
            return dwError;
        }
        else
        {
            // set directory attributes
            if (::SetFileAttributes(sDirectoryName, FILE_ATTRIBUTE_NORMAL) == FALSE)
            {
                return ::GetLastError();
            }
            // delete directory
            if (::RemoveDirectory(sDirectoryName) == FALSE)
            {
                return ::GetLastError();
            }
        }
    }
#else
    int ret;
    struct dirent *dent;
    DIR *dir = ::opendir(sDirectoryName);
    if (dir != NULL) {
        while ((dent = ::readdir(dir)) != NULL) {
            if (dent->d_type & DT_DIR) {
                if ((ret = this->DeleteDirectory(dent->d_name)) != 0) {
                    return ret;
                }
            } else {
                if ((ret = ::unlink(dent->d_name)) != 0) {
                    return ret;
                }
            }
        }

        if ((ret = ::closedir(dir)) != 0) {
            return ret;
        }

        if ((ret = ::rmdir(sDirectoryName)) != 0) {
            return ret;
        }
    }
#endif
    return 0;
}

bool dmsTS_Writer::Load(const wxString &filename, bool bClean)
{
    DELNUL(m_poDoc);

    m_poDoc = new wxXmlDocument();

    m_poDoc->m_bLoadComments = false;

    wxXmlNode *root = m_poDoc->LoadRoot(filename, "TS_Writer");

    m_poDoc->SetLogMissing(true);

    return Load(root, bClean);
}


bool dmsTS_Writer::Load(wxXmlNode* root, bool bClean)
{
    wxXmlNode* node;
    wxString path, name;

    if (!root) return false;

    //LOG0("Configuration file [%s]", m_oConfigFile);

    if ((node = root->Find("Configuration")))
    {
        m_poConfig = new dmsTS_WriterConf();
        if (!m_poConfig->Load(node)) return false;

        wxFileName::SplitPath(m_poConfig->m_oOutputFile, &path, &name, NULL);

        if (bClean)
        {
            DeleteDirectory(path);
        }

        if (! dmsAssumeDir(path))
        {
            LOGE(L"Error opening output dir [%s]", (const char *)path);
            return false;
        }

      // Remove previous output files
      if (wxFileExists(m_poConfig->m_oOutputFile)) wxRemoveFile(m_poConfig->m_oOutputFile);
      if (wxFileExists(m_poConfig->m_oOutputSectionsFile)) wxRemoveFile(m_poConfig->m_oOutputSectionsFile);

        if (m_poConfig->m_bTrace)
        {
            m_oOutputDir = path+"/"+name+"_TRACE";

            dmsAssumeDir(m_oOutputDir);

            m_poMux->m_oOutputDir  = m_oOutputDir + "/Mux";
        }

        m_poMux->m_iLoaderType         = m_poConfig->m_iLoader;
        m_poMux->m_oOutputFile         = m_poConfig->m_oOutputFile;
        m_poMux->m_oOutputSectionsFile = m_poConfig->m_oOutputSectionsFile;
        m_poMux->m_iModeSectionsFile   = m_poConfig->m_iModeSectionsFile;
        m_poMux->m_u32UpdateId         = m_poConfig->m_u32UpdateId;
        m_poMux->m_u32OUI              = m_poConfig->m_u32OUI;
        m_poMux->m_u16PlateformModel   = m_poConfig->m_u16PlateformModel;
        m_poMux->m_u16PlateformVersion = m_poConfig->m_u16PlateformVersion;
        m_poMux->m_u16ProductModel     = m_poConfig->m_u16ProductModel;
        m_poMux->m_u16ProductVersion   = m_poConfig->m_u16ProductVersion;
    }
    if ((node = root->Find("TS")))
    {
        if (! m_poMux->Load(node)) return false;
    }

    if (m_poMux->m_oStreamSectionList.GetCount()==0)
    {
        m_poMux->NewSectionMux();
    }

    if ((node = root->Find("SI/NIT")))
    {
        dmsMPEG_Section section;

        section.SetData(&section.Data, new dmsDVB_NIT_Data(&section));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/SI");

        if (! m_poMux->AddSection(section)) return false;
    }
    if ((node = root->Find("SI/BAT")))
    {
        dmsMPEG_Section section;

        section.SetData(&section.Data, new dmsDVB_BAT_Data(&section));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/SI");

        if (! m_poMux->AddSection(section)) return false;
    }
    if ((node = root->Find("SI/SDT")))
    {
        dmsMPEG_Section section;

        section.SetData(&section.Data, new dmsDVB_SDT_Data(&section));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/SI");

        if (! m_poMux->AddSection(section)) return false;
    }
    if ((node = root->Find("SI/TDT")))
    {
        dmsMPEG_SectionSSI0 section;

        section.SetData(&section.Data, new dmsDVB_TDT_Data(&section));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/SI");

        if (! m_poMux->AddSection(section)) return false;
    }
    if ((node = root->Find("PSI/PAT")))
    {
        dmsMPEG_Section section;

        section.SetData(&section.Data, new dmsMPEG_PAT_Data(&section));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/PSI");

        if (! m_poMux->AddSection(section)) return false;
    }
    if ((node = root->Find("PSI/PMT")))
    {
        dmsMPEG_Section section;

        section.SetData(&section.Data, new dmsMPEG_PMT_Data(&section));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/PSI");

        if (! m_poMux->AddSection(section)) return false;
    }
    if ((node = root->Find("PSI/PMT2")))
    {
        dmsMPEG_Section section;

        section.SetData(&section.Data, new dmsMPEG_PMT_Data(&section,2));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/PSI");

        if (! m_poMux->AddSection(section)) return false;
    }
    if ((node = root->Find("PSI/PMT_Stub")))
    {
        dmsMPEG_Section section;

        section.SetData(&section.Data, new dmsMPEG_PMT_Data(&section));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/PSI/Stub");

        if (! m_poMux->AddSection(section)) return false;
    }
    if ((node = root->Find("SSU/UNT")))
    {
        dmsMPEG_Section section;

        section.SetData(&section.Data, new dmsSSU_UNT_Data(&section));
        if (! section.Load(node)) return false;
        section.Generate1();
        if (m_oOutputDir.Len()) section.Trace(m_oOutputDir+"/SSU");

        if (! m_poMux->AddSection(section)) return false;
    }

    if ((node = root->Find("DataCarousel")))
    {
        m_poCarousel = new dmsDataCarousel();
        if (m_oOutputDir.Len()) m_poCarousel->m_oOutputDir = m_oOutputDir+"/DataCarousel";
        if (! m_poCarousel->Load(node)) return false;

        m_poCarousel->m_poMux = m_poMux->FindSectionMux(m_poCarousel->m_iPID, true);
   }
    if ((node = root->Find("DataCarousel2")))
    {
        m_poCarousel2 = new dmsDataCarousel();
        if (m_oOutputDir.Len()) m_poCarousel2->m_oOutputDir = m_oOutputDir+"/DataCarousel2";
        if (! m_poCarousel2->Load(node)) return false;

        m_poCarousel2->m_poMux = m_poMux->FindSectionMux(m_poCarousel2->m_iPID, true);
    }

    if (root->DisplayUnused()) return false;

    // save base directory in attribute
    m_oOutputDir = path;

    return root->m_doc->m_bLoaded;
}


bool dmsTS_Writer::Generate()
{
    if (m_poCarousel)
    {
        if (m_poCarousel->Compile())
            m_poCarousel->Generate();
        else
        {
            LOGE(L"Processing error");
            return false;
        }
    }

   if (m_poCarousel2)
    {
        if (m_poCarousel2->Compile())
            m_poCarousel2->Generate();
        else
        {
            LOGE(L"Processing caroussel 2 error");
            return false;
        }
    }

    return m_poMux->Play();

    dmsMuxStreamSection* mux = m_poMux->FindSectionMux("");

    if (m_poConfig->m_oOutputSigFile.Len())
    {
        bool ok;

        if (m_oOutputDir.Len())
            ok = mux->Generate(m_poConfig->m_oOutputSigFile, m_oOutputDir+"/mux-info.sig.txt");
        else
            ok = mux->Generate(m_poConfig->m_oOutputSigFile, "");

        if (! ok) return false;

        mux->Clear();
    }

    bool ok;

    if (m_oOutputDir.Len())
        ok = mux->Generate(m_poConfig->m_oOutputFile, m_oOutputDir+"/mux-info.txt");
    else
        ok = mux->Generate(m_poConfig->m_oOutputFile, "");

    if (! ok) return false;
    /*
    if (0) {
    m_poMux->m_iTsMinimalSize=1;
    wxFile f("Stuffing.ts", wxFile::write);
    dmsBuffer buf;
    buf.Set((uchar)0xFF, 1000);
    m_poMux->Generate(f, buf, 0x1FFF);
    }
    */

    return true;
}
