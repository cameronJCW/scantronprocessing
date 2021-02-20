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

/* Constants */
#define LEN 1024
#define FORMS "ABCD"
#define MAXLINES 55

/* Prototypes */
int countQuestions(DIR *d, struct dirent *dir);
char** loadQuestions(DIR *d, struct dirent *dir, int fileCount);

void getClass(char *course);
void generateExams(char **fileList, char **courseInfo, int questionC, int formC);
void createExam(FILE *key, char **fileList, char **courseInfo, char form, int questionC, int formC, int currForm);
void parseQuestion(FILE *fp, FILE *afp, char **fileList, int questionC, int i);
void shuffle(int *array, size_t n);

#endif
