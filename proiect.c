#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFSIZE 4

//SAPTAMANA 6

int main(int argc, char *argv[])
{
    char errorString[50] = "Usage";
    strcat(errorString, " ");
    strcat(errorString, argv[0]);
    if(argc!=2)
    {
        perror(errorString);
        exit(1);
    }
    strcat(errorString, " ");
    strcat(errorString, argv[1]);
    struct stat arg;
    if ((stat(argv[1], &arg)) < 0)
    {
        perror("Bad call");
        exit(2);
    }
    if(!S_ISREG(arg.st_mode) || strcmp(argv[1]+strlen(argv[1])-strlen(".bmp"), ".bmp")!=0)
    {
        perror(errorString);
        exit(3);
    }
    printf("%s\n%s\n", argv[0], argv[1]);

    int fIn, fOut; //descriptor fisier
    int rd, offset;

    if((fIn=open(argv[1], O_RDONLY)) < 0)
    {
        perror("Nu s-a putut deschide fisierul .bmp!");
        exit(4);
    }

    if((fOut=open("statistica.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IWUSR)) < 0)
    {
        perror("Nu s-a putut deschide fisierul de scriere!");
        exit(5);
    }

    offset = lseek(fIn, 2, SEEK_SET);
    printf("%d\n", offset);
    fIn+=offset;

    /*int fIn, fOut, rd;
    struct stat var;
    int count = 0;
    char buffer[BUFFSIZE], buff2[BUFFSIZE], buff3[BUFFSIZE];
    if((fIn=open(argv[1], O_RDONLY)) < 0)
    {
        perror("Eroare!");
        exit(6);
    }
    if((fOut=open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, S_IWUSR)) < 0)
    {
        perror("Eroare!");
        exit(7);
    }
    while((rd=read(fIn, buffer, BUFFSIZE)) != 0)
    {
       // sprintf(buffer, "%s", buff2);
        for(int i = 0; i < rd; i++)
        {
            if(isalnum(buffer[i]))
            {
                count++;
            }
        }
    }
    sprintf(buff2, "count: %d\n", count);
    //sprintf(st_uid, "ID: %d", ); var.st_uid

    if(write(fOut, buff2, strlen(buff2)) < 0)
    {
        perror("Nu s-a putut efectua scrierea!");
        exit(8);
    }

    if (fstat(fIn, &var))
    {
        perror("Bad call");
        exit(9);
    }
    else sprintf(buff3, "User ID : %d", var.st_uid);

    if(write(fOut, buff3, strlen(buff3)) < 0)
    {
        perror("Nu s-a putut efectua scrierea!");
        exit(8);
    }
    close(fIn);
    close(fOut);*/
    
    return 0;
}