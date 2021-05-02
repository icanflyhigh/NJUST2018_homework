#pragma once
#include "TAC.h"
#include "Type.h"
using namespace std;



class AST_node
{
public:
static int temp_node_cnt;
	node_type type; // �ڵ�����
	vector<int> op; // �ӽڵ��ָ��
	vector<TAC> code; // ����
	string name=""; // �ڵ������
	symData tempData;// �ݴ�����
	valueType dType;
	symbol * syb = NULL;
	int line=0;
	bool is_called = false;
	static int get_declared(AST_node& tn, valueType vt, vector<AST_node> & T, bool isleft=false);
	int __function__(vector<AST_node> & T);
	int __nop__(vector<AST_node> & T);
	int __assign__(vector<AST_node> & T);
	int __cal_op__(vector<AST_node> & T, node_type nt);
	int __declare__(vector<AST_node> & T);
	int __if__(vector<AST_node> & T);
	int __while__(vector<AST_node> & T);
	int __return__(vector<AST_node> & T);
	int __call__(vector<AST_node> & T);
	int __and__(vector<AST_node> & T);
	int zip_back(vector<AST_node> & T, int idx, string& false_tag); // �����ַ
	int call(vector<AST_node>&);
	bool type_check(vector<AST_node>&);

	
	void show_code();
};

class AST_tree
{
public:
	
	vector<AST_node> tree;
	int root=-1;
	int add_node(AST_node &); // ���ڵ��������
};

