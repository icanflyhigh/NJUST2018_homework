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
		int left;// 产生式左部
		vector<int> right;// 产生式右部
		form(int a, vector<int> b) :left(a), right(b){}
	};

	struct item{
		int f; // 产生式
		int p = 0;// 产生式的状态
		int fs; // 前向搜索符
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

	// 参数区
	ifstream f1;
	ifstream f2;
	vector<string> V_list;// 记录V的列表
	vector<bool> is_VT;// 是否为终结符
	set<int> to_epsilon;// 会产生epsilon的集合
	vector<form> form_list; // 产生式列表
	map<int, vector<int>> form_map;// 每一个左部为idx的form
	string start_s = "<Start>";// 开始符号
	string end_s = "endC"; // 结束符号
	vector<vector<item>> item_clan; // 项目集族
	vector <map<int, int>> go; // 转移 第一个int表示符号，第二个表示转移到的状态
	vector<map<int, int>> reverse;// 规约，第一个int表示符号，第二个表示规约的状态数
	map<int, set<int>> first_set; // first集
	vector<int> s_stack;
	vector<int> V_stack;
	set<string> Variable;
	int acc_flag;// 标示规约的状态0规约中  1规约成功 2规约失败

	// 搜索s是否在V_LIST中，若不在则加入，返回s在V_LSIT中的编号
	int V_idx(const string & s, bool is_push = true);

	// 寻找产生空的Vn
	void find_epsilon();


	set<int> union_first(const set<int> & a, const set<int> & b);

	// 产生fist集，存在迭代，高危代码段
	set<int> form_first_set(int idx, set<int> & vis);

	// 求FIRST
	void gen_first();

	// 求项目集
	int closure(vector<item> & vi, int fa);


	// 分析序列 返回错误类型，或者接受 0接受 1不识别的Vt 2拒绝
	int parse(vector<string> & s);

	void parse_begin();
	// 分析序列 返回错误类型，或者接受 -1程序错误 0接受 1不识别的Vt 2拒绝 3处理中 
	int parse_Vt(string  s);

	int str2Num(const string  s);


public:
	syntax_parser(string syntax_path, string code_path);

	// action——goto表与项目集族一起产生
	void generate_clan();

	// 读取TASK1的输出
	// TODO给出错误的原因
	void parse_code();

	void end();

	// 读取语法
	void read_syntax();
	
	void output();
};




