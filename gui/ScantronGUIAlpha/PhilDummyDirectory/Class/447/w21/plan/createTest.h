/* Scantron Test Creation Tool Header
 *
 * Cameron Wallace
 * Feb 20, 2021
 * wallac21@wwu.edu
 *
 */

#ifndef createTest_h
#define createTest_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <stdbool.h>

/* Constants */
#define LEN 1024
#define FORMS "ABCD"
#define MAXLINES 55

/* Prototypes */
int countQuestions(DIR *d, struct dirent *dir);
char** loadQuestions(DIR *d, struct dirent *dir, int fileCount);

int parseQuestion(FILE *fp, FILE *afp, char **fileList, int questionC, int i);
void writeQuestionToFile(FILE *fp, FILE *cQ);
void writeAnswersToFile(FILE *fp, FILE *key, FILE *cQ, char *keyBuf, int ans, int d, bool randomize);

void generateExams(char **fileList, char **courseInfo, int questionC, int formC);
void createExam(FILE *key, char **fileList, char **courseInfo, char form, int questionC, int formC, int currForm);

int setupKeyBuffer(char *keyBuf, int questionNum);
void shuffle(int *array, size_t n);

#endif
