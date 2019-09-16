#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>


static FILE* indexStream;
static FILE* fileToReadFrom;
static char* tmpFilesFolder = "/tmp/tmpFiles";


void mallocCheck(void* p) {
	if(p == NULL) {
		fprintf(stderr,"Error allocating more Memory...");
		exit(EXIT_FAILURE);
	}
}



char* insertNumberInString(char* string,int number){
	int digitCount;
	if(number == 0) {
		digitCount = 1;
	} else {
		digitCount = floor(log10(number)) + 1;
	}
	char* buffer = calloc((strlen(string) + 1 + digitCount),sizeof(char));
	mallocCheck(buffer);
	char *c = string;
	int charsToCopy = 0;
	int found = 0;
	int i = 0;
	while(*c) {
		if(*c == '*') {
			charsToCopy = i;
			found = 1;
			break;
		}
		i++;
		c++;
	}
	if(!found) {
		fprintf(stderr,"No '*' found... in Adress\n");
		exit(EXIT_FAILURE);
	}
	char* string1 = calloc(charsToCopy+1,sizeof(char));
	mallocCheck(string1);
	strncpy(string1,string,charsToCopy);
	int sizeOfString2 = (strlen(string)+1) - (strlen(string1)+1);
	char* string2 = calloc(sizeOfString2,sizeof(char));
	mallocCheck(string2);
	strncpy(string2,string+charsToCopy+1,sizeOfString2 -1);
	sprintf(buffer,"%s%d%s",string1,number,string2);
	free(string1);
	free(string2);
	return buffer;
}

char* createDownloadCommand(char* string) {
    char* beforeAdress = "curl \'";
    char basenameCopy[strlen(string) + 1];
    strcpy(basenameCopy,string);
    char* basenameStr = basename(basenameCopy);
    char* afterAdressTmp = "\' --output ";
    char afterAdress[strlen(afterAdressTmp) + strlen(basenameStr) + 1];
	char* buffer = malloc((strlen(beforeAdress) + strlen(afterAdress) + strlen(string) + 1)*sizeof(char));
	mallocCheck(buffer);
	sprintf(buffer,"%s%s%s",beforeAdress,string,afterAdress);
	return buffer;
}

int checkIfDownloadWorked(char* basenameStr) {
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
	char* command = createDownloadCommand(webAddress);
	system(command);
	int exitStatus;
	wait(&exitStatus);
	free(command);
    char basecpy[strlen(webAddress) + 1];
    strcpy(basecpy,webAddress);
    char* basenameStr = basename(basecpy);
	int notworked = checkIfDownloadWorked(basenameStr);
    if(!notworked) {
        fprintf(indexStream,"file\'%s\'\n",basenameStr);
    }
    return notworked;
}

char* handleNewline(char* buf) {
	char* c = buf;
	while(*c) {
		if(*c == '\n') {
			*c = '\0';
		}
		c++;
	}
	return buf;
}

int leseLine(FILE* stream) {
	char* readPath = calloc(200,sizeof(char));
	mallocCheck(readPath);
	char c;
	int i = 0;
	do {
		c = getc(stream);
		if((c != EOF)&&(c != '\n')) { *(readPath+i) = c; }
		else { if(i == 0) {return EOF;}}
		i++;
	}while((c != EOF)&&(c != '\n'));

	printf("Lese %s...\n",readPath);

	fileToReadFrom = fopen(readPath,"r");
	if(strlen(readPath) <= 2) {
		free(readPath);
		return -1;
	}
	free(readPath);
	return 1;
}

void concatenate(char* filePath,char* cwd) {
	FILE* listOfFiles = fopen(filePath,"r");
	if(listOfFiles == NULL) {
		fprintf(stderr,"listOfFiles konnte nicht geoeffnet werden");
		exit(EXIT_FAILURE);
	}
	char* tmp = calloc((strlen(cwd)+strlen("/output.ts")+1),sizeof(char));
	mallocCheck(tmp);
	strcpy(tmp,cwd);
	strcat(tmp,"/output.ts");
	FILE* fileToWriteTo = fopen(tmp,"w");
	fileToReadFrom = NULL;
	int boolean = 1;
	int err;
	while(boolean) {
		err = leseLine(listOfFiles);
		if(err == EOF) { boolean = 0;} else {
			int c;
			do {
				if(fileToReadFrom == NULL) {
					fprintf(stderr,"Das sollte nicht passieren");
					exit(EXIT_FAILURE);
				}
				c = getc(fileToReadFrom);
				if(c != EOF) { fprintf(fileToWriteTo,"%c",c); }
			}while(c != EOF);
		}
		if(err != EOF) {fclose(fileToReadFrom);}
	}
	fclose(listOfFiles);
	fclose(fileToWriteTo);
	free(tmp);
}

void printUsage() {
    fprintf(stderr,"Wrong Usage!\nUsage: ./download startIndex http://www.example.com/example*.ts\n");
}

int main(int argc, char** argv) {
	if(argc < 3) {
        printUsage();
		exit(EXIT_FAILURE);
	}
	if(argc == 3) {
		int lastFileReached = 0;
		chdir(tmpFilesFolder);
		char* str = "/index.txt";
		char* buffer = calloc(strlen(cwd) + 1 + 1 + strlen(tmpFilesFolder),sizeof(char));
        sprintf(buffer,"%s/%s",cwd,tmpFilesFolder)
		indexStream = fopen(buffer,"a");
		char* adresse = malloc((strlen(argv[2])+1) * sizeof(char));
		mallocCheck(adresse);
		strcpy(adresse,argv[2]);
		int index = atoi(argv[1]);
		while(!lastFileReached) {
			char *indexedAddress = insertNumberInString(adresse,index);
			printf("%s\n",indexedAddress);
			lastFileReached = download(indexedAddress);
			index++;
			free(indexedAddress);
		}
		fclose(indexStream);

		concatenate(buffer,cwd);
		printf("Datei output.ts enthaelt Ergebnis\n");
		free(cwd);
		free(buffer);
	} else {	
		char* cwd = getcwd(NULL,0);
		cwd = realloc(cwd,strlen(cwd) + 10);
		mallocCheck(cwd);
		strcat(cwd,"/tmpFiles");
		chdir(cwd);
		char* str = "/inhaltsverzeichnis.txt";
		char* buffer = calloc(strlen(cwd) + 1 + strlen(str),sizeof(char));
		strcpy(buffer,cwd);
		strcat(buffer,str);
		concatenate(buffer,cwd);
	}
	exit(EXIT_SUCCESS);
}
