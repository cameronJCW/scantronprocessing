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

int gradeBubble(CacheView *cache, int x, int y);
void gradeImg(Image *img, int maxQ);
void drawOnAnswers(Image *img, CacheView *cache, int x, int y);

ExceptionInfo *exception;
ImageInfo *imageInfo;
Image *img;

int main(int argc, char **argv) {
	char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    //printf("CWD: %s\n", cwd);
	MagickCoreGenesis(cwd, (MagickBooleanType) 1);
	readImg("../old/ScanTron-1.jpg");
	rotateAndCrop();
	gradeImg(img, 199);
	MagickCoreTerminus();
}

void getRGB(Quantum *pixel, float rgb[3]) {
	for(int i=0; i<3; i++) {
		rgb[i] = roundf((pixel[i]/QuantumRange) * 255);
	}
}

//based on scntrn.c:gradeBubble()
int gradeBubble(CacheView *cache, int x, int y) {
	int pencilthreshold = 100;
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
	DefineImageProperty(img, "auto-crop=true", exception);
	img = DeskewImage(img, 0, exception);
	//writeImg(img, "fixed.jpg");
}

void gradeImg(Image *img, int maxQ) {
	CacheView *cache = AcquireAuthenticCacheView(img, exception);
	int baseX = 106;
	int baseY = 422;
	//iterate over all questions
	for(int i=0; i<maxQ; i++) {	//which question?
		int r = i % 50;			//which row? each row is 34 pixels apart
		int c = (int) i / 50;	//which column? each column is ~266 px apart
		for(int j=0; j<5; j++) {	//which bubble? each bubble is 34 px apart
			int x = baseX + round(c*268) + j*34;
			int y = baseY + (int) round(r*33.15);
			int res = gradeBubble(cache, x, y);
			if(res) {
				printf("Question %d Bubble %c Filled\n", i+1, 'A' + j);
				drawOnAnswers(img, cache, x, y);
			}
		}
	}
	writeImg(img, "../old/BoxedAnswers.jpg");
}

void drawOnAnswers(Image *img, CacheView *cache, int x, int y){

	int pencilthreshold = 100;
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
