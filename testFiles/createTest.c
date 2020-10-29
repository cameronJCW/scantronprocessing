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
void generateExams(char **fileList, char **courseInfo, int questionC, int formC);
void createExam(char **fileList, char **courseInfo, char form, int questionC, int formC, int currentForm);
void parseQuestion(FILE *fp, FILE *afp, char **fileList, int questionC, int formC, int currentForm);
void shuffle(int *array, size_t n);


/* Globals */

/* Usage: course exam_length num_forms */
int main(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr, "usage: ./createTest course chapter num_questions num_forms\n");
    exit(0);
  }
  int questionC;
  int formC = atoi(argv[4]);
  if (0 >= formC || formC > 4) {
    fprintf(stderr, "usage: select between 1 and 4 forms.\n");
    exit(0);
  }
  char **courseInfo;
  courseInfo[0] = argv[1];
  courseInfo[1] = argv[2];
  
  DIR *d;
  struct dirent *dir;
  /* d = opendir("/course/questions/"); add exam specific folder at some point */
  char focusD[LEN];
  //snprintf(focusD, sizeof(focusD),"./%s/Book/%s",courseInfo[0],courseInfo[1]);
  snprintf(focusD, sizeof(focusD), "questions");
  if ((d = opendir(focusD)) == NULL) {
    perror("opendir() error\n");
  }
  
  /* Add questions to array. Will be used to parse files. */
  int fileCount;
  fileCount = countQuestions(d, dir);
  char ** fileList = loadQuestions(d, dir, fileCount);

  /* Use filelist to create exams */
  questionC = (int) fmin(fileCount, atoi(argv[3]));

  generateExams(fileList, courseInfo, questionC, formC);
  
  return(0);
}

/*
void genQueue(int ** queue, int questionC, int formC, int split, int x, int y) {
  printf("here%d\n", formC);
  //int **queue = (int **)malloc((formC) * sizeof(int *));
  int i = 0;
  int cur = 0;
  int index = 0;
  int overflow = questionC % formC;
  int newC = questionC - overflow;
  int capacity = newC / formC;
  
  while (i < formC) {
    printf("index:%d\n", index);
    if (i == (formC - 1)) {
      queue[i][cur++] = index++;
      if (index >= questionC) {
	break;
      }
    }
    else {
      queue[i][cur++] = index++;
      printf("e\n");
      if (cur >= capacity) {
	cur = 0;
	i++;
      }
    }
  }
}
*/

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

/*
char *** generateQueue(char ** fileList, int fileCount, int formC, int split) {
  //char *** fileQueue = (char ***) malloc((fileCount) * sizeof(char *));
  char *fileQueue[formC][];
  char buf[LEN];
  int currSplit = 0;
  int currFile = 0;
  for (int i = 0; i < formC; i++) {
    printf("cs + s = %d\n", (currSplit + split));
    for (int j = 0; j < (currSplit + split); j++) {
      snprintf(buf, sizeof(buf), fileList[currFile]);
      strcpy(fileQueue[i][j], buf);
      //fileQueue[i][j] = fileList[currFile];
      currFile++;
    }
    currSplit += split;
  }
  return fileQueue;
}
*/
/* @author Cameron Wallace
 * function: Generate exam and key files
 */
void generateExams(char **fileList, char **courseInfo, int questionC, int formC) {
  int i;
  for (i = 0; i < formC; i++) {
    createExam(fileList, courseInfo, FORMS[i], questionC, formC, i);
  }
  printf("done\n");
}

/* @author Cameron Wallace
 * function: Get contents from question file and write them to exam tex file,
 *           record correct answers to key file 
 */
void createExam(char **fileList, char **courseInfo, char form, int questionC, int formC, int currentForm) {
  printf("formN:%d\n", currentForm);
  FILE *fp, *afp, *texH, *texM, *texE;
  char c;
  char buf[LEN];
  snprintf(buf, sizeof(buf), "./%s/Book/%s/exam_%c.tex", courseInfo[0], courseInfo[1], form);
  fp = fopen(buf, "w");
  snprintf(buf, sizeof(buf), "./%s/Book/%s/exam_%c_key", courseInfo[0], courseInfo[1], form);
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
  printf("%d\n", questionC);
  int capacity = questionC / formC;
  int overflow = questionC % formC;
  int i;
  for (i = (currentForm * capacity) + overflow; i++) {
    //parse call
  }
  for (i = 0; i < (currentForm * capacity) + overflow; i++) {
    //parse call
  }
  parseQuestion(fp, afp, fileList, questionC, formC, currentForm);
  
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
void parseQuestion(FILE *fp, FILE *afp, char **fileList, int questionC, int formC, int currentForm) {
  int i, x;
  char buf[LEN], buf2[LEN];
  FILE *cQ;
  for (i = (currentForm * capacity) + overflow; i < questionC; i++) {
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
      //printf("pre: %c\n", questionList[x][0]);
      //if (questionList[rando[x]][0] == 'A') {
      if (questionList[x][0] == 'A') {   
	//snprintf(buf, sizeof(buf), "  \\item %s", questionList[rando[x]] + 2);
	snprintf(buf, sizeof(buf), "  \\item %s", questionList[x] + 2);
	fputs(buf, fp);
	//} else if (questionList[rando[x]][0] == 'X') {
      } else if (questionList[x][0] == 'X') {
	switch(x) {
	case 0: correct = 'a'; break;
	case 1: correct = 'b'; break;
	case 2: correct = 'c'; break;
	case 3: correct = 'd'; break;
	case 4: correct = 'e'; break;
	}
	char tmp[LEN];
	//snprintf(buf, sizeof(buf), "  \\item %s", questionList[rando[x]] + 2);
	snprintf(buf, sizeof(buf), "  \\item %s", questionList[x] + 2);
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

/*
  1 form: no split
  2 forms: 1
  3 forms: 2
  4 forms: 3
  for each form, push one more segment to end from previous
  queue:
  - queue item: questions equal to 1/2, 1/3, or 1/4 of total
  - for b: pop and push once, c: pop and push twice, etc.
  - 
 */

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
