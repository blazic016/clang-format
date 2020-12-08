/////////////////////////////////////////////////////////////////////////////
// Name:        xml.cpp
// Purpose:     wxXmlDocument - XML parser & data holder class
// Author:      Vaclav Slavik
// Created:     2000/03/05
// RCS-ID:      $Id: xml.cpp,v 1.9.2.2 2003/04/16 22:33:54 VS Exp $
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/datstrm.h>
#include <wx/zstream.h>
#include <wx/log.h>
#include <wx/intl.h>
#include <wx/strconv.h>
#include <wx/hash.h>
#include <wx/regex.h>
#include <wx/mstream.h>
#include <wx/filename.h>


#include "Xml.h"

#include "Log.h"
#include "Conversion.h"

#include "Expat/xmlparse/xmlparse.h" // from Expat

//-----------------------------------------------------------------------------
//  wxXmlNode
//-----------------------------------------------------------------------------

wxXmlNode::wxXmlNode(wxXmlNode *parent,wxXmlNodeType type,
                     const wxString& name, const wxString& content,
                     wxXmlProperty *props, wxXmlNode *next)
    : m_type(type), m_name(name), m_content(content),
      m_properties(props), m_parent(parent),
      m_children(NULL), m_next(next), m_lastchildren(NULL),
      m_used(false)
{
    if (m_parent)
    {
        if (m_parent->m_lastchildren)
        {
            m_parent->m_lastchildren->m_next = this;
            m_parent->m_lastchildren = this;
            m_next = NULL;
        }
        else
        {
            m_parent->m_children = this;
            m_parent->m_lastchildren = this;
            m_next = NULL;
        }
    }
}

wxXmlNode::wxXmlNode(wxXmlNodeType type, const wxString& name,
                     const wxString& content)
    : m_type(type), m_name(name), m_content(content),
      m_properties(NULL), m_parent(NULL),
      m_children(NULL), m_next(NULL), m_lastchildren(NULL),
      m_used(false)
{}

wxXmlNode::wxXmlNode(const wxXmlNode& node)
{
    m_next = NULL;
    m_parent = NULL;
    m_used = false;
    DoCopy(node);
}

wxXmlNode::~wxXmlNode()
{
    wxXmlNode *c, *c2;
    for (c = m_children; c; c = c2)
    {
        c2 = c->m_next;
        delete c;
    }

    wxXmlProperty *p, *p2;
    for (p = m_properties; p; p = p2)
    {
        p2 = p->GetNext();
        delete p;
    }
}

wxXmlNode& wxXmlNode::operator=(const wxXmlNode& node)
{
    wxDELETE(m_properties);
    wxDELETE(m_children);
    m_lastchildren = NULL;
    DoCopy(node);
    return *this;
}

void wxXmlNode::DoCopy(const wxXmlNode& node)
{
    m_type         = node.m_type;
    m_name         = node.m_name;
    m_content      = node.m_content;
    m_children     = NULL;
    m_lastchildren = NULL;

    wxXmlNode *n = node.m_children;
    while (n)
    {
        AddChild(new wxXmlNode(*n));
        n = n->GetNext();
    }

    m_properties = NULL;
    wxXmlProperty *p = node.m_properties;
    while (p)
    {
       AddProperty(p->GetName(), p->GetValue());
       p = p->GetNext();
    }
}

bool wxXmlNode::HasProp(const wxString& propName) const
{
    wxXmlProperty *prop = GetProperties();

    while (prop)
    {
        if (prop->GetName() == propName) return TRUE;
        prop = prop->GetNext();
    }

    return FALSE;
}


bool wxXmlNode::GetPropVal(const wxString& propName, wxString *value) const
{
    wxXmlProperty *prop = GetProperties();
    wxString rest;

    while (prop)
    {
        if (propName.StartsWith(prop->GetName(), &rest) &&
            (rest.Length() == 0 || (rest.Length()==1 && rest[0]=='?')))
        {
            *value = prop->GetValue();
            return TRUE;
        }
        prop = prop->GetNext();
    }

    return FALSE;
}

wxString wxXmlNode::GetPropVal(const wxString& propName, const wxString& defaultVal) const
{
    wxString tmp;
    if (GetPropVal(propName, &tmp))
        return tmp;
    else
        return defaultVal;
}


wxString wxXmlNode::GetLongName(const wxString &name) const
{
    wxXmlNode *node=this->m_parent;
    wxString result;

    if (m_name != "text") result = "/" + m_name;

    while (node)
    {
        result = "/" + node->m_name + result;
        node = node->m_parent;
    }

    if (name.Len()) result << STR("/%s", name);

    return result;
}


void wxXmlNode::AddChild(wxXmlNode *child)
{
    if (m_children == NULL)
    {
        m_children = child;
        m_lastchildren = child;
    }
    else
    {
        m_lastchildren->m_next = child;
        m_lastchildren = child;
    }
    child->m_next = NULL;
    child->m_parent = this;
}

void wxXmlNode::InsertChild(wxXmlNode *child, wxXmlNode *before_node)
{
    wxASSERT_MSG(before_node->GetParent() == this, wxT("wxXmlNode::InsertChild - the node has incorrect parent"));

    if (m_children == before_node)
       m_children = child;
    else
    {
        wxXmlNode *ch = m_children;
        while (ch->m_next != before_node) ch = ch->m_next;
        ch->m_next = child;
    }

    child->m_parent = this;
    child->m_next = before_node;
}

bool wxXmlNode::RemoveChild(wxXmlNode *child)
{
    if (m_children == NULL)
        return FALSE;
    else if (m_children == child)
    {
        m_children = child->m_next;
        if (m_children == NULL) m_lastchildren = NULL;
        child->m_parent = NULL;
        child->m_next = NULL;
        return TRUE;
    }
    else
    {
        wxXmlNode *ch = m_children;
        while (ch->m_next)
        {
            if (ch->m_next == child)
            {
                ch->m_next = child->m_next;
                child->m_parent = NULL;
                child->m_next = NULL;
                if (ch->m_next == NULL) m_lastchildren = ch;
                return TRUE;
            }
            ch = ch->m_next;
        }
        return FALSE;
    }
}

void wxXmlNode::AddProperty(const wxString& name, const wxString& value)
{
    AddProperty(new wxXmlProperty(name, value, NULL));
}

void wxXmlNode::AddProperty(wxXmlProperty *prop)
{
    if (m_properties == NULL)
        m_properties = prop;
    else
    {
        wxXmlProperty *p = m_properties;
        while (p->GetNext()) p = p->GetNext();
        p->SetNext(prop);
    }
}

bool wxXmlNode::DeleteProperty(const wxString& name)
{
    wxXmlProperty *prop;

    if (m_properties == NULL)
        return FALSE;

    else if (m_properties->GetName() == name)
    {
        prop = m_properties;
        m_properties = prop->GetNext();
        prop->SetNext(NULL);
        delete prop;
        return TRUE;
    }

    else
    {
        wxXmlProperty *p = m_properties;
        while (p->GetNext())
        {
            if (p->GetNext()->GetName() == name)
            {
                prop = p->GetNext();
                p->SetNext(prop->GetNext());
                prop->SetNext(NULL);
                delete prop;
                return TRUE;
            }
            p = p->GetNext();
        }
        return FALSE;
    }
}


/* ========================================================================
   Debut CODE DMS
   ======================================================================== */

void wxXmlNode::Trace(const wxString &level)
{
    wxXmlProperty *prop;
    wxXmlNode     *node;

    // Affichage du noeud
    LOG0(L"%s+ %s = %s", level, GetName(), GetContent());

    // ... de ses arguments
    for (prop=GetProperties(); prop; prop = prop->GetNext())
        LOG0(L"%s  - $%s = %s", level, prop->GetName(), prop->GetValue());

    // ... de ses fils
    for (node=GetChildren(); node; node=node->GetNext())
        node->Trace(level + "  ");
}


wxXmlNode *wxXmlNode::AddNewChild(const wxString &name)
{
    wxXmlNode *node = new wxXmlNode(this, wxXML_ELEMENT_NODE, name, "", NULL, NULL);

    return node;
}

wxXmlNode *wxXmlNode::AddNewChild(const wxString &name, const wxString &value)
{
    wxXmlNode *node = new wxXmlNode(this, wxXML_ELEMENT_NODE, name, "", NULL, NULL);
    new wxXmlNode(node, wxXML_TEXT_NODE, "text", value, NULL, NULL);

    return node;
}


bool wxXmlNode::DeleteChild(wxXmlNode *node)
{
    if (! RemoveChild(node)) return false;

    delete node;

    return true;
}


bool wxXmlNode::NameEQI(const wxString &name)
{
    return (GetName().CmpNoCase(name) == 0);
}

wxXmlNode *wxXmlNode::Find(const wxString &name, bool logError)
{
    wxXmlNode *node = GetChildren();
    wxString rest;

    m_used = true;

    if (name.IsEmpty()) return this;

    for (node=GetChildren(); node; node=node->GetNext())
    {
        if (name.StartsWith(node->GetName(), &rest))
        {
            node->m_used = true;
            if (rest.Length() == 0 || (rest.Length()==1 && rest[0]=='?'))
                return node;
            else if (rest[0] == '/')
                return node->Find(rest.Mid(1));
        }
    }

    if (logError)
    {
        LOGE(L"Tag [%s] not found in [%s]", name, GetLongName());
    }

    return NULL;
}

bool wxXmlNode::Find(const wxString &name, wxXmlNode *&node)
{
    node = Find(name);

    if (node==NULL)
    {
        LOGE(L"Cant find child [%s] in xml root [%s]", name, GetLongName());
        return false;
    }

    return true;
}



wxXmlProperty *wxXmlNode::FindProperty(const wxString &name)
{
    wxXmlProperty *prop;

    for (prop=GetProperties(); prop; prop = prop->GetNext())
    {
        if (name.Cmp(prop->GetName()) == 0) return prop;
    }

    return NULL;
}

wxString wxXmlNode::GetValue()
{
    wxString tmp;

    if (! ReadTmpString("", &tmp,m_doc->m_bLogMissing)) return "";

    return tmp;
}

int wxXmlNode::GetValue(int def)
{
    int result;
    wxString tmp;

    if (! ReadTmpString("", &tmp,m_doc->m_bLogMissing)) return def;
    if (! StrToInt(tmp, result)) return def;

    return result;
}



bool wxXmlNode::ReadTmpString(const wxString &name, wxString *value, bool logMissing)
{
    wxXmlNode *node = NULL;
    bool IsTag = false;

    if (node == NULL)
    {
        // Tentative 1 : on essaye de prendre le contenu (node "text")

        node = Find(name.Len() ? name+"/text" : "text");

        if (node)
        {
            *value = node->GetContent();
        }
    }

    if (node==NULL)
    {
        // Tentative 2 : on essaye de prendre le tag "value"

        node = Find(name);

        if (node)
        {
            IsTag = true;
            if (! node->GetPropVal("value", value)) node = NULL;
        }
    }

    if (node==NULL)
    {
        // Tentative 3 : on essaye de prendre le tag "$name" du noeud père

        wxString father, child;

        wxFileName::SplitPath(name, &father, &child, NULL);

        node = Find(father);

        if (node)
        {
            if (child=="XmlTagName") // Cas particulier : récupération du nom
                *value = node->GetName();
            else
                if (! node->GetPropVal(name, value)) node = NULL;
        }
    }

    if (node)
    {
        *value = value->Trim().Trim(false);
        node->ReplaceVars(*value);

        return true;
    }
    else if (IsTag)
    {
        value->Clear();

        return true;
    }
    else
    {
        if (name.Len() && name.Last() != '?')
        {
            if (logMissing)
            {
                LOGE(L"Missing XML tag [%s]", GetLongName()+"/"+name);
            }
            m_doc->m_bLoaded = false;
            return false;
        }

        value->Clear();

        return false;
    }
}

bool wxXmlNode::Read(const wxString &name, wxString *value)
{
    wxString tmp;

    if (ReadTmpString(name, &tmp,m_doc->m_bLogMissing))
    {
        *value = tmp;

        return true;
    }

    return false;
}

bool wxXmlNode::ReadSFN(const wxString &name, wxString *value)
{
    wxString tmp;

    if (ReadTmpString(name, &tmp,m_doc->m_bLogMissing))
    {
        *value = tmp;

        dmsStandardFileName(*value);

        return true;
    }

    return false;
}


bool wxXmlNode::Read(const wxString &name, u64 *value)
{
    wxString tmp;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing)) return false;

    if (! StrToInt(tmp, *value))
    {
        LOGE(L"Bad int value [%s] in xml tag [%s]", tmp, GetLongName(name));

        return false;
    }

    return true;
}

bool wxXmlNode::Read(const wxString &name, int *value)
{
    u64 lValue;

    if (! Read(name, &lValue)) return false;

    *value = (int)lValue;

    return true;
}

bool wxXmlNode::Read(const wxString &name, int *value, int DefaultValue)
{
    wxString tmp;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing))
    {
        *value = DefaultValue;
        return true;
    }

    if (! StrToInt(tmp, *value))
    {
        LOGE(L"Bad int value [%s] in xml tag [%s]", tmp, GetLongName());

        return false;
    }

    return true;
}



bool wxXmlNode::Read(const wxString &name, int *value, const wxString &labelName, int labelValue)
{
    wxString tmp;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing)) return false;

    if (! StrToInt(tmp, *value, labelName, labelValue))
    {
        LOGE(L"Bad int value [%s] in xml tag [%s]", tmp, GetLongName());

        return false;
    }

    return true;
}



bool wxXmlNode::Read(const wxString &name, unsigned int *value)
{
    u64 lValue;

    if (! Read(name, &lValue) || lValue < 0 || lValue > UINT_MAX) return false;

    *value = (unsigned int)lValue;

    return true;
}


bool wxXmlNode::Read(const wxString &name, unsigned long *value)
{
    u64 lValue;

    if (! Read(name, &lValue) || lValue < 0 || lValue > ULONG_MAX) return false;

    *value = (unsigned long)lValue;

    return true;
}

bool wxXmlNode::Read(const wxString &name, unsigned char *value)
{
    u64 lValue;

    if (! Read(name, &lValue) || lValue < 0 || lValue > UCHAR_MAX) return false;

    *value = (unsigned char)lValue;

    return true;
}


bool wxXmlNode::Read(const wxString &name, int *value, const char *valueList[])
{
    wxString tmp;
    bool ok = true;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing))
    {
        ok = false;
    }
    else
    {
        *value = dmsGetStrRank(tmp.c_str(), valueList);

        if (*value == -1) ok = false;
    }

    if (! ok)
    {
        wxXmlNode *node;

        node = Find(name.Len() ? name+"/text" : "text");
        if (node==NULL) node = Find(name);
        if (node==NULL) return false;

        int i=1;

        LOGE(L"Error in value [%s] of xml tag [%s]", node->GetContent(), node->GetLongName());
        LOGE(L"Possible values");

        while (valueList[i])
        {
            LOGE(L"   - %d: [%s]", i, valueList[i]);
            i++;
        }
    }

    return ok;
}


bool wxXmlNode::Read(const wxString &name, u64 *value, dmsNameRank valueList[])
{
    wxString tmp;
    u64 rank;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing)) return false;

    if (! dmsGetStrRank(tmp.c_str(), valueList, rank))
    {
        LOGE(L"Bad value [%s] in xml tag [%s]", tmp, GetLongName(name));
        LOGE(L"Must belongs to :");
        for (int i=0; valueList[i].name; i++)
        {
            LOGE(L"    \"%s\"", valueList[i].name);
        }
        return false;
    }
    *value = rank;

    return true;
}




bool wxXmlNode::Read(const wxString &name, unsigned short *value)
{
    int tmp;

    if (! Read(name, &tmp)) return false;

    if (tmp < 0 || tmp > 0xFFFF)
    {
        LOGE(L"Bad unsigned short value [%d] in xml tag [%s]", tmp, name);
        return false;
    }

    *value = tmp;

    return true;
}

bool wxXmlNode::Read(const wxString &name, bool *value)
{
    wxString tmp;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing)) return false;

    if (! StrToBool(tmp, value))
    {
        LOGE(L"Bad bool value [%s] in xml tag [%s]", tmp, GetLongName());

        return false;
    }

    return true;
}

bool wxXmlNode::Read(const wxString &name, wxDateTime *value)
{
    wxString tmp;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing)) return false;

    if (! StrToDateTime(tmp, *value))
    {
        LOGE(L"XML tag [%s] has bad value [%s]", name, tmp);

        return false;
    }

    return true;
}


bool wxXmlNode::Read(const wxString &name, dmsDateTimeSpan *value)
{
    wxString tmp;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing)) return false;

    if (! StrToDateTimeSpan(tmp, *value))
    {
        LOGE(L"XML tag [%s] has bad value [%s]", name, tmp);

        return false;
    }

    return true;
}


bool wxXmlNode::Read(const wxString &name, dmsBuffer *value)
{
    wxString tmp;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing)) return false;

    if (! value->SetFromDump(tmp))
    {
        LOGE(L"XML tag [%s] has bad dump value [%s]", name, tmp);

        return false;
    }
    return true;
}


bool wxXmlNode::ReadAscii(const wxString &name, dmsBuffer *value, bool withZero)
{
    wxString tmp;

    if (! ReadTmpString(name, &tmp,m_doc->m_bLogMissing)) return false;

    value->Set(tmp, withZero);

    return true;
}


bool wxXmlNode::Remove(wxXmlNode *node, bool *keep)
{
    if (node == NULL || keep == NULL || *keep) return false;

    wxXmlNode *parent = node->GetParent();

    parent->DeleteChild(node);

    return true;
}


bool wxXmlNode::Remove(const wxString &name, bool *keep)
{
    return Remove(Find(name), keep);
}


bool wxXmlNode::Remove(const wxString &name)
{
    bool keep = false;

    return Remove(Find(name), &keep);
}


wxXmlNode *wxXmlNode::AssumePath(const wxString &name)
{
    wxXmlNode *node = GetChildren();
    wxString path, rest;

    // Recherche le début du path, sort si trouvé

    for (node=GetChildren(); node; node=node->GetNext())
    {
        if (name.StartsWith(node->GetName(), &rest))
        {
            if (rest.Length() == 0)
                return node;
            else if (rest[0] == '/')
                return node->AssumePath(rest.Mid(1));
        }
    }

    // Début du path non trouvé : créer le noeud

    wxFileName::SplitPath(name, &path, &rest, NULL);

    if (name == "text") // Noeud text (particulier)
    {
        node = new wxXmlNode(this, wxXML_TEXT_NODE, name, "", NULL, NULL);
    }
    else
    {
        int sep = name.Find('/');

        if (sep == -1)
        {
            // Pas de slash : c'est le noeud final
            node = new wxXmlNode(this, wxXML_ELEMENT_NODE, name, "", NULL, NULL);
        }
        else if (sep == 0)
        {
            // Slash en premiere position, saute le slash (nom absolu transformé en relatif)
            AssumePath(name.Mid(1));
        }
        else if (name.IsEmpty())
        {
            // Fini par un slash : retourne le dernier noeud
            return node;
        }
        else
        {
            // Un slash en plein milieu : crée le noeud et appelle la récursivité
            node = new wxXmlNode(this, wxXML_ELEMENT_NODE, name.SubString(0, sep-1), "", NULL, NULL);
            node = node->AssumePath(name.Mid(sep+1));
        }
    }

    return node;
}



void wxXmlNode::Write(const wxString &name, const wxString &value, bool *keep)
{
    if (Remove(name, keep)) return;

    wxXmlNode *node = AssumePath(name.Len() ? name+"/text" : "text");

    if (node == NULL) return;
    if (Remove(node, keep)) return;

    node->SetContent(value);

}


void wxXmlNode::Write(const wxString &name, int value, bool *keep)
{
    if (Remove(name, keep)) return;

    wxXmlNode *node = AssumePath(name.Len() ? name+"/text" : "text");

    if (node == NULL) return;
    if (Remove(node, keep)) return;

    node->SetContent(STR("%d", value));
}


void wxXmlNode::SetDoc(wxXmlDocument *doc)
{
    m_doc = doc;

    for (wxXmlNode *node = GetChildren(); node; node=node->GetNext())
    {
        node->SetDoc(doc);
    }
}


void wxXmlNode::LoadVars()
{
    wxXmlNode *node;

    node = Find("SetVars");

    if (node)
    {
        wxString name, value;
        for (node=node->GetChildren(); node; node=node->GetNext())
        {
            node->ReadTmpString("Name",  &name,m_doc->m_bLogMissing);
            node->ReadTmpString("Value", &value,m_doc->m_bLogMissing);
            if (m_doc->m_varNames == NULL)
            {
                m_doc->m_varNames = new wxArrayString;
                m_doc->m_varValues = new wxArrayString;
            }

            ReplaceVars(value);

            m_doc->m_varNames->Add(name);
            m_doc->m_varValues->Add(value);
        }
    }
}


void wxXmlNode::ReplaceVars(wxString &value)
{
    if (m_doc->m_varNames == NULL) return;

    wxRegEx regex;

    if (regex.Compile("\\$\\((.*)\\)"))
    {
        while (regex.Matches(value))
        {
            int index = m_doc->m_varNames->Index(regex.GetMatch(value, 1));

            if (index != wxNOT_FOUND)
            {
                regex.ReplaceFirst(&value, m_doc->m_varValues->Item(index));
            }
            else
            {
                regex.ReplaceFirst(&value, STR("Undefined(%s)", value));
            }
        }
    }
}



bool wxXmlNode::DisplayUnused()
{
    wxXmlNode *node;
    bool result = false;

    for (node=GetChildren(); node; node=node->GetNext())
    {
        if (!node->m_used)
        {
            LOGE(L"Xml : Unused tag [%s]", node->GetLongName());
            result = true;
        }
        else
            result = node->DisplayUnused() || result;
    }

    return result;
}


/* ========================================================================
   Fin CODE DMS
   ======================================================================== */

//-----------------------------------------------------------------------------
//  wxXmlDocument
//-----------------------------------------------------------------------------

wxXmlDocument::wxXmlDocument()
    : m_version(wxT("1.0")), m_fileEncoding(wxT("utf-8")), m_root(NULL), m_varNames(NULL)
{
    m_bLogMissing   = true;
    m_bLoaded       = true;
    m_bLoadComments = false;
#if !wxUSE_UNICODE
    m_encoding = wxT("UTF-8");
#endif
}

wxXmlDocument::wxXmlDocument(const wxString& filename, const wxString& encoding)
                          : wxObject(), m_root(NULL), m_varNames(NULL)
{
    m_bLogMissing = true;
    m_bLoaded     = true;
    if ( !Load(filename, encoding) )
    {
        wxDELETE(m_root);
    }
}

wxXmlDocument::wxXmlDocument(wxInputStream& stream, const wxString& encoding)
                          : wxObject(), m_root(NULL), m_varNames(NULL)
{
    m_bLogMissing = true;
    m_bLoaded     = true;
    if ( !Load(stream, encoding) )
    {
        wxDELETE(m_root);
    }
}

wxXmlDocument::wxXmlDocument(const wxXmlDocument& doc) : m_varNames(NULL)
{
    DoCopy(doc);
}

wxXmlDocument::~wxXmlDocument()
{
    delete m_root;

    if (m_varNames)
    {
        delete m_varNames;
        delete m_varValues;
    }
}

wxXmlDocument& wxXmlDocument::operator=(const wxXmlDocument& doc)
{
    wxDELETE(m_root);
    if (m_varNames)
    {
        DELNUL(m_varNames);
        delete m_varValues;
    }
    DoCopy(doc);
    return *this;
}

void wxXmlDocument::DoCopy(const wxXmlDocument& doc)
{
    m_bLogMissing = doc.m_bLogMissing;
    m_bLoaded     = doc.m_bLoaded;
    m_version = doc.m_version;
#if !wxUSE_UNICODE
    m_encoding = doc.m_encoding;
#endif
    m_fileEncoding = doc.m_fileEncoding;
    m_root = new wxXmlNode(*doc.m_root);
}

bool wxXmlDocument::Load(const wxString& filename, const wxString& encoding)
{
    bool result;
    m_bLoaded = true;
    wxFileInputStream stream(filename);
    result = Load(stream, encoding);

    return result && m_bLoaded;
}


wxXmlNode *wxXmlDocument::LoadText(const wxString& text,
                                   const wxString& rootname,
                                   const wxString& encoding)
{
    wxXmlNode *root = NULL;

    wxMemoryInputStream stream(text.c_str(), text.Len());

    if (! Load(stream, encoding))
    {
        LOGE(L"Xml Error in xml text");
    }
    else if (! GetRoot())
    {
        LOGE(L"No root tag in xml text");
    }
    else if (rootname.Len() && GetRoot()->GetName().CmpNoCase(rootname) != 0)
    {
        LOGE(L"Bad root xml tag [%s] in xml text ('%s' expected)", GetRoot()->GetName(), rootname);
    }
    else
    {
        root = GetRoot();
        if (root == NULL)
        {
            LOGE(L"No root in xml text");
        }
    }

    if (root == NULL)
    {
        return NULL;
    }

    root->SetDoc(this);

    return root;
}



wxXmlNode *wxXmlDocument::LoadRoot(const wxString& filename,
                                   const wxString& rootname,
                                   const wxString& encoding)
{
    wxXmlNode *root = NULL;

    if (! wxFileExists(filename))
    {
        LOGE(L"Error opening file [%s]", filename);
    }
    else if (! Load(filename))
    {
        LOGE(L"Xml Error in file [%s]", filename);
    }
    else if (! GetRoot())
    {
        LOGE(L"No root tag in [%s]", filename);
    }
    else if (GetRoot()->GetName().CmpNoCase(rootname) != 0)
    {
        LOGE(L"Bad root xml tag [%s] in [%s] ('%s' expected)", GetRoot()->GetName(), filename, rootname);
    }
    else
    {
        root = GetRoot();
        if (root == NULL)
        {
            LOGE(L"No root in [%s]", filename);
        }
    }

    if (root == NULL)
    {
        return NULL;
    }

    root->SetDoc(this);

    return root;
}

bool wxXmlDocument::Save(const wxString& filename) const
{
    bool result;
    wxString temp = filename + ".temp";

    {
        wxFileOutputStream stream(temp);
        result = Save(stream);
        // Appel du destructeur de "stream" (libere l'acces au fichier)
    }
    if (result)
    {
        result = wxRenameFile(temp, filename);
        return result;
    }
    else
    {
        wxRemoveFile(temp);
        return false;
    }
}


void wxXmlDocument::SetLogMissing(bool value)
{
    m_bLogMissing = value;
}



//-----------------------------------------------------------------------------
//  wxXmlDocument loading routines
//-----------------------------------------------------------------------------

/*
    FIXME:
       - process all elements, including CDATA
 */

// converts Expat-produced string in UTF-8 into wxString.
inline static wxString CharToString(wxMBConv *conv,
                                    const char *s, size_t len = wxString::npos)
{
#if wxUSE_UNICODE
    (void)conv;
    return wxString(s, wxConvUTF8, len);
#else
    if ( conv )
    {
        size_t nLen = (len != wxString::npos) ? len :
                          nLen = wxConvUTF8.MB2WC((wchar_t*) NULL, s, 0);

        wchar_t *buf = new wchar_t[nLen+1];
        wxConvUTF8.MB2WC(buf, s, nLen);
        buf[nLen] = 0;
        wxString str(buf, *conv, len);
        delete[] buf;
        return str;
    }
    else
        return wxString(s, len);
#endif
}

struct wxXmlParsingContext
{
    wxMBConv  *conv;
    wxXmlNode *root;
    wxXmlNode *node;
    wxXmlNode *lastAsText;
    wxString   encoding;
    wxString   version;
};

static void StartElementHnd(void *userData, const char *name, const char **atts)
{
    wxXmlParsingContext *ctx = (wxXmlParsingContext*)userData;
    wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, CharToString(ctx->conv, name));
    const char **a = atts;
    while (*a)
    {
        node->AddProperty(CharToString(ctx->conv, a[0]), CharToString(ctx->conv, a[1]));
        a += 2;
    }
    if (ctx->root == NULL)
        ctx->root = node;
    else
        ctx->node->AddChild(node);
    ctx->node = node;
    ctx->lastAsText = NULL;
}

static void EndElementHnd(void *userData, const char* name)
{
    wxXmlParsingContext *ctx = (wxXmlParsingContext*)userData;

    wxXmlNode *tmp = ctx->node;

    ctx->node = ctx->node->GetParent();
    ctx->lastAsText = NULL;

    if (strncmp(name, "NO_", 3)==0 && tmp->GetParent()) {tmp->GetParent()->RemoveChild(tmp);delete tmp;}
}

static void TextHnd(void *userData, const char *s, int len)
{
    wxXmlParsingContext *ctx = (wxXmlParsingContext*)userData;
    char *buf = new char[len + 1];

    buf[len] = '\0';
    memcpy(buf, s, (size_t)len);

    if (ctx->lastAsText)
    {
        ctx->lastAsText->SetContent(ctx->lastAsText->GetContent() +
                                    CharToString(ctx->conv, buf));
    }
    else
    {
        bool whiteOnly = TRUE;
        for (char *c = buf; *c != '\0'; c++)
            if (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\r')
            {
                whiteOnly = FALSE;
                break;
            }
        if (!whiteOnly)
        {
            ctx->lastAsText = new wxXmlNode(wxXML_TEXT_NODE, wxT("text"),
                                            CharToString(ctx->conv, buf));
            ctx->node->AddChild(ctx->lastAsText);
        }
    }

    delete[] buf;
}

static void CommentHnd(void *userData, const char *data)
{
    wxXmlParsingContext *ctx = (wxXmlParsingContext*)userData;

    if (ctx->node)
    {
        // VS: ctx->node == NULL happens if there is a comment before
        //     the root element (e.g. wxDesigner's output). We ignore such
        //     comments, no big deal...
        ctx->node->AddChild(new wxXmlNode(wxXML_COMMENT_NODE,
                            wxT("comment"), CharToString(ctx->conv, data)));
    }
    ctx->lastAsText = NULL;
}

static void DefaultHnd(void *userData, const char *s, int len)
{
    // XML header:
    if (len > 6 && memcmp(s, "<?xml ", 6) == 0)
    {
        wxXmlParsingContext *ctx = (wxXmlParsingContext*)userData;

        wxString buf = CharToString(ctx->conv, s, (size_t)len);
        int pos;
        pos = buf.Find(wxT("encoding="));
        if (pos != wxNOT_FOUND)
            ctx->encoding = buf.Mid(pos + 10).BeforeFirst(buf[(size_t)pos+9]);
        pos = buf.Find(wxT("version="));
        if (pos != wxNOT_FOUND)
            ctx->version = buf.Mid(pos + 9).BeforeFirst(buf[(size_t)pos+8]);
    }
}

static int UnknownEncodingHnd(void * WXUNUSED(encodingHandlerData),
                              const XML_Char *name, XML_Encoding *info)
{
    // We must build conversion table for expat. The easiest way to do so
    // is to let wxCSConv convert as string containing all characters to
    // wide character representation:
    wxCSConv conv(wxString(name, wxConvLibc));
    char mbBuf[2];
    wchar_t wcBuf[10];
    size_t i;

    mbBuf[1] = 0;
    info->map[0] = 0;
    for (i = 0; i < 255; i++)
    {
        mbBuf[0] = (char)(i+1);
        if (conv.MB2WC(wcBuf, mbBuf, 2) == (size_t)-1)
        {
            // invalid/undefined byte in the encoding:
            info->map[i+1] = -1;
        }
        info->map[i+1] = (int)wcBuf[0];
    }

    info->data = NULL;
    info->convert = NULL;
    info->release = NULL;

    return 1;
}

bool wxXmlDocument::Load(wxInputStream& stream, const wxString& encoding)
{
#if wxUSE_UNICODE
    (void)encoding;
#else
    m_encoding = encoding;
#endif

    const size_t BUFSIZE = 1024;
    char buf[BUFSIZE];
    wxXmlParsingContext ctx;
    bool done;
    XML_Parser parser = XML_ParserCreate(NULL);

    ctx.root = ctx.node = NULL;
    ctx.encoding = wxT("UTF-8"); // default in absence of encoding=""
    ctx.conv = NULL;
#if !wxUSE_UNICODE
    if ( encoding != wxT("UTF-8") && encoding != wxT("utf-8") )
        ctx.conv = new wxCSConv(encoding);
#endif

    XML_SetUserData(parser, (void*)&ctx);
    XML_SetElementHandler(parser, StartElementHnd, EndElementHnd);
    XML_SetCharacterDataHandler(parser, TextHnd);
    if (m_bLoadComments) XML_SetCommentHandler(parser, CommentHnd);
    XML_SetDefaultHandler(parser, DefaultHnd);
    XML_SetUnknownEncodingHandler(parser, UnknownEncodingHnd, NULL);

    bool ok = TRUE;
    do
    {
        size_t len = stream.Read(buf, BUFSIZE).LastRead();
        done = (len < BUFSIZE);
        if (!XML_Parse(parser, buf, len, done))
        {
            LOGE(L"XML parsing error: [%s] at line [%d]",
                       XML_ErrorString(XML_GetErrorCode(parser)),
                       XML_GetCurrentLineNumber(parser));
            ok = FALSE;
            break;
        }
    } while (!done);

    if (ok)
    {
        SetVersion(ctx.version);
        SetFileEncoding(ctx.encoding);
        SetRoot(ctx.root);
    }

    XML_ParserFree(parser);
#if !wxUSE_UNICODE
    if ( ctx.conv )
        delete ctx.conv;
#endif

    return ok;

}



//-----------------------------------------------------------------------------
//  wxXmlDocument saving routines
//-----------------------------------------------------------------------------

// write string to output:
inline static void OutputString(wxOutputStream& stream, const wxString& str,
                                wxMBConv *convMem, wxMBConv *convFile)
{
    if (str.IsEmpty()) return;
#if wxUSE_UNICODE
    const wxWX2MBbuf buf(str.mb_str(convFile ? *convFile : wxConvUTF8));
    stream.Write((const char*)buf, strlen((const char*)buf));
#else
    if ( convFile == NULL )
        stream.Write(str.mb_str(), str.Len());
    else
    {
        wxString str2(str.wc_str(*convMem), *convFile);
        stream.Write(str2.mb_str(), str2.Len());
    }
#endif
}

// Same as above, but create entities first.
// Translates '<' to "&lt;", '>' to "&gt;" and '&' to "&amp;"
static void OutputStringEnt(wxOutputStream& stream, const wxString& str,
                            wxMBConv *convMem, wxMBConv *convFile)
{
    wxString buf;
    size_t i, last, len;
    wxChar c;

    len = str.Len();
    last = 0;
    for (i = 0; i < len; i++)
    {
        c = str.GetChar(i);
        if (c == wxT('<') || c == wxT('>') ||
            (c == wxT('&') && str.Mid(i+1, 4) != wxT("amp;")))
        {
            OutputString(stream, str.Mid(last, i - last), convMem, convFile);
            switch (c)
            {
                case wxT('<'):
                    OutputString(stream, wxT("&lt;"), NULL, NULL);
                    break;
                case wxT('>'):
                    OutputString(stream, wxT("&gt;"), NULL, NULL);
                    break;
                case wxT('&'):
                    OutputString(stream, wxT("&amp;"), NULL, NULL);
                    break;
                default: break;
            }
            last = i + 1;
        }
    }
    OutputString(stream, str.Mid(last, i - last), convMem, convFile);
}

inline static void OutputIndentation(wxOutputStream& stream, int indent)
{
    wxString str = wxT("\n");
    for (int i = 0; i < indent; i++)
        str << wxT(' ') << wxT(' ');
    OutputString(stream, str, NULL, NULL);
}

static void OutputNode(wxOutputStream& stream, wxXmlNode *node, int indent,
                       wxMBConv *convMem, wxMBConv *convFile)
{
    wxXmlNode *n, *prev;
    wxXmlProperty *prop;

    switch (node->GetType())
    {
        case wxXML_TEXT_NODE:
            OutputStringEnt(stream, node->GetContent(), convMem, convFile);
            break;

        case wxXML_ELEMENT_NODE:
            OutputString(stream, wxT("<"), NULL, NULL);
            OutputString(stream, node->GetName(), NULL, NULL);

            prop = node->GetProperties();
            while (prop)
            {
                OutputString(stream, wxT(" ") + prop->GetName() +
                             wxT("=\"") + prop->GetValue() + wxT("\""),
                             NULL, NULL);
                // FIXME - what if prop contains '"'?
                prop = prop->GetNext();
            }

            if (node->GetChildren())
            {
                OutputString(stream, wxT(">"), NULL, NULL);
                prev = NULL;
                n = node->GetChildren();
                while (n)
                {
                    if (n && n->GetType() != wxXML_TEXT_NODE)
                        OutputIndentation(stream, indent + 1);
                    OutputNode(stream, n, indent + 1, convMem, convFile);
                    prev = n;
                    n = n->GetNext();
                }
                if (prev && prev->GetType() != wxXML_TEXT_NODE)
                    OutputIndentation(stream, indent);
                OutputString(stream, wxT("</"), NULL, NULL);
                OutputString(stream, node->GetName(), NULL, NULL);
                OutputString(stream, wxT(">"), NULL, NULL);
            }
            else
                OutputString(stream, wxT("/>"), NULL, NULL);
            break;

        case wxXML_COMMENT_NODE:
            OutputString(stream, wxT("<!--"), NULL, NULL);
            OutputString(stream, node->GetContent(), convMem, convFile);
            OutputString(stream, wxT("-->"), NULL, NULL);
            break;

        default:
            wxFAIL_MSG(wxT("unsupported node type"));
    }
}

bool wxXmlDocument::Save(wxOutputStream& stream) const
{
    if ( !IsOk() )
        return FALSE;

    wxString s;

    wxMBConv *convMem = NULL, *convFile = NULL;
#if wxUSE_UNICODE
    convFile = new wxCSConv(GetFileEncoding());
#else
    if ( GetFileEncoding() != GetEncoding() )
    {
        convFile = new wxCSConv(GetFileEncoding());
        convMem = new wxCSConv(GetEncoding());
    }
#endif

    s.Printf(wxT("<?xml version=\"%s\" encoding=\"%s\"?>\n"),
             GetVersion().c_str(), GetFileEncoding().c_str());
    OutputString(stream, s, NULL, NULL);

    OutputNode(stream, GetRoot(), 0, convMem, convFile);
    OutputString(stream, wxT("\n"), NULL, NULL);

    if ( convFile )
        delete convFile;
    if ( convMem )
        delete convMem;

    return TRUE;
}


/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsXmlContext::dmsXmlContext(dmsvContextManager* manager, dmsValidator* mapping, const wxString &path) : dmsvContext(manager, mapping)
{
    m_oPath = path;
}


dmsXmlContext::~dmsXmlContext()
{

}

/* ========================================================================

   ======================================================================== */

bool dmsXmlContext::GetValue(wxString &value)
{
    return ((dmsXmlContextManager*) m_poManager)->m_poNode->ReadTmpString(m_oPath, &value,false);
}

bool dmsXmlContext::SetValue(const wxString &value)
{
    TODO();
    return false;
}


void dmsXmlContext::ShowError()
{
    dmsXmlContextManager *m = (dmsXmlContextManager*) m_poManager;

    LOGE(L"Tag [%s] : %s", m->m_poNode->GetLongName(m_oPath), m_poValidator->GetError());

    m_poValidator->Clear();
}

/* ########################################################################

   ######################################################################## */

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsXmlContextManager::dmsXmlContextManager(wxXmlNode *node) : dmsvContextManager()
{
    m_poNode = node;

    m_poValidatorManager = new dmsValidatorManager();
}


dmsXmlContextManager::~dmsXmlContextManager()
{
    DELNUL(m_poValidatorManager);
}


/* ========================================================================

   ======================================================================== */

dmsXmlContext* dmsXmlContextManager::Add(const wxString &path, dmsValidator* mapping)
{
    mapping->SetManager(m_poValidatorManager);
    mapping->SetName(path);

    return new dmsXmlContext(this, mapping, path);
}

