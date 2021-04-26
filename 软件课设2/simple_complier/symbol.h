#pragma once
#include "Type.h"
#include <vector>
#include <map>
using namespace std;
// 用来装标识符和常量
class symtab
{
public :
	symtab();
	int global_table;// 全局的参数表
	// TODO 重大错误 我忽略了vector会让地址变更，我就是弱智
	vector<symbol_table*> local_tables;// 局部的参数表
	map<int, int> fa;
	int cur_table; // 当前的参数表
	symbol* add_symbol(string & ,symbol &);
	void add_table(int);;
	void clear();
};

