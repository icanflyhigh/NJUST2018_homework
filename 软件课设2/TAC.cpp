#include "TAC.h"
#include <iostream>
#include <cstdio>
#include "Type.h"

using namespace std;

TAC::TAC(func f):type(f)
{
}
map <func, string> func2str
{
	{ADD, "ADD"},
	{SUB, "SUB"},
	{MUL, "MUL"},
	{DIV, "DIV"},
	{GREATER, "GREATER"},
	{BEQ, "BEQ"},
	{BNE, "BNE"},
	{GOTO, "GOTO"},
	{PRINT, "PRINT"},
	{ASSIGN, "ASSIGN"},
	{TAG, "TAG"},
	{CALL, "CALLL"},
	{RETURN, "RETURN"},
};
void TAC::show()
{
	if (type == TAG)
	{
		printf("%s\n", tag.c_str());
	}
	else
	{
		string output = func2str[type];
		//printf("%d  %s", type, func2str[type].c_str());
		for (auto & s : op)output += "\t" + s;
		printf("%s\n", output.c_str());
	}
}
