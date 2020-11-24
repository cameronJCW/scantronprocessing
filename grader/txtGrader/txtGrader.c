#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#define MAXLINE 100

int gradeQuestion(char* ans, char* keyAns, int testStats[][4]);

int main(int argc, char **argv) {
	if(argc != 4){
		fprintf(stderr, "Bad input. Usage:\n./txtGrader path_to_tests path_to_keys numQuestions\n");
		exit(EXIT_FAILURE);
	}

	int numQuestions = atoi(argv[3]);
	int testStats[numQuestions][4];
	memset(testStats, 0, numQuestions*4*sizeof(int));

	char* keyNames[4] = {"testKeyA.txt", "testKeyB.txt","testKeyC.txt","testKeyD.txt"};
	char* versions[4] = {"A\n", "B\n", "C\n", "D\n"};

	char testLoc[1024];
	char keyLoc[1024];

	FILE *testFile, *keyFile, *gradeFile, *statsFile;

	char ans[MAXLINE];
	char keyAns[MAXLINE];
	char wNum[10];
	int testIterator = 1;

	gradeFile = fopen("grades.csv", "w+");
	statsFile = fopen("stats.csv", "w+");

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
		while(fgets(keyAns, MAXLINE, keyFile) != NULL){
			if(currentQ > numQuestions){
				fprintf(stderr, "Number of questions exceeded input number of questions.\n");
				exit(1);
			}
			fgets(ans, MAXLINE, testFile);
			numCorrect = numCorrect + gradeQuestion(ans, keyAns, testStats);
			currentQ++;
		}
		if(currentQ-1 != numQuestions){
			fprintf(stderr, "Number of questions was less than input number\n");
			exit(1);
		}
		fprintf(gradeFile, "%s,%d\n", wNum, numCorrect);

		fclose(testFile);
		fclose(keyFile);
	}
	fclose(gradeFile);

	strcpy(keyLoc, argv[2]);
	strcat(keyLoc, keyNames[0]);
	keyFile = fopen(keyLoc, "r");
	char statsKey[numQuestions];
	int idx = 0;
	while(fgets(keyAns, MAXLINE, keyFile) != NULL){
		statsKey[idx] = keyAns[0];
		idx++;
	}

	fprintf(statsFile, "Question Number - Answer,A,B,C,D,percent correct\n");
	for(int i=0; i<numQuestions; i++){
		double numStudent = testIterator-1;
		double numCorrect = testStats[i][statsKey[i] - 65];
		fprintf(statsFile, "%d - %c,%d,%d,%d,%d,%.2f\n", i+1, statsKey[i], testStats[i][0], testStats[i][1], testStats[i][2], testStats[i][3], numCorrect/numStudent);
	}
	fclose(statsFile);
}

int gradeQuestion(char* ans, char* keyAns, int testStats[][4]){
	char keyVal[strlen(keyAns)+1];
	strncpy(keyVal, keyAns, sizeof(keyVal));

	int wasCorrect = 0;
	char* realAns = strtok(keyVal, " ");
	char* mapToQ = strtok(NULL, ":");
	char* mapToAns = strtok(NULL, "\n");


	if(strlen(ans) == 2){
		if(ans[0] == realAns[0]){
			wasCorrect = 1;
		}
		int mapToQuestion = atoi(mapToQ);
		int qIndx = ans[0] - 65;
		int standardizedAns = mapToAns[qIndx] - '0';

		testStats[mapToQuestion-1][standardizedAns-1] += 1;
	}


	return wasCorrect;
}
