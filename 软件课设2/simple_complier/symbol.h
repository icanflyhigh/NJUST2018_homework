#pragma once
#include "Type.h"
using namespace std;
// ����װ��ʶ���ͳ���
class symtab
{
public :
	symtab();
	int global_table;// ȫ�ֵĲ�����
	vector<symbol_table*> local_tables;// �ֲ��Ĳ�����
	map<int, int> fa;
	int cur_table, prev_table; // ��ǰ�Ĳ�����
	symbol* add_symbol(string & ,symbol &, int dom=-1);
	void add_table(int);;
	void clear();
};

