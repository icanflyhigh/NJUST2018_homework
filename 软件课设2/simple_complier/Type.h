#pragma once
#ifndef TYPE_H
#define TYPE_H
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <istream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <queue>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <stack>
#include <list>
#include <set>
#include <time.h>
#include <string>
#include <iterator>
#include <sstream>
#include <cassert>
#include <map>
#include <utility>
#include <cassert>
// TODO�����е�include��������

#define _DEBUG_


constexpr auto MAX_SYMBOL_NUM = 10000;        // ���symbol��
constexpr auto MAX_SYMBOL_LENGTH = 1000;      // ���symbol����

constexpr auto MAX_QUAD = 2000;

constexpr auto MAX_MIPS_TEXT_LENGTH = 6000;
constexpr auto MAX_MIPS_DATA_LENGTH = 600;
constexpr auto MAX_MIPS_CODE_LENGTH = 1000;

using namespace std;

//���ű�
enum objectType {
	CONSTANT,
	VARIABLE,
	FUNCTION,
	IF,
	WHILE,
	ERROR,
};

enum valueType {
	VOID_TYPE,
	INT_TYPE,
	DOUBLE_TYPE,
	INT_ARRAY_TYPE,
	CHAR_ARRAY_TYPE,
	STRING_TYPE,
	FUNC_TYPE,
};

union symData {
	int ival;
	double dval;
};

struct symbol {
	objectType oType;
	valueType vType;
	symData data;
	bool is_declare = false, not_in=-1;
	int domain, hold_domain; // ���ڵĲ�����, ����ӵ�еĲ�����
	int level; // ���ڲ�����Ĳ���
	int parameter;
	int length;
	int offset;
	int constValue;
};

typedef map<string, symbol> symbol_table;






//typedef map<string, string> allocationTable;


//AST
enum node_type {
	node_nop,
	node_assign,
	node_add,
	node_sub,
	node_mul,
	node_div,
	node_if,
	node_while,
	node_declare,
	node_and,
	node_greater,
	node_ge,
	node_eq,
	node_func,
	node_error,
	node_call,
	node_return,
};



//TAC
enum func
{
	ADD,
	SUB,
	MUL,
	DIV,
	GREATER,
	BEQ,
	BNE,
	GOTO,
	PRINT,
	ASSIGN,
	TAG,
	CALL,
	RETURN,
};








#endif // !TYPE_H

