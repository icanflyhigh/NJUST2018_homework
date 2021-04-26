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
	struct nstate
	{
		int idx;
		vector<int> to;
		vector<char> to_char;
		nstate(int a = 0) :idx(a) {}
	};

	struct dstate
	{
		set<int> ns;
		vector<int> to;
		vector<char> to_char;
		bool isEnd = false;// 是否终结
	};
	
	// 参数区
	int now_state = 0;
	string start_s, end_s = "<End>";
	vector<string> s_list;
	lex_parser(string ss);
	// 读取<>中内容
	void grammar_parse(char *buf);

	static string line_parse(char *&c);

	void output(bool show_nfa = true, bool show_dfa = true);

	bool code_parse(char c);

	void clear();

	bool code_parse(string str);

	void nfa2dfa();

private:
	int nfa_cnt = 0;
	set<char> VT{ '\0' };
	map <string, nstate>nfa;
	vector<dstate> dfa;

	int e_closure(dstate d);

	void move_dfa(int idx, char to_char);

	int is_in_dfa(const dstate & d);
};

class All_lex_parser {
public:
	All_lex_parser(string, string);

	~All_lex_parser();

	void parse_all_grammar();

	void check_code(string file_path);

private:
	ifstream lexf;
	FILE * tokenf;
	vector<lex_parser> tv;

	int judge_token(string sample);
};