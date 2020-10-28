/*   Scantron Test Creation Tool 
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
#include <math.h>

/* Constants*/
#define LEN 1024
#define FORMS "ABCD"
#define MAXLINES 55

/* Prototypes */
/* format: file -> char -> int*/
int countQuestions(DIR *d, struct dirent *dir);
char** loadQuestions(DIR *d, struct dirent *dir, int fileCount);

void getClass(char* course);
void generateExams(char **fileList, char *course, int questionC, int numF);
void createExam(char **fileList, char *course, char form, int questionC);
void parseQuestion(FILE *fp, FILE *afp, char **fileList, int questionC);
void shuffle(int *array, size_t n);


/* Globals */

/* Usage: course exam_length num_forms */
int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: ./createTest course num_questions num_forms\n");
    exit(0);
  }
  int questionC;
  int formC = atoi(argv[3]);
  if (0 >= formC || formC > 4) {
    fprintf(stderr, "usage: select between 1 and 4 forms.\n");
    exit(0);
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

  /* Use filelist to create exams */
  questionC = (int) fmin(fileCount, atoi(argv[2]));
  generateExams(fileList, argv[1], questionC, formC);
  
  return(0);
}

/* @author Cameron Wallace
 * function: Return the total number of question files in directory
 */
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
  //printf("files:%d\n", fileCount);
  rewinddir(d);
  return fileCount;
}

/* @author Cameron Wallace
 * function: Add names of question files to list
 */
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

/* @author Cameron Wallace
 * function: Generate exam and key files
 */
void generateExams(char **fileList, char *course, int questionC, int numF) {
  //printf("generate\n");
  int i;
  for (i = 0; i < numF; i++) {
    createExam(fileList, course, FORMS[i], questionC);
  }
  printf("done\n");
}

/* @author Cameron Wallace
 * function: Get contents from question file and write them to exam tex file,
 *           record correct answers to key file 
 */
void createExam(char **fileList, char *course, char form, int questionC) {
  FILE *fp, *afp, *texH, *texM, *texE;
  char c;
  char buf[LEN];
  snprintf(buf, sizeof(buf), "./exams/%s_%c.tex", course, form);
  fp = fopen(buf, "w");
  snprintf(buf, sizeof(buf), "./exams/%s_%c_key", course, form);
  afp = fopen(buf, "w");
  texH = fopen("./texTemplate/head", "r");
  texM = fopen("./texTemplate/mid", "r");
  texE = fopen("./texTemplate/end", "r");

  /* Header */
  c = fgetc(texH);
  while (c != EOF) {
    fputc(c, fp);
    c = fgetc(texH);
  }
  fclose(texH);

  /* Specific Test/Year */
  c = fgetc(texM);
  while (c != EOF) {
    fputc(c, fp);
    c = fgetc(texM);
  }
  fclose(texH);

  /* Questions */
  parseQuestion(fp, afp, fileList, questionC);
  
  /* End */
  c = fgetc(texE);
  while (c != EOF) {
    fputc(c, fp);
    c = fgetc(texE);
  }
  fclose(texE);
  
  fclose(fp);
}

/* @author Cameron Wallace
 * function: get contents from question file and write them to exam tex file,
 *           record correct answers to key file 
 */
void parseQuestion(FILE *fp, FILE *afp, char **fileList, int questionC) {
  int i, x;
  char buf[LEN], buf2[LEN];
  FILE *cQ;
  for (i = 0; i < questionC; i++) {
    int answers = 0;
    snprintf(buf, sizeof(buf), "./questions/%s", fileList[i]);
    /* current question */
    cQ = fopen(buf, "r");
    /* Count the number of answers in file */
    while (fgets(buf, LEN, cQ) != NULL) {
      if ((buf[0] == 'A' || buf[0] == 'X') && buf[1] == '-') {
	answers++;
      }
    }
    rewind(cQ);
    /* Populate question list */
    char questionList[answers][LEN];
    int fill = 0;
    while (fgets(buf, LEN, cQ) != NULL) {
      if ((buf[0] == 'A' || buf[0] == 'X') && buf[1] == '-') {
	strcpy(questionList[fill], buf);
	fill++;
      }
    }
    /* Randomize order of questions */
    int rando[answers];
    for (x = 0; x < answers; x++) {
      rando[x] = x;
      printf("for:ql=%s", questionList[x]);
    }
    shuffle(rando, answers);
    /* Write to new file */
    /* --In question or In verbatim */
    int inQ = 0, inV = 0;
    //char t = '\t';
    rewind(cQ);
    while (fgets(buf, LEN, cQ) != NULL) {
      if (buf[0] == 'Q' && buf[1] == '-') {
	inQ = 1;
	snprintf(buf2, sizeof(buf2), "\\item (3 points) %s", buf + 2);
	fputs(buf2, fp);
      } else if (buf[0] == 'V' && buf[1] == '-' && !inV) {
	inQ = 0; inV = 1;
	snprintf(buf2, sizeof(buf2), "\\begin{verbatim}\n    %s", buf + 2);
	fputs(buf2, fp);
      } else if ((buf[0] == 'A' || buf[0] == 'X') && buf[1] == '-') {
	if (inV == 1) {
	  snprintf(buf2, sizeof(buf2), "\\end{verbatim}\n");
	  fputs(buf2, fp);
	}
	inV = 0;
	continue;
      } else {
	if (inQ) {
	  fputs(buf, fp);
	} else if (inV) {
	  snprintf(buf2, sizeof(buf2), "    %s", buf);
	  fputs(buf2, fp);
	}
      }
    }
    char correct;
    fputs("  \\begin{enumerate}\n", fp);
    for (x = 0; x < answers; x++) {
      printf("floop ql[r[x]]: %s", questionList[x]);
      if (questionList[rando[x]][0] == 'A') {   
	snprintf(buf, sizeof(buf), "  \\item %s", questionList[rando[x]] + 2);
	fputs(buf, fp);
      } else if (questionList[rando[x]][0] == 'X') {
	printf("found ans\n");
	switch(x) {
	case 0: correct = 'a'; break;
	case 1: correct = 'b'; break;
	case 2: correct = 'c'; break;
	case 3: correct = 'd'; break;
	case 4: correct = 'e'; break;
	}
	char tmp[LEN];
	snprintf(buf, sizeof(buf), "  \\item %s", questionList[rando[x]] + 2);
	strcpy(tmp, buf+1);
	if (tmp[strlen(tmp) - 1] == '\n') {
	  tmp[strlen(tmp) - 1] = '\0';
	}
	snprintf(buf2, sizeof(buf2), "  %s  \\ans{%c}\n", tmp + 1, correct);
	fputs(buf2, fp);
      }
    }
    fputs("  \\end{enumerate}\n\n", fp);
  }
}

/* Arrange the N elements of ARRAY in random order.
   Taken from Ben Pfaff: CITE THIS MAN!
*/
void shuffle(int *array, size_t n) {
  if (n > 1) {
    size_t i;
    for (i = 0; i < n - 1; i++) {
      size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}
