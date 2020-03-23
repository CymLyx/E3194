#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define random(x) (rand()%x )
#define FILE_NUMBER 5       //生成文件数量
#define MAX_LENGTH (10485760)         //10M=10485760B


int main()
{
    srand(time(0));
    for (int i = 0; i < FILE_NUMBER; i++) {

        char path[100];
        sprintf_s(path, 100, "C:/Users/MECHREVO/%d.bin", (i + 1));
        printf("FILE%d:%s\n",i+1,path);
        long length = random(MAX_LENGTH)* random(MAX_LENGTH)% MAX_LENGTH +1; //文件大小上限10M

        FILE* fw;
        fopen_s(&fw, path, "wb");
        if (fw == NULL)
            return;

        for (long x = 0; x < length; x++)
        {
            unsigned char a[1] = { random(256) };
            fwrite(a, 1, 1, fw);
        }
        fclose(fw);
        printf("FILE%d CREATE OK , LENGTH:%f M\n",(i+1),(double)length/1024/1024);
    }
}