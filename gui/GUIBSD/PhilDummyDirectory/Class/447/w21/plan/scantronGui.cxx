#include "scantronGui.h"
#include <FL/x.H>
#include <errno.h>

extern "C" {
#include "createTest.h"
}

/*Globals*/
char *course;
char *year
int num;
int score;
int forms;
char **fileList //stores question paths 
int fli;
int selected;

int flSize = 100;

Fl_Input *filter;
Fl_Input *courseNameInput;
Fl_Input *courseYearInput;
Fl_Int_Input *testNumInput;
Fl_Int_Input *testScoreInput;
Fl_Browser *preview;
Fl_Multi_Browser *selector;
Fl_Native_File_Chooser *fc;
Fl_Choice *ch_extra;

int main(int argc, char **argv) {
	Fl_Double_Window *window;
	Fl_Button *button;
	Fl_File_Icon *icon;
	
	course   = (char *) malloc(20 * sizeof(char *));
	year     = (char *) malloc(20 * sizeof(char *));
	fileList = (char **) malloc(flSize * sizeof(char *));
	fli = 0;
	forms = 1;
	
	// Make the file chooser
	Fl::scheme(NULL);
	
	/// Make the main window
	window = new Fl_Double_Window(640, 480, "Scantron Grading Tool");
	
	// Test Course Input Field
	courseNameInput = new Fl_Input(120,
																	10,
																	180,
																	25,
																	"Course Name: ");
	courseNameInput->maximum_size(20);
	
	// Test Year Input Field
	courseYearInput = new Fl_Input(courseNameInput->x(),
																	courseNameInput->y()+30,
																	180,
																	25,
																	"Test Date: ");
	courseYearInput->maximum_size(20);
	
	// Test Number Input Field
	testNumInput = new Fl_Input(courseNameInput->x(),
																	courseYearInput->y()+30,
																	180,
																	25,
																	"Test Number: ");
	courseYearInput->maximum_size(2);
	
	// Test Number Input Field
	courseYearInput = new Fl_Input(courseNameInput->x(),
																	testNumInput->y()+30,
																	180,
																	25,
																	"Total Score: ");
	courseYearInput->maximum_size(3);
	
	// Regex for filesnames
	filter = new Fl_Input(courseNameInput->x() + courseNameInput->w() + 70,
	courseNameInput->y(),
	150,
	25,
	"Filter:");
	
	// Make the buttons
	button = new Fl_Button(filter->x()+155, 10, 25, 25);
	button->labelcolor(FL_YELLOW);
	button->callback((Fl_Callback *)show_callback);
	
	icon = Fl_File_Icon::find(".", Fl_File_Icon::DIRECTORY);
	icon->label(button);
	
	// Select number of choices
	ch_extra = new Fl_Choice(filter->x(),
	filter->y()+30,
	50,
	25,
	"Forms:");
	ch_extra->add("1|2|3|4");
	ch_extra->value(0);
	ch_extra->callback((Fl_Callback *)extra_callback);
	
	// File browser: displays selected questions
	selector = new Fl_Multi_Browser(25, 
	150, 
	291, 
	280, 
	"Selected Questions");
	selector->align(FL_ALIGN_TOP);
	selector->callback((Fl_Callback *)display_question);
	
	// Shows the contents of the currently selected file
	preview = new Fl_File_Browser(selector->w()+selector->x()+4,
	150, 
	291, 
	220, 
	"Question Preview");
	preview->aligned(FL_ALIGN_TOP);
	
	// Removes file from fileList
	button = new Fl_Button(preview->x() + (preview->w()/2) - 50,
	preview->y() + preview->h() + 10,
	100,
	25,
	"Remove");
	button->callback((Fl_Callback *)remove_item);
	
	// Creates exam
	button = new Fl_Button(selector->x() + (selector->w()/2) - 50,
	selector->y() + selector->h() + 10,
	100,
	25,
	"Create Exam");
	button->callback((Fl_Callback *)create_exam);
	
	window->resizable(NULL); // files
	window->end();
	window->show(1, argv);
	
	Fl::run();
	
	return(0);
}

void display_question(Fl_Widget *w, void *v) {
	int i;
	for (i = 1; i <= selector->size(); i++) {
		if (selector->selected(i)) {
				if (preview->load(fileList[i - 1]) < 0) {
					printf("error: %s\n", strerror(errno));
				}
		}
	}
}

void remove_item(Fl_Widget *w, void *v) {
	int i;
	for (i = 1; i <= selector->size(); i++) {
		if (selector->selected(i)) {
			realloc_filelist(i - 1);
		}
	}
}

void remove_from_filelist(int index) {
	int i;
	for (i = index; i < flSize; i++) {
		fileList[i] = fileList[i + 1];
	}
	selector->remove(index + 1);
	fli = fli - 1;
}

void realloc_filelist(int index) {
	remove_from_filelist(index);
	char **tmp = (char **) realloc(fileList, flSize * sizeof(char*));
	if (tmp == NULL && flSize > 1) {
		exit(-1);
	}
	flSize = flSize - 1;
	fileList = tmp;
}
	
void create_exam(Fl_Widget *w, void *v) {
	if (fli < 1) {
		return;
	}
	
	strncpy (course, courseNameInput->value(), 20);
	strncpy (year, courseYearInput->value(), 20);
	num = atoi(testNumInput->value());
	score = atoi(testScoreInput->value());
	
	if (course == NULL || year == NULL || !num || !score) {
		return;
	}
	
	setCourseName(course);
	setCourseYear(year);
	setTestNum(num);
	setTestScore(score);
	
	generateExams(fileList, fli, forms);
}

void extra_callback(Fl_Choice *w, void*) {
	int val = w->value();
	switch (val) {
		case 0: forms = 1; break;
		case 1: forms = 2; break;
		case 2: forms = 3; break;
		case 3: forms = 4; break;
	}
}

void show_callback() {
	int i;                       // Looping var
	int count;                   // Number of files selected
	char relative(FL_PATH_MAX];  // Relative filename
	Fl_Native_File_Chooser fnfc; // 
	
	fnfc.type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
	fnfc.title("Select Questions");
	fnfc.directory("../../test-db");
	
	switch (fnfc.show()) {
		case -1: printf("ERROR: %s\n", fnfc.errmsg()); break;
		case  1: printf("CANCEL\n"); break;
		default:
			count = fnfc.count();
			if (count > 0) {
				for (i = 0; i <= count; i++) {
					if (fnfc.filename(i)[0] != '\0') {
						fl_filename_relative(relative, sizeof(relative, fnfc.filename(i));
						selector->add(&relative[14]);
						if (fli > flSize) {
							fileList = (char **) realloc(fileList, (fli *2) * sizeof(char*));
						}
						fileList[fli] = (char *) malloc(sizeof(relative) * sizeof(char));
						strncpy(fileList[fli], fnfc.filename(i), sizeof(relative) * sizeof(char));
						fli++;
					}
				}
			}
			break;
	}
}