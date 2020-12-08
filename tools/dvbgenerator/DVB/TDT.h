/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DVB_HEADERS_TDT_H_
#define _DVB_HEADERS_TDT_H_

#include <Tools/Header.h>

#include <MPEG/MPEG.h>
#include <MPEG/DESC.h>


class wxXmlNode;
class dmsDVB_UTC_Time;


class dmsDVB_TDT_Data : public dmsData
{
public:
    dmsDVB_UTC_Time* UTC_Time;

public:
   dmsDVB_TDT_Data(dmsMPEG_SectionSSI0* section);
};



#endif /* _DVB_HEADERS_TDT_H_ */
