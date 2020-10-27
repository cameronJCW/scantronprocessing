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

/* Constants*/

/* Prototypes */
void loadQuestions(int qNum);
void generateExams(int forms, int qNum);
//

/* Globals */
#define LEN 1024

/* Usage: course section exam_length num_forms */
int main(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr, "usage: ./createTest course section num_questions num_forms\n");
    exit(0);
  }
  char cwd[LEN];
  getcwd(cwd, sizeof(cwd));
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working dir: %s\n", cwd);
  } else {
    perror("getcwd() error");
  }

  
  return(0);
}

