/*   Scantron Test Creation Tool 
 *
 *   Cameron Wallace, Angie Luty
 *   Oct 27, 2020
 *   wallac21@wwu.edu
 * 
 */

#include "createTest.h"

/* Globals */
char courseName[20];
char courseYear[20];
int testNum;
int testScore;
bool debug     = false;
bool randomize = false;

/* Constants */
char *class    = "Class";
char *classnum = "447"; 
char *qtr      = "w21";
char *db       = "../../test-db";

/* Base Directory:   ../class/classnum/qtr/plan
 * Exam Destination: ../class/classnum/qtr/plan
 * Question DB:      ../class/classnum/test-db
 */

struct question {
  char q_name[256];  /* question filename */
  char q_path[256];  /* question filepath */
};

struct topic {
  char t_name[256];                 /* name of topic */
  char t_path[256];                 /* directory path */
  int  t_total;                     /* total number of questions */
  int  t_count;                     /* requested number of questions */
  struct question t_questions[25];  /* questions belonging to topic */
};

void buildTopic(struct topic *t);

/* Usage: 
 * ./createTest [-rd] -f num
 *
 *  args:  -f   -- number of forms
 *         -r   -- randomize order of m/c answers
 *         -d   -- debug
 */

int cli(int argc, char **argv) {
  extern char *optarg;    /* defined by getopt(3)         */
  int ch;                 /* for use with getopt(3)       */
  int i, j, x;            /* incrementors                 */
  int formC = 0;          /* number of forms to make      */
  int fileCount = 0;      /* */
  int totalCount = 0;     /* number of questions per exam */
  DIR *d;                 /* */
  struct dirent *dir;     /* */
  char ** topicList;      /* topic directory names        */
  char ** fileList;       /* question file names          */
  
  /* Option Data */
  while ((ch = getopt(argc, argv, "d:r:f:")) != -1) {
    switch (ch) {
      case 'd':
        debug = true;
        break;
      case 'r':
        randomize = true;
        break;
      case 'f':
        formC = atoi(optarg);
        break;
      case '?':
      default:
        usage(argv[0]);
    }
  }
  
  /* Verify options are correct */
  if (0 >= formC || formC > 4) {
    fprintf(stderr, "Number of forms must be between 1 and 4: -f n\n");
    usage(argv[0]);
  }

  /* Test name inputs */
  printf("Enter Course Name (e.g. CS 447):\n");
  fgets(courseName, 20, stdin);
  courseName[strcspn(courseName, "\n")] = 0;
  
  printf("Enter Course Year (e.g. W2021):\n");
  fgets(courseYear, 20, stdin);
  courseYear[strcspn(courseYear, "\n")] = 0;
  
  printf("Enter Test Number:\n");
  scanf("%d", &testNum);
  
  printf("Enter Total Points:\n");
  scanf("%d", &testScore);

  /* Get topics from test-db */
  if ((d = opendir(db)) == NULL) {
    perror("opendir() error\n");
    exit(0);
  }
  dir = readdir(d);

  /* Number of directory files i.e. topics */
  fileCount = countFiles(d, dir, DT_DIR);

  /* Struct array used to stored topics and their questions */
  struct topic topics[fileCount];
  topicList = loadFiles(d, dir, DT_DIR, fileCount);
  if (debug) {
    printf("dircount:%d\n\n", fileCount);
    printf("topiclist:\n"); 
  }

  x = 0;
  fileList = (char **)malloc((fileCount) * sizeof(char *));
  for (i = 0; i < fileCount; i++) {
    if (debug) {
      printf("%s\n", topicList[i]);
    }
    /* Copy directory name to topic name */
    strncpy(topics[i].t_name, topicList[i], 256);
    /* Construct individual topic */
    buildTopic(&topics[i]);
    totalCount += topics[i].t_count;
    for (j = 0; j < topics[i].t_count; j++) {
      fileList[x] = malloc(256*sizeof(fileList[x]));
      strncpy(fileList[x], topics[i].t_questions[j].q_path, (100) * sizeof(char *));
      if (debug) {
        printf("fileList[%d]: %s  |  ", x, fileList[x]);
        printf("\n");
      }
      x++;
    }
  }

  if (totalCount <= 0) { // Check num_questions argument
    if (fileCount <= 0) { // No questions exist
      fprintf(stderr, "Error: No questions found, make sure they are placed in the corrrect directory.\n");
      exit(0);
    }
    else { // Invalid Argument (Non-numberical or less than 0)
      fprintf(stderr, "Error: the num_questions requested must be an integer greater than 0.\n");
      exit(0);
    }
  }
  if (debug) {
    printf("totalcount:%d\n", totalCount);
    for (i = 0; i < totalCount; i++) {
      printf("fileList[%d] = %s\n", i, fileList[i]);
    }
  }

  generateExams(fileList, totalCount, formC);
  
  return(0);
}

/* @author Cameron Wallace
 * function: Tell the user how to run the program
 */
void usage(char *prog) {
  fprintf (stderr, "%s: [-dr] -f num\n", prog);
  exit(1);
}

/* @author Cameron Wallace
 * function: A struct is passed in which represents a subdirectory of the test-db. This function
 *           will fill the nested substruct with the the file names and paths of the questions 
 *           belonging to the given topic.
 */
void buildTopic(struct topic *t) {      // Selected topic
  int i;
  char buf[256];
  DIR *d;
  struct dirent *dir;
  char ** fileList;

  snprintf(buf, sizeof(buf), "%s/%s", db, t->t_name);

  if (debug) {
    printf("%s\n", buf);
  }

  /* Open topic directory */
  if ((d = opendir(buf)) == NULL) {
    perror("opendir() error\n");
    exit(0);
  }
  dir = readdir(d);
  
  /* Count the number of regular files in the directory and assign to topics total size */
  t->t_total = countFiles(d, dir, DT_REG);
  printf("Select number of questions from %s (0 - %d) -- ADD ERROR CHECK:\n", t->t_name, t->t_total);
  scanf("%d", &t->t_count);
  
  fileList = loadFiles(d, dir, DT_REG, t->t_total);

  /* Debug: print contents of filelist */
  if (debug) {
    for (i = 0; i < t->t_total; i++) {
      printf("fl[%d]=%s", i, fileList[i]);
    }
    printf("\n\n");
  }

  /* Randomize which questions will be picked from topic */
  int rando[t->t_total];
  buildIndex(rando, t->t_total);
  shuffle(rando, t->t_total);

  for (i = 0; i < t->t_count; i++) {
    if (debug) {
      printf("fileList[%d] = %s\n", i, fileList[i]); 
    }
    /* Assign question name */
    strncpy(t->t_questions[i].q_name, fileList[rando[i]], 256);

    /* Assign filepath of question, relative to /test-db */
    snprintf(buf, sizeof(buf), "%s/%s/%s", db, t->t_name, fileList[rando[i]]);
    strncpy(t->t_questions[i].q_path, buf, 256);

    /* Assign name of question */
    snprintf(buf, sizeof(buf), "%s", fileList[i]);
    strncpy(t->t_questions[i].q_name, buf, 256);
    if (debug) {
      printf("q_path: \"%s\", q_name: %s\n",t->t_questions[i].q_path, t->t_questions[i].q_name);
    }
  }
}

/* @author Cameron Wallace
 * function: Return the total number of specified files in directory
 */
int countFiles(DIR *d,                    // Current working directory
              struct dirent *dir,         // Directory entry
              unsigned char type) {       // File type
  int fileCount = 0;
  int l;
  /* Count total questions in directory */
  while ((dir = readdir(d)) != NULL) {
    if (dir->d_type == type) {
      l = strlen(dir->d_name);
      if (dir->d_name[0] != '.' && dir->d_name[l - 1] != '~') {
	      fileCount++;
      }
    }
  }
  rewinddir(d);
  return fileCount;
}

/* @author Cameron Wallace
 * function: Add names of files to given list and return
 */
char ** loadFiles(DIR *d,                   // Current working directory
                  struct dirent *dir,       // Directory entry
		              unsigned char type,       // File type
		              int fileCount) {          // Number of files in directory
  int i = 0;
  int l;
  char ** fileList = (char **)malloc((fileCount) * sizeof(char *));
  while ((dir = readdir(d)) != NULL) {
    if (dir->d_type == type) {
      l = strlen(dir->d_name);
      if (dir->d_name[0] != '.' && dir->d_name[l - 1] != '~') {
	      fileList[i] = dir->d_name;
        if (debug) {
          printf("i:%d, filename: %s, type: %hhu\n", i, dir->d_name, dir->d_type);
        }
	      i++;
      }
    }
  }
  rewinddir(d);
  return fileList;
}

/* @author Cameron Wallace
 * function: Generate exam and key files
 */
void generateExams(char **fileList,          // List containing question file paths
		               int questionC,            // Total number of questions on exam
		               int formC) {              // Total number of forms
  int i;
  FILE *key;
  char buf[LEN];
  snprintf(buf, sizeof(buf), "examKey");
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
    createExam(key, fileList, FORMS[i], questionC, formC, i);
  }
  printf("done\n");
  fclose(key);
}

/* @author Cameron Wallace
 * function: Get contents from question file and write them to exam tex file,
 *           record correct answers to key file 
 */
void createExam(FILE *key,                 // File pointer for exam key
                char **fileList,           // List containing question file paths
                char form,                 // Form of current exam: A, B, C or D
                int questionC,             // Total number of questions on exam
                int formC,                 // Total number of forms
                int currentForm) {         // Current form number
  FILE *fp, *texH, *texM, *texE;
  char c;
  char buf[LEN];
  int i;
  
  snprintf(buf, sizeof(buf), "./exam_%c.tex", form);
  fp = fopen(buf, "w");
  /* Static .TeX portion of all exams */
  texH = fopen("./texTemplate/head", "r");
  texM = fopen("./texTemplate/mid", "r");
  texE = fopen("./texTemplate/end", "r");

  /* Write tex header template to new exam. */
  c = fgetc(texH);
  while (c != EOF) {
    fputc(c, fp);
    c = fgetc(texH);
  }
  fclose(texH);
  
  /* Write specific Test/Year string to new exam. */
  snprintf(buf, sizeof(buf), "%s, %s - Test %d (%d pts), Form %C\n",
	   courseName, courseYear, testNum, testScore ,form);
  fputs(buf, fp);

  /* Write tex mid section template to new exam. */
  c = fgetc(texM);
  while (c != EOF) {
    fputc(c, fp);
    c = fgetc(texM);
  }
  fclose(texH);

  /* Questions */
  if (debug) {
    printf("form: %c\n", form);
  }
  
  /* Declare start of form and write to key file */
  snprintf(buf, sizeof(buf), "Form_%c\n", form);
  fputs(buf, key);

  /* Calculate how the questions will be split - used for scrambling */
  int capacity = questionC / formC;
  /* Initialize array to store long answers positions. */
  int * longAnswers= calloc(questionC, sizeof (int));
  
  /* Loop through all questions in two chunks. All questions are split into n sections
     where n is the number of forms. These two loops shift the question order by 1/n
     per form. When parseQuestion returns < 0, that question is marked to be added to 
     the end of the exam */
  for (i = (currentForm * capacity); i < questionC; i++) {
    if (parseQuestion(fp, key, fileList, questionC, i) == -1) {
      longAnswers[i] = 1;
    }
  }
  for (i = 0; i < (currentForm * capacity); i++) {
    if (parseQuestion(fp, key, fileList, questionC, i) == -1) {
      longAnswers[i] = 1;
    }
  }
  
  /* Put the long answer questions at the end of the other questions. */
  for (i = 0; i < questionC; i++) {
    if (longAnswers[i] == 1) {
      snprintf(buf, sizeof(buf), "%s", fileList[i]);
      FILE *cQ = fopen(buf, "r");
      writeQuestionToFile(fp, cQ);
      fputs("\n", fp);
    }
  }
  
  /* Write the end tex template to the new exam file. */
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
int parseQuestion(FILE *fp,                  // File pointer for exam
                  FILE *key,                // File pointer for exam key
                  char **fileList,          // List containing question file paths
                  int questionC,            // Number of total questions
                  int questionNum) {        // Number of current question
  char buf[LEN];
  FILE *cQ;
  int answers = 0;

  /**/
  snprintf(buf, sizeof(buf), "%s", fileList[questionNum]);
  if (debug) {
    printf("question %d = %s\n", questionNum, buf);
  }

  /* cQ is the file of the current question to be written. */
  cQ = fopen(buf, "r");

  /* Count the number of answers in the question. */
  while (fgets(buf, LEN, cQ) != NULL) {
    /* Return -1 if the question is long answer style. */
    if (buf[0] == 'L') {
      return -1;
    }
    /* If the character is an A (answer) or X (correct answer) increment */
    if ((buf[0] == 'A' || buf[0] == 'X') && buf[1] == '-') {
      answers++;
    }
  }
  rewind(cQ);

  /* Write to new exam file */
  writeQuestionToFile(fp, cQ);

  /* Key Buffer Sizing */
  char *keyBuf = malloc(10 * sizeof(char));
  int digits = setupKeyBuffer(keyBuf, questionNum);

  /* Write answers to file */
  rewind(cQ);
  writeAnswersToFile(fp, key, cQ, keyBuf, answers, digits, false);

  fclose(cQ);
  return 0;
}

/* @author Cameron Wallace
 * function: Helper for parseQuestions
 * Key file lines follow the format: a 1:12345
 * - [correct answer] [question num on form A]:[order of answers]
 */
int setupKeyBuffer (char *keyBuf,            // Buffer used to generate exam key
		                int questionNum) {       // Number of questions on the exam
  int digits = 0;
  /* Determine length needed for the buffer used to write the exam key */
  if (0 <= questionNum+1 && questionNum+1 < 10) digits = 1;
  if (10 <= questionNum+1 && questionNum+1 < 100) digits = 2;

  /* Format keyBuf depending on number of questions */
  if (digits == 1) keyBuf[2] = (questionNum + 1) + 48;
  if (digits == 2 ) {
    keyBuf[2] = (((questionNum + 1) / 10) % 10) + 48;
    keyBuf[3] = ((questionNum + 1) % 10) + 48;
  }
  keyBuf[1] = ' ';
  keyBuf[digits + 2] = ':';

  return digits;
}

/* @author Cameron Wallace
 * function: Helper for parseQuestions
 */
void writeQuestionToFile(FILE *fp,            // File pointer for the exam
			                   FILE *cQ) {          // File pointer for the current question
  char buf[LEN];
  char buf2[LEN];
  
  /* Write to new file */
  int inQ = 0, inV = 0; /* In question or In verbatim */
  rewind(cQ);
  while (fgets(buf, LEN, cQ) != NULL) {
    /*  Question */
    if ((buf[0] == 'Q' || buf[0] == 'L') && buf[1] == '-') {
      inQ = 1;
      snprintf(buf2, sizeof(buf2), "\\item (3 points) %s", buf + 2);
      fputs(buf2, fp);
    }
    /* Start Verbatim */
    else if (buf[0] == 'V' && buf[1] == '-' && !inV) {
      inQ = 0; inV = 1;
      snprintf(buf2, sizeof(buf2), "\\begin{verbatim}\n    %s", buf + 2);
      fputs(buf2, fp);
    }
    /* End Verbatim */
    else if ((buf[0] == 'A' || buf[0] == 'X') && buf[1] == '-') { 
      if (inV == 1) {
	snprintf(buf2, sizeof(buf2), "\\end{verbatim}\n");
	fputs(buf2, fp);
      }
      inV = 0;
      continue;
    }
    else {
      /* Line is still part of the question */
      if (inQ) {
	fputs(buf, fp);
      }
      /* Line is still part of the verbatim */
      else if (inV) { 
	snprintf(buf2, sizeof(buf2), "    %s", buf);
	fputs(buf2, fp);
      }
    }    
  }
  if (inV == 1) {
    snprintf(buf2, sizeof(buf2), "\\end{verbatim}\n");
    fputs(buf2, fp);
  }
}

/* @author Cameron Wallace
 * function: Helper for parseQuestions
 */
void writeAnswersToFile(FILE *fp,                  // File pointer for exam
                        FILE *key,                 // File pointer for exam key
                        FILE *cQ,                  // File pointer for current question
                        char *keyBuf,              // Buffer used to generate exam key 
                        int ans,                   // Number of answers in question file
                        int digits,                // Number of digits in key buffer
                        bool randomize) {          // Whether answer choice order is random
  int i;
  int dCount;
  char buf[LEN];
  char buf2[LEN];

  /* Populate answer list */
  char answerList[ans][LEN];
  int fill = 0;
  while (fgets(buf, LEN, cQ) != NULL) {
    if ((buf[0] == 'A' || buf[0] == 'X') && buf[1] == '-') {
      strcpy(answerList[fill], buf);
      fill++;
    }
  }
  
  /* Randomize order of questions */
  if (randomize) { // qL[i] -> qL[rando[i]]
    int rando[ans];
    shuffle(rando, ans);
  }
  
  char correct;
  fputs("  \\begin{enumerate}\n", fp);
  for (i = 0; i < ans; i++) {
    dCount = i + digits + 3;
    /* Non-Answer */
    if (answerList[i][0] == 'A') {   
      snprintf(buf, sizeof(buf), "  \\item %s", answerList[i] + 2);
      fputs(buf, fp);
      keyBuf[dCount] = (i+1) + 48;
    }
    /* Answer */
    else if (answerList[i][0] == 'X') {  
      switch(i) {
        case 0:
        correct = 'a'; break;
        case 1:
        correct = 'b'; break;
        case 2:
        correct = 'c'; break;
        case 3:
        correct = 'd'; break;
        case 4:
        correct = 'e'; break;
        }
      char tmp[LEN];
      snprintf(buf, sizeof(buf), "  \\item %s", answerList[i] + 2);
      strcpy(tmp, buf+1);
      if (tmp[strlen(tmp) - 1] == '\n') {
	      tmp[strlen(tmp) - 1] = '\0';
      }
      snprintf(buf2, sizeof(buf2), "  %s  \\ans{%c}\n", tmp + 1, correct);
      fputs(buf2, fp);
      keyBuf[0] = correct;
      keyBuf[dCount] = (i+1) + 48;
    }
  }
  
  fputs("  \\end{enumerate}\n\n", fp);
  char nl = '\n';
  strncat(keyBuf, &nl, 1); 
  fputs(keyBuf, key);
}

void setCourseName(char *name) {
  strncpy(courseName, name, 20);
}

void setCourseYear(char *year) {
  strncpy(courseYear, year, 20);
}

void setTestNum(int num) {
  testNum = num;
}

void setTestScore(int score) {
  testScore = score;
}

/* Arrange the N elements of ARRAY in random order.
   Taken from Ben Pfaff: https://benpfaff.org/writings/clc/shuffle.html */
void shuffle(int *array,         // array to be shuffled
             size_t n) {         // size of array
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

/* @author Cameron Wallace
 * function: Helper for shuffle
 */
void buildIndex(int *ind, int n) {
  int i;
  for (i = 0; i < n; i++) {
    ind[i] = i;
  }
}
