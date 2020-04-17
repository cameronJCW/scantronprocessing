/***********************************************************
 * TronScan
 * An Amazing piece of software written by:
 * Ethan Whitesel-Jones 
 * Chris Amundson
 * of Western Washington University
 * 
 * Our appologies for the single file.
 * 
 * Usage: tronscan -k/f/b imagename.extension
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <magick/MagickCore.h>

#include <getopt.h>

//prototypes
int GetCornersFromSkewedImage();
int GetCornersFromStraightImage();
void initializeAll(char** argv, int optind);
int GetBottomRightCornerFromStraightImage();
int cornerCheck();
int gradeBubble(int x, int y);
void find_essay();
void find_essay_row(int essay_row);
void getfrontcorners();
void uninitializeAll();
void calculateLengthAndSlope();
void find_question(int question);
void gradeQuestion(int x, int y);
int GetCornersFromSkewedNameImage();
void find_form_row(int form_row, char *form_char);
char find_form();
void find_wnum();
void getbackcorners();
void find_name();
void find_name_row(int name_row);
void find_wnum_row(int wnum_row);

/* scantron relative distance variables */
#define ROW_HEIGHT  0.016268
#define ROW_OFFSET  0.182174
#define BBL_OFFSET  0.016161
#define BBL_RADIUS  0.005877
#define COL1_OFFSET 0.449070
#define COL2_OFFSET 0.318315
#define COL3_OFFSET 0.188051
#define COL4_OFFSET 0.057786
#define BUBBLE_DIAMETER 0.01114206128
#define Y_TOP_THRESHOLD 0.06087360
#define Y_BOTTOM_THRESHOLD 0.9442379
#define INFO_OFFSET 0.003426
#define ESSAY_OFFSET 0.057268
#define FORM_OFFSET 0.138521
#define WNUM_OFFSET 0.220264

// name side offsets
#define NAME_ROW_OFFSET 0.583996
#define NAME_X_OFFSET 0.091451
#define NAME_ROW_HEIGHT 0.016401
#define NAME_BBL_OFFSET 0.016401
#define NAME_BBL_RADIUS 0.005964

//struct quest_struct questions[200];
char wnum[9];
char essay[3];
char name[20];

/* needed globals */
ExceptionInfo *exception;
Image *image, *rotated_image, *images, *edited_image;
ImageInfo *image_info;

int notblank = 1; // for the key output to stop
int b_x, b_y, t_x, t_y = -1;
int rows, cols;
int debug = 0;
int debugessay = 0;
int debugname = 0;
int testing = 0;
int debugwnum = 0;
int debugform = 0;
int argument = 0;
double diameter;
double length, slope;

// for fun
int currentnumber;


/* threshold:
 *   above the threshold is white
 *   below the threshold is black */
int threshold = 36000;
int pencilthreshold = 37000;

void usage(char *arg0) {
  printf("usage:\n%s -k/f/b filename [#]\n", arg0);
  exit(1);
}


int main(int argc,char **argv)
{

  int kflag = 0;
  int fflag = 0;
  int bflag = 0;
  int ch;
  int essaydigit;
  int i;

  extern char *optarg;
  extern int optind;

  while ((ch = getopt(argc, argv, "bdfkt:w")) != -1) {
    switch (ch) {
    case 'b':
      bflag = 1;
      break;
    case 'f':
      fflag = 1;
      break;
    case 'k':
      kflag = 1;
      break;
    case 't':
      pencilthreshold = atoi(optarg);
      break;
    case 'd':
      debug = 1;
      break;
    case 'w':
      debugwnum = 1;
      break;
    default:
      usage(argv[0]);
    }
  }
  
  if (argc-optind < 1 || argc-optind > 2 || fflag+kflag+bflag > 1)
    usage(argv[0]);
  
  currentnumber = 1;
  // make function to initialize exception, rotated imge, images, image_info.
  initializeAll(argv, optind);
  
	if(kflag){
		argument = 0;
		// print out until its blank
		getfrontcorners(); // gets the corners


		find_wnum();
		for(essaydigit = 0; essaydigit < 9; essaydigit++)
			printf("%c", wnum[essaydigit]);
		printf("\n"); // for w number
		printf("%c\n", find_form());
		find_essay();
		for(essaydigit = 0; essaydigit < 3; essaydigit++){
			printf("%c", essay[essaydigit]);
		}
		printf("\n"); // for the essay number


		i = 1;
		while(notblank && i <= 200){
			find_question(i);
			if(notblank) printf("\n");
			i++;
		}
	} else if (fflag) {
		argument = 0;

		getfrontcorners();	

		// find the w number and output it with a new line
		find_wnum();
		for(essaydigit = 0; essaydigit < 9; essaydigit++)
			printf("%c", wnum[essaydigit]);
		printf("\n"); // for the W number

		printf("%c\n", find_form());

		find_essay();
		for(essaydigit = 0; essaydigit < 3; essaydigit++){
			printf("%c", essay[essaydigit]);
		}
		printf("\n"); // for the essay number

		// begin outputting questions
		for(i = 1; i <= 200; i++){
			if(testing) printf("question %d    ", i);
			find_question(i);
			printf("\n");
			currentnumber = i;
		}
	} else if(bflag) {
		int namelen;
		argument = 1;
		// output information for front
		getbackcorners();
		
		find_name();
		for(namelen = 0; namelen < 20; namelen++){
			printf("%c", name[namelen]);
		}
		printf("\n");
	} else {
		usage(argv[0]);
	}

	// dont forget to destroy the image
	uninitializeAll();
	return(0);
}

void getfrontcorners (){

	if(debug) printf("started skewed (ethan's part)\n");
	GetCornersFromSkewedImage();      
	if(testing)printf("ethan's\n%d, %d\n %d,%d\n", t_x, t_y, b_x, b_y );
	if(cornerCheck()){
		GetCornersFromStraightImage();	
		if(testing) printf("chris'\n%d, %d\n%d, %d\n", t_x, t_y, b_x, b_y );
	}

	// code for debugging (making sure reference corners are where they should be).
	if(testing) printf("\n when all is done:\nb_x,  b_y %d, %d\n", b_x, b_y);
	if(testing) printf("t_x, t_y: %d,  %d\n", t_x, t_y); 

	// now that we have the corners, lets start getting questions

	// calculate the length and the slope variables.  
	calculateLengthAndSlope();
}

void getbackcorners(){
	if(debug) printf("started skewed (ethan's part)\n");
	GetCornersFromSkewedNameImage();
	if(testing)printf("ethan's\n%d, %d\n %d, %d\n", t_x, t_y, b_x, b_y);
	if(cornerCheck()){
		GetCornersFromStraightImage();
		if(testing) printf("chris'\n%d, %d\n%d, %d\n", t_x, t_y, b_x, b_y);
	}

	// code for debugging (making sure reference corners are where they should be).
	if(testing) printf("\n when all is done:\nb_x,  b_y %d, %d\n", b_x, b_y);
	if(testing) printf("t_x, t_y: %d,  %d\n", t_x, t_y); 

	// now that we have the corners, lets start getting questions

	// calculate the length and the slope variables.  
	calculateLengthAndSlope();
}

void initializeAll(char **argv, int optind){


	// all of these functions are used to initialize an image for processing
	InitializeMagick(*argv);
	exception=AcquireExceptionInfo();
	image_info=CloneImageInfo((ImageInfo *) NULL);
	(void) strcpy(image_info->filename,argv[optind]);
	images=ReadImage(image_info,exception);
	if (exception->severity != UndefinedException)
		CatchException(exception);
	if (images == (Image *) NULL)
		exit(1);


	image=RemoveFirstImageFromList(&images);

	//edited_image = NewImageList();


}

void uninitializeAll(){

	// all of these functions were used to destroy an image
	DestroyImage(image);
	image_info=DestroyImageInfo(image_info);
	exception=DestroyExceptionInfo(exception);
	DestroyMagick();

}

void calculateLengthAndSlope(){
	length = sqrt((double)((b_x - t_x) * (b_x - t_x) + (b_y - t_y) * (b_y - t_y)));
	slope  = (double)(b_y - t_y) / (double)(b_x - t_x);
	diameter = (double)(rows * BUBBLE_DIAMETER);
}

int cornerCheck(){
	if(debug) printf("\nchecking corners\n");
	//      if(debug)
	//	if(t_y > (rows * Y_TOP_THRESHOLD) || b_y < (rows * Y_BOTTOM_THRESHOLD))
	//		return 1;
	//	if(debug) printf("corners checked");


	if(abs(t_x - b_x) <= 10){
		return 1;
	}

	return 0;
}



/************************
 *
 * GetCornersFromStraightImage:
 *
 * params: char** argv : simply used to get the filename
 * return values: not important, it sets the global corners (b_x, b_y, t_x, t_y)
 *                to their correct vales
 * 
 * this function should be called when the global corners are not within 10% of the edge of the image
 * it simply calls the two functions that each find a corner
 * this was seperated incase one corner turns out to be correct and the other does not.  
 * the functions it calls are:
 * GetBottomRightCornerFromStraightImage
 *
 ************************/
int GetCornersFromStraightImage(){
	// display we are attempging to start 
	if(testing) printf("\nstarting GetCornersFromStraightImage\n");
	fflush(stdout);

	// call get for each bottom right and top right
	GetBottomRightCornerFromStraightImage();

	return 0;

}

/************************
 *
 * GetBottomRightCornerFromStraightImage
 * 
 * params: argv - simply to open the file passed in
 * starts from the given global bottom right (b_x/y)
 * finds the vertical center, then the horizontal center of the current box
 * moves to the next box down, if no box is found, we have hit the last box.  
 *
 * 
 *
 *************************/
int GetBottomRightCornerFromStraightImage(){


	/* all image initialization is done in initializeAll() */


	rows = image->rows;
	cols = image->columns;
	if (debug) printf ("GetBRCFSI: rows = %d, cols = %d\n", rows, cols);
	//int orig_x = b_x;
	//int orig_y = b_y;
	int cur_x = b_x;   // box bottom right x
	int cur_y = b_y;   // box bottom right y
	//int cur_red;
	//int bot_x;
	int spot; // used in testing for artifacts
	int v_center, h_center;
	int x = -1;
	int y = -1;  
	int lr_temp = -1; // lower right
	int ur_temp = -1; // upper right
	int le_temp = -1; // left edge
	int re_temp = -1; // right edge
	int tr_y, br_x, br_y; // to stick to the global x and y theme, these are what we will set the globals as.  

	// threshold: 30000
	// display value of known white and known black (on Scan0001.tif)
	if(debug){
		const PixelPacket ppwhite = AcquireOnePixel(image, 1077, 340, exception);
		const PixelPacket ppblack = AcquireOnePixel(image, 1117, 340, exception);
		printf("white: %d\nblack: %d\n", ppwhite.red, ppblack.red);
	}


	// grab a column from the current X spot.  
	const PixelPacket *ppcol = AcquireImagePixels(image, (int)(cur_x - (cols * .005)), 0, \
			1, (rows - 1), exception);

	// having just x and y to make things simpler
	y = cur_y;
	x = cur_x;

	// display starting conditions
	if(debug) printf("cur_x: %d\ncur_y: %d\nstarting pp[y].red: %d\n",cur_x, cur_y, ppcol[y].red);

	if(debug) printf("Looking at the current column, until we hit the bottom\n");
	if(debug) printf("\n*** COLUMN LOOP START ***\n");

	// while we're in the black (red < threshold) move down until we hit white.
	// this loop finds the lower right corner of the box
	while((int)ppcol[y].red < threshold && y < cols){
		y++;	

		// if we are now above the threshold...
		// check the next 3 pixels, if they are all above the threshold I am assuming we have cleared the block.
		// modify spot to test for more pixels if this doesnt pick up the white artifacts
		if(ppcol[y].red >= threshold){
			for(spot = 1; spot <= 3; spot++){
				if((int)ppcol[y + spot].red < threshold){
					// we hit an artifact, skip it by spot plus 1.
					y += (spot + 1);
					fprintf(stderr, "artifact hit.\n");
					break; // break out of the for loop (hopefully not the while loop)
				}
			}
		}
		if(debug) printf("Y moved to: %d, current ppcol[y].red: %d\n", y, ppcol[y].red);
	}



	// the lower right corner of the box has been found, mark it and fin the upper right
	lr_temp = y;
	if(debug) printf("the lower right corner of the image is: %d\n\n", lr_temp);
	y = cur_y;

	// while we're in the black (red < threshold) move up until we hit the top of the box.
	while((int)ppcol[y].red < threshold && y > 0){
		y--;	

		// if we are now above the threshold...
		// check the next 3 pixels, if they are all above the threshold I am assuming we have cleared the block.
		// modify spot to test for more pixels if this doesnt pick up the white artifacts
		if(ppcol[y].red >= threshold){
			for(spot = 1; spot <= 3; spot++){
				if((int)ppcol[y - spot].red < threshold){
					// we hit an artifact, skip it by spot plus 1.
					y -= (spot + 1);
					fprintf(stderr, "artifact hit.\n");
					break; // break out of the for loop (hopefully not the while loop)
				}
			}
		}
		if(debug) printf("Y moved to: %d, current ppcol[y].red: %d\n", y, ppcol[y].red);
	}

	// we now have the upper right corner of the box, calculate where the vertical center of the box is.
	ur_temp = y;
	y = cur_y;

	// vertical_center = ur_temp + ((lr_temp - ur_temp) / 2);
	v_center = ur_temp + (int)((lr_temp - ur_temp) / 2);
	if(debug) printf("the verticle center is: %d", v_center);

	// we should have cleared the first block
	if(debug) printf("*** COLUMN LOOPS COMPLETE ***\nVertical Center: %d\n", v_center);

	// now get the current row based off of the vertical center
	const PixelPacket *pprow = AcquireImagePixels(image, 0, (int)v_center, (cols - 1), 1, exception);

	if(debug) printf("\n\ncur_x: %d\ncur_y: %d\nstarting pprow[x].red: %d\n\n", x, y, pprow[x].red);
	if(debug) printf("*** ROW LOOP START ***\n");

	//if(debug) printf("starting from x value: %d\n\n", x);
	// while we're in the black (red < threshold) move up until we hit the left side of the box.
	while((int)pprow[x].red < threshold){
		x--;
		// if we are now above the threshold...
		// check the next 3 pixels, if they are all above the threshold I am assuming we have cleared the block.
		// modify spot to test for more pixels if this doesnt pick up the white artifacts
		if(pprow[x].red >= threshold){
			for(spot = 1; spot <= 3; spot++){
				if((int)pprow[x - spot].red < threshold){
					// we hit an artifact, skip it by spot plus 1.
					x -= (spot + 1);
					if(testing) fprintf(stderr, "artifact hit.\n");
					break; // break out of the for loop (hopefully not the while loop)
				}
			}
		}
		if(debug) printf("X moved to: %d, current pprow[x].red: %d\n", x, pprow[x].red);
	}

	// we have found the left edge, not it and reset x for the right edge (shouldnt need to find the right edge
	le_temp = x;
	if(debug) printf("left edge temp: %d\n", le_temp);
	x = cur_x;

	// while we're in the black (red < threshold) move up until we hit the top of the box.
	while((int)pprow[x].red < threshold && x < rows){
		x++;
		// if we are now above the threshold...
		// check the next 3 pixels, if they are all above the threshold I am assuming we have cleared the block.
		// modify spot to test for more pixels if this doesnt pick up the white artifacts
		if(pprow[x].red >= threshold){
			for(spot = 1; spot <= 3; spot++){
				if((int)pprow[x + spot].red < threshold){
					// we hit an artifact, skip it by spot plus 1.
					x += (spot + 1);
					fprintf(stderr, "artifact hit.\n");
					break; // break out of the for loop (hopefully not the while loop)
				}
			}
		}
		if(debug) printf("X moved to: %d, current pprow[y].red: %d\n", x, pprow[x].red);
	}

	// the right edge has been found.
	re_temp = x;
	if(debug) printf("right edge temp: %d\n", re_temp);

	// calculate the horizontal center
	h_center = re_temp + (int)((le_temp - re_temp)/2);



	if(debug) printf("*** ROW LOOP COMPLETE ***\nx, y: %d, %d\n", x, y);

	// this should represent the center of the pixel
	if(debug) printf("V_Center, H_Center:   %d, %d\n", v_center, h_center);

	if(debug) printf("column grabbed at x: %d\n", h_center);
	// now, grab the center column
	const PixelPacket *colcenter = AcquireImagePixels(image, (int)h_center, 0, 1, (rows-1), exception);

	if(debug) printf("\n*** Finding Corners ***\n");

	y = rows - 2;
	while(colcenter[y].red > threshold){
		// move back until we hit a box, which should be our lower right box.
		y--;
		if(debug) printf("y: %d, colcenter[y].red: %d\n", y, colcenter[y].red);
	}

	// note y and continue on, to find the top box
	br_y = y;
	//if(debug) printf("the lower box is located at x, y: %d\n\n", h_center, br_y);

	// set x as the horizontal center
	// move right until we hit the edge, check down a pixel.
	x = h_center;
	if(debug) printf("*** working on bottom right corner ***\n");
	while(x>0 && x<rows){
		const PixelPacket currentpixel = AcquireOnePixel(image, x, y, exception);
		if(currentpixel.red < threshold){ // if its black
			if(debug) printf("x, y: %d, %d\n", x, y);
			if(argument == 0) x++;
			else x--;
		} else {
			// we hit white, break out
			break;
		}
	}

	if(debug) printf("the lower right corner...\nx, y: %d, %d\n", x, y);
	br_x = x;
	b_x = x;
	br_y = y;
	b_y = y;

	// now, looking down from the top of the page
	if(debug) printf("*** working on the top right corner ***\n");
	y = 0;

	while(colcenter[y].red > threshold){
		y++;
		if(debug) printf("y: %d, colcenter[y].red: %d\n", y, colcenter[y].red);
	}

	// note top right y and reset x as the horizontal center
	tr_y = y;
	x = h_center;
	if(debug) printf("the upper box is located at y: %d\n\n", tr_y);

	while(1){
		const PixelPacket currentpixel = AcquireOnePixel(image, x, y, exception);
		if(currentpixel.red < threshold){ // if its black
			if(debug) printf("x, y: %d, %d\n", x, y);
			if(argument == 0) x++;
			else x--;
		} else {
			// we hit white, break out
			break;
		}
	}

	if(debug) printf("\n*** the upper right corner...\nx, y: %d, %d\n", x, y);
	t_x = x;
	t_y = y;



	// 
	if(debug) printf("finished GetBottomRightCornerFromStraightImage\n\n");
	return 0;
}




int GetCornersFromSkewedImage(char **argv)
{




	rows = image->rows;
	cols = image->columns;

	int br_x = -1;
	int br_y = -1;
	int tr_x = -1;
	int tr_y = -1;
	int temp_x = -1;
	int temp_y = -1;
	int temp_x_bound;
	int y = rows - 1;
	int x = cols - 1;
	int i = 0;
	long counter = 0;

	for(x = cols-1; x >= cols*0.75;x--)
	{
		const PixelPacket *p = AcquireImagePixels(image, x, 0, 1, rows-1, exception);
		for(y = rows - 1; y >= 0; y--)
		{
			if( p[y].red < 30000 && br_x == -1)
			{
				//printf("x,y,red:\t%i,\t%i,\t%i\n", x,y, p[y].red);
				if( (int)(x - cols*0.015) < (int)(cols*0.75) )
					temp_x_bound = (int)(cols*0.75);
				else
					temp_x_bound = (int)(x-cols*0.015);

				const PixelPacket *q = AcquireImagePixels(image, temp_x_bound, y,
						x - temp_x_bound, 1, exception);
				i = 0;
				counter = 0;

				// to make sure we hit a block and not an artifact color
				// that was below the threshhold, then check the values
				// in a line to the left, if we hit a block, we should
				// continue hitting dark colored values the length of
				// the block
				for(i = 0; i < (x - temp_x_bound);i++)
				{
					counter += q[i].red;
				}
				// average the counter value, to see if the majority of
				// the blocks were below the threshhold, indicating we
				// most likely hit a block and not an artifact
				if( (counter / (x - temp_x_bound)) < 28000 )
				{
					br_x = x;
					br_y = y;
				}
			}
		}
	}
	if( br_y > rows * 0.5 )
	{
		// we probably hit the bottom right corner, though
		// do a check later because this could be wrong with
		// some images, like Scan0001.tif

		// now go from the upper right corner, and scan columns
		// from the right to left, scanning each column from the
		// top down.
		for(x = cols-1; x >= cols*0.75;x--)
		{
			const PixelPacket *p = AcquireImagePixels(image, x, 0, 1, rows-1,
					exception);
			for(y = 0; y < rows; y++)
			{
				if( p[y].red < 30000 && tr_x == -1)
				{
					if( (int)(x - cols*0.01) < (int)(cols*0.75) )
						temp_x_bound = (int)(cols*0.75);
					else
						temp_x_bound = (int)(x-cols*0.01);

					const PixelPacket *q = AcquireImagePixels(image, temp_x_bound,
							y, x - temp_x_bound, 1, exception);
					i = 0;
					counter = 0;

					// to make sure we hit a block and not an artifact color
					// that was below the threshhold, then check the values
					// in a line to the left, if we hit a block, we should
					// continue hitting dark colored values the length of
					// the block
					for(i = 0; i < (x - temp_x_bound);i++)
					{
						counter += q[i].red;
					}
					// average the counter value, to see if the majority of
					// the blocks were below the threshhold, indicating we
					// most likely hit a block and not an artifact
					if( (counter / (x - temp_x_bound)) < 28000 )
					{
						// if we haven't hit a block yet, then
						// these x,y vals will be the current block
						if( temp_y == -1 )
						{
							temp_x = x;
							temp_y = y;
						}
						else
						{
							// keep resetting the top corner x,y coords
							// if they are in fact higher up
							if( y < temp_y )
							{
								temp_x = x;
								temp_y = y;
							}
							// check to see if there is a sudden
							// dropoff when hitting black values
							// if so, this probably means that we have
							// scanned to far to the left of the very
							// top block and are hitting white space
							// so set the previously stored temp values
							// as the tr_x, tr_y, as they were the highest
							// y coordinates we recorded for the top right box
							if( y > (temp_y + (0.03 * rows) ) )
							{
								// we've started to scan farther
								tr_x = temp_x;
								tr_y = temp_y;
							}
						}
						// do this break, to break out of the inner y loop, and
						// to begin scanning the next column to the left, if this
						// break isn't here, then the loop keeps scanning down the
						// column, and then will find a dark block far below the
						// current one, and will think it must be at the top block
						// and will stop scanning. This explanation I know is real
						// confusing probably, but I'll try to explain it better
						// later
						break;
					}
				}
			}
		}
	}
	if( br_y < rows * 0.5 )
	{
		// reassign br_x/y to the appropriate variables
		// tr_x and tr_y
		tr_y = br_y;
		tr_x = br_x;
		br_y = -1;
		br_x = -1;
		// we probably hit the top right corner, though
		// do a check later because this could be wrong with
		// some images, like Scan0001.tif

		// now go from the bottom right corner, and scan columns
		// from the right to left, scanning each column from the
		// down up.
		for(x = cols-1; x >= cols*0.75;x--)
		{
			const PixelPacket *p = AcquireImagePixels(image, x, 0, 1, rows-1,
					exception);
			for(y = rows-1; y >= 0; y--)
			{
				if( p[y].red < 30000 && br_x == -1)
				{
					if( (int)(x - cols*0.01) < (int)(cols*0.75) )
						temp_x_bound = (int)(cols*0.75);
					else
						temp_x_bound = (int)(x-cols*0.01);

					const PixelPacket *q = AcquireImagePixels(image, temp_x_bound,
							y, x - temp_x_bound, 1, exception);
					i = 0;
					counter = 0;

					// to make sure we hit a block and not an artifact color
					// that was below the threshhold, then check the values
					// in a line to the left, if we hit a block, we should
					// continue hitting dark colored values the length of
					// the block
					for(i = 0; i < (x - temp_x_bound);i++)
					{
						counter += q[i].red;
					}
					// average the counter value, to see if the majority of
					// the blocks were below the threshhold, indicating we
					// most likely hit a block and not an artifact
					if( (counter / (x - temp_x_bound)) < 28000 )
					{
						// if we haven't hit a block yet, then
						// these x,y vals will be the current block
						if( temp_y == -1 )
						{
							temp_x = x;
							temp_y = y;
						}
						else
						{
							// keep resetting the top corner x,y coords
							// if they are in fact lower on the image
							// keeping in mind, 0 is at the top, and a
							// larger y val is farther down the image
							if( y > temp_y )
							{
								temp_x = x;
								temp_y = y;
							}
							// check to see if there is a sudden
							// dropoff when hitting black values
							// if so, this probably means that we have
							// scanned to far to the left of the very
							// bottom block and are hitting white space
							// so set the previously stored temp values
							// as the br_x, br_y, as they were the lowest
							// y coordinates we recorded for the bottom right box
							if( y < (temp_y - (0.03 * rows) ) )
							{
								// we've started to scan farther to the left
								// of the line of blocks it looks like
								// so set the br_x/y values at their prevoius point
								br_x = temp_x;
								br_y = temp_y;
							}
						}
						// do this break, to break out of the inner y loop, and
						// to begin scanning the next column to the left, if this
						// break isn't here, then the loop keeps scanning down the
						// column, and then will find a dark block far below the
						// current one, and will think it must be at the top block
						// and will stop scanning. This explanation I know is real
						// confusing probably, but I'll try to explain it better
						// later
						break;
					}
				}
			}
		}
	}
	if(debug)printf("br_x, br_y: %i, %i\n", br_x, br_y);
	if(debug)printf("tr_x, tr_y: %i, %i\n", tr_x, tr_y);
	b_x = br_x;
	b_y = br_y;
	t_x = tr_x;
	t_y = tr_y;

	if(debug)printf("****************************\n");
	if(debug)printf("b_x, b_y: %d, %d\nt_x, t_y: %d, %d\n", b_x, b_y, t_x, t_y);
	if(debug)printf("****************************\n");

	// DestroyImage release the image from memory



	/*
	   Write the image thumbnail.
	 */
	//      (void) strcpy(thumbnails->filename,argv[2]);
	//      WriteImage(image_info,thumbnails);
	/*
	   Destroy the image thumbnail and exit.
	 */
	//thumbnails=DestroyImageList(thumbnails); */
	/*image_info=DestroyImageInfo(image_info);
	  exception=DestroyExceptionInfo(exception);
	  DestroyMagick();*/
	return(0);
}



void find_question(int question)
{
	double q_dist_percent, row_dist_percent, row_offset_x, row_offset_y, start_x, start_y, row_slope, q_offset_x, q_offset_y, q_x, q_y;
	int xa, xb, xc, xd, xe, ya, yb, yc, yd, ye, answered;
	// find how many rows to move down
	// do question - 1, so question (200-1) % 50 is 49, so it skips
	// the first 49 rows, and question (1 - 1) % 50 is 0, so question 1
	// skips 0 rows
	int row_number = (question - 1) % 50;

	// now find the % distance to move down the line
	// by multiplying the rise and run, by the distance percentage
	row_dist_percent = ROW_OFFSET + (ROW_HEIGHT * row_number);
	row_offset_x = (b_x - t_x) * row_dist_percent;
	row_offset_y = (b_y - t_y) * row_dist_percent;

	// get the starting row x,y coords by adding these offsets to the tr x,y coords
	start_x = t_x + row_offset_x;
	start_y = t_y + row_offset_y;

	if (debug) printf ("Question %d: ", question);
	if(debug)printf("Starting x,y coords: %f, %f\n", start_x, start_y);

	// now based off the question number, find out the column offset
	// for how far to move to the left in the row
	q_dist_percent = 0;
	if( question >= 151 )
		q_dist_percent = COL4_OFFSET;
	else if( question >= 101 )
		q_dist_percent = COL3_OFFSET;
	else if( question >= 51 )
		q_dist_percent = COL2_OFFSET;
	else if( question >= 0 )
		q_dist_percent = COL1_OFFSET;

	// now we know how far to move to the left of the row block, so
	// find the x,y offsets to move from the row end block to the question
	// we do this, by finding the line perpendicular to the row blocks,
	// which has a slope that is the negative recipricol of the black block line's
	// slope, then use our q_dist value, as the hypotenuse of a triangle, and then
	// find the x, and y values of that triangle from the neg. recip. of the slope
	row_slope = -(1 / slope);

	// x is always going to be a negative offset, since we are moving left in the image
	// and subtracting values from the row
	q_offset_x = -(sqrt((double)(((q_dist_percent * length)*(q_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	// y, will be positive if the row_slope is positive as the line is rotated like
	// this \  while it will be negative if the row_slope is negative like this /
	// remember that normally / is a positive slope, but reversed with the image coord system
	q_offset_y = (q_offset_x * row_slope);

	// this is the pixel coordinate for the middle of the 'E' bubble
	q_x = start_x + q_offset_x;
	q_y = start_y + q_offset_y;
	xe = q_x;
	ye = q_y;

	if(debug)printf("E x,y: %f, %f\n", q_x, q_y);

	// this is the pixel coordinate for the middle of the 'D' bubble
	q_dist_percent += BBL_OFFSET; // we're now at D
	q_offset_x = -(sqrt((double)(((q_dist_percent * length)*(q_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	q_offset_y = (q_offset_x * row_slope);
	q_x = start_x + q_offset_x;
	q_y = start_y + q_offset_y;

	xd = q_x;
	yd = q_y;
	if(debug)printf("D x,y: %f, %f\n", q_x, q_y);

	q_dist_percent += BBL_OFFSET; // we're now at D
	q_offset_x = -(sqrt((double)(((q_dist_percent * length)*(q_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	q_offset_y = (q_offset_x * row_slope);
	q_x = start_x + q_offset_x;
	q_y = start_y + q_offset_y;
	xc = q_x;
	yc = q_y;
	if(debug)printf("C x,y: %f, %f\n", q_x, q_y);

	q_dist_percent += BBL_OFFSET; // we're now at D
	q_offset_x = -(sqrt((double)(((q_dist_percent * length)*(q_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	q_offset_y = (q_offset_x * row_slope);
	q_x = start_x + q_offset_x;
	q_y = start_y + q_offset_y;
	xb = q_x;
	yb = q_y;
	if(debug)printf("B x,y: %f, %f\n", q_x, q_y);

	q_dist_percent += BBL_OFFSET; // we're now at D
	q_offset_x = -(sqrt((double)(((q_dist_percent * length)*(q_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	q_offset_y = (q_offset_x * row_slope);
	q_x = start_x + q_offset_x;
	q_y = start_y + q_offset_y;
	xa = q_x;
	ya = q_y;
	if(debug)printf("A x,y: %f, %f\n", q_x, q_y);


	answered = 0;

	if(gradeBubble(xa, ya)){
		printf("a"); //else printf("x ");
		answered = 1;
	}
	if(gradeBubble(xb, yb)){ 
		printf("b"); //else printf("x ");
		answered = 1;
	}
	if(gradeBubble(xc, yc)){
		printf("c"); //else printf("x ");
		answered = 1;
	}
	if(gradeBubble(xd, yd)){ 
		printf("d"); //else printf("x ");	
		answered = 1;
	}
	if(gradeBubble(xe, ye)){ 
		printf("e"); //else printf("x ");
		answered = 1;
	}

	//if(testing) printf("threshold: %d", pencilthreshold);      


	if(!answered){
		printf("x");
		notblank = 0; // we hit a blank, 
	}

}

// gradeBubble
// the x and y passed in should represent the center of a bubble
int gradeBubble(int x, int y){
	int numberchecked, numbermarked = 0;
	int startx, starty, halfwidth;
	int i, totalred, avgred;
	//if(debug) printf("x, y: %d, %d\n", x, y);
	//if(debug) printf("bubble_length: %d\n", bubble_length);

	totalred = 0;

	halfwidth = (int)(sqrt(((b_y - t_y)*(b_y - t_y)) + ((b_x - t_x) * (b_x - t_x))) * BBL_RADIUS);
	startx = x - halfwidth;
	starty = y - halfwidth;
	if(debug) printf("starting x,y: %d, %d\n", startx, starty);

	const PixelPacket *p = AcquireImagePixels(image, startx, starty, halfwidth * 2, halfwidth * 2, exception);
	for(i = 0; i < ((halfwidth * 2)*(halfwidth * 2)); i++){
	//	totalred += (int)p[i].red;
	  if (debug) printf ("i=%d, p[i].red=%d\n", i, (int)p[i].red);
	  if (debug) printf ("i=%d, p[i].green=%d\n", i, (int)p[i].green);
	  if (debug) printf ("i=%d, p[i].blue=%d\n", i, (int)p[i].blue);
		if((int)p[i].red < pencilthreshold &&
		   (int)p[i].green < pencilthreshold &&
		   (int)p[i].blue < pencilthreshold){
			numbermarked++;
		}
	}

	numberchecked = (int)((halfwidth * 2)*(halfwidth * 2));
	avgred = (int)totalred / ((halfwidth * 2)*(halfwidth * 2));
     
//	if(avgred <= pencilthreshold)
	if(numbermarked > (int)(numberchecked * .45))
		return 1;

	return 0;
}


void find_essay()
{
	int i = 0;
	for(i=0; i <= 9;i++)
	{
		find_essay_row(i);
	}
}

void find_essay_row(int essay_row)
{
	double e_dist_percent, row_dist_percent, row_offset_x, row_offset_y, start_x, start_y, row_slope, e_offset_x, e_offset_y, e_x, e_y;
	int col1_x, col1_y, col2_x, col2_y, col3_x, col3_y;
	row_dist_percent = INFO_OFFSET + (ROW_HEIGHT * essay_row);
	row_offset_x = (b_x - t_x) * row_dist_percent;
	row_offset_y = (b_y - t_y) * row_dist_percent;
	if(debugessay) printf("row_offset_x: %f\n", row_offset_x);
	if(debugessay) printf("row_offset_y: %f\n", row_offset_y);
	start_x = t_x + row_offset_x;
	start_y = t_y + row_offset_y;
	if(debugessay)printf("Starting essay x,y coords: %f, %f\n", start_x, start_y);

	////e_dist_percent = ESSAY_OFFSET + essay_row * ROW_HEIGHT;
	e_dist_percent = ESSAY_OFFSET;
	row_slope = -(1 / slope);

	// x is always going to be a negative offset, since we are moving left in the image
	// and subtracting values from the row
	e_offset_x = -(sqrt((double)(((e_dist_percent * length)*(e_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	// y, will be positive if the row_slope is positive as the line is rotated like
	// this \  while it will be negative if the row_slope is negative like this /
	// remember that normally / is a positive slope, but reversed with the image coord system
	e_offset_y = (e_offset_x * row_slope);
	// this is the pixel coordinate for the middle of the third essay '0' bubble
	e_x = start_x + e_offset_x;
	e_y = start_y + e_offset_y;

	col3_x = e_x;
	col3_y = e_y;

	e_dist_percent += BBL_OFFSET; // we're now at the second column 0
	e_offset_x = -(sqrt((double)(((e_dist_percent * length)*(e_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	e_offset_y = (e_offset_x * row_slope);
	e_x = start_x + e_offset_x;
	e_y = start_y + e_offset_y;

	col2_x = e_x;
	col2_y = e_y;

	e_dist_percent += BBL_OFFSET; // we're now at C
	e_offset_x = -(sqrt((double)(((e_dist_percent * length)*(e_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	e_offset_y = (e_offset_x * row_slope);
	e_x = start_x + e_offset_x;
	e_y = start_y + e_offset_y;

	col1_x = e_x;
	col1_y = e_y;
	if(debugessay)printf("Essay0: %i x,y: %i, %i\n", essay_row, col1_x, col1_y);
	if(debugessay)printf("Essay1: %i x,y: %i, %i\n", essay_row, col2_x, col2_y);
	if(debugessay)printf("Essay2: %i x,y: %i, %i\n", essay_row, col3_x, col3_y);

	if(gradeBubble(col1_x, col1_y))
	{
		essay[0] = essay_row + 48 ;
		if(debugessay) printf("BLAH\n");
	}
	if(gradeBubble(col2_x, col2_y))
	{
		essay[1] = essay_row + 48 ;
		if(debugessay) printf("BLAH\n");
	}
	if(gradeBubble(col3_x, col3_y))
	{
		essay[2] = essay_row + 48 ;
		if(debugessay) printf("BLAH\n");
	}
}

int GetCornersFromSkewedNameImage()
{
	rows = image->rows;
	cols = image->columns;

	int bl_x = -1;
	int bl_y = -1;
	int tl_x = -1;
	int tl_y = -1;
	int temp_x = -1;
	int temp_y = -1;
	int temp_x_bound;
	int y = rows - 1;
	int x = cols - 1;
	int i = 0;
	long counter = 0;

	int quarter_out_x = cols*0.25;
	for(x = 0; x <= quarter_out_x; x++)
	{
		const PixelPacket *p = AcquireImagePixels(image, x, 0, 1, rows-1, exception);
		for(y = rows - 1; y >= 0; y--)
		{
			if( p[y].red < 30000 && bl_x == -1)
			{
				//if( (int)(x - cols*0.015) < (int)(cols*0.75) )
				//	temp_x_bound = (int)(cols*0.75);
				//else
				// Now that I think about it, those if statements aren't needed
				// as if it moves past the 25%, or with question side the 75% boundary
				// it doesn't matter
				temp_x_bound = (int)(x+cols*0.015);

				const PixelPacket *q = AcquireImagePixels(image, temp_x_bound, y,
						temp_x_bound - x, 1, exception);
				i = 0;
				counter = 0;

				// to make sure we hit a block and not an artifact color
				// that was below the threshhold, then check the values
				// in a line to the right, if we hit a block, we should
				// continue hitting dark colored values the length of
				// the block
				for(i = 0; i < (temp_x_bound - x);i++)
				{
					counter += q[i].red;
				}
				// average the counter value, to see if the majority of
				// the blocks were below the threshhold, indicating we
				// most likely hit a block and not an artifact
				if( (counter / (temp_x_bound - x)) < 28000 )
				{
					bl_x = x;
					bl_y = y;
				}
			}
		}
	}
	if( bl_y > rows * 0.5 )
	{
		// now go from the upper right corner, and scan columns
		// from the right to left, scanning each column from the
		// top down.
		for(x = 0; x <= quarter_out_x; x++)
		{
			const PixelPacket *p = AcquireImagePixels(image, x, 0, 1, rows-1,
					exception);
			for(y = 0; y < rows; y++)
			{
				if( p[y].red < 30000 && tl_x == -1)
				{
					//if( (int)(x - cols*0.01) < (int)(cols*0.75) )
					//	temp_x_bound = (int)(cols*0.75);
					//else
					temp_x_bound = (int)(x+cols*0.01);

					const PixelPacket *q = AcquireImagePixels(image, temp_x_bound,
							y, temp_x_bound - x, 1, exception);
					i = 0;
					counter = 0;

					// to make sure we hit a block and not an artifact color
					// that was below the threshhold, then check the values
					// in a line to the left, if we hit a block, we should
					// continue hitting dark colored values the length of
					// the block
					for(i = 0; i < (temp_x_bound - x);i++)
					{
						counter += q[i].red;
					}
					// average the counter value, to see if the majority of
					// the blocks were below the threshhold, indicating we
					// most likely hit a block and not an artifact
					if( (counter / (temp_x_bound - x)) < 28000 )
					{
						// if we haven't hit a block yet, then
						// these x,y vals will be the current block
						if( temp_y == -1 )
						{
							temp_x = x;
							temp_y = y;
						}
						else
						{
							// keep resetting the top corner x,y coords
							// if they are in fact higher up
							if( y < temp_y )
							{
								temp_x = x;
								temp_y = y;
							}
							// check to see if there is a sudden
							// dropoff when hitting black values
							// if so, this probably means that we have
							// scanned to far to the left of the very
							// top block and are hitting white space
							// so set the previously stored temp values
							// as the tl_x, tl_y, as they were the highest
							// y coordinates we recorded for the top right box
							if( y > (temp_y + (0.03 * rows) ) )
							{
								// we've started to scan farther
								tl_x = temp_x;
								tl_y = temp_y;
							}
						}
						// do this break, to break out of the inner y loop, and
						// to begin scanning the next column to the left, if this
						// break isn't here, then the loop keeps scanning down the
						// column, and then will find a dark block far below the
						// current one, and will think it must be at the top block
						// and will stop scanning. This explanation I know is real
						// confusing probably, but I'll try to explain it better
						// later
						break;
					}
				}
			}
		}
	}
	if( bl_y < rows * 0.5 )
	{
		// reassign bl_x/y to the appropriate variables
		// tl_x and tl_y
		tl_y = bl_y;
		tl_x = bl_x;
		bl_y = -1;
		bl_x = -1;
		// we probably hit the top right corner, though
		// do a check later because this could be wrong with
		// some images, like Scan0001.tif

		// now go from the bottom right corner, and scan columns
		// from the right to left, scanning each column from the
		// down up.

		for(x = 0; x <= quarter_out_x; x++)
		{
			const PixelPacket *p = AcquireImagePixels(image, x, 0, 1, rows-1,
					exception);
			for(y = rows-1; y >= 0; y--)
			{
				if( p[y].red < 30000 && bl_x == -1)
				{
					//if( (int)(x - cols*0.01) < (int)(cols*0.75) )
					//	temp_x_bound = (int)(cols*0.75);
					//else
					temp_x_bound = (int)(x+cols*0.01);

					const PixelPacket *q = AcquireImagePixels(image, temp_x_bound,
							y, temp_x_bound - x, 1, exception);
					i = 0;
					counter = 0;

					// to make sure we hit a block and not an artifact color
					// that was below the threshhold, then check the values
					// in a line to the left, if we hit a block, we should
					// continue hitting dark colored values the length of
					// the block
					for(i = 0; i < (temp_x_bound - x);i++)
					{
						counter += q[i].red;
					}
					// average the counter value, to see if the majority of
					// the blocks were below the threshhold, indicating we
					// most likely hit a block and not an artifact
					if( (counter / (temp_x_bound - x)) < 28000 )
					{
						// if we haven't hit a block yet, then
						// these x,y vals will be the current block
						if( temp_y == -1 )
						{
							temp_x = x;
							temp_y = y;
						}
						else
						{
							// keep resetting the top corner x,y coords
							// if they are in fact lower on the image
							// keeping in mind, 0 is at the top, and a
							// larger y val is farther down the image
							if( y > temp_y )
							{
								temp_x = x;
								temp_y = y;
							}
							// check to see if there is a sudden
							// dropoff when hitting black values
							// if so, this probably means that we have
							// scanned to far to the left of the very
							// bottom block and are hitting white space
							// so set the previously stored temp values
							// as the bl_x, bl_y, as they were the lowest
							// y coordinates we recorded for the bottom right box
							if( y < (temp_y - (0.03 * rows) ) )
							{
								// we've started to scan farther to the left
								// of the line of blocks it looks like
								// so set the bl_x/y values at their prevoius point
								bl_x = temp_x;
								bl_y = temp_y;
							}
						}
						// do this break, to break out of the inner y loop, and
						// to begin scanning the next column to the left, if this
						// break isn't here, then the loop keeps scanning down the
						// column, and then will find a dark block far below the
						// current one, and will think it must be at the top block
						// and will stop scanning. This explanation I know is real
						// confusing probably, but I'll try to explain it better
						// later
						break;
					}
				}
			}
		}
	}
	if(debug)printf("bl_x, bl_y: %i, %i\n", bl_x, bl_y);
	if(debug)printf("tl_x, tl_y: %i, %i\n", tl_x, tl_y);
	b_x = bl_x;
	b_y = bl_y;
	t_x = tl_x;
	t_y = tl_y;

	if(debug)printf("****************************\n");
	if(debug)printf("b_x, b_y: %d, %d\nt_x, t_y: %d, %d\n", b_x, b_y, t_x, t_y);
	if(debug)printf("****************************\n");

	return(0);
}

char find_form()
{
	char form_char = ' ';
	int i = 0;
	for(i=0; i <= 3; i++)
	{
		find_form_row(i, &form_char);
	}
	return form_char;
}

void find_form_row(int form_row, char *form_char)
{
	double f_dist_percent, row_dist_percent, row_offset_x, row_offset_y, start_x, start_y, row_slope, f_offset_x, f_offset_y, f_x, f_y;
	row_dist_percent = INFO_OFFSET + (ROW_HEIGHT * form_row);
	row_offset_x = (b_x - t_x) * row_dist_percent;
	row_offset_y = (b_y - t_y) * row_dist_percent;
	if(debugform) printf("row_offset_x: %f\n", row_offset_x);
	if(debugform) printf("row_offset_y: %f\n", row_offset_y);
	start_x = t_x + row_offset_x;
	start_y = t_y + row_offset_y;
	if(debugform)printf("Starting form x,y coords: %f, %f\n", start_x, start_y);

	////e_dist_percent = ESSAY_OFFSET + essay_row * ROW_HEIGHT;
	f_dist_percent = FORM_OFFSET;
	row_slope = -(1 / slope);

	// x is always going to be a negative offset, since we are moving left in the image
	// and subtracting values from the row
	f_offset_x = -(sqrt((double)(((f_dist_percent * length)*(f_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	// y, will be positive if the row_slope is positive as the line is rotated like
	// this \  while it will be negative if the row_slope is negative like this /
	// remember that normally / is a positive slope, but reversed with the image coord system
	f_offset_y = (f_offset_x * row_slope);
	// this is the pixel coordinate for the middle of the third essay '0' bubble
	f_x = start_x + f_offset_x;
	f_y = start_y + f_offset_y;

	if(gradeBubble(f_x, f_y))
	{
		*form_char = form_row + 65;
		if(debugform) printf("BLAHFORM\n");
	}
}

void find_wnum()
{
	int i = 0;
	wnum[0] = 'W';
	for(i=1; i < 9; i++)
		wnum[i] = 0; 
	for(i=0; i <= 9;i++)
	{
		find_wnum_row(i);
	}
}

void find_wnum_row(int wnum_row)
{
	double w_dist_percent, row_dist_percent, row_offset_x, row_offset_y, start_x, start_y,
	       row_slope, w_offset_x, w_offset_y, w_x, w_y;
	double x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, x6, y6, x7, y7, x8, y8;
	row_dist_percent = INFO_OFFSET + (ROW_HEIGHT * wnum_row);
	row_offset_x = (b_x - t_x) * row_dist_percent;
	row_offset_y = (b_y - t_y) * row_dist_percent;
	if(debugwnum) printf("row_offset_x: %f\n", row_offset_x);
	if(debugwnum) printf("row_offset_y: %f\n", row_offset_y);
	start_x = t_x + row_offset_x;
	start_y = t_y + row_offset_y;
	if(debugwnum)printf("Starting WNUM x,y coords: %f, %f\n", start_x, start_y);

	////e_dist_percent = ESSAY_OFFSET + essay_row * ROW_HEIGHT;
	w_dist_percent = WNUM_OFFSET;
	row_slope = -(1 / slope);

	// x is always going to be a negative offset, since we are moving left in the image
	// and subtracting values from the row
	w_offset_x = -(sqrt((double)(((w_dist_percent * length)*(w_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	// y, will be positive if the row_slope is positive as the line is rotated like
	// this \  while it will be negative if the row_slope is negative like this /
	// remember that normally / is a positive slope, but reversed with the image coord system
	w_offset_y = (w_offset_x * row_slope);
	// this is the pixel coordinate for the 8th column
	w_x = start_x + w_offset_x;
	w_y = start_y + w_offset_y;
 
	// 8th row
	x8 = w_x;
	y8 = w_y;

	// starting 7th row
	w_dist_percent += BBL_OFFSET; // we're now at the 7th column
	w_offset_x = -(sqrt((double)(((w_dist_percent * length)*(w_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	w_offset_y = (w_offset_x * row_slope);
	w_x = start_x + w_offset_x;
	w_y = start_y + w_offset_y;

	x7 = w_x;
	y7 = w_y;
	// starting 6th row
	w_dist_percent += BBL_OFFSET; // 
	w_offset_x = -(sqrt((double)(((w_dist_percent * length)*(w_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	w_offset_y = (w_offset_x * row_slope);
	w_x = start_x + w_offset_x;
	w_y = start_y + w_offset_y;

	x6 = w_x;
	y6 = w_y;

	// starting 5th row
	w_dist_percent += BBL_OFFSET; // 
	w_offset_x = -(sqrt((double)(((w_dist_percent * length)*(w_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	w_offset_y = (w_offset_x * row_slope);
	w_x = start_x + w_offset_x;
	w_y = start_y + w_offset_y;

	x5 = w_x;
	y5 = w_y;

	// starting 4th row
	w_dist_percent += BBL_OFFSET; // 
	w_offset_x = -(sqrt((double)(((w_dist_percent * length)*(w_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	w_offset_y = (w_offset_x * row_slope);
	w_x = start_x + w_offset_x;
	w_y = start_y + w_offset_y;

	x4 = w_x;
	y4 = w_y;

	// starting 3rd row
	w_dist_percent += BBL_OFFSET; // 
	w_offset_x = -(sqrt((double)(((w_dist_percent * length)*(w_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	w_offset_y = (w_offset_x * row_slope);
	w_x = start_x + w_offset_x;
	w_y = start_y + w_offset_y;

	x3 = w_x;
	y3 = w_y;

	// starting 2nd row
	w_dist_percent += BBL_OFFSET; // 
	w_offset_x = -(sqrt((double)(((w_dist_percent * length)*(w_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	w_offset_y = (w_offset_x * row_slope);
	w_x = start_x + w_offset_x;
	w_y = start_y + w_offset_y;

	x2 = w_x;
	y2 = w_y;

	// starting 1st row
	w_dist_percent += BBL_OFFSET; // 
	w_offset_x = -(sqrt((double)(((w_dist_percent * length)*(w_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	w_offset_y = (w_offset_x * row_slope);
	w_x = start_x + w_offset_x;
	w_y = start_y + w_offset_y;

	x1 = w_x;
	y1 = w_y;

	if(debugwnum)printf("wnum1: %i x,y: %f, %f\n", wnum_row, x1, y1);
	if(debugwnum)printf("wnum2: %i x,y: %f, %f\n", wnum_row, x2, y2);
	if(debugwnum)printf("wnum3: %i x,y: %f, %f\n", wnum_row, x3, y3);
	if(debugwnum)printf("wnum4: %i x,y: %f, %f\n", wnum_row, x4, y4);
	if(debugwnum)printf("wnum5: %i x,y: %f, %f\n", wnum_row, x5, y5);
	if(debugwnum)printf("wnum6: %i x,y: %f, %f\n", wnum_row, x6, y6);
	if(debugwnum)printf("wnum7: %i x,y: %f, %f\n", wnum_row, x7, y7);
	if(debugwnum)printf("wnum8: %i x,y: %f, %f\n", wnum_row, x8, y8);

	if(gradeBubble(x1, y1))
	{
		wnum[1] = wnum_row + 48 ;
		if(debugessay) printf("BLAHNUM\n");
	}
	if(gradeBubble(x2, y2))
	{
		wnum[2] = wnum_row + 48 ;
		if(debugessay) printf("BLAHNUM\n");
	}
	if(gradeBubble(x3, y3))
	{
		wnum[3] = wnum_row + 48 ;
		if(debugessay) printf("BLAHNUM\n");
	}
	if(gradeBubble(x4, y4))
	{
		wnum[4] = wnum_row + 48 ;
		if(debugessay) printf("BLAHNUM\n");
	}
	if(gradeBubble(x5, y5))
	{
		wnum[5] = wnum_row + 48 ;
		if(debugessay) printf("BLAHNUM\n");
	}
	if(gradeBubble(x6, y6))
	{
		wnum[6] = wnum_row + 48 ;
		if(debugessay) printf("BLAHNUM\n");
	}
	if(gradeBubble(x7, y7))
	{
		wnum[7] = wnum_row + 48 ;
		if(debugessay) printf("BLAHNUM\n");
	}
	if(gradeBubble(x8, y8))
	{
		wnum[8] = wnum_row + 48 ;
		if(debugessay) printf("BLAHNUM\n");
	}
}

void find_name()
{
	int i = 0;
	// initialize the name to spaces
	for(i=0; i <20; i++)
	{
		name[i] = ' ';
	}
	for(i=0; i <26; i++)
	{
		find_name_row(i);
	}
}

void find_name_row(int name_row)
{
	double n_dist_percent, row_dist_percent, row_offset_x, row_offset_y, start_x, start_y,
		row_slope, n_offset_x, n_offset_y;
	double x[20];
	double y[20];
	row_dist_percent = NAME_ROW_OFFSET + (NAME_ROW_HEIGHT * name_row);
	row_offset_x = (b_x - t_x) * row_dist_percent;
	row_offset_y = (b_y - t_y) * row_dist_percent;
	if(debugname) printf("row_offset_x: %f\n", row_offset_x);
	if(debugname) printf("row_offset_y: %f\n", row_offset_y);
	start_x = t_x + row_offset_x;
	start_y = t_y + row_offset_y;
	if(debugname)printf("Starting Name x,y coords: %f, %f\n", start_x, start_y);

	////e_dist_percent = ESSAY_OFFSET + essay_row * ROW_HEIGHT;
	n_dist_percent = NAME_X_OFFSET;
	row_slope = -(1 / slope);

	// x is always going to be a positive offset, since we are moving right in the image
	// and adding values from the row
	n_offset_x = (sqrt((double)(((n_dist_percent * length)*(n_dist_percent * length)) /
					(1 + row_slope * row_slope))));
	// y, will be positive if the row_slope is positive as the line is rotated like
	// this \  while it will be negative if the row_slope is negative like this /
	// remember that normally / is a positive slope, but reversed with the image coord system
	//##################
	//##################
	//##################
	// CHANGED IT SO n_offset_x is positive, so then for n_offset_y, trying
	// flipping it's sign accordingly, since x's sign is flipped
	n_offset_y = (-n_offset_x * row_slope);
	// this is the pixel coordinate for the first bubble in the name row
	x[0] = start_x + n_offset_x;
	y[0] = start_y + n_offset_y;


	if(debugname)printf("First column x,y coords: %f, %f\n", x[0], y[0]);

	int loop = 1;
	for(loop = 1; loop < 20; loop++)
	{
		n_dist_percent += NAME_BBL_OFFSET;
		n_offset_x = (sqrt((double)(((n_dist_percent * length)*(n_dist_percent * length)) /
						(1 + row_slope * row_slope))));
		n_offset_y = (-n_offset_x * row_slope);
		x[loop] = start_x + n_offset_x;
		y[loop] = start_y + n_offset_y;
	}

	for(loop = 0; loop < 20; loop++)
	{
		
		if(gradeBubble(x[loop], y[loop]))
		{
			name[loop] = name_row + 65 ;
			if(debugname) printf("loopnum: %d,   ", loop);
		}
	}
}
