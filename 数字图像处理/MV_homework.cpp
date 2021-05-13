// author: M@
#include "bmpConverter.h"
#include <iostream>
using namespace std;

int main()
{
	bmpConverter bmpCvt = bmpConverter("./pic/H0101Gry.bmp");
	bmpCvt.Img2Bmp("./pic/H0101Gry_pesudo.bmp", 8, 'k');
	bmpCvt.InvertImg();
	bmpCvt.Img2Bmp("./pic/H0101Gry_invert.bmp",8);
	bmpCvt.Img2Bmp("./pic/H0101Gry_invert_pesudo.bmp", 8, 'k');
	bmpCvt.BmpFile2Img("./pic/H0102Rgb.bmp");
	bmpCvt.InvertImg();
	bmpCvt.Img2Bmp("./pic/H0102Rgb_invert.bmp", 0);



	return 0;
}

