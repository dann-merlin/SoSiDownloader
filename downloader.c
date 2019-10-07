#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>


static FILE* indexStream;
static char* tmpFilesFolder = "/tmp/SoSiDownloader";
static char* indexFile = "index.txt";
static char* outputFileName = "output.ts";

void mallocCheck(void* p) {
	if(p == NULL) {
		fprintf(stderr,"Error allocating more Memory...");
		exit(EXIT_FAILURE);
	}
}



char* insertNumberInString(char** strings,int number){
	int digitCount;
	if(number == 0) {
		digitCount = 1;
	} else {
		digitCount = floor(log10(number)) + 1;
	}
    int buffersize = (strlen(strings[0]) + strlen(strings[1]) + 1 + digitCount);
    printf("%s\n",strings[0]);
    printf("%s\n",strings[1]);
	char* buffer = malloc(buffersize * sizeof(char));
	mallocCheck(buffer);
	sprintf(buffer,"%s%d%s",strings[0],number,strings[1]);
	return buffer;
}

void createDownloadCommand(char* string, char** results) {
    char* beforeAdress = "curl \'";
    strcpy(results[1],string);
    char* basenameStr = basename(results[1]);
    char* afterAdressTmp = "\' --output ";
    char afterAdress[strlen(afterAdressTmp) + strlen(basenameStr) + 1];
    sprintf(afterAdress,"%s%s",afterAdressTmp,basenameStr);
	char* buffer = malloc((strlen(beforeAdress) + strlen(string) + strlen(afterAdress) + 1)*sizeof(char));
	mallocCheck(buffer);
	sprintf(buffer,"%s%s%s",beforeAdress,string,afterAdress);
	results[0] = buffer;
    results[1] = basenameStr;
}

int checkIfDownloadFailed(char* basenameStr) {
    FILE* dlFile = fopen(basenameStr,"r");
    if(!dlFile) {
        perror("Opening File failed.");
        return -1;
    }
    char* compare = "<HTML>";
    char buffer[strlen(compare) + 1];
    int c;
    for(int i = 0; i < strlen(compare) && ((c = fgetc(dlFile)) != EOF); i++) {
        buffer[i] = c;
    }
    buffer[strlen(compare)] = '\0';
    if(strcmp(buffer,compare) == 0) {
        return 1;
    }
    return 0;
}

int download(char* webAddress) {
    char* commandAndBasename[2];
    char tmp[strlen(webAddress) + 1];
    commandAndBasename[1] = tmp;
	createDownloadCommand(webAddress,commandAndBasename);
    char* command = commandAndBasename[0];
	system(command);
	int exitStatus;
	wait(&exitStatus);
	free(command);
    char* basenameStr = commandAndBasename[1];
	int failed = checkIfDownloadFailed(basenameStr);
    if(!failed) {
        fprintf(indexStream,"%s\n",basenameStr);
    }
    return failed;
}

FILE* nextFile(FILE* indexFile) {
    char* line = NULL;
    size_t n = 0;
    errno = 0;
    if(getline(&line,&n,indexFile) == -1) {
        if(errno != 0) {
            perror("Line in indexFile couldn\'t be read\n");
            exit(EXIT_FAILURE);
        }
        free(line);
        return NULL;
    }
    printf("line %s wurde gelesen", line);
    if(line[strlen(line)-1] == '\n') line[strlen(line) - 1] = '\0';
    FILE* next = fopen(line,"r");
    if(!next) {
        perror("next File couldn\'t be opened.\n");
    }
    free(line);
    return next;
}

void concatenate(char* cwd) {
    printf("hi");
	FILE* listOfFiles = fopen(indexFile,"r");
	if(listOfFiles == NULL) {
		perror("indexFile couldn\'t be opened.\n");
		exit(EXIT_FAILURE);
	}
    int outputStrLen = strlen(cwd) + 1 + strlen(outputFileName) + 1;
	char outputStr[outputStrLen];
    sprintf(outputStr,"%s/%s",cwd,outputFileName);
	FILE* outputFile = fopen(outputStr,"a");
    if(outputFile == NULL) {
        perror("outputFile couldn\'t be opened.\n");
        exit(EXIT_FAILURE);
    }
	FILE* inputFile = NULL;
    int i = 0;
    printf("nochda");
	while((inputFile = nextFile(listOfFiles)) != NULL) {
        int c = 0;
        printf("reading %d\n",i++);
        do {
            c = fgetc(inputFile);
            if(c == EOF) {
                break;
            }
            if(fputc(c,outputFile) == EOF) {
                fprintf(stderr,"Error writing to outputFile");
                exit(EXIT_FAILURE);
            }
        } while(1337);
        fclose(inputFile);
    }
    printf("fertig\n");
	fclose(listOfFiles);
	fclose(outputFile);
}

void printUsage() {
    fprintf(stderr,"Wrong Usage!\nUsage: ./download http://www.example.com/example*.ts [startIndex=1] \n");
}

void fillInBeforeAndAfterAsterisk(char** parts, char* string) {
    char* c = string;
    parts[0] = string;
	int found = 0;
	while(*c) {
		if(*c == '*') {
			*c = '\0';
			parts[1] = ++c;
            found = 1;
			break;
		}
		c++;
	}
	if(!found) {
		fprintf(stderr,"No '*' found... in Adress\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char** argv) {
	if(argc > 3 || argc < 2) {
        printUsage();
		exit(EXIT_FAILURE);
	}
    //TODO rm -rf /tmp/SoSiDownloader && check if output.ts exists
	int lastFileReached = 0;
    char *cwd = getcwd(NULL,0);
    mkdir(tmpFilesFolder,0700);
	chdir(tmpFilesFolder);
	indexStream = fopen(indexFile,"a");
	int index = 1;
    if(argc == 3) { index = atoi(argv[2]);};
    char* beforeAndAfterAsterisk[2];
    fillInBeforeAndAfterAsterisk(beforeAndAfterAsterisk,argv[1]);
	while(!lastFileReached) {
		char *indexedAddress = insertNumberInString(beforeAndAfterAsterisk,index);
		printf("%s\n",indexedAddress);
		lastFileReached = download(indexedAddress);
		index++;
		free(indexedAddress);
	}
	fclose(indexStream);
	concatenate(cwd);
    free(cwd);
	printf("Datei output.ts enthaelt Ergebnis\n");
	exit(EXIT_SUCCESS);
}
