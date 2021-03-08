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
void usage(char *prog);
void buildIndex(int *ind, int n);

int countFiles(DIR *d, struct dirent *dir, unsigned char type);
char** loadFiles(DIR *d, struct dirent *dir, unsigned char type, int fileCount);

int parseQuestion(FILE *fp, FILE *afp, char **fileList, int questionC, int i);
void writeQuestionToFile(FILE *fp, FILE *cQ);
void writeAnswersToFile(FILE *fp, FILE *key, FILE *cQ, char *keyBuf, int ans, int d);

void generateExams(char **fileList, int questionC, int formC);
void createExam(FILE *key, char **fileList, char form, int questionC, int formC, int currForm);

int setupKeyBuffer(char *keyBuf, int questionNum);
void shuffle(int *array, size_t n);

void setCourseName(char *name);
void setCourseYear(char *year);
void setTestNum(int num);
void setTestScore(int score);
void setRandomize(bool r);

#endif
