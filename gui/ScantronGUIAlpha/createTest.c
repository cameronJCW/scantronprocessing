/*   Scantron Test Creation Tool 
 *
 *   Cameron Wallace, Angie Luty
 *   Oct 27, 2020
 *   wallac21@wwu.edu
 * 
 */

#include "createTest.h"

/* Usage: course chapter exam_length num_forms 
   ./createTest CSCI247 Chapter1 10 3 */
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
    exit(0);
  }
  
  /* Add questions to array. Will be used to parse files. */
  int fileCount;
  fileCount = countQuestions(d, dir);
  char ** fileList = loadQuestions(d, dir, fileCount);

  /* Use filelist to create exams */
  questionC = (int) fmin(fileCount, atoi(argv[3]));

  if (questionC <= 0) { // Check num_questions argument
    if (fileCount <= 0) { // No questions exist
      fprintf(stderr, "Error: No questions found, make sure they are placed in the corrrect directory.\n");
      exit(0);
    }
    else { // Invalid Argument (Non-numberical or less than 0
      fprintf(stderr, "Error: the num_questions requested must be an integer greater than 0.\n");
      exit(0);
    }
  }
      

  generateExams(fileList, courseInfo, questionC, formC);
  
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
void generateExams(char **fileList, char **courseInfo, int questionC, int formC) {
  int i;
  FILE *key;
  char buf[LEN];
  snprintf(buf, sizeof(buf), "./%s/Book/%s/examKey", courseInfo[0], courseInfo[1]);
  key = fopen(buf, "w");

  /* Test to see if path to key is correct
     This tests to make sure the examKey (and course, Book, and chapter folder) exist
   */
  if (!key) {
    perror("fopen() error. Make sure the following path is valid ./{course}/Book/{chapter}/examKey\n");
    exit(0);
  }
  
  snprintf(buf, sizeof(buf), "%d\n%d\n", formC, questionC);
  fputs(buf, key);
  for (i = 0; i < formC; i++) {
    createExam(key, fileList, courseInfo, FORMS[i], questionC, formC, i);
  }
  printf("done\n");
  fclose(key);
}

/* @author Cameron Wallace
 * function: Get contents from question file and write them to exam tex file,
 *           record correct answers to key file 
 */
void createExam(FILE *key, char **fileList, char **courseInfo, char form, int questionC, int formC, int currentForm) {
  FILE *fp, *texH, *texM, *texE;
  char c;
  char buf[LEN];
  snprintf(buf, sizeof(buf), "./%s/Book/%s/exam_%c.tex", courseInfo[0], courseInfo[1], form);
  fp = fopen(buf, "w");
  /* Static .TeX portion of all exams */
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
  printf("form: %c\n", form);
  
  /* Declare start of form in key file */
  snprintf(buf, sizeof(buf), "Form_%c\n", form);
  fputs(buf, key);
  int capacity = questionC / formC;
  //int overflow = questionC % formC;
  int i;
  for (i = (currentForm * capacity); i < questionC; i++) {
    parseQuestion(fp, key, fileList, questionC, i);
  }
  for (i = 0; i < (currentForm * capacity); i++) {
    parseQuestion(fp, key, fileList, questionC, i);
  }

  
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
void parseQuestion(FILE *fp, FILE *key, char **fileList, int questionC, int i) {
  int x;
  char buf[LEN], buf2[LEN];
  FILE *cQ;
  int answers = 0;
  snprintf(buf, sizeof(buf), "./questions/%s", fileList[i]);
  printf("question %d = %s\n", i, buf);
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
  int inQ = 0, inV = 0; /* In question or In verbatim */
  rewind(cQ);
  while (fgets(buf, LEN, cQ) != NULL) {
    if (buf[0] == 'Q' && buf[1] == '-') { /* Question */
      inQ = 1;
      snprintf(buf2, sizeof(buf2), "\\item (3 points) %s", buf + 2);
      fputs(buf2, fp);
    } else if (buf[0] == 'V' && buf[1] == '-' && !inV) { /* Start Verbatim */
      inQ = 0; inV = 1;
      snprintf(buf2, sizeof(buf2), "\\begin{verbatim}\n    %s", buf + 2);
      fputs(buf2, fp);
    } else if ((buf[0] == 'A' || buf[0] == 'X') && buf[1] == '-') { /* End Verbatim */
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
  
  /* Key Buffer Sizing */ 
  int digits = 0;
  if (0 <= i+1 && i+1 < 10) digits = 1;
  if (10 <= i+1 && i+1 < 100) digits = 2;
  char *keyBuf = malloc(10 * sizeof(char));
  if (digits == 1) keyBuf[2] = (i + 1) + 48;
  if (digits == 2 ) { keyBuf[2] = (((i + 1) / 10) % 10) + 48; keyBuf[3] = ((i + 1) % 10) + 48; }
  keyBuf[1] = ' ';
  keyBuf[digits + 2] = ':';
  for (x = 0; x < answers; x++) {
    int xDigits = x + digits + 3;
    //if (questionList[rando[x]][0] == 'A') {
    if (questionList[x][0] == 'A') {   
      //snprintf(buf, sizeof(buf), "  \\item %s", questionList[rando[x]] + 2);
      snprintf(buf, sizeof(buf), "  \\item %s", questionList[x] + 2);
      fputs(buf, fp);
      keyBuf[xDigits] = (x+1) + 48;
      //} else if (questionList[rando[x]][0] == 'X') {
    } else if (questionList[x][0] == 'X') {  /* Answer */
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
      keyBuf[0] = correct;
      keyBuf[xDigits] = (x+1) + 48;
    }
  }
  fputs("  \\end{enumerate}\n\n", fp);
  char nl = '\n';
  strncat(keyBuf, &nl, 1); 
  fputs(keyBuf, key);
  fclose(cQ);
}

/* Arrange the N elements of ARRAY in random order.
   Taken from Ben Pfaff: https://benpfaff.org/writings/clc/shuffle.html */
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
