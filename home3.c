/*
 Name:Revanth Thandamalla
 BlazerId:rt2
 project:searching files
 compile: gcc -o filename
 run: ./filename -commands -commands folder
 */

#include <dirent.h> 
#include <stdio.h> 
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#define MIN 0x01

#define INVALID 0xFFFFFFFF

#define FILE_SIZE 0x01
#define MIN_SIZE 0x02
#define STRING_PATTERN 0x04
#define GIVEN_PATH 0x08
#define GIVEN_COMMAND 0x10

uint8_t indexOfCommand = 0;


void printingItems(char* path, uint64_t args, char** argv, uint8_t depth);

uint64_t parsingArguments(int argc, char** argv) {
    uint64_t args = 0;

    if (argc < MIN) {
        return INVALID;
    }
    uint8_t i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-S") == 0) {
            args |= FILE_SIZE;
        }
        else if (strcmp(argv[i], "-s") == 0) {
            args |= MIN_SIZE;
            i++;
            args |= (i << 24);
            if (i == argc) {
                return INVALID;
            }
        }
        else if (strcmp(argv[i], "-f") == 0) {
            args |= STRING_PATTERN;
            i++;
            args |= (i << 16);
            if (i == argc) {
                return INVALID;
            }
        }
        else if (strcmp(argv[i], "-e") == 0) {
            args |= GIVEN_COMMAND;
            indexOfCommand = i;
            i++;

            if (i == argc) {
                return INVALID;
            }
        }
        
        else if (strcmp(argv[i], "-E") == 0) {
            args |= GIVEN_COMMAND;
            indexOfCommand = i;
            i++;

            if (i == argc) {
                return INVALID;
            }
        } else {
            if ((args & GIVEN_PATH) == GIVEN_PATH) {
                return INVALID;
            }
            args |= GIVEN_PATH;
            args |= (i << 8);
        }
    }

    return args;
}



void displayDirectories(char* path, uint64_t args, char** argv, uint8_t depth) {
    DIR* d = opendir(path);
    struct dirent* dir;
    struct stat sBuff;
    char* fPath = (char *) "";

    if (depth == 1) {
        if (strcmp(path, ".") == 0) {
            char cwd[50];
            getcwd(cwd, 50);
            uint8_t i;

            for (i = strlen(cwd) - 1; i > 0; i--) {
                if (cwd[i] == '/' || cwd[i] == '\\') {
                    printf("%s\n", cwd + i + 1);
                    break;
                }
            }
        } else {
            uint8_t i;
            for (i = strlen(path) - 1; i > 0; i--) {
                if (path[i] == '/' || path[i] == '\\') {
                    printf("%s\n", path + i + 1);
                    break;
                }
            }
        }
    }

    dir = readdir(d);
    dir = readdir(d);

    while ((dir = readdir(d)) != NULL) {
        uint16_t fPathLength = strlen(path) + 2 + strlen(dir->d_name);

        fPath = (char*) malloc(fPathLength);
        memset(fPath, 0, fPathLength);
        strcat(fPath, path);
        strcat(fPath, "/");
        strcat(fPath, dir->d_name);

        if (stat(fPath, &sBuff) != 0) {
            printf("Error!");
        }

        if (dir->d_type == DT_DIR) {
            if(depth){ 
             uint8_t i;
              for (i = 0; i < depth; i++) {
                printf("\t");}}
            printf("%s\n", dir->d_name);
            printingItems(fPath, args, argv, depth + 1);
        }

        memset(fPath, 0, fPathLength);
        free(fPath);
    }
    closedir(d);
}

void format(uint8_t hasSize, char *fileName, int fileSize) {
    if (hasSize) {
        printf("%s (%u bytes)\n", fileName, fileSize);
    } else {
        printf("%s\n", fileName);
    }
}

void execBinWithArgs(char *command, char *file) {
    char *saveptr = command;
    char *word = NULL;
    char commandParsed[10][10];
    uint8_t i = 0;

    while ((word = strtok_r(saveptr, " ", &saveptr)) != NULL) {
        strcpy(commandParsed[i], word);
        i++;
    }

    execlp(commandParsed[0], commandParsed[0], commandParsed[1], file, NULL);
}

void printingItems(char* path, uint64_t args, char** argv, uint8_t depth) {
    displayDirectories(path, args, argv, depth);
    DIR* d = opendir(path);
    struct dirent* dir;
    struct stat sBuff;
    char* fPath = (char *) "";
    uint8_t fPrinting = 0;

    while ((dir = readdir(d)) != NULL) {
        uint16_t fPathLength = strlen(path) + 2 + strlen(dir->d_name);
        char *fileName = dir->d_name;

        fPath = (char*) malloc(fPathLength);
        memset(fPath, 0, fPathLength);

        strcat(fPath, path);
        strcat(fPath, "/");
        strcat(fPath, dir->d_name);
        stat(fPath, &sBuff);

        if (dir->d_type == DT_LNK) {
            char realPath[261] = {0};
            realpath(fPath, realPath);
            uint8_t i;

            for (i = strlen(realPath); i > 0; i--) {
                if (realPath[i] == '/' || realPath[i] == '\\') {
                    fileName = realPath + i + 1;
                    break;
                }
            }
        }

        if (dir->d_type == DT_REG || dir->d_type == DT_LNK) {
            if (args & MIN_SIZE) {
                if (sBuff.st_size >= atoi(argv[(uint8_t) (args >> 24)])) {
                    if ((args & STRING_PATTERN)) {
                        if (strstr(fileName, argv[(uint8_t) (args >> 16)])) {
                            fPrinting = 1;
                        }
                    } else {
                        fPrinting = 1;
                    }
                }
            } else {
                if ((args & STRING_PATTERN)) {
                    if (strstr(fileName, argv[(uint8_t) (args >> 16)])) {
                        fPrinting = 1;
                    }
                } else {
                    fPrinting = 1;
                }
            }
        }

        if (fPrinting) {
            uint8_t i;
              for (i = 0; i < depth; i++) {
                printf("\t");
            }
            format(args & FILE_SIZE, fileName, sBuff.st_size);
            if (args & GIVEN_COMMAND) {
                int pid = fork();

                if (pid == 0) {
                    // Son process
                    execBinWithArgs(argv[indexOfCommand + 1], fileName);
                    exit(0);
                } else {
                    wait(NULL);
                }
            }
            fPrinting = 0;
        }

        memset(fPath, 0, fPathLength);
        free(fPath);
    }
    closedir(d);
}



void dictionaryteach(uint64_t args, char** argv) {
    char* path = (char *) ".";

    if (args & GIVEN_PATH) {
        path = argv[(uint8_t) (args >> 8)];
    }

    printingItems(path, args, argv, 1);
}

int main(int argc, char** argv) {
    uint64_t args = parsingArguments(argc, argv);

    if (args == INVALID) {
        printf("Invalid number of entered args!\n");
        return 0;
    }

    dictionaryteach(args, argv);

    return(0);
}