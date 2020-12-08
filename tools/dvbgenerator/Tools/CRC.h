/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _DVB_HEADER_CRC_H_
#define _DVB_HEADER_CRC_H_

unsigned int  CalculateCRC32(unsigned char *buffer, unsigned int size);
unsigned short CalculateCRC16(unsigned char *buffer, unsigned int size);

#endif /* _DVB_HEADER_CRC_H_ */
