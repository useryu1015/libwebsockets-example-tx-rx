#include <stdio.h>

#include "zlog.h"



int main()
{

    char str[] = "{\"json\":{\"name\":\"终端1\",\"id\":\"202303110001xl\",\"terminal\":\"COL\",\"mtype\":\"MSGE\",\"t\":\"2023-04-05 09:28:47.475\",\"params\":[{\"kks\":\"kks编码\",\"ref\":\"发变811层/机组1.水压1\",\"val\":\"0.56-Mpa\",\"t\":\"2023-04-05 09:28:47.872\"}]}}";
    // char str[] = "{\"json\":{\"name\":\"terminal1\",\"id\":\"202303110001xl\",\"terminal\":\"COL\",\"mtype\":\"MSGE\",\"t\":\"2023-04-05 09:28:47.475\",\"params\":[{\"kks\":\"kkstest\",\"ref\":\"rekkstestff1\",\"val\":\"0.56-Mpa\",\"t\":\"2023-04-05 09:28:47.872\"}]}}";
    char buf[10240];
    char *ps = str;
    int i;

    printf("test: \n%s\n\n",str);


    for (i = 0; *(str + i) != '\0'; i++)
    {
        printf("%d,", *(str + i));
        // printf("%c,", *(str + i));

        // usleep(10 * 1000);
    }

    printf("\n\n\n\n");

    for (i = 0; *(str + i) != '\0'; i++)
    {
        // printf("%d,", *(str + i));
        printf("%c", *(str + i));

        // usleep(10 * 1000);
    }

}


