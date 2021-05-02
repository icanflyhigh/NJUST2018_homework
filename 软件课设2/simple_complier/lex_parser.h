#pragma once
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
#include<set>
#include<string>
using namespace std;


class lex_parser
{

public:
	// 用于表示nfa中的状态
	struct nstate
	{
		int idx;
		vector<int> to;
		vector<char> to_char;
		nstate(int a = 0) :idx(a) {}
	};
	// 用于表示dfa中的状态
	struct dstate
	{
		set<int> ns;
		vector<int> to;
		vector<char> to_char;
		bool isEnd = false;// 是否终结
	};
	
	// 参数区
	int now_state = 0; // 用于指示现在dfa处于哪个状态
	string start_s, end_s = "<End>"; // 开始以及终止的终结符
	vector<string> s_list; // 非终结符的表
	lex_parser(string ss); // 构造函数
	// 读取<>中内容
	void grammar_parse(char *buf); // 处理语法

	static string line_parse(char *&c); // 处理一行的语法

	void output(bool show_nfa = true, bool show_dfa = true); // 输出lex_prser中的参数的状态（debug用）

	bool code_parse(char c); // 分析处理一个字符代码

	void clear(); // 清理，以便于反复使用分析器

	bool code_parse(string str); // 处理一个字符串（已经废废弃）

	void nfa2dfa(); // nfa转dfa

private:
	int nfa_cnt = 0; // nfa的个数
	set<char> VT{ '\0' }; // 终结符集合
	map <string, nstate>nfa; // nfa
	vector<dstate> dfa; // dfa

	int e_closure(dstate d); // 求闭包

	void move_dfa(int idx, char to_char); // 求move

	int is_in_dfa(const dstate & d); // 判断状态是否在dfa中
};

class All_lex_parser {
public:
	All_lex_parser(string, string); // 构造函数

	~All_lex_parser(); // 析构函数

	void parse_all_grammar(); // 处理输入的文法

	int check_code(string file_path); // 处理输入的code

private:
	ifstream lexf; // 文法输入流
	FILE * tokenf; // token输出流
	vector<lex_parser> tv; // 词法分析器集合

	int judge_token(string sample); // 判断字符串是否是界符，操作符，关键字，限定符，//
};