#pragma once
#include "Type.h"
using namespace std;
// 用来装标识符和常量
class symtab
{
public :
	symtab();
	int global_table;// 全局的参数表
	vector<symbol_table*> local_tables;// 局部的参数表
	map<int, int> fa;
	int cur_table, prev_table; // 当前的参数表
	symbol* add_symbol(string & ,symbol &, int dom=-1);
	void add_table(int);;
	void clear();
};

