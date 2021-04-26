#pragma once
// author: M@
#include<iostream>
#include <fstream>
#include<queue>
#include<vector>
#include<map>
#include<string>
#include <set>

using namespace std;
class syntax_parser{
private:
	struct form{
		int left;// ����ʽ��
		vector<int> right;// ����ʽ�Ҳ�
		form(int a, vector<int> b) :left(a), right(b){}
	};

	struct item{
		int f; // ����ʽ
		int p = 0;// ����ʽ��״̬
		int fs; // ǰ��������
		item(int form_idx, int pos, int fs_idx) :f(form_idx), p(pos), fs(fs_idx){}
		bool operator==(const item & a)const{
			return f == a.f && p == a.p && fs == a.fs;
		}
		bool operator<(const item & a)const{
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
	vector <map<int, int>> go; // ת�� ��һ��int��ʾ���ţ��ڶ�����ʾת�Ƶ���״̬
	vector<map<int, int>> reverse;// ��Լ����һ��int��ʾ���ţ��ڶ�����ʾ��Լ��״̬��
	map<int, set<int>> first_set; // first��
	vector<int> s_stack;
	vector<int> V_stack;
	set<string> Variable;
	int acc_flag;// ��ʾ��Լ��״̬0��Լ��  1��Լ�ɹ� 2��Լʧ��

	// ����s�Ƿ���V_LIST�У�����������룬����s��V_LSIT�еı��
	int V_idx(const string & s, bool is_push = true);

	// Ѱ�Ҳ����յ�Vn
	void find_epsilon();


	set<int> union_first(const set<int> & a, const set<int> & b);

	// ����fist�������ڵ�������Σ�����
	set<int> form_first_set(int idx, set<int> & vis);

	// ��FIRST
	void gen_first();

	// ����Ŀ��
	int closure(vector<item> & vi, int fa);


	// �������� ���ش������ͣ����߽��� 0���� 1��ʶ���Vt 2�ܾ�
	int parse(vector<string> & s);

	void parse_begin();
	// �������� ���ش������ͣ����߽��� -1������� 0���� 1��ʶ���Vt 2�ܾ� 3������ 
	int parse_Vt(string  s);

	int str2Num(const string  s);


public:
	syntax_parser(string syntax_path, string code_path);

	// action����goto������Ŀ����һ�����
	void generate_clan();

	// ��ȡTASK1�����
	// TODO���������ԭ��
	void parse_code();

	void end();

	// ��ȡ�﷨
	void read_syntax();
	
	void output();
};




