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
#include <sys/wait.h>
#include <unistd.h>

#define BUFFSIZE 4
#define STR_SIZE 280
#define PIXELBUFFSIZE 3

//initialization function: building the error message and checking the correctness of the arguments
void initialize(int argc, char *argv[], struct stat arg)
{
    char errorString[STR_SIZE] = "Usage";
    strcat(errorString, " ");
    strcat(errorString, argv[0]);
    if(argc!=4)
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

    strcat(errorString, " ");
    strcat(errorString, argv[2]);

    if ((stat(argv[2], &arg)) < 0)
    {
        perror("Bad call");
        exit(2);
    }

    if(!S_ISDIR(arg.st_mode))
    {
        perror(errorString);
        exit(3);
    }

    strcat(errorString, " ");
    strcat(errorString, argv[3]);

    if(argv[3][1]!='\0' || !isalnum(argv[3][0]))
    {
        perror(errorString);
        exit(errno);
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
void openOutputFile(int *fOut, char *path)
{
    if((*fOut=open(path, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR)) < 0)
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

void changeColor(off_t offset, int *fIn, __uint8_t buffer[], char *path)
{
    __uint8_t buff[BUFFSIZE];
    int fOut;
    __uint32_t width, height;
    __uint64_t noOfPixels;
    int i = 0;

    //I open the file with options to read and also write
    if((*fIn=open(path, O_RDWR)) < 0)
    {
        perror("Could not open the source file!");
        exit(4);
    }

    //there are 54 bytes of header information until I get to width and height info
    offset = lseek(*fIn, 18, SEEK_SET);

    //reading the width
    if(read(*fIn, buff, BUFFSIZE) != -1)
    {
        width = (__uint32_t)buff[0] | ((__uint32_t)buff[1] << 8) | ((__uint32_t)buff[2] << 16) | ((__uint32_t)buff[3] << 24);
    }
    else {
        perror("Reading error!");
        exit(7);
    }

    //reading the height
    if(read(*fIn, buff, BUFFSIZE) != -1)
    {
        height = (__uint32_t)buff[0] | ((__uint32_t)buff[1] << 8) | ((__uint32_t)buff[2] << 16) | ((__uint32_t)buff[3] << 24);
    }
    else {
        perror("Reading error!");
        exit(7);
    }

    //there are 28 bytes of header information remaining until I get to the raster data
    offset = lseek(*fIn, 28, SEEK_CUR);

    noOfPixels = width * height;

    while(i < noOfPixels)
    {
        if(read(*fIn, buffer, PIXELBUFFSIZE) != -1)
        {
            __uint8_t red = buffer[0];
            __uint8_t green = buffer[1];
            __uint8_t blue = buffer[2];

            __uint8_t grey = 0.299 * red + 0.587 * green + 0.114 * blue;

            buffer[0] = grey;
            buffer[1] = grey;
            buffer[2] = grey;

            lseek(*fIn, -3, SEEK_CUR);

            if(write(*fIn, buffer, PIXELBUFFSIZE) < 0)
            {
                perror("Could not write!");
                exit(6);
            }

        }
        else {
            perror("Reading error!");
            exit(7);
        }
        i++;
    }

    closeOutputFile(&fOut);
}

int main(int argc, char *argv[])
{
    struct stat arg;                               //the struct stat type variable used to store statistics about the .bmp file

    initialize(argc, argv, arg);

    int fInBmp, fInReg, fInLnk, fOut;              //file descriptors
    off_t offset;                                  //the offset I manually set so I can read what I want from the .bmp header
    __uint8_t buffer[BUFFSIZE], buffer2[STR_SIZE]; //buffers used for reading and writing info
    __uint8_t pixelBuffer[PIXELBUFFSIZE];
    DIR *dir, *dirOut;                             //pointers of type DIR* used to store the addresses of the directories I work with
    struct dirent *dirInput;                       //the struct dirent type pointer used to point to each directory entry we parse

    int pid;                                       //the process id returned by fork function
    int status;                                    //the code (status) a child process ends with
    int count = 0;                                 //counts the number of child processes created so I can check later on if they are done

    int child2child[2], child2parent[2];           //file descriptors used for pipe ends

    openDirectory(argv[1], &dir);

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

        //building the path of the output file for each entry
        char outputPath[STR_SIZE] = "";
        strcpy(outputPath, argv[2]);
        strcat(outputPath, "/");
        strcat(outputPath, dirInput->d_name);
        strcat(outputPath, "_statistica.txt");

        //getting information about each entry
        if((lstat(path, &arg)) < 0)
        {
            perror("Bad call");
            exit(11);
        }

        //checking each entry's type and writing the correct information to the output file
        if(S_ISLNK(arg.st_mode))
        {
            count++;

            if((pid = fork()) < 0)
            {
                perror("Error when creating child process!");
                exit(12);
            }

            if(pid == 0)
            {
                openOutputFile(&fOut, outputPath);

                openSourceFile(&fInLnk, path);
                
                writeSymLinkInfoToOutput(path, buffer2, &fOut, arg, dirInput);

                closeFile(&fInLnk);

                closeOutputFile(&fOut);

                exit(count);
            }
        }
        else{
            if(S_ISREG(arg.st_mode))
            {
                if(strcmp(path+strlen(path)-strlen(".bmp"), ".bmp")==0)
                {
                    count++;

                    if((pid = fork()) < 0)
                    {
                        perror("Error when creating child process!");
                        exit(12);
                    }

                    if(pid == 0)
                    {
                        openOutputFile(&fOut, outputPath);

                        openSourceFile(&fInBmp, path);

                        writeFileNameToOutput(dirInput, buffer2, &fOut);
                        
                        readFromBMPHeaderWriteInfoToOutput(offset, &fInBmp, &fOut, buffer, buffer2);
                        
                        writeInfoToOutputUsingStatInfo(path, arg, buffer2, &fOut);
                        
                        closeFile(&fInBmp);

                        closeOutputFile(&fOut);

                        exit(count);
                    }

                    count++;

                    if((pid = fork()) < 0)
                    {
                        perror("Error when creating child process!");
                        exit(12);
                    }

                    if(pid == 0)
                    {
                        changeColor(offset, &fInBmp, pixelBuffer, path);
                        
                        closeFile(&fInBmp);

                        exit(count);
                    }
                }
                else
                {
                    if (pipe(child2child) < 0)
                    {
                        perror("Error when creating a pipe!");
                        exit(errno);
                    }

                    if (pipe(child2parent) < 0)
                    {
                        perror("Error when creating a pipe!");
                        exit(errno);
                    }

                    count++;
                    
                    if((pid = fork()) < 0)
                    {
                        perror("Error when creating child process!");
                        exit(12);
                    }

                    if(pid == 0)
                    {
                        openOutputFile(&fOut, outputPath);

                        openSourceFile(&fInReg, path);

                        writeFileNameToOutput(dirInput, buffer2, &fOut);

                        writeFileSizeToOutputUsingStat(path, arg, buffer2, &fOut);

                        writeInfoToOutputUsingStatInfo(path, arg, buffer2, &fOut);

                        lseek(fInReg, 0, SEEK_SET); //vezi

                        //mai trebuie aici scrierea in pipe a continutului fisierului
                        if(close(child2child[0]) < 0) //inchidere capat citire => acest proces va scrie in pipe
                        {
                            perror("Error when closing a pipe end!");
                            exit(errno);
                        }

                        if(close(child2parent[0]) < 0) 
                        {
                            perror("Error when closing a pipe end!");
                            exit(errno);
                        }
                        if(close(child2parent[1]) < 0) 
                        {
                            perror("Error when closing a pipe end!");
                            exit(errno);
                        }

                        dup2(child2child[1], 1); //redirectare iesire standard

                        if(close(child2child[1]) < 0) //inchidere capat folosit
                        {
                            perror("Error when closing a pipe end!");
                            exit(errno);
                        }

                        execlp("cat", "cat", path, NULL);

                        //aici se termina cu pipeu

                        closeFile(&fInReg);

                        closeOutputFile(&fOut);

                        exit(count);
                    }

                    count++;

                    if((pid = fork()) < 0)
                    {
                        perror("Error when creating child process!");
                        exit(12);
                    }

                    if(pid == 0)
                    {
                        if(close(child2child[1]) < 0) //inchidere capat scriere => acest proces va citi din pipe
                        {
                            perror("Error when closing a pipe end!");
                            exit(errno);
                        }

                        if(close(child2parent[0]) < 0) 
                        {
                            perror("Error when closing a pipe end!");
                            exit(errno);
                        }

                        dup2(child2child[0], 0); //redirectare STDIN
                        dup2(child2parent[1], 1); //redirectare STDOUT

                        if(close(child2child[0]) < 0)
                        {
                            perror("Error when closing a pipe end!");
                            exit(errno);
                        }

                        if(close(child2parent[1]) < 0)
                        {
                            perror("Error when closing a pipe end!");
                            exit(errno);
                        }

                        execlp("bash", "bash", "script.sh", argv[3], NULL);

                        closeFile(&fInReg);

                        closeOutputFile(&fOut);

                        exit(count);
                    }

                    //cod parinte
                    if (close(child2child[0]) < 0)
                    {
                        perror("Error when closing a pipe end!");
                        exit(errno);
                    }

                    if (close(child2child[1]) < 0)
                    {
                        perror("Error when closing a pipe end!");
                        exit(errno);
                    }

                    if (close(child2parent[1]) < 0)
                    {
                        perror("Error when closing a pipe end!");
                        exit(errno);
                    }
                }
            }
            else
            {
                if (S_ISDIR(arg.st_mode))
                {
                    count++;

                    if((pid = fork()) < 0)
                    {
                        perror("Error when creating child process!");
                        exit(12);
                    }

                    if(pid == 0)
                    {
                        openOutputFile(&fOut, outputPath);

                        DIR *entryDir;

                        openDirectory(path, &entryDir);

                        writeDirInfoToOutput(buffer2, dirInput, &fOut, arg);

                        closeDirectory(&entryDir);

                        closeOutputFile(&fOut);

                        exit(count);
                    }
                }
            }
        }
    }

    // COD PARINTE!!

    

    for(int i = 0 ; i < count; i++)
    {
        int child;
        if((child = wait(&status)) < 0)
        {
            perror("Error when waiting for child process to end!");
            exit(13);
        }
        if(WIFEXITED(status))
        {
            printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", child, WEXITSTATUS(status));
        }
    }

    __uint8_t readFromPipeBuffer[BUFFSIZE];

    int countSentences = 0;

    if (read(child2parent[0], readFromPipeBuffer, BUFFSIZE) != -1)
    {
        //printf("%s\n\n\n", readFromPipeBuffer);
        countSentences += atoi(readFromPipeBuffer);
    }
    else
    {
        perror("Error when reading from pipe!");
        exit(errno);
    }

    printf("Numar propozitii corecte: %d\n", countSentences);

    closeDirectory(&dir);

    if(close(child2parent[0]) < 0)
    {
        perror("Error when closing a pipe end!");
        exit(errno);
    }

    printf("\nProgram ended successfully! Every information is now written inside the output folder!\n");

    return 0;
}