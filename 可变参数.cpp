typedef char * va_list;

#define _INTSIZEOF(n) \                     // 按int对齐的sizeof
((sizeof(n) + sizeof(int)-1) & ~(sizeof(int) - 1) ) 

// pArg = arg1 + size(arg1)
#define va_start(pArg, arg1) ( pArg = (va_list)&arg1 + _INTSIZEOF(arg1) )

#define va_arg(pArg, type) \                // *(pArg++)
    ( *(type *)((pArg += _INTSIZEOF(type)) - _INTSIZEOF(type)) ) 

#define va_end(pArg) ( pArg = (va_list)0 )  // pArg = 0

#define printf(format, ...)  WriteLogFile(format, __VA_ARGS__)  // 可变参数宏