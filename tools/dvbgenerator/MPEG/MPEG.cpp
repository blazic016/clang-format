/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 12/2004 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <wx/ffile.h>

#include <Tools/Header.h>
#include <Tools/Xml.h>
#include <Tools/CRC.h>

#include "MPEG.h"


/* ########################################################################

   ######################################################################## */

//#include <wx/listimpl.cpp>
//WX_DEFINE_LIST(dmsMPEG_SectionList);


/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */

dmsMPEG_MuxSection::dmsMPEG_MuxSection() : dmsData()
{

    m_iPID             = 0;
    m_iOutputFrequency = 0;
    m_iGroup           = 0;
    m_iDebugMissing    = 0;
}


dmsMPEG_MuxSection::~dmsMPEG_MuxSection()
{

}


/* ========================================================================

   ======================================================================== */


bool dmsMPEG_MuxSection::Load(wxXmlNode *node)
{
    node->Read("OutputFrequency?", &m_iOutputFrequency);
    node->Read("PID?",             &m_iPID);

    return true;
}




/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsMPEG_Section::dmsMPEG_Section() : dmsMPEG_MuxSection()
{
    // Intialisation

    HDR_INIT(TableId,                8);
    HDR_INIT(SectionSyntaxIndicator, 1);
    HDR_INIT(PrivateIndicator,       1);
    HDR_INIT(Reserved1,              2);
    HDR_INIT(SectionLength,          12);
    HDR_INIT(TableIdExtension,       16);
    HDR_INIT(Reserved2,              2);
    HDR_INIT(VersionNumber,          5);
    HDR_INIT(CurrentNextIndicator,   1);
    HDR_INIT(SectionNumber,          8);
    HDR_INIT(LastSectionNumber,      8);
    HDR_INIT(Data,                   0);
    HDR_INIT(CRC,                    32);

    SetName("Section");

    SectionSyntaxIndicator = 1;
    PrivateIndicator       = 0;
    Reserved1              = 3;
    Reserved2              = 3;
    CurrentNextIndicator   = 1;

    // Calculs des longueurs

    SetLenLimit(&SectionLength, &CRC);

    SetLoad(&TableId,          true);
    SetLoad(&TableIdExtension, true);
    SetLoad(&VersionNumber,    true);
    SetLoad(&Data,             "");
}


void dmsMPEG_Section::Generate(dmsHtoN &hton)
{
    dmsData::Generate(hton);

    hton.UpdateCRC32(CRC);
}



bool dmsMPEG_Section::Load(wxXmlNode *node)
{
    if (! dmsData::Load(node)) return false;
    if (! dmsMPEG_MuxSection::Load(node)) return false;

    return true;
}




/* ########################################################################

   ######################################################################## */


/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsMPEG_SectionSSI0::dmsMPEG_SectionSSI0() : dmsMPEG_MuxSection()
{
    // Intialisation

    HDR_INIT(TableId,                8);
    HDR_INIT(SectionSyntaxIndicator, 1);
    HDR_INIT(PrivateIndicator,       1);
    HDR_INIT(Reserved1,              2);
    HDR_INIT(SectionLength,          12);
    HDR_INIT(Data,                   0);

    SetName("Section");

    SectionSyntaxIndicator = 0;
    PrivateIndicator       = 0;
    Reserved1              = 3;

    // Calculs des longueurs

    SetLenLimit(&SectionLength, &Data);

    SetLoad(&TableId, true);
    SetLoad(&Data, "");
}


bool dmsMPEG_SectionSSI0::Load(wxXmlNode *node)
{
    if (! dmsData::Load(node)) return false;
    if (! dmsMPEG_MuxSection::Load(node)) return false;

    return true;
}



/* ########################################################################

   ######################################################################## */



#define POINTER_FIELD                   0x00
#define PACKET_POINTER_FIELD_OFFSET     0
#define PACKET_DATA_BYTE_OFFSET         1

//int g_PID;

/*****************************************************************************
*
* BCD_extract_packet_from_buffer()
*
* Parameters:
*       buffer  :   contains the expected section which is composed by its header,
*                   data and crc32
*       packet  :   structure containning the packet informations
*
* Description:
*       extracts data from the buffer and completes the packet structure. We must
*       distinguish two treatments depending on whether it's the first packet of
*       the expected section or not.
*
* Returns:
*       true    :   a packet has been extracted
*       false   :   the end of the section has been reached
*
******************************************************************************/

int BCD_extract_packet_from_buffer(unsigned char *buffer, BCD_mpeg_packet_t *packet, int buffer_size, int pid, int *byteRead, u8 *cc)
{
    //static int                    byteRead = 0;   /* save the number of byte read before */
    //static unsigned char      cc = 0;         /* save the number of packet created */
    int                         i;              /* increment */
    int                         diff = buffer_size - (*byteRead);   /* account of byte which
                                                                stay to read */
    /*************************************************/
    /* wrap around cc variable if it's equal to 0x0F */
    /*************************************************/
    if ((*cc) > 0x0F)
    {
        (*cc) = 0;
    }

    /**********************************************************************/
    /* creation of the adaptation field, continuity counter and transport */
    /* scrambling control                                                 */
    /**********************************************************************/
    packet->tsc_afc_continuity_counter = (*cc) | (unsigned char) (1 << 4);

    (*cc)++;

    if ((*byteRead) == 0)
    {
        /*********************************************************************************/
        /* that is the first packet of the section. So, the payload_unit_start_indicator */
        /* field is set to 1 and one byte (pointer field) set to 0 is added after the    */
        /* header of the packet                                                          */
        /*********************************************************************************/
        packet->payload_unit_start_indicator_pid = (unsigned short)(pid | 1 << 14);

        /*******************************************************************************/
        /* In this case, we check if the staying byte account is superior to 183 bytes */
        /* or not.                                                                     */
        /*******************************************************************************/
        if (diff >= MPEG_PACKET_DATA_SIZE-1)
        {

            /*********************************************************/
            /* We added the pointer field byte and 183 bytes of data */
            /*********************************************************/
            packet->packet_data[PACKET_POINTER_FIELD_OFFSET] = POINTER_FIELD;
            memcpy(&packet->packet_data[PACKET_DATA_BYTE_OFFSET], &buffer[(*byteRead)],
                                                            MPEG_PACKET_DATA_SIZE-1);
            (*byteRead) += MPEG_PACKET_DATA_SIZE-1;
            return TRUE;
        }
        else
        {
            /***********************************************************************/
            /* The staying byte account is inferior to 183 bytes. So, we added the */
            /* pointer field, the staying bytes and stuffing bis 183 bytes         */
            /***********************************************************************/
            packet->packet_data[PACKET_POINTER_FIELD_OFFSET] = POINTER_FIELD;
            memcpy(&packet->packet_data[PACKET_DATA_BYTE_OFFSET], &buffer[(*byteRead)], diff);
            for (i = diff+1; i < MPEG_PACKET_DATA_SIZE; i++)
            {
                packet->packet_data[i] = (unsigned char) 0xFF;
            }
            (*byteRead) = 0;
            return FALSE;
        }
    }
    else
    {

        /************************************************************************/
        /* this isn't the first packet. So, the payload_unit_start_indicator is */
        /* set to 0 and there isn't pointer field                               */
        /************************************************************************/
        packet->payload_unit_start_indicator_pid = (unsigned short)(pid);

        /******************************************************************/
        /* We check if the staying byte account is superior to 184 bytes. */
        /******************************************************************/
        if (diff >= MPEG_PACKET_DATA_SIZE)
        {
            /***************************************************/
            /* If it's right, we copy 184 bytes of the section */
            /***************************************************/
            memcpy(packet->packet_data, &buffer[(*byteRead)], MPEG_PACKET_DATA_SIZE);
            (*byteRead) += MPEG_PACKET_DATA_SIZE;
            return TRUE;
        }
        else
        {
            /*********************************************************************/
            /* if it's wrong, we copy the number of staying bytes and stuff with */
            /* 0xFF bis 184 bytes                                                */
            /*********************************************************************/
            memcpy(packet->packet_data, &buffer[(*byteRead)], diff);

            for (i = diff; i < MPEG_PACKET_DATA_SIZE; i++)
            {
                packet->packet_data[i] = (unsigned char) 0xFF;
            }

            (*byteRead) = 0;
            return FALSE;
        }
    }
}


/* ========================================================================

   ======================================================================== */


static void AddPidAndCC(dmsHtoNBuffer &hton, unsigned char *buffer, int pid, u8* cc)
{
    if ((*cc)>0x0F) (*cc) = 0;

    hton.Skip(1,3);
    hton.Write((u16)pid, 13);
    hton.Skip(0,4);
    hton.Write(*cc, 4);

    (*cc)++;
}


/* ========================================================================

   ======================================================================== */

static const u_8 StuffingPacket[188] =
{
    0x47,0x5f,0xff,0x10,0x00,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff
};


void dmsMpegGetStuffingPacket(unsigned char *buffer, u8* cc)
{
    dmsHtoNBuffer hton(buffer, 188, false);

    memcpy(buffer, StuffingPacket, sizeof(StuffingPacket));

    AddPidAndCC(hton, buffer, 0x1FFF, cc);
}



/* ------------------------------------------------------------------------
   Paquet de Stuffing avec PCR
   ------------------------------------------------------------------------

    [ 0]  8 : Sync-Byte 0x47               :  71 (0x47)
    [ 1]  1 : Transport_error_indicator    :   0 (0x00)    [= packet ok]
    [  ]  1 : Payload_unit_start_indicator :   0 (0x00)    [= Packet data continues]
    [  ]  1 : transport_priority           :   0 (0x00)
    [  ] 13 : PID                          : 510 (0x01fe)  [= NIT, PMT or Elementary PID, etc.]
    [ 3]  2 : transport_scrambling_control :   0 (0x00)    [= No scrambling of TS packet payload]
    [  ]  2 : adaptation_field_control     :   2 (0x02)    [= adaptation_field only, no payload]
    [  ]  4 : continuity_counter           :  15 (0x0f)    [= (sequence ok)]
    [  ]  - :     Adaptation_field:
    [ 4]  8 :         adaptation_field_length              : 183 (0xb7)
    [ 5]  1 :         discontinuity_indicator              :   0 (0x00)
    [  ]  1 :         random_access_indicator              :   0 (0x00)
    [  ]  1 :         elementary_stream_priotity_indicator :   0 (0x00)
    [  ]  1 :         PCR_flag                             :   1 (0x01)
    [  ]  1 :         OPCR_flag                            :   0 (0x00)
    [  ]  1 :         splicing_point_flag                  :   0 (0x00)
    [  ]  1 :         transport_private_data_flag          :   0 (0x00)
    [  ]  1 :         adaptation_field_extension_flag      :   0 (0x00)
    [ 6] 48 :         program_clock_reference:



    PCR = Heure en micro-seconde * 27 =
    [  ]  8             baseH     :  0 (0x00)         = PCR / 300 / 2^32
    [  ] 25             baseL     :  0 (0x00000000)   = PCR / 300 % 2^32
    [  ]  6             reserved  : 63 (0x3f)
    [  ]  9             extension :  0 (0x0000)       = PCR % 300

                ==> program_clock_reference: 0 (0x00000000)  [= PCR-Timestamp: 0:00:00.000000]

   ------------------------------------------------------------------------ */

static const u_8 StuffingPcrPacket[188] =
{
    /*       PID      CC                        PCR                               */
    /*     .......    .           ..............................                  */
    0x47,0x00,0x00,0x20,0xb7,0x10,0x00,0x00, 0x00,0x00,0x7e,0x00,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff
};


void dmsMpegGetStuffingPcrPacket(unsigned char *buffer, int pid, u64 pcr, u8* cc)
{
    dmsHtoNBuffer hton(buffer, 188, false);

    memcpy(buffer, StuffingPcrPacket, sizeof(StuffingPcrPacket));

    AddPidAndCC(hton, buffer, pid, cc);

    hton.Skip(2,0);
    hton.Write(pcr/300, 33);
    hton.Skip(0,6);
    hton.Write((u16)(pcr%300), 9);
}
