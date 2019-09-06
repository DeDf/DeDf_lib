
#include <stdio.h>
#include <windows.h>

char *my_strstr(const char *s1, const char *s2)  // ´Ó×óµ½ÓÒ
{
    char *str = NULL;

    if (s1 && s2)
    {
        unsigned int i;

        if (*s2) 
        { 
            while (*s1) 
            { 
                for (i = 0; *(s1 + i) == *(s2 + i); i++)
                { 
                    if (!*(s2 + i + 1)) 
                        return (char *)s1;
                }
                ++s1;
            }
        } 
    }

    return str;
}

int main(int argc, char *argv[])
{
    char *a = my_strstr("asdfaaa", "aa");

    char *b = my_strstr("asdfaaa", "ef");
    getchar();
    return 0;
}
