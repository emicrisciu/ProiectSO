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
    int rd;
    off_t offset;
    __uint8_t buffer[BUFFSIZE], buffer2[50];

    if((fIn=open(argv[1], O_RDONLY)) < 0)
    {
        perror("Nu s-a putut deschide fisierul .bmp!");
        exit(4);
    }

    if((fOut=open("statistica.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR)) < 0)
    {
        perror("Nu s-a putut deschide fisierul de scriere!");
        exit(5);
    }

    offset = lseek(fIn, 2, SEEK_SET);
    printf("%ld\n", offset);

    if(read(fIn, buffer, BUFFSIZE) != -1)
    {
        //sprintf(buffer2, "FileSize: %u\n", buffer);
        //stat(argv[1], &arg); sprintf(buffer2, "FileSize: %ld bytes\n", arg.st_size);

        sprintf(buffer2, "FileSize: %u bytes\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Nu s-a putut efectua scrierea!");
            exit(6);
        }
    }
    else {
        perror("Eroare la citire!");
        exit(7);
    }

    offset = lseek(fIn, 12, SEEK_CUR);
    printf("%ld\n", offset);

    if(read(fIn, buffer, BUFFSIZE) != -1)
    {
        //sprintf(buffer2, "FileSize: %u\n", buffer);
        //stat(argv[1], &arg); sprintf(buffer2, "FileSize: %ld bytes\n", arg.st_size);

        sprintf(buffer2, "Width: %u px\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Nu s-a putut efectua scrierea!");
            exit(6);
        }
    }
    else {
        perror("Eroare la citire!");
        exit(7);
    }    

    if(read(fIn, buffer, BUFFSIZE) != -1)
    {
        //sprintf(buffer2, "FileSize: %u\n", buffer);
        //stat(argv[1], &arg); sprintf(buffer2, "FileSize: %ld bytes\n", arg.st_size);

        sprintf(buffer2, "Height: %u px\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Nu s-a putut efectua scrierea!");
            exit(6);
        }
    }
    else {
        perror("Eroare la citire!");
        exit(7);
    } 

    offset = lseek(fIn, 8, SEEK_CUR);
    printf("%ld\n", offset);

    if(read(fIn, buffer, BUFFSIZE) != -1)
    {
        //sprintf(buffer2, "FileSize: %u\n", buffer);
        //stat(argv[1], &arg); sprintf(buffer2, "FileSize: %ld bytes\n", arg.st_size);

        sprintf(buffer2, "ImageSize: %u bytes\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Nu s-a putut efectua scrierea!");
            exit(6);
        }
    }
    else {
        perror("Eroare la citire!");
        exit(7);
    } 

    close(fIn);
    close(fOut);
    
    return 0;
}