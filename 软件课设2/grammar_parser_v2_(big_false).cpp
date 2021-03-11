// author: M@
# define m_
#define _CRT_SECURE_NO_WARNINGS
#define _DEBUG_
#pragma comment(linker, "/STACK:102400000,102400000") 
#include<iostream>
#include<stdlib.h>
#include<fstream>
#include<istream>
#include<cstdio>
#include<cstdlib>
#include<string.h>
#include<queue>
#include<vector>
#include<map>
#include<algorithm>
#include<cmath>
#include<list>
#include<set>
#include<time.h>
#include<string>
#include<assert.h>
#define FOR(i,a,b) for(int i=a;i<=b;i++)
#define Swap(a,b) (a=a^b,b=b^a,a=a^b)
#define clr(a)   memset((a),0,sizeof (a))
#define li idx<<1
#define ri idx<<1|1

using namespace std;
typedef unsigned int ui;
typedef long long LL;
typedef pair<int,int> pii;
const int INF=0x3f3f3f3f;
const LL LINF=1e18;
const double DINF=1e9;
const double EPS=1e-9;
int dir[4][2]{{1,0},{0,1},{-1,0},{0,-1}};
const int maxn=1e2+5;

/*
词法分析器
TODO: 
4.读取输入
1.建立token_grammar
2. 处理token_grammar
3.dfa
*/
const string token_names[] = {"关键词", "操作符", "限定符", "界符", "标识符", "常量"};
const set<string> keyword = {"break","case","char","continue","do","default","double","else","float","for","if",
"int","include","long","main","return","switch","typedef","void","unsigned","while"};
const set<string> op = {"+","-","*","/","!","^","=","%","&","&&","|","||","<","<=",">",">=","==","!=","++","--"};
const set<string> delimiters ={";","(",")","{","}",",","[","]","#","<",">"};
const set<string> qualifier = {"const", "static"};




class token_parser
{

	public :
	token_parser(string ss)
	{
		nfa_cnt = 0;
		start_s = ss;
		nfa[start_s] = nstate(nfa_cnt++);
		nfa[end_s] = nstate(nfa_cnt++);
		s_list.push_back(start_s);
		s_list.push_back(end_s);
	}

	struct nstate
	{
		int idx;
		vector<int> to;
		vector<string> to_string;
		// string name;
		// nstate(string a):name(a)
		// {
		// }
		// nstate(const char * a):name(a)
		// {
		// }
		nstate(int a=0):idx(a)
		{
			
		}
	};

	struct dstate
	{
		set<int> ns;
		vector<int> to;
		vector<string> to_string;
		bool isEnd = false;// 是否终结
	};
// 参数区
	string start_s , end_s = "<End>";
	int nfa_cnt = 0;
	vector<string> s_list;
	set<string> VT{""};
	map <string, nstate>nfa;
	vector<dstate> dfa;

	// 读取<>中内容
	static string line_parse(string s)
	{
		if(s[0] == '<'&&s[s.size() -1] == '>')
		{
			return s.substr(1, s.size() - 2);
		}
		else
		{
			return "";
		}
	}
	
	void grammar_parse(char * buf)
	{
		//读取3NF，加入NFA
		int p = 0;
		string sa = "", sb = "", sc = "";
		//读取sa
		while(buf[p] != ' ')
		{
			sa += buf[p++];
		}
		p++;
		//读取sb
		while(buf[p] != 0)
		{
			sb += buf[p++];
			if(buf[p] == ' ')
			{
				p++;
				break;
			}
		}
		//读取sc
		while(buf[p] != 0)
		{
			sc += buf[p++];
		}
		bool flag = false;
		//判断sa有没有
		sa = line_parse(sa);
		if(nfa.find(sa) == nfa.end())
		{
			s_list.push_back(sa);
			nfa[sa] = nstate(nfa_cnt++);
		}

		// 处理sb，sc
		if(line_parse(sc).size() > 0)
		{
			//A->aA
			sc = line_parse(sc);
		}
		else
		{
			// A->a
			if(line_parse(sb).size() == 0)
			{
				sc = end_s;
			}
			else //A->B
			{
				sc = line_parse(sb);
				sb = string("");
			}
		}
		VT.insert(sb);
		if(nfa.find (sc) == nfa.end())
		{
			s_list.push_back(sc);
			nfa[sc] = nstate(nfa_cnt++);
		}

		
		nfa[sa].to.push_back(nfa[sc].idx);
		nfa[sa].to_string.push_back(sb);
	}

	
	void output(bool show_nfa=true, bool show_dfa=true)
	{
		printf("*output_nfa*\n");
		printf("\nVT\n");
		for(auto a: VT)
		{
			cout << a << endl;
		}
		
		if(show_nfa)
		{
			printf("\nnfa\n");
			for(auto a : nfa)
			{
				printf("%s  %d\n",a.first.c_str() , a.second.idx);
				for(auto b:a.second.to_string)
				{
					printf("%s", b.c_str());
				}puts("");
				for(auto b : a.second.to)
				{
					printf("%d", b);
				}
				printf("\n--------\n");
				

			}			
		}
		
		cout << "\ns_list" << endl;
		for(auto a:s_list)
		{
			cout << a << endl;
		}
		if(show_dfa)
		{
			cout << "\ndfa! size:" << dfa.size()<<endl;
			#ifdef _DEBUG_
			printf("dfa_size: %d\n", dfa.size());
			#endif
			int idx = 0;
			for(auto a:dfa)
			{
				printf("dfa-%d :\n", idx++);
				printf("isEnd %d\n", a.isEnd);
				for(auto i: a.ns)
				{
					printf("%s ", s_list[i].c_str());
				}
				puts("");
				int len = a.to.size();
				for (int i = 0; i < len; i++)
				{
					printf("%s %d\n", a.to_string[i].c_str(), a.to[i]);
				}
				cout << "--------\n";
			}
		}
		cout << endl;
	}

	int is_in_dfa(const dstate & d)
	{
		//判断d 是否已经在dfa中
		for (int i = 0; i < dfa.size(); i++)
		{
			if(dfa[i].ns.size() != d.ns.size())
			{
				continue;
			}
			bool match_flag = true;
			for(auto b:dfa[i].ns)
			{
				if(d.ns.find(b) == d.ns.end())
				{
					match_flag = false;
					break;
				}
			}
			if(match_flag == true)
			{
				return i;
			}
		}
		return dfa.size();
	}

	int e_closure(dstate d)
	{
		// dstate ret = dstate(d);
		for(auto i:d.ns)
		{
			int len = nfa[s_list[i]].to_string.size();
			for (int j = 0; j < len; j++)
			{
				// j 为epsilon, 将对应nfa状态编号加入ns
				if(nfa[s_list[i]].to_string[j].size() ==0)
				{
					d.ns.insert(nfa[s_list[i]].to[j]);
					#ifdef _DEBUG_
					// cout << " nfa[s_list]:";
					// cout << nfa[s_list[i]].to[j] << endl;
					#endif
				}
			}
		}
		// 计算完闭包在dfa中的位置
		int temp = is_in_dfa(d);
		if(temp == dfa.size())
		{
			for(auto i : d.ns)
			{
				if(i == 1)
				{
					d.isEnd = true;
				}
			}
			dfa.push_back(d);
		#ifdef _DEBUG_
			// printf("dfa_push_back: ");
			// for(auto a : d.ns)
			// {
			// 	printf("%d ", a);
			// }
			// puts("");
		#endif
		}
		
		#ifdef _DEBUG_
		// cout << "dfa in \n";
		// for(auto a:ret.ns)
		// 	cout << "ret.ns " << a << endl;
		// output();
		#endif
		return temp;
	}

	void move_dfa(int idx, string to_string)
	{
		dstate ret;
		for(auto i:dfa[idx].ns)
		{
			int len = nfa[s_list[i]].to_string.size();
			for (int j = 0; j < len; j++)
			{
				// j 为epsilon, 将对应nfa状态编号加入ns
				if(nfa[s_list[i]].to_string[j]==to_string)
				{
					ret.ns.insert(nfa[s_list[i]].to[j]);
				#ifdef _DEBUG_
					// cout << " nfa[s_list]:";
					// cout << nfa[s_list[i]].to[j] << endl;
				#endif
				}
			}
		}
		if(ret.ns.size() ==0)
		{
			return;
		}
		int temp = e_closure(ret);
		dfa[idx].to.push_back(temp);
		dfa[idx].to_string.push_back(to_string);	
	}

	void nfa2dfa()
	{
		dstate d0;
		d0.ns.insert(0);
		int p = 0;
		e_closure(d0);
		while(p < dfa.size())
		{
			for(auto a:VT)
			{
				if(a.size() != 0)
				{
					move_dfa(p, a);
				}
			}
			p++;
		}
	}

	// bool code_parse(string str)
	// {
	// 	int p = 0, s = 0;
	// 	while(str[p] != 0)
	// 	{
	// 		int i;
	// 		int ps = s;
	// 		// output();
	// 		for (i = 0; i < dfa[s].to_string.size(); i++)
	// 		{
	// 			if(dfa[s].to_string[i] == str[p])
	// 			{
	// 				s = dfa[s].to[i];
	// 				break;
	// 			}
	// 		}
	// 		p++;
	// 		if(i == dfa[ps].to_string.size() )
	// 		{
	// 			return false;
	// 		}
	// 		if(str[p] == 0)
	// 		{
	// 			if(dfa[s].isEnd == true)
	// 			{
	// 				return true;
	// 			}
	// 			else
	// 			{
	// 				return false;
	// 			}
	// 		}
	// 	}
	// 	return true;
	// }



};

vector<token_parser> tv;

void parse_all_grammar()
{
	char buf[256];
	while(cin.getline(buf, 255))
	{
		char tc[256];
		sscanf(buf, "%s", tc);
		
		string ts = token_parser::line_parse(string(tc));
		bool flag = false;
		int i;
		for (i = 0; i < tv.size(); i++)
		{
			
			for (int j = 0; j < tv[i].s_list.size(); j++)
			{
				if(tv[i].s_list[j] == ts)
				{
					flag = true;
					break;
				}
			}
			if(flag == true)
			{
				break;
			}
		}
		if(flag == false)
		{
			tv.push_back(token_parser(ts));
		}
		
		
		tv[i].grammar_parse(buf);
	}
	for (int i = 0; i < tv.size(); i++)
	{
		tv[i].nfa2dfa();
		#ifdef _DEBUG_
		tv[i].output(true, true);
		#endif
		
	}
}

int graddy_judge(string sample)
{
	int token_type = -1;
	if(keyword.find(sample) != keyword.end())
	{
		token_type = 0;
	}
	else if(op.find(sample) != op.end())
	{
		token_type = 1;
	}
	else if(delimiters.find(sample) != delimiters.end())
	{
		token_type = 2;
	}
	else if(qualifier.find(sample) != qualifier.end())
	{
		token_type = 3;
	}
	return token_type;
}

// string judge_token(string sample)
// {
// 	string token_type = "错误输入";
// 	int idx = graddy_judge(sample);
// 	if(idx == -1)
// 	{
// 		for(auto t:tv)
// 		{
// 			if(t.code_parse(sample) == true)
// 			{
// 				token_type = t.start_s;
// 				break;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		token_type = token_names[idx];
// 	}
// 	return token_type;
// }

// void check_code(string file_path)
// {
// 	ifstream f1;
// 	f1.open(file_path, std::ios::in);
// 	string buf;
// 	int line_idx = 0;
// 	while(getline(f1, buf))
// 	{
// 		line_idx++;
// 		int p1 = 0, p2 = 0;
		
// 		while(buf[p2] != 0)
// 		{
			
// 			while(buf[p2] != ' '&&buf[p2] != 0)
// 			{
// 				p2++;
// 			}
// 			string sample = buf.substr(p1, p2 - p1);
// 			printf("%d %s %s\n", line_idx, sample.c_str(), judge_token(sample).c_str());
			
// 			while(buf[p2] == ' ')
// 			{
// 				p2++;
// 			}
// 			p1 = p2;
// 		}
// 	}

// }
int main()
{

#ifdef m_
	// freopen("output.txt","w",stdout);
	// freopen("rjks/task1/token_grammar.txt", "r", stdin);
	freopen("input.txt","r",stdin);
	freopen("output.txt","w",stdout);
#endif
	// token_parser t("input.txt", "rjks/task1/code.txt");

	parse_all_grammar();
	// check_code("rjks/task1/code.txt");
	//TODO 将vt变为string类型
	//TODO 三型文法中文表示

#ifdef m_
	fclose(stdin);
	fclose(stdout);
#endif
	return 0;
}


