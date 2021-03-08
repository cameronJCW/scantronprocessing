fndef scantronGui_h
#define scantronGui_h
#include <stdio.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Text_Display.H>

void remove_item(Fl_Widget *w, void *v);
void display_question(Fl_Widget *w, void *v);
void remove_from_filelist(int index);
void realloc_filelist(int index);
void create_exam(Fl_Widget *w, void *v);
void extra_callback(Fl_Choice *w, void*);
void fc_callback(Fl_File_Chooser *fc, void *data);
void show_callback();
#endif
