
#include "stdio.h" 
#include <Windows.h>  

int main()     
{
    getchar();
    return 0;  
}

const int step_table[89] = {
    7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,
    34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,
    157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,
    724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,
    2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,
    9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,
    27086,29794,32767
};

const int index_adjust[8] = {
    -1, -1, -1, -1, 2, 4, 6, 8
}; 

SHORT getnextsample();

VOID encode()
{
    int index=0;
    SHORT prev_sample=0;
    UCHAR sb;

    while (1)
    {
        SHORT cur_sample=getnextsample();   // 得到当前的采样数据
        int delta=cur_sample-prev_sample;   // 计算出和上一个的增量
        if (delta<0)
            delta = -delta, sb=8;   
        else
            sb=0;                           // sb 保存的是符号位

        ULONG code = 4*delta / step_table[index]; // 根据 step_table[] 得到一个 0~7 的值
        if (code>7)
            code=7;                         // 它描述了声音强度的变化量

        index+=index_adjust[code];          // 根据声音强度调整下次取 step_table 的序号
        if (index<0)
            index=0;                        // 便于下次得到更精确的变化量的描述
        else if (index>88)
            index=88;

        prev_sample=cur_sample;

        output4bit(code|sb);                // 加上符号位保存起来
    }
}

UCHAR getnext4bit();

VOID decode()
{
    int index=0;
    int cur_sample=0;
    UCHAR sb;

    while (1)
    {
        UCHAR code=getnext4bit();       // 得到下一个数据

        if ((code & 8))
            sb=1
        else
            sb=0;

        code &= 7;                      // 将 code 分离为数据和符号

        // 后面加的一项是为了减少误差
        int delta = (step_table[index]*code)/4 + step_table[index] / 8;

        if (sb==1)
            delta = -delta;

        cur_sample += delta;            // 计算出当前的波形数据

        if (cur_sample>32767)
            output_sample(32767);
        else if (cur_sample<-32768)
            output_sample(-32768);
        else
            output_sample(cur_sample);

        index+=index_adjust[code];
        if (index<0)
            index=0;
        if (index>88)
            index=88;
    }
}
