/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 11/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#ifndef _DVB_HEADER_DSMCC_H_
#define _DVB_HEADER_DSMCC_H_

#include <Tools/Header.h>
#include <MPEG/MPEG.h>

class dmsMPEG_Section;

class dmsDSMCC_Data : public dmsData
{
public:
    u_8     ProtocolDiscriminator;
    u_8     DsmccType;
    u16     MessageId;
    u32     TransactionId;
    u_8     Reserved;
    u_8     AdaptationLength;
    u16     MessageLength;
    dmsData *Data;

    dmsMPEG_Section* m_poSection;

    dmsDSMCC_Data(dmsMPEG_Section* section);
};


#endif /* _DVB_HEADER_DSMCC_H_ */
