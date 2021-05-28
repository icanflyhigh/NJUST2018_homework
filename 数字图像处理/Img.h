#pragma once
#include <iostream>
#include <Windows.h>
using namespace std;
class Img
{
public:
	Img();
	Img(int width, int heigth, int channel = 1);
	Img(BYTE* p, int w, int h, int channel = 1);
	Img(Img && a);
	Img(Img & a);
	~Img();
protected:
	int width, height, channel;
	BYTE* pImg=nullptr;
	void T();

};

