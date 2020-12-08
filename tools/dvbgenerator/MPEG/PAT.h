/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _DVB_HEADERS_PAT_H_
#define _DVB_HEADERS_PAT_H_

#include <Tools/Header.h>

#include <MPEG/MPEG.h>
#include <MPEG/DESC.h>


class wxXmlNode;
class dmsMPEG_Section;


/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */

class dmsMPEG_DescProgramNumberPID : public dmsData
{
public:
    u16 ProgramNumber;
    u3  Reserved;
    u13 PID;

    dmsMPEG_DescProgramNumberPID();
};

class dmsMPEG_DescProgramNumberPIDList : public dmsDataList
{
public:
    dmsMPEG_DescProgramNumberPIDList():dmsDataList(){;}
    dmsData *Create(void *pt, const char *tag);
};

/* ------------------------------------------------------------------------

   ------------------------------------------------------------------------ */


class dmsMPEG_PAT_Data : public dmsData
{
public:
    dmsMPEG_DescProgramNumberPIDList *Loop;

    dmsMPEG_PAT_Data(dmsMPEG_Section* section);
};

#endif /* _DVB_HEADERS_PAT_H_ */
