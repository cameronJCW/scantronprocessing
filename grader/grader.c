#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <MagickCore/MagickCore.h>

void readImg(const char *name);
void writeImg(Image *out, const char *name);
void rotateAndCrop();

int readBubble(CacheView *cache, int x, int y);
void processImg(Image *img, int maxQ, FILE *file);
char **getAnswers(Image *img, CacheView *cache, int maxQ, int baseX, int baseY);
char **getMetaInfo(Image *img, CacheView *cache);
char *getVerticalItem(Image *img, CacheView *cache, int entries, int baseX, int baseY, int form);
void drawOnAnswers(Image *img, CacheView *cache, int x, int y);
void manualCrop(Image *img);

ExceptionInfo *exception;
ImageInfo *imageInfo;
Image *img;
short DEBUG;

//arg1: path to scantron img, arg2: T/F debug flag
int main(int argc, char **argv) {
	char imgPath[1000];
	if(argc < 2) {
		//error
		fprintf(stderr, "To few arguments:\n");
		exit(1);
	} else {
		strncpy(imgPath, argv[1], 1000);
	}
	DEBUG = argc >= 2 && argv[2][0] == 'T';

	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	MagickCoreGenesis(cwd, (MagickBooleanType) 1);
	readImg(imgPath);
	rotateAndCrop();
	//FILE *out = fopen("../TestOut/out.txt", "w+");
	//processImg(img, 200, out);
	MagickCoreTerminus();
}

void getRGB(Quantum *pixel, float rgb[3]) {
	for(int i=0; i<3; i++) {
		if(QuantumRange != 65535) {
			printf("ERROR\n");
		}
		rgb[i] = roundf((pixel[i]/QuantumRange) * 255);
	}
}

//based on scntrn.c:gradeBubble()
int readBubble(CacheView *cache, int x, int y) {
	int pencilthreshold = 150;
	int numbermarked = 0;

	int diameter = 28;
	int pixelCount = diameter * diameter;
	//printf("starting x,y: %d, %d\n", x, y);

	Quantum *p = GetCacheViewAuthenticPixels(cache, x, y, diameter, diameter, exception);
	for(int i = 0; i < pixelCount; i++) {
		float rgb[3];
		getRGB(&p[3*i], rgb);
		if(rgb[0] < pencilthreshold && rgb[1] < pencilthreshold && rgb[2] < pencilthreshold) {
			int r = (int) i/diameter;
			int c = i%diameter;
			//fprintf(stderr, "%d,%d:\t%.2f\t%.2f\t%.2f\n", c, r, rgb[3*i], rgb[3*i+1], rgb[3*i+2]);
			numbermarked++;
		}
	}
	//printf("%.2f\n%d\n", pixelCount * 0.45, numbermarked);
	if(numbermarked > 150) {
		return 1;
	}
	return 0;
}

void readImg(const char *name) {
	exception = AcquireExceptionInfo();
	imageInfo = CloneImageInfo((ImageInfo *) NULL);
	strcpy(imageInfo->filename, name);
	img = ReadImage(imageInfo, exception);
	if(!img) {
		CatchException(exception);
		printf("Error\n");
		exit(1);
	}
}

void writeImg(Image *out, const char *name) {
	//imageInfo = CloneImageInfo((ImageInfo *) NULL);
	FILE *outFile = fopen(name, "w");
	SetImageInfoFile(imageInfo, outFile);
	//strcpy(imageInfo->filename, "ScanTron-1-features.png");
	ClearMagickException(exception);
	WriteImage(imageInfo, out, exception);
	CatchException(exception);
}

void rotateAndCrop() {
	DefineImageProperty(img, "deskew:auto-crop", exception);
	//img = DeskewImage(img, 1, exception);
	//img = ScaleImage(img, 1200, 2200, exception);
	manualCrop(img);
	if(DEBUG) {
		writeImg(img, "../TestOut/fixed.jpg");
	}
}

void manualCrop(Image *img) {
	int maxY = img->rows;
	int maxX = img->columns;
	CacheView *cache = AcquireAuthenticCacheView(img, exception);
	//if true need manual cropping
	//fabs((double)maxY/(double)maxX - 11.0/6.0) >= 0.001
	if(1) {
		//identify black sliver in top right
		int x = maxX-1;
		int y = 1;
		//int limit = round(3*maxX / 4);
		int state = 0;
		float rgb[3];
		int pencilthreshold = 150;		//TODO make this a global constant
		int upperLeft[2] = {-1, -1};
		int bottomRight[2] = {-1, -1};
		int boxCount = 0;
		int inBox = 0;
		int offset = -1;
		int pagewidth = -1;
		//276 323
		//Quantum *p = GetCacheViewAuthenticPixels(cache, x, y, 1, 1, exception);
		//getRGB(p, rgb);
		//printf("%d,%d: %f|%f|%f\n", x, y, rgb[0], rgb[1], rgb[2]);
		//return;
		while(x >= 0 && x < maxX && y >= 0 && y < maxY && state != 5) {	//x > limit
			//printf("State: %d x: %d y: %d\n", state, x, y);
			Quantum *p = GetCacheViewAuthenticPixels(cache, x, y, 1, 1, exception);
			CatchException(exception);
			getRGB(p, rgb);
			if(state == 0) { //finds x coord of top-right sliver
				if(rgb[0] < pencilthreshold || rgb[1] < pencilthreshold || rgb[2] < pencilthreshold) {
					state = 1;
				} else {
					x--;
				}
			} else if(state == 1) { //finds bottom of top-right sliver
				if(rgb[0] > pencilthreshold && rgb[1] > pencilthreshold && rgb[2] > pencilthreshold) {
					state = 2;
					//printf("Sliver: %d %d\n", x, y);
					upperLeft[1] = y;
					offset = maxX - x;
					pagewidth = maxX - 2*offset;
					x = (int) round((double)pagewidth * 0.95) + offset;
				}
				y++;
			} else if(state == 2) { //finds bottom edge of bottom-right rectangle
				if(rgb[0] < pencilthreshold && rgb[1] < pencilthreshold && rgb[2] < pencilthreshold) {
					if(!inBox) {
						boxCount++;
						//printf("Boxes: %d\n", boxCount);
						inBox = 1;
					}
				} else {
					if(inBox) {
						bottomRight[1] = y-1;
						if(boxCount == 62) {
							state = 3;
							y -= 2;
						}
					}
					inBox = 0;
				}
				y++;
			} else if(state == 3) {	//finds right edge of bottom-right rectangle
				if(rgb[0] > pencilthreshold && rgb[1] > pencilthreshold && rgb[2] > pencilthreshold) {
					state = 4;
					bottomRight[0] = x;
					//offset = maxX - x;
					//pagewidth = maxX - 2*offset;
					x = (int) round((double)pagewidth * 0.04) + offset;
				} else {
					x++;
				}
			} else if(state == 4) {
				if(rgb[0] < pencilthreshold && rgb[1] < pencilthreshold && rgb[2] < pencilthreshold) {
					state = 5;
					upperLeft[0] = x;
				} else {
					x++;
				}
			}
		}
		x = upperLeft[0];
		y = upperLeft[1];
		int w = bottomRight[0] - upperLeft[0];
		int h = bottomRight[1] - upperLeft[1];
		printf("x: %d, y: %d, w: %d, h: %d\n", x, y, w, h);
	}
}

void processImg(Image *img, int maxQ, FILE *file) {
	CacheView *cache = AcquireAuthenticCacheView(img, exception);
	char **answers = getAnswers(img, cache, maxQ, 95, 446);
	char **meta = getMetaInfo(img, cache);

	if(DEBUG) {
		writeImg(img, "../TestOut/BoxedAnswers.jpg");
	}

	fprintf(file, "W%s\n", meta[0]);
	fprintf(file, "%s\n", meta[1]);
	fprintf(file, "%s\n", meta[2]);
	for(int i=0; i<maxQ; i++) {
		if(answers[i][0]) {
			fprintf(file, "%s\n", answers[i]);
		} else {
			fprintf(file, "-\n");
		}
	}
}

char **getMetaInfo(Image *img, CacheView *cache) {
	char **info = malloc(sizeof(char *) * 3);
	info[0] = getVerticalItem(img, cache, 8, 460, 80, 0);
	info[1] = getVerticalItem(img, cache, 1, 861, 80, 1);
	info[2] = getVerticalItem(img, cache, 3, 962, 80, 0);
	return info;
}

char *getVerticalItem(Image *img, CacheView *cache, int entries, int baseX, int baseY, int form) {
	char *item = malloc(sizeof(char) * entries);
	for(int i=0; i<entries; i++) {
		char *fill = malloc(sizeof(char *) * 2);
		strncpy(fill, "0\0", 2);
		int rows = 10;
		if(form) {
			rows = 4;
		}
		for(int j=0; j<rows; j++) {
			int x = baseX + round(i*33.5);
			int y = baseY + (int) round(j*33.28);
			int res = readBubble(cache, x, y);
			if(res) {
				if(form) {
					fill[0] = 'A' + j;
				} else {
					fill[0] = '0' + j;
				}
				strncpy(&item[i], fill, 10-j);
				if(DEBUG) {
					printf("Entry %d Bubble %c Filled\n", i+1, '0' + j);
				}
			}
			if(DEBUG) {
				drawOnAnswers(img, cache, x, y);
			}
		}
	}
	return item;
}

char **getAnswers(Image *img, CacheView *cache, int maxQ, int baseX, int baseY) {
	char **answers = malloc(sizeof(char *) * maxQ);
	char bubbles[5][3] = {"A \0", "B \0", "C \0", "D \0", "E \0"};
	//iterate over all questions
	for(int i=0; i<maxQ; i++) {	//which question?
		int r = i % 50;			//which row? each row is 34 pixels apart
		int c = (int) i / 50;	//which column? each column is ~266 px apart
		int answerIdx = 0;
		for(int j=0; j<5; j++) {	//which bubble? each bubble is 34 px apart
			if(j == 0) {
				answers[i] = calloc(20, sizeof(char *));
			}
			int x = baseX + round(c*266) + round(j*33.5);
			int y = baseY + (int) round(r*33.28);
			int res = readBubble(cache, x, y);
			if(res) {
				strncpy(&answers[i][answerIdx], bubbles[j], 20-answerIdx);
				answerIdx += 2;
				if(DEBUG) {
					printf("Question %d Bubble %c Filled\n", i+1, 'A' + j);
				}
			}
			if(DEBUG) {
				drawOnAnswers(img, cache, x, y);
			}
		}
	}
	return answers;
}

void drawOnAnswers(Image *img, CacheView *cache, int x, int y){
	int pencilthreshold = 150;
	int numbermarked = 0;
	int diameter = 28;
	int pixelCount = diameter * diameter;
	//printf("starting x,y: %d, %d\n", x, y);
	Quantum *p = GetCacheViewAuthenticPixels(cache, x, y, diameter, diameter, exception);
	//Color the pixels found to be an answer red
	for(int i = 0; i < pixelCount; i++) {
		float rgb[3];
		getRGB(&p[3*i], rgb);
		if(rgb[0] < pencilthreshold && rgb[1] < pencilthreshold && rgb[2] < pencilthreshold) {
			p[(3*i)] = QuantumRange;
			p[(3*i)+1] = 0;
			p[(3*i)+2] = 0;
		}
	}
	//Draw the box around the answer
	for(int i = 0; i < diameter; i++) {
		p[(3*i)] = 0;
		p[(3*i)+1] = 0;
		p[(3*i)+2] = 0;
		int leftSide = (3*diameter*i);
		p[leftSide] = 0;
		p[leftSide+1] = 0;
		p[leftSide+2] = 0;
		int rightSide = (3*diameter*i) + (3*(diameter-1));
		p[rightSide] = 0;
		p[rightSide+1] = 0;
		p[rightSide+2] = 0;
		int bottom = (3*i) + (3*diameter*(diameter-1));
		p[bottom] = 0;
		p[bottom+1] = 0;
		p[bottom+2] = 0;
	}
	SyncCacheViewAuthenticPixels(cache, exception);

}
