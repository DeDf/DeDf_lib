// 1.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

/*IPͷ���壬��20���ֽ�*/
typedef struct _IP_HEADER 
{
    char cVersionAndHeaderLen;  // �汾��Ϣ(ǰ4λ)��ͷ����(��4λ)
    char cTypeOfService;        // ��������8λ
    short sTotalLenOfPacket;    // ���ݰ�����
    short sPacketID;            // ���ݰ���ʶ
    short sSliceinfo;           // ��Ƭʹ��
    char cTTL;                  // ���ʱ��
    char cTypeOfProtocol;       // Э������
    short sCheckSum;            // У���
    unsigned int uiSourIp;      // Դip
    unsigned int uiDestIp;      // Ŀ��ip
} IP_HEADER, *PIP_HEADER;

/*TCPͷ���壬��20���ֽ�*/
typedef struct _TCP_HEADER 
{
    short sSourPort;               // Դ�˿ں�16bit
    short sDestPort;               // Ŀ�Ķ˿ں�16bit
    unsigned int uiSequNum;        // ���к�32bit
    unsigned int uiAcknowledgeNum; // ȷ�Ϻ�32bit
    short sHeaderLenAndFlag;       // ǰ4λ��TCPͷ���ȣ���6λ����������6λ����־λ
    short sWindowSize;             // ���ڴ�С16bit
    short sCheckSum;               // �����16bit
    short surgentPointer;          // ��������ƫ����16bit
} TCP_HEADER, *PTCP_HEADER;

typedef struct _UDP_HEADER 
{
    unsigned short usSourPort;    // Դ�˿ں�16bit
    unsigned short usDestPort;    // Ŀ�Ķ˿ں�16bit
    unsigned short usLength;      // ���ݰ�����16bit
    unsigned short usCheckSum;    // У���16bit
} UDP_HEADER, *PUDP_HEADER;

// TCP/UDPα�ײ�������У��ʱ��
typedef struct _PSD_HEADER
{
    unsigned long saddr;  //Դ��ַ
    unsigned long daddr;  //Ŀ�ĵ�ַ
    char mbz;             //�ÿ�
    char proto_type;      //Э������
    unsigned short len;   //����
} PSD_HEADER, *PPSD_HEADER;


// cksum Ĭ�ϱ���Ϊ 0
USHORT CheckSum(USHORT *buffer, ULONG size, ULONG cksum)
{
    while(size > 1)
    {
        cksum += *buffer++;
        size -= sizeof(USHORT);
    }

    if(size) 
    {
        cksum += *(UCHAR*)buffer;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (USHORT)(~cksum);
}

int main()
{
    ULONG cksum;
    UCHAR IP[20] = {0x45, 0x00, 0x00, 0x75, 0x6b, 0x57, 0x40, 0x00, 0x80, 0x06, 0x8a, 0xcc, 0xac, 0x00, 0x02, 0x5b, 0xd3, 0x9f, 0x82, 0x64};
    PIP_HEADER pIPHeader = (PIP_HEADER)IP;

    //����IP�ײ�У���
    pIPHeader->sCheckSum = 0;
    pIPHeader->sCheckSum = CheckSum((USHORT *)IP, sizeof(IP), 0);

    UCHAR TCP[97] = {0xfe, 0xed, 0x00, 0x50, 0xa0, 0x95, 0xec, 0xd3, 0x7e, 0x3a, 0xee, 0x4a, 0x50, 0x18, 0x40, 0xa9, 0xca, 0xcf, 0x00, 0x00, 
        0x01, 0x01, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2c, 0x01, 0x01, 0x0c, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2c, 0x01, 0x01, 0x0c, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2c, 0x01, 0x01, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x01, 0x2c, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
        0x2c, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

    PTCP_HEADER pTCPHeader = (PTCP_HEADER)TCP;
    USHORT tcplen = ntohs(pIPHeader->sTotalLenOfPacket) - ((pIPHeader->cVersionAndHeaderLen & 0xF) * 4);

    //����TCPУ���,����У���α�ײ���TCPͷ��TCP����
    {
        PSD_HEADER psdhdr;
        PUSHORT pus_psdheder = (PUSHORT)&psdhdr;
        ULONG PsdHeaderLen = sizeof(PSD_HEADER);

        //���TCPα�ײ�
        psdhdr.saddr = pIPHeader->uiSourIp;
        psdhdr.daddr = pIPHeader->uiDestIp;
        psdhdr.mbz   = 0;
        psdhdr.proto_type = 6;
        psdhdr.len   = htons(tcplen);

        cksum = 0;
        while(PsdHeaderLen > 0)
        {
            cksum += *pus_psdheder++;
            PsdHeaderLen -= 2;
        }

        pTCPHeader->sCheckSum = 0;
        pTCPHeader->sCheckSum = CheckSum((USHORT *)pTCPHeader, tcplen, cksum);
    }

    //---------------------------------------------------------------------------------

    UCHAR IP2[20] = {0x45, 0x00, 0x00, 0x3d, 0x6b, 0x52, 0x00, 0x00, 0x80, 0x11, 0x74, 0x81, 0xac, 0x00, 0x02, 0x5b, 0xac, 0x00, 0x00, 0x81};
    UCHAR UDP[41] = {0xfc, 0x7c, 0x00, 0x35, 0x00, 0x29, 0xac, 0x9c, 0xb2, 0x7f, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x69, 0x6e, 0x66, 0x6f, 0x63, 0x32, 0x04, 0x64, 0x75, 0x62, 0x61, 0x03, 0x6e, 0x65, 0x74, 0x00, 0x00, 0x01, 0x00, 0x01};
    pIPHeader = (PIP_HEADER)IP2;
    PUDP_HEADER pUDPHeader = (PUDP_HEADER)UDP;
    USHORT udplen  = ntohs(pUDPHeader->usLength);

    //����UDPУ���,����У���α�ײ���UDPͷ��UDP����
    {
        PSD_HEADER psdhdr;
        PUSHORT pus_psdheder = (PUSHORT)&psdhdr;
        ULONG PsdHeaderLen = sizeof(PSD_HEADER);

        //���UDPα�ײ�
        psdhdr.saddr = pIPHeader->uiSourIp;
        psdhdr.daddr = pIPHeader->uiDestIp;
        psdhdr.mbz   = 0;
        psdhdr.proto_type = 17;
        psdhdr.len   = pUDPHeader->usLength;

        cksum = 0;
        while(PsdHeaderLen > 0)
        {
            cksum += *pus_psdheder++;
            PsdHeaderLen -= 2;
        }

        pUDPHeader->usCheckSum = 0;
        pUDPHeader->usCheckSum = CheckSum((USHORT *)pUDPHeader, udplen, cksum);
    }

    getchar();
    return 0;
}
