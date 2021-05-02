#pragma once
// author: M@
// #define _DEBUG_

#include<iostream>
#include<cstdlib>
#include<fstream>
#include<istream>
#include<sstream>
#include<cstdio>
#include<cstdlib>
#include<string.h>
#include<queue>
#include<vector>
#include<map>
#include<stack>
#include<list>
#include<set>
#include<string>
#include <utility>
#include "Type.h"
#include "AST_node.h"
#include "symbol.h"

using namespace std;
typedef pair<string, node_type> s2n;
/*
���������
TODO: 1.�����﷨����֮���AST 2.����optimizer��sematicparser
*/
const set<string> qualifier = { "const", "static" };
const vector<string> atom_op = { "print", "+", "-", "*", "/", ":=" };
const vector<string> property = { "val", "type" };
const map<string, node_type>  node_type_map = {
	{{"nop"}, node_nop},
	{{"assign"}, node_assign},
	{{"add"}, node_add},
	{{"sub"}, node_sub},
	{{"mul"}, node_mul},
	{{"div"}, node_div},
	{{"if"}, node_if},
	{{"while"}, node_while},
	{{"func"}, node_func},
	{{"greater"}, node_greater},
	{{"declare"}, node_declare,},
	{{"call"}, node_call},
	{{"return"}, node_return},
	{{"and"}, node_and},
};




class sematic_parser
{
public:
	sematic_parser(string syntax_path, string code_path);

	//struct operand {
	//	int v, property; //vָ�����ʽ�ĵڼ���V -1 ��ʾ��ʽ
	//	operand(int a = 0, int b = 0) : v(a), property(b) {
	//	}
	//};

	struct form
	{
		int left;// ����ʽ��
		vector<int> right;// ����ʽ�Ҳ�
		vector<node_type> ops;// node�������ͺ�
		vector<int> var;// ͨ��λ��ָʾ��Ӧ��Ԫ���ڱ��ʽ��ʲôλ��, -1 ��ʾ��ʽ����Ȼ����ʾ��ʽ
		form(int a, vector<int> b) :left(a), right(b) {}
	};

	struct item
	{
		int f; // ����ʽ
		int p = 0;// ����ʽ��״̬
		int fs; // ǰ��������
		item(int form_idx, int pos, int fs_idx) :f(form_idx), p(pos), fs(fs_idx)
		{}
		bool operator==(const item & a)const
		{
			return f == a.f && p == a.p && fs == a.fs;
		}
		bool operator<(const item & a)const
		{
			if (f != a.f)
			{
				return f < a.f;
			}
			else if (p != a.p)
			{
				return p < a.p;
			}
			return fs < a.fs;
		}
	};


	// ������
	ifstream f1;
	ifstream f2;
	vector<string> V_list;// ��¼V���б�
	vector<bool> is_VT;// �Ƿ�Ϊ�ս��
	set<int> to_epsilon;// �����epsilon�ļ���
	vector<form> form_list; // ����ʽ�б�
	map<int, vector<int>> form_map;// ÿһ����Ϊidx��form
	string start_s = "<Start>";// ��ʼ����
	string end_s = "endC"; // ��������
	vector<vector<item>> item_clan; // ��Ŀ����
	vector<map<int, int>> go; // ת�� ��һ��int��ʾ���ţ��ڶ�����ʾת�Ƶ���״̬
	vector<map<int, int>> reverse;// ��Լ����һ��int��ʾ���ţ��ڶ�����ʾ��Լ��״̬��
	map<int, set<int>> first_set; // first��

	vector<int> s_stack; // ״̬ջ
	vector<int> V_stack; // ����ջ
	vector<AST_node> a_stack;

	//___________new______________
	symtab symbolTable;
	AST_tree ASTTree;




	void output();

	void test();


	static node_type check_node_type(const string &);



	// ����s�Ƿ���V_LIST�У�����������룬����s��V_LSIT�еı��
	int V_idx(const string & s, bool is_push = true);

	// Ѱ�Ҳ����յ�Vn
	void find_epsilon();

	// p ָ���ַ�����ȡ���ֵĵ�һ��λ��
	inline int read_int(const string & s, int & p);

	inline int chk_dot(const string & s);


	// p ָ���ַ�����ȡ���ֵĵ�һ��λ��
	inline double read_double(const string & s, int p);

	// ��ȡ�﷨
	void read_syntax();

	set<int> union_first(const set<int> & a, const set<int> & b);

	// ���ڵ�������Σ�����
	set<int> form_first_set(int idx, set<int> & vis);

	// ��FIRST
	void gen_first();

	// ����Ŀ��
	int closure(vector<item> & vi, int fa);

	// action����goto������Ŀ����һ�����
	void generate_clan();

	// ��ȡ����
	symbol read_COSNT(const string & s);
	
	symbol read_VARIABLE(const string & s);
	

	void parse_begin();

	// id_stack������ԭ����������pop
	// ����ֻ������ʽ
	AST_node deal_expression(int id_num, int form_idx);

	// TODO����parse��1.���������� 2.�﷨�� 3.���⴦��ĳЩ����
	// �������� ���ش������ͣ����߽��� -1������� 0���� 1��ʶ���Vt 2�ܾ� 3������ 4��ʶ�����ʹ���
	int parse_Vt(string  s, string Vt, string type);

	int str2Num(const string  s);

	// TODO���������ԭ��
	int parse_code();// ��ȡTASK1�����

	int AST2TAC();// �������﷨��ת��Ϊ��Ԫʽ

	int dfs_AST(int idx); // ����AST
		
};
