#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAXLINE 100

int main(int argc, char **argv) {
	if(argc != 3){
		fprintf(stderr, "Bad input. Usage:\n./txtGrader path_to_tests path_to_keys\n");
		exit(EXIT_FAILURE);
	}
	char* keyNames[4] = {"testKeyA.txt", "testKeyB.txt","testKeyC.txt","testKeyD.txt"};
	char* versions[4] = {"A\n", "B\n", "C\n", "D\n"};

	char testLoc[1024];
	char keyLoc[1024];

	FILE *testFile;
	FILE *keyFile;
	FILE *gradeFile;

	char ans[MAXLINE];
	char keyAns[MAXLINE];
	char wNum[10];
	int testIterator = 1;

	gradeFile = fopen("grades.csv", "w+");

	while(1){
		char currentTest[1024];
		sprintf(currentTest, "test-%d.txt", testIterator);
		strcpy(testLoc, argv[1]);
		strcat(testLoc, currentTest);


		if((testFile = fopen(testLoc, "r")) == NULL){
			fprintf(stderr, "%d exams graded\n", testIterator-1);
			break;
		}
		testIterator++;

		fgets(ans, MAXLINE, testFile);
		strcpy(wNum, strtok(ans, "\n"));

		fgets(ans, MAXLINE, testFile);
		char *version = ans;
		for(int i=0; i<4; i++){
			if(strcmp(version, versions[i]) == 0){
				strcpy(keyLoc, argv[2]);
				strcat(keyLoc, keyNames[i]);
				keyFile = fopen(keyLoc, "r");
			}
		}
		if(keyFile==NULL){
			fprintf(stderr, "key file not found: %c\n", version[0]);
			exit(1);
		}

		fgets(ans, MAXLINE, testFile);
		char *essayQ = ans;

		int numCorrect = 0;
		int currentQ = 1;
		char multipleFlag[400];
		bzero(multipleFlag, strlen(multipleFlag));
		while(fgets(keyAns, MAXLINE, keyFile) != NULL){
			fgets(ans, MAXLINE, testFile);
			if(strlen(ans) > 2){
				char str[10];
				bzero(str, sizeof(str));
				snprintf(str, sizeof(str), "%d ", currentQ);
				strcat(multipleFlag, str);
			}else if(strcmp(ans, keyAns)==0){
				numCorrect++;
			}
			currentQ++;
		}
		if(strlen(multipleFlag)>0){
			fprintf(gradeFile, "%s,%d, Bad Input For: %s\n", wNum, numCorrect, multipleFlag);
		}else{
			fprintf(gradeFile, "%s,%d\n", wNum, numCorrect);
		}
		fclose(testFile);
		fclose(keyFile);
	}
	fclose(gradeFile);


}
