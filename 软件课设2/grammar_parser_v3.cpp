// author: M@
# define m_
#define _CRT_SECURE_NO_WARNINGS
// #define _DEBUG_
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

vector<string> token_names = {"关键词", "操作符", "限定符", "界符"};
const set<string> keyword = {"break","case","char","continue","do","default","double","else","float","for","if",
"int","include","long","main","return","switch","typedef","void","unsigned","while", "using", "namespace"};
const set<string> op = {"+","-","*","/","!","^","=","%","&","&&","|","||","<","<=",">",">=","==","!=","++","--"};
const set<string> delimiters ={";","(",")","{","}",",","[","]","#", "\"", "\'"};
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
		now_state = 0;
	}

	struct nstate
	{
		int idx;
		vector<int> to;
		vector<char> to_char;
		nstate(int a=0):idx(a)
		{
			
		}
	};

	struct dstate
	{
		set<int> ns;
		vector<int> to;
		vector<char> to_char;
		bool isEnd = false;// 是否终结
	};
// 参数区
	string start_s , end_s = "<End>";
	int nfa_cnt = 0;
	vector<string> s_list;
	set<char> VT{'\0'};
	map <string, nstate>nfa;
	vector<dstate> dfa;
	int now_state = 0;
	// 读取<>中内容
	static string line_parse(char * &c)
	{
		string ret = "";
		bool flag = false;
		while(*c)
		{
			if(*c == '<')
			{
				flag = true;
				c++;
				continue;
			}
			else if(*c == '>')
			{
				if(flag)
				{
					return ret;
				}
				
			}
			if(flag)
			{
				ret += *c;
			}
			c++;
		}
		return ret;
	}
	
	void grammar_parse(char * buf)
	{
		//读取3NF，加入NFA
		char *c = buf;
		string ts = line_parse(c);
		#ifdef _DEBUG_
		// cout << ts << "   "<<nfa.size()<<endl;
		#endif
		if(nfa.find(ts) == nfa.end())
		{
			s_list.push_back(ts);
			nfa[ts] = nstate(nfa_cnt++);
		}
		
		char rc = *(c+3);
		bool flag = false;
		//判断rc有没有
		if(rc != '<')
		{
			VT.insert(rc);
		}
		else
		{
			rc = 0;
		}
		string ts2 = line_parse(c);
		// 判断ts2是否为空
		if(ts2.length()==0)
		{
			ts2 = end_s;
		}
		else if(nfa.find (ts2) == nfa.end())
		{
			s_list.push_back(ts2);
			nfa[ts2] = nstate(nfa_cnt++);
		}
		// TODO:偷懒写法,可能存在问题
		nfa[ts].to.push_back(nfa[ts2].idx);
		nfa[ts].to_char.push_back(rc);
		// cout << ts << "  " << (rc) << "  " << ts2 << endl;

		
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
				for(auto b:a.second.to_char)
				{
					printf("%c", b);
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
					printf("%c %d\n", a.to_char[i], a.to[i]);
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
			int len = nfa[s_list[i]].to_char.size();
			for (int j = 0; j < len; j++)
			{
				// j 为epsilon, 将对应nfa状态编号加入ns
				if(nfa[s_list[i]].to_char[j]==0)
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

	void move_dfa(int idx, char to_char)
	{
		dstate ret;
		for(auto i:dfa[idx].ns)
		{
			int len = nfa[s_list[i]].to_char.size();
			for (int j = 0; j < len; j++)
			{
				// j 为epsilon, 将对应nfa状态编号加入ns
				if(nfa[s_list[i]].to_char[j]==to_char)
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
		dfa[idx].to_char.push_back(to_char);	
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
				if(a != 0)
				{
					move_dfa(p, a);
				}
			}
			p++;
		}
	}

	bool code_parse(string str)
	{
		int p = 0, s = 0;
		while(str[p] != 0)
		{
			int i;
			int ps = s;
			// output();
			for (i = 0; i < dfa[s].to_char.size(); i++)
			{
				if(dfa[s].to_char[i] == str[p])
				{
					s = dfa[s].to[i];
					break;
				}
			}
			p++;
			if(i == dfa[ps].to_char.size() )
			{
				return false;
			}
			if(str[p] == 0)
			{
				if(dfa[s].isEnd == true)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		return true;
	}

	bool code_parse(char c)
	{
		for (int i = 0; i < dfa[now_state].to_char.size(); i++)
		{
			if(dfa[now_state].to_char[i] == c)
			{

				now_state = dfa[now_state].to[i];
				return true;
			}
		}
		now_state = 0;
		return false;
	}

	void clear()
	{
		now_state = 0;
	}

};

vector<token_parser> tv;

void parse_all_grammar()
{
	char buf[256];
	while(cin.getline(buf, 255))
	{
		char *tc = buf;

		string ts = token_parser::line_parse(tc);
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
		#ifdef _DEBUG_
		// printf("%s   %d\n", buf, i);
		#endif
		tv[i].grammar_parse(buf);
	}
	for (int i = 0; i < tv.size(); i++)
	{
		tv[i].nfa2dfa();
		#ifdef _DEBUG_
		// tv[i].output(true, true);
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
	else if(qualifier.find(sample) != qualifier.end())
	{
		token_type = 2;
	}
	else if(delimiters.find(sample) != delimiters.end())
	{
		token_type = 3;
	}
	else if(op.find(sample) != op.end())
	{
		token_type = 1;
	}
	return token_type;
}

int judge_token(string sample)
{
	int token_type = -1;
	int idx = graddy_judge(sample);
	if(idx == -1)
	{
		for (int i = 0; i < tv.size(); i++)
		{
			if(tv[i].code_parse(sample) == true)
			{
				token_type = i + 4;
			}
		}
	}
	else
	{
		token_type = idx;
	}
	return token_type;
}

void check_code(string file_path)
{
	ifstream f1;
	f1.open(file_path, std::ios::in);
	string buf;
	int line_idx = 0;
	
	while(getline(f1, buf))
	{
		line_idx++;
		int p1 = 0, p2 = 0;
		string ctemp = "";
		int t_cnt = 0;
		while(buf[p2] != 0)
		{
			while(buf[p2] == ' ')
			{
				p2++;
			}
			p1 = p2;
			int type;
			bool tvflag[2]{true, true};
			while(1)
			{
				string sample = buf.substr(p1, p2 - p1 + 1);
				type = graddy_judge(sample);
				// 如果之前的字符
				if(type == -1)
				{
					string sample = buf.substr(p2, 1);
					type = graddy_judge(sample);
				}
				for (int i = 0; i < 2; i++)
				{
					if(tvflag[i] == true)
					{
						int tn = tv[i].now_state;
						if(tv[i].code_parse(buf[p2]) == true)
						{
							tvflag[i] = true;
						}
						else
						{
							tvflag[i] = false;
						}
						
						
					}
				}
				#ifdef _DEBUG_
				// cout << buf[p2] << "  ";
				// cout << tvflag[0] << "   " << tvflag[1] << endl;
				#endif
					
				if(type == 1 || type == 3)
				{
					if (tvflag[1] == false && tvflag[0] == false)
					{
						for (int i = 0; i < 2; i++)
						{
							tvflag[i] = true;
							tv[i].clear();
						}
						if (p2 > p1)
						{
							string s1 = buf.substr(p1, p2 - p1);
							int tt = judge_token(s1);
							if (tt != -1)
							{
								#ifdef _DEBUG_
								printf("%d %s %s 1\n", line_idx, s1.c_str(), token_names[tt].c_str());
								#endif
								printf("%d %s %s\n", line_idx, s1.c_str(), token_names[tt].c_str());
							}
							else
							{
								#ifdef _DEBUG_
								printf("%d %s %s 1\n", line_idx, s1.c_str(), "错误输入");
								#endif
								printf("%d %s %s\n", line_idx, s1.c_str(), "错误输入");
							}
						}
						#ifdef _DEBUG_
						printf("%d %s %s 2\n", line_idx, buf.substr(p2, 1).c_str(), token_names[type].c_str());
						#endif
						printf("%d %s %s\n", line_idx, buf.substr(p2, 1).c_str(), token_names[type].c_str());
						p1 = p2 + 1;
					}
						
					}
				
				p2++;
				if(buf[p2] == ' ' || buf[p2] == 0)
				{
					if(p2 > p1 )
					{
						string s1 = buf.substr(p1, p2 - p1);
						int tt = judge_token(s1);
						if(tt != -1)
						{
							#ifdef _DEBUG_
							printf("%d %s %s 3\n", line_idx, s1.c_str(), token_names[tt].c_str());
							#endif
							printf("%d %s %s\n", line_idx, s1.c_str(), token_names[tt].c_str());
						}
						else
						{
							#ifdef _DEBUG_
							printf("%d %s %s 3\n", line_idx, s1.c_str(), "错误输入");
							#endif
							printf("%d %s %s\n", line_idx, s1.c_str(), "错误输入");
						}
					}
					break;
				}
			}
			for (int i = 0; i < tv.size(); i++)
			{
				tv[i].clear();
			}
		}
	}

}

int main()
{

#ifdef m_
	freopen("rjks/task1/token_grammar.txt", "r", stdin);
	freopen("output.txt","w",stdout);
#endif

	parse_all_grammar();
	for (int i = 0; i < tv.size(); i++)
	{
		token_names.push_back(tv[i].start_s);
	}
	check_code("rjks/task1/code.txt");
	
#ifdef m_
	fclose(stdin);
	fclose(stdout);
#endif
	return 0;
}


