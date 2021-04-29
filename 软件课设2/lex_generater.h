#pragma once
#include <string>
#include <cstring>
#include <cstdio>
using namespace std;
class lex_generater
{
private:
	void outChar(const char *s1, const char* s2);
	void outNum(const char *s1, const char* s2, int a = 0);
	void out(const char * s1, const char * s2, const char * s3 = "");
	void E(const char * str);// ��ʶ��
	void Z(const char * s);// ����
	void X(const char * s);// С��
	void COM(const char * s);
	void F(const char * str);
	FILE * f;

public:
	lex_generater(const char *);
	~lex_generater();
	void gen();
};

