#pragma once
#include "Type.h"
#include <vector>
#include <map>
using namespace std;
// ����װ��ʶ���ͳ���
class symtab
{
public :
	symtab();
	int global_table;// ȫ�ֵĲ�����
	// TODO �ش���� �Һ�����vector���õ�ַ������Ҿ�������
	vector<symbol_table*> local_tables;// �ֲ��Ĳ�����
	map<int, int> fa;
	int cur_table; // ��ǰ�Ĳ�����
	symbol* add_symbol(string & ,symbol &);
	void add_table(int);;
	void clear();
};

