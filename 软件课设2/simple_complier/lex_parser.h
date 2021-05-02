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
	// ���ڱ�ʾnfa�е�״̬
	struct nstate
	{
		int idx;
		vector<int> to;
		vector<char> to_char;
		nstate(int a = 0) :idx(a) {}
	};
	// ���ڱ�ʾdfa�е�״̬
	struct dstate
	{
		set<int> ns;
		vector<int> to;
		vector<char> to_char;
		bool isEnd = false;// �Ƿ��ս�
	};
	
	// ������
	int now_state = 0; // ����ָʾ����dfa�����ĸ�״̬
	string start_s, end_s = "<End>"; // ��ʼ�Լ���ֹ���ս��
	vector<string> s_list; // ���ս���ı�
	lex_parser(string ss); // ���캯��
	// ��ȡ<>������
	void grammar_parse(char *buf); // �����﷨

	static string line_parse(char *&c); // ����һ�е��﷨

	void output(bool show_nfa = true, bool show_dfa = true); // ���lex_prser�еĲ�����״̬��debug�ã�

	bool code_parse(char c); // ��������һ���ַ�����

	void clear(); // �����Ա��ڷ���ʹ�÷�����

	bool code_parse(string str); // ����һ���ַ������Ѿ��Ϸ�����

	void nfa2dfa(); // nfaתdfa

private:
	int nfa_cnt = 0; // nfa�ĸ���
	set<char> VT{ '\0' }; // �ս������
	map <string, nstate>nfa; // nfa
	vector<dstate> dfa; // dfa

	int e_closure(dstate d); // ��հ�

	void move_dfa(int idx, char to_char); // ��move

	int is_in_dfa(const dstate & d); // �ж�״̬�Ƿ���dfa��
};

class All_lex_parser {
public:
	All_lex_parser(string, string); // ���캯��

	~All_lex_parser(); // ��������

	void parse_all_grammar(); // ����������ķ�

	int check_code(string file_path); // ���������code

private:
	ifstream lexf; // �ķ�������
	FILE * tokenf; // token�����
	vector<lex_parser> tv; // �ʷ�����������

	int judge_token(string sample); // �ж��ַ����Ƿ��ǽ�������������ؼ��֣��޶�����//
};