#pragma once
#include <string>
#include <vector>
#include "Type.h"

using namespace std;

class TAC
{
public:
	static int tag_cnt;
	TAC(func=ADD);
	func type;
	vector<string> op;
	string tag = "";
	void show();
};

