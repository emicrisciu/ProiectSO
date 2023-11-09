#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define BUFFSIZE 4

//SAPTAMANA 6

int main(int argc, char *argv[])
{
    //initial phase: building the error message and checking the correctness of the argument
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
    printf("%s\n%s\n", argv[0], argv[1]); //printing the argument(s) into the console to make sure everything works fine

    int fIn, fOut; //file descriptors
    off_t offset; //the offset I manually set so I can read what I want from the .bmp header
    __uint8_t buffer[BUFFSIZE], buffer2[50]; //buffers used for reading and writing info

    //reading and writing phase
    //reading from the .bmp header and writing into statstica.txt
    if((fIn=open(argv[1], O_RDONLY)) < 0)
    {
        perror("Could not open the .bmp file!");
        exit(4);
    }

    if((fOut=open("statistica.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR)) < 0)
    {
        perror("Could not open the output (.txt) file!");
        exit(5);
    }

    //here I take the file's name directly from the argument since everyhing's already checked
    sprintf(buffer2, "FileName: %s\n", argv[1]);
    if(write(fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //reading from header starts
    offset = lseek(fIn, 2, SEEK_SET);
    printf("%ld\n", offset);

    if(read(fIn, buffer, BUFFSIZE) != -1)
    {
        sprintf(buffer2, "FileSize: %u bytes\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Could not write!");
            exit(6);
        }
    }
    else {
        perror("Reading error!");
        exit(7);
    }

    offset = lseek(fIn, 12, SEEK_CUR);
    printf("%ld\n", offset);

    if(read(fIn, buffer, BUFFSIZE) != -1)
    {
        sprintf(buffer2, "Width: %u px\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Could not write!");
            exit(6);
        }
    }
    else {
        perror("Reading error!");
        exit(7);
    }    

    if(read(fIn, buffer, BUFFSIZE) != -1)
    {
        sprintf(buffer2, "Height: %u px\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Could not write!");
            exit(6);
        }
    }
    else {
        perror("Reading error!");
        exit(7);
    } 

    offset = lseek(fIn, 8, SEEK_CUR);
    printf("%ld\n", offset);

    if(read(fIn, buffer, BUFFSIZE) != -1)
    {
        sprintf(buffer2, "ImageSize: %u bytes\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Could not write!");
            exit(6);
        }
    }
    else {
        perror("Reading error!");
        exit(7);
    }
    //reading from header ends

    //writing inside statistica.txt using stats provided by stat function
    //UID
    sprintf(buffer2, "User ID: %u\n", arg.st_uid);
    if(write(fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //Time of last modification
    time_t last_modif_time = arg.st_mtime;
    sprintf(buffer2, "Time of last modification: %s", ctime(&last_modif_time));
    if(write(fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //LinksCount
    sprintf(buffer2, "Links count: %ld\n", arg.st_nlink);
    if(write(fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //User, group and others rights
    mode_t rights = arg.st_mode;
    char bufferRights[3] = "";

    //building output for user rights
    if(rights & S_IRUSR) strcat(bufferRights, "R");
    else strcat(bufferRights, "-");

    if(rights & S_IWUSR) strcat(bufferRights, "W");
    else strcat(bufferRights, "-");

    if(rights & S_IXUSR) strcat(bufferRights, "X");
    else strcat(bufferRights, "-");

    //writing the output for user
    sprintf(buffer2, "User rights: %s\n", bufferRights);
    if(write(fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //empty buffer
    strcpy(bufferRights, "");

    //building output for group rights
    if(rights & S_IRGRP) strcat(bufferRights, "R");
    else strcat(bufferRights, "-");

    if(rights & S_IWGRP) strcat(bufferRights, "W");
    else strcat(bufferRights, "-");

    if(rights & S_IXGRP) strcat(bufferRights, "X");
    else strcat(bufferRights, "-");

    //writing the output for group
    sprintf(buffer2, "Group rights: %s\n", bufferRights);
    if(write(fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //empty buffer
    strcpy(bufferRights, "");

    //building output for others rights
    if(rights & S_IROTH) strcat(bufferRights, "R");
    else strcat(bufferRights, "-");

    if(rights & S_IWOTH) strcat(bufferRights, "W");
    else strcat(bufferRights, "-");

    if(rights & S_IXOTH) strcat(bufferRights, "X");
    else strcat(bufferRights, "-");

    //writing the output for others
    sprintf(buffer2, "Others rights: %s\n", bufferRights);
    if(write(fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //empty buffer
    strcpy(bufferRights, "");

    //closing the files I worked with
    close(fIn);
    close(fOut);
    
    return 0;
}