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
	char testLoc[1024];
	char keyLoc[1024];
	char ans[MAXLINE];
	char keyAns[MAXLINE];
	char wNum[10];

	FILE *testFile, *keyFile, *gradeFile, *statsFile;

	int testIterator = 1;

	if(argc != 3){
		fprintf(stderr, "Bad input. Usage:\n./txtGrader path_to_tests path_to_keys/keyFile.txt\n");
		exit(EXIT_FAILURE);
	}

//Take key file and populate array with each form
	if((keyFile = fopen(argv[2], "r")) == NULL){
		fprintf(stderr, "Key file not found\n");
		exit(EXIT_SUCCESS);
	}
	fgets(keyAns, MAXLINE, keyFile);
	int numVersion = atoi(keyAns);
	fgets(keyAns, MAXLINE, keyFile);
	int numQuestions = atoi(keyAns);

	char* keys[numVersion][numQuestions];
	for(int i=0; i<numVersion; i++){
		for(int j=0; j<numQuestions; j++){
			keys[i][j] = malloc(sizeof(char) * 10);
		}
	}

	int readVersion = -1;
	int readQuestion = 0;
	while(fgets(keyAns, MAXLINE, keyFile) != NULL){
		if(keyAns[0] == 'F'){
			readVersion++;
			readQuestion = 0;
		}else{
			strcpy(keys[readVersion][readQuestion],keyAns);
			readQuestion++;
		}
	}

//initialize array for aggregating statistical information
	int testStats[numQuestions][4];
	memset(testStats, 0, numQuestions*4*sizeof(int));

	gradeFile = fopen("grades.csv", "w+");
	statsFile = fopen("stats.csv", "w+");

//Start grabbing and grading exams
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

		//Use acii value of A,B,C, or D to index into keys
		fgets(ans, MAXLINE, testFile);
		int version = ans[0] - 65;


		fgets(ans, MAXLINE, testFile);
		char *essayQ = ans;

		int numCorrect = 0;
		int currentQ = 1;
		while(fgets(ans, MAXLINE, testFile) != NULL){
			if(currentQ > numQuestions){
				fprintf(stderr, "Number of questions exceeded given number of questions.\n");
				exit(EXIT_SUCCESS);
			}
			numCorrect = numCorrect + gradeQuestion(ans, keys[version][currentQ-1], testStats);
			currentQ++;
		}
		if(currentQ-1 != numQuestions){
			fprintf(stderr, "Number of questions was less than given number\n");
			exit(EXIT_SUCCESS);
		}
		fprintf(gradeFile, "%s,%d\n", wNum, numCorrect);

		fclose(testFile);
		fclose(keyFile);
	}
	fclose(gradeFile);

	fprintf(statsFile, "Question Number - Answer,A,B,C,D,percent correct\n");
	for(int i=0; i<numQuestions; i++){
		double numStudent = testIterator-1;
		double numCorrect = testStats[i][keys[0][i][0] - 65];
		fprintf(statsFile, "%d - %c,%d,%d,%d,%d,%.2f\n", i+1, keys[0][i][0], testStats[i][0], testStats[i][1], testStats[i][2], testStats[i][3], (numCorrect/numStudent)*100);
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
