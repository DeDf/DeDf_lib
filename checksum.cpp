// 1.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

/*IP头定义，共20个字节*/
typedef struct _IP_HEADER 
{
    char cVersionAndHeaderLen;  // 版本信息(前4位)，头长度(后4位)
    char cTypeOfService;        // 服务类型8位
    short sTotalLenOfPacket;    // 数据包长度
    short sPacketID;            // 数据包标识
    short sSliceinfo;           // 分片使用
    char cTTL;                  // 存活时间
    char cTypeOfProtocol;       // 协议类型
    short sCheckSum;            // 校验和
    unsigned int uiSourIp;      // 源ip
    unsigned int uiDestIp;      // 目的ip
} IP_HEADER, *PIP_HEADER;

/*TCP头定义，共20个字节*/
typedef struct _TCP_HEADER 
{
    short sSourPort;               // 源端口号16bit
    short sDestPort;               // 目的端口号16bit
    unsigned int uiSequNum;        // 序列号32bit
    unsigned int uiAcknowledgeNum; // 确认号32bit
    short sHeaderLenAndFlag;       // 前4位：TCP头长度；中6位：保留；后6位：标志位
    short sWindowSize;             // 窗口大小16bit
    short sCheckSum;               // 检验和16bit
    short surgentPointer;          // 紧急数据偏移量16bit
} TCP_HEADER, *PTCP_HEADER;

typedef struct _UDP_HEADER 
{
    unsigned short usSourPort;    // 源端口号16bit
    unsigned short usDestPort;    // 目的端口号16bit
    unsigned short usLength;      // 数据包长度16bit
    unsigned short usCheckSum;    // 校验和16bit
} UDP_HEADER, *PUDP_HEADER;

// TCP/UDP伪首部，计算校验时用
typedef struct _PSD_HEADER
{
    unsigned long saddr;  //源地址
    unsigned long daddr;  //目的地址
    char mbz;             //置空
    char proto_type;      //协议类型
    unsigned short len;   //长度
} PSD_HEADER, *PPSD_HEADER;


// cksum 默认必须为 0
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

    //计算IP首部校验和
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

    //计算TCP校验和,包括校验和伪首部、TCP头、TCP数据
    {
        PSD_HEADER psdhdr;
        PUSHORT pus_psdheder = (PUSHORT)&psdhdr;
        ULONG PsdHeaderLen = sizeof(PSD_HEADER);

        //填充TCP伪首部
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

    //计算UDP校验和,包括校验和伪首部、UDP头、UDP数据
    {
        PSD_HEADER psdhdr;
        PUSHORT pus_psdheder = (PUSHORT)&psdhdr;
        ULONG PsdHeaderLen = sizeof(PSD_HEADER);

        //填充UDP伪首部
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
