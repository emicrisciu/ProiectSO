#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>

#define BUFFSIZE 4
#define STR_SIZE 50

//initialization function: building the error message and checking the correctness of the argument
void initialize(int argc, char *argv[], struct stat arg)
{
    char errorString[STR_SIZE] = "Usage";
    strcat(errorString, " ");
    strcat(errorString, argv[0]);
    if(argc!=2)
    {
        perror(errorString);
        exit(1);
    }
    strcat(errorString, " ");
    strcat(errorString, argv[1]);
    
    if ((stat(argv[1], &arg)) < 0)
    {
        perror("Bad call");
        exit(2);
    }

    if(!S_ISDIR(arg.st_mode))
    {
        perror(errorString);
        exit(3);
    }
}

/*every function which works with the file descriptors fIn and fOut will have their adresses passed so I can work with
  them through all my code*/

//this function opens the files for reading/writing
void openFiles(int *fIn, int *fOut, char* path)
{
    if((*fIn=open(path, O_RDONLY)) < 0)
    {
        perror("Could not open the .bmp file!");
        exit(4);
    }

    if((*fOut=open("statistica.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR)) < 0)
    {
        perror("Could not open the output (.txt) file!");
        exit(5);
    }
}

//this functions writes inside the output file the name of the input file
//I created a separate function because this information is not contained within the .bmp file header
void writeFileNameToOutput(char *path, __uint8_t buffer2[], int *fOut)
{
    //here I take the file's name directly from the argument since everyhing's already checked
    sprintf(buffer2, "File Name: %s\n", path);
    if(write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }
}

/*this function reads only the necessary bytes of information from the .bmp file header and the writes the information obtained
  to the output file;
  every offset is calculated manually knowing the sizes and positions of the bytes I need from the .bmp header structure*/
void readFromBMPHeaderWriteInfoToOutput(off_t offset, int *fIn, int *fOut, __uint8_t buffer[], __uint8_t buffer2[])
{
    offset = lseek(*fIn, 2, SEEK_SET);

    if(read(*fIn, buffer, BUFFSIZE) != -1)
    {
        sprintf(buffer2, "File Size: %u bytes\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(*fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Could not write!");
            exit(6);
        }
    }
    else {
        perror("Reading error!");
        exit(7);
    }

    offset = lseek(*fIn, 12, SEEK_CUR);

    if(read(*fIn, buffer, BUFFSIZE) != -1)
    {
        sprintf(buffer2, "Width: %u px\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(*fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Could not write!");
            exit(6);
        }
    }
    else {
        perror("Reading error!");
        exit(7);
    }    

    if(read(*fIn, buffer, BUFFSIZE) != -1)
    {
        sprintf(buffer2, "Height: %u px\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(*fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Could not write!");
            exit(6);
        }
    }
    else {
        perror("Reading error!");
        exit(7);
    } 

    offset = lseek(*fIn, 8, SEEK_CUR);

    if(read(*fIn, buffer, BUFFSIZE) != -1)
    {
        sprintf(buffer2, "Image Size: %u bytes\n", (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
        if(write(*fOut, buffer2, strlen(buffer2)) < 0)
        {
            perror("Could not write!");
            exit(6);
        }
    }
    else {
        perror("Reading error!");
        exit(7);
    }
}

//this function is writing inside statistica.txt using stats provided by stat function
void writeInfoToOutputUsingStatInfo(char *path, struct stat arg, __uint8_t buffer2[], int *fOut)
{
    if ((stat(path, &arg)) < 0)
    {
        perror("Bad call");
        exit(2);
    }

    sprintf(buffer2, "User ID: %u\n", arg.st_uid);
    if(write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    time_t last_modif_time = arg.st_mtime;
    sprintf(buffer2, "Time Of Last Modification: %s", ctime(&last_modif_time));
    if(write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    sprintf(buffer2, "Links Count: %ld\n", arg.st_nlink);
    if(write(*fOut, buffer2, strlen(buffer2)) < 0)
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
    sprintf(buffer2, "User Rights: %s\n", bufferRights);
    if(write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //emptying buffer
    strcpy(bufferRights, "");

    //building output for group rights
    if(rights & S_IRGRP) strcat(bufferRights, "R");
    else strcat(bufferRights, "-");

    if(rights & S_IWGRP) strcat(bufferRights, "W");
    else strcat(bufferRights, "-");

    if(rights & S_IXGRP) strcat(bufferRights, "X");
    else strcat(bufferRights, "-");

    //writing the output for group
    sprintf(buffer2, "Group Rights: %s\n", bufferRights);
    if(write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //emptying buffer
    strcpy(bufferRights, "");

    //building output for others rights
    if(rights & S_IROTH) strcat(bufferRights, "R");
    else strcat(bufferRights, "-");

    if(rights & S_IWOTH) strcat(bufferRights, "W");
    else strcat(bufferRights, "-");

    if(rights & S_IXOTH) strcat(bufferRights, "X");
    else strcat(bufferRights, "-");

    //writing the output for others
    sprintf(buffer2, "Others Rights: %s\n", bufferRights);
    if(write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    //emptying buffer
    strcpy(bufferRights, "");
}

//this function closes the files I worked with
void closeFiles(int *fIn, int *fOut)
{
    close(*fIn);
    close(*fOut);
    printf("Program ended successfully! Every information is now written in statistica.txt!\n");
}

int main(int argc, char *argv[])
{
    struct stat arg;                               //the struct stat type variable used to store statistics about the .bmp file


    stat(argv[1], &arg);
    if(S_ISDIR(arg.st_mode)) printf("DIRECTOR!\n");
    else exit(-2);
    initialize(argc, argv, arg);
    printf("Am ajuns pana aici\n");

    int fIn, fOut;                                 //file descriptors
    off_t offset;                                  //the offset I manually set so I can read what I want from the .bmp header
    __uint8_t buffer[BUFFSIZE], buffer2[STR_SIZE]; //buffers used for reading and writing info

    //AICI COD PT S7 - init is done, open is next
    DIR *dir = opendir(argv[1]);
    printf("Am ajuns pana aici2\n");
    if(dir==NULL)
    {
        perror("Error when opening directory!");
        exit(10);
    }
    printf("Am ajuns pana aici3\n");

    struct dirent *dirInput;
    while((dirInput=readdir(dir))!=NULL)
    {
        printf("Am ajuns pana aici4\n");
        char path[STR_SIZE] = "";
        strcpy(path, argv[1]);
        char filename[STR_SIZE] = "";
        strcpy(filename, dirInput->d_name);
        strcat(path, "/");
        strcat(path, filename);
        printf("CALEA: %s\n", path);
        if((stat(path, &arg)) < 0)
        {
            perror("Bad call");
            exit(11);
        }
        if(S_ISREG(arg.st_mode) && strcmp(path+strlen(path)-strlen(".bmp"), ".bmp")==0)
        {
            printf("Am ajuns pana aici5\n");
            openFiles(&fIn, &fOut, path);
            printf("Am ajuns pana aici6\n");
            writeFileNameToOutput(path, buffer2, &fOut);
            printf("Am ajuns pana aici7\n");
            readFromBMPHeaderWriteInfoToOutput(offset, &fIn, &fOut, buffer, buffer2);
            printf("Am ajuns pana aici8\n");
            writeInfoToOutputUsingStatInfo(path, arg, buffer2, &fOut);
            printf("Am ajuns pana aici9\n");
            closeFiles(&fIn, &fOut);
        }
    }

    if((closedir(dir))!=0)
    {
        perror("Error when closing directory!");
        exit(13);
    }

    //openFiles(&fIn, &fOut, argv);

    //writeFileNameToOutput(argv, buffer2, &fOut);

    //readFromBMPHeaderWriteInfoToOutput(offset, &fIn, &fOut, buffer, buffer2);

    //writeInfoToOutputUsingStatInfo(argv, arg, buffer2, &fOut);

    //closeFiles(&fIn, &fOut);
    
    return 0;
}