
#include <stdio.h>

int main()
{
    ud_t u;
    ud_init(&u);
    ud_set_mode(&u, 32);
    ud_set_syntax(&u, UD_SYN_INTEL);

    char *DisasmStart = ImageBase + p_nt_opt_hdr->AddressOfEntryPoint;
    ud_set_input_buffer(&u, (uint8_t *)DisasmStart, 200);
    ud_set_pc(&u, (uint64_t)0x4AD05046);  // ���õ�ǰ�����ڴ�ӳ����ԭʼλ�ã�ʹ�����ת�ĵ�ַ����ȷ��ʾ

    while (ud_disassemble(&u))
    {
        const char *instructions = ud_insn_asm(&u);
        printf("%s\n", instructions);
    }
}