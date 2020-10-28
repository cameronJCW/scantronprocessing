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
#include <math.h>

/* Constants*/
#define LEN 1024
#define FORMS "ABCD"

/* Prototypes */
int countQuestions(DIR *d, struct dirent *dir);
char** loadQuestions(DIR *d, struct dirent *dir, int fileCount);

void getClass(char* course);
void generateExams(int questionC, int numF, char *course, char **fileList);
void createExam(char *course, char **fileList, char form, int questionC);
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
  for(int i=0;i<fileCount;i++) printf("%s\n", fileList[i]);

  /* Use filelist to create exams */
  //int numQ, int numF, char *course, char **fileList
  questionC = (int) fmin(fileCount, atoi(argv[2]));
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

void generateExams(int questionC, int numF, char *course, char **fileList) {
  //printf("generate\n");
  int i;
  for (i = 0; i < numF; i++) {
    createExam(course, fileList, FORMS[i], questionC);
  }
  printf("done\n");
}

void createExam(char *course, char **fileList, char form, int questionC) {
  FILE *fp, *texH, *texM, *texE, *cQ;
  char c;
  char buf[LEN], buf2[LEN];
  int i, j;
  snprintf(buf, sizeof(buf), "./exams/%s_%c", course, form);
  fp = fopen(buf, "w");
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
  printf("questions%d\n", questionC);
  for (i = 0; i < 1; i++) {
    printf("i%d\n",i);
    //char ** questionList = (char **)malloc((questionC) * sizeof(char *));
    char questionList[questionC][LEN];
    char correct;
    int correctI;
    snprintf(buf, sizeof(buf), "./questions/%s", fileList[i]);
    cQ = fopen(buf, "r");
    /* mc or tf : 8 or 5 lines to read */
    fgets(buf, LEN, cQ);
    if (buf[0] == 'm') {
      fgets(buf2, LEN, cQ);
      snprintf(buf, sizeof(buf), "\\item (3 points) %s", buf2);
      fputs(buf, fp);
      fputs("  \\begin{enumerate}\n", fp);
      correct = fgetc(cQ);
      /* A:0 B:1 C:2 D:3 E:4 */
      switch(correct) {
      case 'A':
	correctI = 0; break;
      case 'B':
	correctI = 1; break;
      case 'C':
	correctI = 2; break;
      case 'D':
	correctI = 3; break;
      case 'E':
	correctI = 4; break;
      }
      /* Populate question list while keeping track of correct answer */
      fgets(buf2, LEN, cQ);
      for (j = 0; j < 5; j++) {
	//printf("buf2pre: %s", buf2);
	fgets(buf2, LEN, cQ);
	if (j == correctI) {
	  snprintf(buf, sizeof(buf), "@%s", buf2);
	  strcpy(questionList[j], buf);
	} else {
	  strcpy(questionList[j], buf2);
	}
      }
      /* Randomize order of list */
      int rando[5] = { 0,1,2,3,4 };
      shuffle(rando, 5);
      int index;
      for (j = 0; j < 5; j++) {
	//printf("rando:%d", rando[j]);
	index = rando[j];
	snprintf(buf, sizeof(buf), "%s", questionList[index]);
	//printf("%c\n", buf[sizeof(buf) -1]);
	if (buf[0] == '@') {
	  switch(j) {
	  case 0: correct = 'a'; break;
	  case 1: correct = 'b'; break;
	  case 2: correct = 'c'; break;
	  case 3: correct = 'd'; break;
	  case 4: correct = 'e'; break;
	  }
	  char tmp[LEN];
	  strcpy(tmp, buf+1);
	  int t = strlen(tmp);
	  if (tmp[t-1] == '\n') {
	    tmp[t-1] = '\0';
	  }
	  printf("tmp: %s\n", tmp);
	  snprintf(buf2, sizeof(buf2), "  \\item %s   \\ans{%c}\n", tmp, correct);
	  printf("buf2: %s\n", buf2);
	  fputs(buf2, fp);
	} else {
	  snprintf(buf2, sizeof(buf2), "  \\item %s", buf);
	  fputs(buf2, fp);
	}
	printf("j:%d buf:%s", j, buf);
      }
      fputs("  \\end{enumerate}\n\n", fp);
      printf("here\n");
    }
    //else if (buf[0] == 't') {

    // }
    // ERROR HANDLING -> point to misformatted question
    /* get title */

    /* get questions */

    /* get answer */
  }
  
  /* End */
  fclose(fp);
}
/*
  \item (3 points) The first function run in a C++ program is the function
  \begin{enumerate}
  \item main().  \ans{a}
  \item cpp\_main().
  \item win\_main().
  \item start().
  \item None of the above.
  \end{enumerate}
 */
void multipleChoice() {

}

/*
\item (3 points) In C++, there is no way to have the compiler
guarantee that an array passed into a function is not modified.
  \begin{enumerate}
  \item True
  \item False  \ans{b}
  \end{enumerate}
 */
void trueFalse() {

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
