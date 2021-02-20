#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#define MAXLINE 100

char* getName(char* wNum, FILE *classList, char* ans);
int gradeQuestion(char* ans, char* keyAns, int testStats[][4]);

int main(int argc, char **argv) {
	char testLoc[MAXLINE];
	char keyLoc[MAXLINE];
	char ans[MAXLINE];
	char keyAns[MAXLINE];
	char wNum[15];
	double studentScores[MAXLINE*2];

	FILE *testFile, *keyFile, *gradeFile, *statsFile, *classList;

	int testIterator = 1;
	int max = 0;
	int min = 0;
	double average = 0;

	if(argc != 4){
		fprintf(stderr, "Bad input. Usage:\n./txtGrader path_to_tests path_to_keys/keyFile.txt path_to_classList/classList\n");
		exit(EXIT_FAILURE);
	}

//Take key file and populate array with each form
	if((keyFile = fopen(argv[2], "r")) == NULL){
		fprintf(stderr, "Key file not found\n");
		exit(EXIT_FAILURE);
	}

	fgets(keyAns, MAXLINE, keyFile);
	int numVersion = atoi(keyAns);
	fgets(keyAns, MAXLINE, keyFile);
	int numQuestions = atoi(keyAns);
	min = numQuestions;

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
	fclose(keyFile);

//initialize array for aggregating statistical information
	int testStats[numQuestions][4];
	memset(testStats, 0, numQuestions*4*sizeof(int));
	gradeFile = fopen("grades.csv", "w+");
	statsFile = fopen("stats.csv", "w+");
	fprintf(gradeFile, "name,,mc,essay,total\n");

//Start grabbing and grading exams
	while(1){
		char currentTest[MAXLINE];
		sprintf(currentTest, "test-%d.txt", testIterator);
		strcpy(testLoc, argv[1]);
		strcat(testLoc, currentTest);
		if((testFile = fopen(testLoc, "r")) == NULL){
			fprintf(stderr, "%d exams graded\n", testIterator-1);
			break;
		}
		if((classList = fopen(argv[3], "r")) == NULL){
			fprintf(stderr, "Class list not found\n");
			exit(EXIT_FAILURE);
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
				fprintf(stderr, "Number of questions exceeded indicated number of questions.\n");
				exit(EXIT_FAILURE);
			}
			numCorrect = numCorrect + gradeQuestion(ans, keys[version][currentQ-1], testStats);
			currentQ++;
		}
		if(numCorrect > max){
			max = numCorrect;
		}
		if(numCorrect < min){
			min = numCorrect;
		}
		average += numCorrect;

		if(currentQ-1 != numQuestions){
			fprintf(stderr, "Number of questions was less than given number\n");
			exit(EXIT_FAILURE);
		}
		char* name = getName(wNum, classList, ans);
		if(name == NULL){
			fprintf(stderr, "student name not found for wNum %s\n", wNum);
			exit(EXIT_FAILURE);
		}
		fprintf(gradeFile, "%s,%d,,=C%d+D%d\n", name, numCorrect, testIterator, testIterator);
		studentScores[testIterator-2] = numCorrect;
		fclose(testFile);
	}
	fclose(gradeFile);

	double numStudent = testIterator-1;
	double stdSum;
	double variance;
	double stdDev;

	fprintf(statsFile, "Question Number - Answer,A,B,C,D,percent correct\n");
	for(int i=0; i<numQuestions; i++){
		double numCorrect = testStats[i][keys[0][i][0] - 65];
		fprintf(statsFile, "%d - %c,%d,%d,%d,%d,%.2f\n", i+1, keys[0][i][0],\
				testStats[i][0], testStats[i][1], testStats[i][2], testStats[i][3],\
				(numCorrect/numStudent)*100);
	}
	average = average/numStudent;
	for(int i=0; i < numStudent; i++){
		stdSum = stdSum + pow( (studentScores[i] - average), 2.0);
	}
	variance = stdSum / numStudent;
	stdDev = sqrt(variance);

	fprintf(statsFile, "\n" );
	fprintf(statsFile, "min - %d\n", min);
	fprintf(statsFile, "max - %d\n", max);
	fprintf(statsFile, "average - %.2f\n", average);
	fprintf(statsFile, "variance - %.2f\n", variance);
	fprintf(statsFile, "standard deviation - %.2f\n", stdDev);

	fclose(statsFile);

	for(int i=0; i<numVersion; i++){
		for(int j=0; j<numQuestions; j++){
			free(keys[i][j]);
		}
	}
}

char* getName(char* wNum, FILE *classList, char* ans){
	fgets(ans, MAXLINE, classList);
	int isStudent = 1;
	while(fgets(ans, MAXLINE, classList) != NULL){
		isStudent = 1;
		char* token = strtok(ans, ",");
		for(int i=0; i<strlen(wNum); i++){
			if(wNum[i] != token[i+1]){
				isStudent = 0;
				break;
			}
		}
		if(isStudent == 1){
			token = strtok(NULL, "\",");
			sprintf(ans,"%s", token);
			token = strtok(NULL, "\"");
			sprintf(ans, "%s,%s", ans, token);
			return ans;
		}
	}
	return NULL;
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
