/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#ifndef _MPEG_HEADER_H_
#define _MPEG_HEADER_H_

#include <Tools/Header.h>



#include <wx/wx.h>

#include <Tools/Header.h>
#include <Tools/Tools.h>

class wxXmlNode;


class dmsMPEG_MuxSection : public dmsData
{
public:
    int m_iPID;
    int m_iOutputFrequency;
    int m_iGroup;
    int m_iDebugMissing;

public:
    dmsMPEG_MuxSection();
    virtual ~dmsMPEG_MuxSection();

    bool Load(wxXmlNode *node);
};


class dmsMPEG_Section : public dmsMPEG_MuxSection
{
public:
    u8       TableId;
    u1       SectionSyntaxIndicator;
    u1       PrivateIndicator;
    u2       Reserved1;
    u12      SectionLength;
    u16      TableIdExtension;
    u2       Reserved2;
    u5       VersionNumber;
    u1       CurrentNextIndicator;
    u8       SectionNumber;
    u8       LastSectionNumber;
    dmsData* Data;
    u32      CRC;

public:
    dmsMPEG_Section();

    void Generate(dmsHtoN &hton);

    bool Load(wxXmlNode *node);
};



class dmsMPEG_SectionSSI0 : public dmsMPEG_MuxSection
{
public:
    u8       TableId;
    u1       SectionSyntaxIndicator;
    u1       PrivateIndicator;
    u2       Reserved1;
    u12      SectionLength;
    dmsData* Data;
    u32      CRC;

public:
    dmsMPEG_SectionSSI0();

    bool Load(wxXmlNode *node);
};



//WX_DECLARE_LIST(dmsMPEG_Section, dmsMPEG_SectionList);




#define MPEG_PACKET_DATA_SIZE           184

#define SYNC_BYTE                       0x47

#define MPEG_SYNC_BYTE_OFFSET           0
#define MPEG_PUSI_PID_OFFSET            1
#define MPEG_TSC_AFC_CC_OFFSET          3
#define MPEG_DATA_BYTE_OFFSET           4


typedef struct
{
    unsigned short  payload_unit_start_indicator_pid;
    unsigned char   tsc_afc_continuity_counter;
    unsigned char   packet_data[MPEG_PACKET_DATA_SIZE];
} BCD_mpeg_packet_t;


int BCD_extract_packet_from_buffer(unsigned char *buffer, BCD_mpeg_packet_t *packet, int buffer_size, int pid, int *byteRead, u8 *cc);


void dmsMpegGetStuffingPacket(unsigned char *buffer, u8* cc);
void dmsMpegGetStuffingPcrPacket(unsigned char *buffer, int pid, u64 pcr, u8* cc);



#endif /* _MPEG_HEADER_H_ */
