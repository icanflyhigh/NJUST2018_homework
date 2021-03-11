// author: M@
# define m_
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker, "/STACK:102400000,102400000") 
#include<iostream>
#include<fstream>
#include<istream>
#include<cstdio>
#include<cstdlib>
#include<string.h>
#include<queue>
#include<vector>
#include<map>
#include<algorithm>
#include<cmath>
#include<list>
#include<set>
#include<time.h>
#include<string>
#define FOR(i,a,b) for(int i=a;i<=b;i++)
#define Swap(a,b) (a=a^b,b=b^a,a=a^b)
#define clr(a)   memset((a),0,sizeof (a))
#define li idx<<1
#define ri idx<<1|1

using namespace std;
typedef unsigned int ui;
typedef long long LL;
typedef pair<int,int> pii;
const int INF=0x3f3f3f3f;
const LL LINF=1e18;
const double DINF=1e9;
const double EPS=1e-9;
int dir[4][2]{{1,0},{0,1},{-1,0},{0,-1}};
const int maxn=1e2+5;


const string token_names[] = {"关键词", "操作符", "限定符", "界符", "标识符", "常量"};
const set<string> keyword = {"break","case","char","continue","do","default","double","else","float","for","if",
"int","include","long","main","return","switch","typedef","void","unsigned","while"};
const set<string> op = {"+","-","*","/","!","^","=","%","&","&&","|","||","<","<=",">",">=","==","!=","++","--"};
const set<string> delimiters ={";","(",")","{","}",",","[","]","#","<",">"};
const set<string> qualifier = {"const", "static"};

void outChar(const char *s1, const char* s2)
{
	for (int i = 0; i <= 25; i++)
	{
		if(strlen(s2))
		{
			printf("<%s>->%c<%s>\n", s1, 'a' + i, s2);
			printf("<%s>->%c<%s>\n", s1, 'A' + i, s2);
		}
		else
		{
			printf("<%s>->%c\n", s1, 'a' + i);
			printf("<%s>->%c\n", s1, 'A' + i);
		}

	}
}

void outNum(const char *s1, const char* s2, int a=0)
{
	for (int i = a; i <= 9; i++)
	{
		if(strlen(s2))
		{
			printf("<%s>->%c<%s>\n", s1, '0' + i, s2);		
		}
		else
		{
			printf("<%s>->%c\n", s1, '0' + i);	
		}
		
	}
}

void out(const char * s1, const char * s2, const char * s3="")
{
	if(strlen(s3))
	{
		printf("<%s>->%s<%s>\n", s1, s2, s3);
	}
	else
	{
		printf("<%s>->%s\n", s1, s2);
	}
	

}

void E(const char * str)
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

void Z(const char * s)
{
	outNum(s, ""); // s->(0-9)
	out(s, "+", "Z");// s->+Z
	out(s, "-", "Z");// s->-Z
	outNum(s, "Z", 1);// s->(1-9)Z
	outNum("Z", ""); // Z->(0-9)
	outNum("Z", "Z");// Z->(0-9)Z
	// TODO增加对于5e-1的判断，非常不正规
	out("Z", "e", "Z1");// Z->eZ1
	out("Z", "E", "Z1");// Z->EZ1
	out("Z1", "+", "Z2");// Z1->+Z2
	out("Z1", "-", "Z2");// Z1->-Z2
	outNum("Z1", "Z2", 1);
	out("Z1", "0");
	outNum("Z2", "Z2");
	outNum("Z2", "");

}

void X(const char * s)
{
	
	out(s, "+", "X1"); // s->+X1
	out(s, "-","X1"); // s->-X1
	out(s, ".", "X2");// s->.X2
	outNum(s, "X1", 1);// s->(1-9)X1
	outNum(s, "XX1", 1);// s->0XX1
	out("XX1", ".", "X2");// XX1->.X2
	outNum("X1", "X1");// X1->(0-9)X1
	out("X1", ".", "X2");// X1 -> .X2
	outNum("X2", "");// X2->0-9
	outNum("X2", "X2");// X2->(0-9)X2
	out("X2", "E", "X3");// X2->EX3
	out("X2", "e", "X3");// X2->eX3
	out("X3", "+", "X4");// X3->+X4
	out("X3", "-", "X4");// X3->-X4
	outNum("X3", "X4", 1);// X3->(1-9)X4
	out("X3", "0");// X3->0
	outNum("X4", "X4");// X4->(0-9)X4
	outNum("X4", "");// X4->(0-9)
}

void A(const char * s)
{
	for(auto k: keyword)
	{
		out(s, k.c_str());
	}
}

void B(const char * s)
{
	for(auto k: op)
	{
		out(s, k.c_str());
	}
}

void C(const char * s)
{
	for(auto k: qualifier)
	{
		out(s, k.c_str());
	}
}

void D(const char * s)
{
	for(auto k: delimiters)
	{
		out(s, k.c_str());
	}
}


void F(const char * str)
{
	Z(str);
	X(str);
}


int main()
{

#ifdef m_
	freopen("input.txt","r",stdin);
	freopen("rjks/task1/token_grammar.txt","w",stdout);
#endif
	// S();
	// A("关键词");
	// B("操作符");
	// C("限定符");
	// D("界符");
	E("标识符");
	F("常量");
	// X("科学计数法");
	// Z("复数");
#ifdef m_
	fclose(stdin);
	fclose(stdout);
#endif
	return 0;
}


