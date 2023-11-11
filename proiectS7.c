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
#define STR_SIZE 280

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

//this function opens the directory given as argument to my program
void openDirectory(char *path, DIR **dir)
{
    *dir = opendir(path);
    
    if(*dir==NULL)
    {
        perror("Error when opening directory!");
        exit(10);
    }
}

/*every function which works with the file descriptors fIn and fOut will have their adresses passed so I can work with
  them through all my code*/

//this function opens any type of file met inside the directory I work with
void openSourceFile(int *fIn, char *path)
{
    if((*fIn=open(path, O_RDONLY)) < 0)
    {
        perror("Could not open the source file!");
        exit(4);
    }
}

//this function opens the output file using specific options
void openOutputFile(int *fOut)
{
    if((*fOut=open("statistica.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR)) < 0)
    {
        perror("Could not open the output (.txt) file!");
        exit(5);
    }
}

//this function writes inside the output file the name of the input file
//I created a separate function because this information is not contained within the .bmp file header
void writeFileNameToOutput(struct dirent *dirInput, __uint8_t buffer2[], int *fOut)
{
    sprintf(buffer2, "File Name: %s\n", dirInput->d_name);
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

//this function writes the access rights information in a simple format to output
void writeAccessRightsInfo(struct stat arg, __uint8_t buffer2[], int *fOut)
{
    mode_t rights = arg.st_mode;
    char bufferRights[3] = "";

    // building output for user rights
    if (rights & S_IRUSR)
        strcat(bufferRights, "R");
    else
        strcat(bufferRights, "-");

    if (rights & S_IWUSR)
        strcat(bufferRights, "W");
    else
        strcat(bufferRights, "-");

    if (rights & S_IXUSR)
        strcat(bufferRights, "X");
    else
        strcat(bufferRights, "-");

    // writing the output for user
    sprintf(buffer2, "User Rights: %s\n", bufferRights);
    if (write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    // emptying buffer
    strcpy(bufferRights, "");

    // building output for group rights
    if (rights & S_IRGRP)
        strcat(bufferRights, "R");
    else
        strcat(bufferRights, "-");

    if (rights & S_IWGRP)
        strcat(bufferRights, "W");
    else
        strcat(bufferRights, "-");

    if (rights & S_IXGRP)
        strcat(bufferRights, "X");
    else
        strcat(bufferRights, "-");

    // writing the output for group
    sprintf(buffer2, "Group Rights: %s\n", bufferRights);
    if (write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    // emptying buffer
    strcpy(bufferRights, "");

    // building output for others rights
    if (rights & S_IROTH)
        strcat(bufferRights, "R");
    else
        strcat(bufferRights, "-");

    if (rights & S_IWOTH)
        strcat(bufferRights, "W");
    else
        strcat(bufferRights, "-");

    if (rights & S_IXOTH)
        strcat(bufferRights, "X");
    else
        strcat(bufferRights, "-");

    // writing the output for others
    sprintf(buffer2, "Others Rights: %s\n\n", bufferRights);
    if (write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    // emptying buffer
    strcpy(bufferRights, "");
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

    writeAccessRightsInfo(arg, buffer2, fOut);
}

/*this function writes a regular file's size (using stat) to output because for the .bmp files I used information provided by 
  its header*/
void writeFileSizeToOutputUsingStat(char *path, struct stat arg, char buffer2[], int *fOut)
{
    if ((stat(path, &arg)) < 0)
    {
        perror("Bad call");
        exit(2);
    }

    sprintf(buffer2, "File Size: %lu bytes\n", arg.st_size);
    if(write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }
}

//this function extracts the necessary data regarding a symbolic link
void writeSymLinkInfoToOutput(char path[], __uint8_t buffer2[], int *fOut, struct stat arg, struct dirent *dirInput)
{
    //I call the stat function so I can get the info from the file which is pointed by the symbolic link
    struct stat target;
    if ((stat(path, &target)) < 0)
    {
        perror("Bad call");
        exit(11);
    }

    sprintf(buffer2, "Symbolic Link Name: %s\n", dirInput->d_name);
    if (write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    sprintf(buffer2, "Symbolic Link Size: %lu bytes\n", arg.st_size);
    if (write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    sprintf(buffer2, "Target File Size: %lu bytes\n", target.st_size);
    if (write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Could not write!");
        exit(6);
    }

    writeAccessRightsInfo(arg, buffer2, fOut);
}

//this function outputs information about a directory met as an entry inside the directroy I work with
void writeDirInfoToOutput(__uint8_t buffer2[], struct dirent *dirInput, int *fOut, struct stat arg)
{
    sprintf(buffer2, "Directory name: %s\n", dirInput->d_name);
    if (write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Error while writing! change message");
        exit(15);
    }

    sprintf(buffer2, "User ID: %u\n", arg.st_uid);
    if (write(*fOut, buffer2, strlen(buffer2)) < 0)
    {
        perror("Error while writing! change message");
        exit(15);
    }

    writeAccessRightsInfo(arg, buffer2, fOut);
}

//this function closes any type of file
void closeFile(int *fd)
{
    if((close(*fd)) < 0)
    {
        perror("Could not close the file!");
        exit(14);
    }
}


//this function closes the output file
void closeOutputFile(int *fOut)
{
    if((close(*fOut)) < 0)
    {
        perror("Could not close the file!");
        exit(14);
    }
    printf("Program ended successfully! Every information is now written in statistica.txt!\n");
}

//this function closes any directory
void closeDirectory(DIR **dir)
{
    if((closedir(*dir))!=0)
    {
        perror("Error when closing directory!");
        exit(13);
    }
}

int main(int argc, char *argv[])
{
    struct stat arg;                               //the struct stat type variable used to store statistics about the .bmp file

    initialize(argc, argv, arg);

    int fInBmp, fInReg, fInLnk, fOut;              //file descriptors
    off_t offset;                                  //the offset I manually set so I can read what I want from the .bmp header
    __uint8_t buffer[BUFFSIZE], buffer2[STR_SIZE]; //buffers used for reading and writing info
    DIR *dir;                                      //pointer of type DIR* used to store the address of the directory I work with
    struct dirent *dirInput;                       //the struct dirent type pointer used to point to each directory entry we parse

    openDirectory(argv[1], &dir);
    
    openOutputFile(&fOut);

    //parsing every entry in our directory
    while((dirInput=readdir(dir))!=NULL)
    {
        //building the path for each entry
        char path[STR_SIZE] = "";
        strcpy(path, argv[1]);
        char filename[STR_SIZE] = "";
        strcpy(filename, dirInput->d_name);
        strcat(path, "/");
        strcat(path, filename);
        //printf("CALEA: %s\n", path);

        //getting information about each entry
        if((lstat(path, &arg)) < 0)
        {
            perror("Bad call");
            exit(11);
        }

        //checking each entry's type and writing the correct information to the output file
        if(S_ISLNK(arg.st_mode))
        {
            openSourceFile(&fInLnk, path);
            
            writeSymLinkInfoToOutput(path, buffer2, &fOut, arg, dirInput);

            closeFile(&fInLnk);
        }
        else{
            if(S_ISREG(arg.st_mode))
            {
                if(strcmp(path+strlen(path)-strlen(".bmp"), ".bmp")==0)
                {
                    openSourceFile(&fInBmp, path);

                    writeFileNameToOutput(dirInput, buffer2, &fOut);
                    
                    readFromBMPHeaderWriteInfoToOutput(offset, &fInBmp, &fOut, buffer, buffer2);
                    
                    writeInfoToOutputUsingStatInfo(path, arg, buffer2, &fOut);
                    
                    closeFile(&fInBmp);
                }
                else
                {
                    openSourceFile(&fInReg, path);

                    writeFileNameToOutput(dirInput, buffer2, &fOut);

                    writeFileSizeToOutputUsingStat(path, arg, buffer2, &fOut);

                    writeInfoToOutputUsingStatInfo(path, arg, buffer2, &fOut);

                    closeFile(&fInReg);
                }
            }
            else
            {
                if (S_ISDIR(arg.st_mode))
                {
                    DIR *entryDir;

                    openDirectory(path, &entryDir);

                    writeDirInfoToOutput(buffer2, dirInput, &fOut, arg);

                    closeDirectory(&entryDir);
                }
            }
        }
    }

    closeOutputFile(&fOut);

    closeDirectory(&dir);

    return 0;
}