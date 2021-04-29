// author: M@
#include "lex_generater.h"
#include<iostream>
#include<cstdio>
#include <cstdlib>
using namespace std;

void lex_generater::outChar(const char *s1, const char* s2)
{
	for (int i = 0; i <= 25; i++)
	{
		if (strlen(s2))
		{
			fprintf(f, "<%s>->%c<%s>\n", s1, 'a' + i, s2);
			fprintf(f, "<%s>->%c<%s>\n", s1, 'A' + i, s2);
		}
		else
		{
			fprintf(f, "<%s>->%c\n", s1, 'a' + i);
			fprintf(f, "<%s>->%c\n", s1, 'A' + i);
		}

	}
}

void lex_generater::outNum(const char *s1, const char* s2, int a)
{
	for (int i = a; i <= 9; i++)
	{
		if (strlen(s2))
		{
			fprintf(f, "<%s>->%c<%s>\n", s1, '0' + i, s2);
		}
		else
		{
			fprintf(f, "<%s>->%c\n", s1, '0' + i);
		}

	}
}

void lex_generater::out(const char * s1, const char * s2, const char * s3)
{
	if (strlen(s3))
	{
		fprintf(f, "<%s>->%s<%s>\n", s1, s2, s3);
	}
	else
	{
		fprintf(f, "<%s>->%s\n", s1, s2);
	}


}

void lex_generater::E(const char * str)// 标识符
{
	out(str, "_");// B->_
	outChar(str, "");// B->(a-z A-Z)
	outChar(str, "B1");// B->(a-z A-Z)B1
	out(str, "_", "B1"); // B->_B1
	outChar("B1", "");// B1->(a-z A-Z)
	out("B1", "_");// B1->_
	outNum("B1", "");// B1->(0-9)
	out("B1", "_", "B1");// B1->_B1
	outChar("B1", "B1");// B1->(a-z A-Z)B1
	outNum("B1", "B1");// B1->(0-9)B1
}

void lex_generater::Z(const char * s)// 整数
{
	outNum(s, ""); // s->(0-9)
	out(s, "+", "Z3");// s->+Z3
	out(s, "-", "Z3");// s->-Z3
	outNum("Z3", "Z", 1);// s->(1-9)Z
	outNum("Z3", "");// s->(1-9)Z
	outNum(s, "Z", 1);// s->(1-9)Z
	outNum("Z", ""); // Z->(0-9) 
	outNum("Z", "Z");// Z->(0-9)Z
	out("Z", "e", "Z1");// Z->eZ1
	out("Z", "E", "Z1");// Z->EZ1
	out("Z1", "+", "Z2");// Z1->+Z2
	out("Z1", "-", "Z2");// Z1->-Z2
	outNum("Z1", "Z2", 1);
	outNum("Z1", ""); // Z1 -> (0-9)
	outNum("Z2", "Z2");
	outNum("Z2", "");

	out("Z", "i");
	out("Z", "I");

}

void lex_generater::X(const char * s)// 小数
{

	//out(s, "+", "X1"); // s->+X1
	//out(s, "-", "X1"); // s->-X1
	outNum(s, "X2", 1);// s->(1-9)X2
	outNum("X2", "X2");// X2->(0-9)X2
	out(s, "0", "X3");// s->0X3
	out("X3", ".", "X4");// X3->.X4
	out("X2", ".", "X4");// X2->.X4
	out(s, ".", "X4"); // s->.X4
	outNum(s, "X5", 1); // s->(1-9)X5
	outNum("X5", "X5"); // X5->(0-9)X5
	out("X5", ".", "X4"); // X5->.X4
	out(s, ".", "X4"); // s->.X4
	out(s, "0", "X6"); // s->0X6
	out("X6", ".", "X4"); // X6->.X4
	outNum("X4", "X4"); // X4 ->(0-9)X4
	outNum("X4", ""); // X4 ->(0-9)
	out("X4", "I"); // X4 ->I
	out("X4", "i"); // X4 ->i
	out("X4", "e", "Z1");// X4->eZ1
	out("X4", "E", "Z1");// X4->EZ1
}

void lex_generater::COM(const char * s) {

}

void lex_generater::F(const char * str)
{
	Z(str);
	X(str);
	COM(str);
}

void lex_generater::gen() {
	E("标识符");
	F("常量");
	puts("Generate Lex Done!");
}

lex_generater::lex_generater(const char * s) {
	int err =  fopen_s( &f, s, "w");
	if (err == 0) {
		printf("The file '%s' was opened\n", s);
	}
	else {
		printf("The file '%s' was not opened\n", s);
	}
}

lex_generater::~lex_generater() {
	fclose(f);
}






