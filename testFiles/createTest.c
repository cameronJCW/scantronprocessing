/*   Scantron Test Creation Tool  authors date email year
 *
 *   Cameron Wallace
 *   Oct 27, 2020
 *   wallac21@wwu.edu
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

/* Constants*/
#define LEN 1024
#define FORMS "ABCD"

/* Prototypes */
int countQuestions(DIR *d, struct dirent *dir);
char** loadQuestions(DIR *d, struct dirent *dir, int fileCount);

void getClass(char* course);
void generateExams(int numQ, int numF, char *course, char **fileList);
void createExam(char *course, char **fileList, char form);
//

/* Globals */



/* Usage: course exam_length num_forms */
int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: ./createTest course num_questions num_forms\n");
    exit(0);
  }
  int questionC = atoi(argv[2]);
  int formC = atoi(argv[3]);
  if (0 >= formC && formC > 4) {
    fprintf(stderr, "usage: select between 1 and 4 forms.\n");
  }
  char cwd[LEN];
  getcwd(cwd, sizeof(cwd));
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working dir: %s\n", cwd);
  } else {
    perror("getcwd() error\n");
  }
  
  
  DIR *d;
  struct dirent *dir;
  /* d = opendir("/course/questions/"); add exam specific folder at some point */
  if ((d = opendir(argv[1])) == NULL) {
    perror("opendir() error\n");
  }
  
  /* Add questions to array. Will be used to parse files. */
  int fileCount;
  fileCount = countQuestions(d, dir);
  char ** fileList = loadQuestions(d, dir, fileCount);
  for(int i=0;i<fileCount;i++) printf("%s\n", fileList[i]);

  /* Use filelist to create exams */
  //int numQ, int numF, char *course, char **fileList
  generateExams(questionC, formC, argv[1], fileList);
  
  return(0);
}

int countQuestions(DIR *d, struct dirent *dir) {
  int fileCount = 0;
  int l;
  /* Count total questions in directory */
  while ((dir = readdir(d)) != NULL) {
    if (dir->d_type == DT_REG) {
      l = strlen(dir->d_name);
      if (dir->d_name[l - 1] != '~') {
	fileCount++;
      }
    }
  }
  printf("files:%d\n", fileCount);
  rewinddir(d);
  return fileCount;
}

char ** loadQuestions(DIR *d, struct dirent *dir, int fileCount) {
  int i = 0;
  int l;
  char **fileList = (char **)malloc((fileCount) * sizeof(char *));
  while ((dir = readdir(d)) != NULL) {
    if (dir->d_type == DT_REG) {
      l = strlen(dir->d_name);
      //printf("--%c\n", dir->d_name[l - 1]);
      if (dir->d_name[l - 1] != '~') {
	fileList[i] = dir->d_name;
      }
      i++;
    }
  }
  rewinddir(d);
  return fileList;
}

void generateExams(int numQ, int numF, char *course, char **fileList) {\
  printf("generat\n");
  int i;
  for (i = 0; i < numF; i++) {
    createExam(course, fileList, FORMS[i]);
  }
}

void createExam(char *course, char **fileList, char form) {
  FILE *fp;
  char buf[LEN];
  snprintf(buf, sizeof(buf), "./exams/%s_%c", course, form);
  fp = fopen(buf, "w");
}

