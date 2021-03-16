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
语法分析器

*/
//						0		1			2		3		4		5
vector<string> token_names = {"关键词", "操作符", "限定符", "界符"};
const set<string> keyword = {"break","case","char","continue","do","default","double","else","float","for","if",
"int","include","long","main","return","switch","typedef","void","unsigned","while"};
const set<string> op = {"+","-","*","/","!","^","=","%","&","&&","|","||","<","<=",">",">=","==","!=","++","--"};
const set<string> delimiters ={";","(",")","{","}",",","[","]","#", "\"", "\'"};
const set<string> qualifier = {"const", "static"};

class syntax_parser
{
	public:
	syntax_parser(string syntax_path, string code_path)
	{
		f1.open(syntax_path, std::ios::in);
		f2.open(code_path, std::ios::in);
	}

	struct form
	{
		int left;// 产生式左部
		vector<int> right;// 产生式右部
		form(int a, vector<int> b):left(a), right(b)
		{

		}
	};

	struct item
	{
		int f; // 产生式
		int p = 0;// 产生式的状态
		int fs; // 前向搜索符
		item(int form_idx, int pos, int fs_idx):f(form_idx), p(pos), fs(fs_idx)
		{}
		bool operator==(const item & a)const
		{
			return f == a.f && p == a.p && fs == a.fs;
		}
		bool operator<(const item & a)const
		{
			if(f != a.f)
			{
				return f < a.f;
			}
			else if(p != a.p)
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

	void output()
	{
		puts("-----* output *-----");

		for (int i = 0; i < form_list.size(); i++)
		{
			cout << V_list[form_list[i].left] << " ";
			for(auto j :  form_list[i].right)
			{
				cout << V_list[j] << " ";
			}
			puts("");
		}
		
		puts("\n V_LIST");
		for (int i = 0; i < V_list.size(); i++)
		{
			cout << V_list[i] << "  is_VT: " << is_VT[i] << endl;
		}
		puts("");

		cout << "\n to_epilon \nto_epilon_size  " << to_epsilon.size() << endl;
		for(auto i:to_epsilon)
		{
			cout << V_list[i] << "  ";
		}
		puts("\n\n");

		puts("first_set");
		for (auto & s:first_set)
		{
			cout << V_list[s.first] << ": ";
			for(auto i:s.second)
			{
				cout << V_list[i] << " ";
			}
			puts("");
		}

		puts("item_clan");
		for (int i = 0; i < item_clan.size(); i++)
		{
			printf("item%d : \n", i);
			for(auto & j : item_clan[i])
			{
				cout << V_list[form_list[j.f].left] << "->";
				for(auto k : form_list[j.f].right)
				{
					cout << V_list[k];
				}
				cout << "  ";
				cout << " " << j.p << "  " << V_list[j.fs] << endl;
			}
			cout <<endl;
		}

		puts("go & reverse");
		for (int i = 0; i < go.size(); i++)
		{
			printf("item%d:\n", i);
			puts("go:");
			for(auto & j:go[i])
			{
				cout << i << "  " << V_list[j.first] << "  " << j.second << endl;
			}
			puts("reverse:");
			for(auto & j:reverse[i])
			{
				cout << i << "  " << V_list[j.first] << "  ";
				printf("%s->", V_list[form_list[j.second].left].c_str());
				for (int i : form_list[j.second].right)
				{
					cout << V_list[i];
				}
				cout<< endl;
			}
			cout << endl;
		}

		cout << endl;
	}

	// 搜索s是否在V_LIST中，若不在则加入，返回s在V_LSIT中的编号
	int V_idx(const string & s, bool is_push=true)
	{
		int i = 0;
		for (i = 0; i < V_list.size(); i++)
		{
			if(V_list[i] == s)
			{
				break;
			}
		}
		if(i == V_list.size() && is_push)
		{
			V_list.push_back(s);
			// 如果左右为<>则认为是VN
			if(s[0] == '<' &&s[max(int(s.size()) - 1, 0)] == '>')
			{
				is_VT.push_back(false);
			}
			else
			{
				is_VT.push_back(true);
			}
		}
				
		return i;
	}
	
	// 寻找产生空的Vn
	void find_epsilon()
	{
		bool change = false;
		while(1)
		{
			change = false;
			for (int i = 0; i < V_list.size(); i++)
			{	
				if(is_VT[i] == true || (is_VT[i] ==false && to_epsilon.find(i) != to_epsilon.end()))
				{
					continue;
				}
				for(auto f:form_map[i])
				{
					bool flag = false;
					for (int j = 0; j < form_list[f].right.size(); j++)
					{
						//如果是Vt或者不指向epsilon的Vt跳过
						int idx = form_list[f].right[j];
						if(is_VT[idx] == true || (is_VT[idx] ==false && to_epsilon.find(idx) == to_epsilon.end()))
						{
							break;
						}
						if(j == form_list[f].right.size() -1)
						{
							flag = change = true;
							to_epsilon.insert(i);
						}
						
					}
					if(flag == true)
					{
						break;
					}
				}	

			}
			if(change == false)
			{
				break;
			}
			
		}
	}

	// 读取语法
	void read_syntax()
	{
		string buf;
		// 预处理
		V_idx(end_s);
		V_idx(start_s);
		form_list.push_back(form(V_idx(start_s), {V_idx("<S>")}));
		
		while(getline(f1, buf))
		{
			if(buf.size() == 0)
			{
				continue;
			}
			int p = 0, pp = 0;
			int left;
			vector<int> right;
			//读取左部
			while(buf[p] != ' ' &&buf[p] != 0)
			{
				p++;
			}
			string temp = buf.substr(pp, p - pp);
			left = V_idx(temp);
			// 读取右部
			while(buf[p] != 0)
			{
				while(buf[p] == ' ')
				{
					p++;
				}
				pp = p;
				while(buf[p] != ' ' && buf[p] != 0)
				{
					p++;
				}
				if(p - pp ==0)
				{
					continue;
				}
				string temp = buf.substr(pp, p - pp);
				right.push_back(V_idx(temp));
			}
			// 默认语法中的产生式不会重复
			// 如果右部没有东西，则为epsilon
			if(right.size() == 0)
			{
				right.push_back(0);
				to_epsilon.insert(left);
			}
			form_list.push_back(form(left, right));
			for (int i = 0; i < form_list.size(); i++)
			{
				form_map[form_list[i].left].push_back(i);
			}


		}


		find_epsilon();
		gen_first();
	}

	set<int> union_first(const set<int> & a, const set<int> & b)
	{
		set<int> c;
		set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(c, c.begin()));
		return c;
	}
	
	// 存在迭代，高危代码段
	set<int> form_first_set(int idx,set<int> & vis)
	{
		set<int> ret;
		int t = form_list[idx].right[0];
		int p = 0, len = form_list[idx].right.size();
		if(is_VT[t] == true)
		{
			return set<int>{t};
		}
		else if(to_epsilon.find(t) != to_epsilon.end())
		{
			
			for (; p < len; p++)
			{
				if(form_list[idx].right[p] != 0 && to_epsilon.find(form_list[idx].right[p]) == to_epsilon.end())
				{
					break;
				}
			}
			if(p == len)
			{
				return set<int>{0};
			}
			else
			{
				t = form_list[idx].right[p];
				if(is_VT[t])
				{
					return set<int>{t};
				}
			}
		}

		// 接下来的这个t是一个不为空的Vn
		if(first_set[t].size() != 0)
		{
			ret = union_first(ret, first_set[t]);
		}
		else if(vis.find(t) == vis.end())
		{
			vis.insert(t);
			set<int> temp;
			for (auto f : form_map[t])
			{

				temp = union_first(temp, form_first_set(f, vis));
			}
			first_set[t] = temp;
			ret = form_first_set(idx, vis);
		}
		return ret;
	}
	
	// 求FIRST
	void gen_first()
	{
		//对每个Vn求firstset
		// first_set[1].insert(0); 说实话我看不懂为什么要写这句话
		for (int i = 0; i < V_list.size(); i++)
		{
			if(is_VT[i] == true)
			{
				continue;
			}
			set<int> temp;
			for(auto f:form_map[i])
			{
				//设置vis是为了切断循环 first
				set<int> vis{i};
				temp = union_first(temp, form_first_set(f,vis));
			}
			first_set[i] = temp;

		}
		
	}
	
	// 求项目集
	int closure(vector<item> & vi, int fa)
	{
		int p = 0;
		set<item> chk(vi.begin(), vi.end());
		// 对每个项目扩展
		while(p < vi.size())
		{
			// 当前项目的form
			form tf = form_list[vi[p].f];
			// 如果当前项目的.指向最后则continue
			if(vi[p].p == tf.right.size())
			{
				p++;
				continue;
			}
			// 当前原点右边的V
			int vv = tf.right[vi[p].p];
			// 如果是VT继续
			if(is_VT[vv] == true)
			{
				p++;
				continue;
			}
			// 当前.所指的位置
			int tp = vi[p].p;
			// 如果是Vn则要扩展
			// 提前计算出前向搜索符
			vector<int> fss;
			while(1)
			{
				tp++;
				if(tp == tf.right.size())
				{
					break;
				}
				if(is_VT[tf.right[tp]])
				{
					// 读取到VT则退出
					fss.push_back(tf.right[tp]);
					break;
				}
				bool to_e = false;
				for(auto first_c : first_set[tf.right[tp]])
				{
					if(first_c != 0)
					{
						fss.push_back(first_c);
					}
					else
					{
						to_e = true;
					}
				}
				if(to_e == false)
				{
					break;
				}
			}
			if(fss.size() == 0)
			{
				fss.push_back(vi[p].fs);
			}
			// 对Vn左部的form f
			for(auto f:form_map[vv])
			{
				for(auto first_c : fss)
				{
					item ti(f, 0, first_c);
					if(chk.find(ti) == chk.end())
					{
						chk.insert(ti);
						vi.push_back(ti);
					}
					
				}
			}
			p++;
		}
		bool flag = false;
		if(fa != -1)
		{
			int i = 0;
			for ( i = 0; i < item_clan.size();i++)
			{
				flag = false;
				for (auto &j : item_clan[i])
				{
					if (chk.find(j) == chk.end())
					{
						flag = true;
						break;
					}
				}
				if(flag == false)
				{
					return i;
				}
			}
		}
		item_clan.push_back(vi);
		return item_clan.size() - 1;
		
		
	}

	// action——goto表与项目集族一起产生
	void generate_clan()
	{
		// 初始化
		item it0 = item(0, 0, 0);// <start>->.<S>, #加入项目集
		vector<item> vi{it0};
		closure(vi, -1);
		int p = 0;
		while (p < item_clan.size())
		{	
			go.push_back(map<int, int>{});
			reverse.push_back(map<int, int>{});
			// 先得到规约
			for(auto & f:item_clan[p])
			{
				
				if(f.p == form_list[f.f].right.size() || form_list[f.f].right[0] == 0)
				{
					(reverse[p])[f.fs] = f.f;
				}
			}
			// 没考虑epsilon
			for (int i = 1; i < V_list.size(); i++)
			{
				vector<item> it;
				vector<int> goi;
				for(auto & f:item_clan[p])
				{
					if(f.p < form_list[f.f].right.size())
					{
						if(form_list[f.f].right[f.p] == i)
						{
							goi.push_back(i);
							it.push_back(item(f.f, f.p + 1, f.fs));
						} 
					}
				}
				if(it.size() > 0)
				{
					int dest = closure(it, p);
					for (int j = 0; j < goi.size(); j++)
					{
						go[p][goi[j]] = dest;
					}
				}
			}
			p++;
		}
		
	}

	// 分析序列 返回错误类型，或者接受 0接受 1不识别的Vt 2拒绝
	int parse(vector<string> & s)
	{
		s.push_back(V_list[0]);
		s_stack.clear();
		V_stack.clear();
		s_stack.push_back(0);
		V_stack.push_back(0);
		int p = 0;
		while(p < s.size())
		{
			int idx = V_idx(s[p], false);
			if(idx == V_list.size()) // 未识别Vt
			{
				return 1;
			}
			int back = s_stack.back();
			if(go[back ].find(idx) != go[back ].end())
			{
				V_stack.push_back(idx);
				s_stack.push_back(go[back ][idx]);
				p++;
			}
			else if(reverse[back ].find(idx) != reverse[back ].end())
			{
				
				int form_idx = reverse[back ][idx];
				int cnt = form_list[form_idx].right.size();
				// 特判空产生式
				if(form_list[reverse[back ][idx]].right[0] == 0)
				{
					cnt = 0;
				}
				while(cnt--)
				{
					s_stack.pop_back();
					V_stack.pop_back();
				}
				V_stack.push_back(form_list[form_idx].left);
				if(V_stack.size() == 2 && V_stack.back() == 1)
				{
					return 0;
				}
				int tl = s_stack.size();
				s_stack.push_back(go[s_stack[tl - 1]][form_list[form_idx].left]);
			}
			else // 输入错误，拒绝接受
			{
				return 2;
			}
#ifdef _DEBUG_
			for(auto & s:s_stack)
			{
				cout << s;
			}
			puts("");
			for(auto & v:V_stack)
			{
				cout << V_list[v];
			}
			puts("\n");
#endif
		}

		return 0;
	}
	
	void parse_begin()
	{
		s_stack.clear();
		V_stack.clear();
		s_stack.push_back(0);
		V_stack.push_back(0);
		Variable.clear();
	}
	// 分析序列 返回错误类型，或者接受 -1程序错误 0接受 1不识别的Vt 2拒绝 3处理中 
	int parse_Vt(string  s)
	{
		int idx = V_idx(s, false);
		if(idx == V_list.size()) // 未识别Vt
		{
			return 1;
		}
		int back = s_stack.back();
		if(go[back ].find(idx) != go[back ].end())
		{
			V_stack.push_back(idx);
			s_stack.push_back(go[back ][idx]);
			return 3;
		}
		else if(reverse[back ].find(idx) != reverse[back ].end())
		{
			
			int form_idx = reverse[back ][idx];
			int cnt = form_list[form_idx].right.size();
			// 特判空产生式
			if(form_list[reverse[back ][idx]].right[0] == 0)
			{
				cnt = 0;
			}
			while(cnt--)
			{
				s_stack.pop_back();
				V_stack.pop_back();
			}
			V_stack.push_back(form_list[form_idx].left);
			if(V_stack.size() == 2 && V_stack.back() == 1)
			{
				return 0;
			}
			int tl = s_stack.size();
			s_stack.push_back(go[s_stack[tl - 1]][form_list[form_idx].left]);
			// 规约不会消耗下一个符号
			return parse_Vt(s);
		}
		else // 输入错误，拒绝接受
		{
			return 2;
		}
		return -1;
	}

	int str2Num(const string  s)
	{
		int ret = 0, p = 0;
		while(s[p] != 0)
		{
			ret *= 10;
			ret += s[p] - '0';
			p++;
		}
		return ret;
	}

	// 读取TASK1的输出
	void parse_code()
	{
		string buf;
		// 开始处理，初始化，感觉有点类似openGL
		parse_begin();
		int result;
		bool have_main = false;
		while(getline(f2, buf))
		{
			int bp = 0, pp = 0;
			// 读取行号
			while(buf[bp] != ' ')
			{
				bp++;
			}
			int line_idx = str2Num(buf.substr(pp, bp - pp));
			
			// 读取Vt
			pp = ++bp;
			while(buf[bp] != ' ')
			{
				bp++;
			}
			string Vt = buf.substr(pp, bp - pp);
			// 读取词性
			pp = ++bp;
			while(buf[bp] != 0)
			{
				bp++;
			}
			string type = buf.substr(pp, bp - pp);
			//TODO 接下来就是根据Vt细致地处理，然后扔到parse里面去
			// 如果Vt是关键字，op，界符，限定词，则将原来的符号扔进去
			// TODO 处理main
			if(type == "关键词"  || type == "界符")
			{
				if(Vt == "main")
				{
					if(have_main == true)
					{
						// 4出现多个main
						result = 4;
					}
					else
					{
						have_main = true;
						result = parse_Vt("标识符");
					}
				}
				else
				{
					result = parse_Vt(Vt);
				}
			}
			else if(type == "标识符")
			{
				result = parse_Vt(type);
			}
			else  if(type == "操作符")
			{
				if(Vt == "=")
				{
					result = parse_Vt(Vt);
				}
				else
				{
					result = parse_Vt(type);
				}
			}
			else  if(type == "常量" || type == "限定符")// 如果是常数 标识符 则把type扔进去
			{
				result = parse_Vt(type);
			}
			else
			{
				result = 1;
			}
			cout << line_idx << "  " << result << endl;
			if(result == 2)
			{
				break;
			}
			#ifdef _DEBUG_
			for(auto & s:s_stack)
			{
				cout << s;
			}
			puts("");
			for(auto & v:V_stack)
			{
				cout << V_list[v];
			}
			puts("\n");
			#endif

		}
		result = parse_Vt(end_s);
		cout << result << endl;
		#ifdef _DEBUG_
		for(auto & s:s_stack)
		{
			cout << s;
		}
		puts("");
		for(auto & v:V_stack)
		{
			cout << V_list[v];
		}
		puts("\n");
		#endif
		
		// cout << result << endl;
	}

	void end()
	{
		f1.close();
		f2.close();
	}

};



int main()
{

#ifdef m_
	// freopen("output.txt","w",stdout);
	freopen("rjks/task1/token_grammar.txt", "r", stdin);
	// freopen("input.txt","r",stdin);
	freopen("output.txt","w",stdout);
#endif
	string file_path = "rjks/task2/syntax.txt";
	// string file_path = "input.txt";
	string code_path = "rjks/task2/token_list.txt";
	syntax_parser sp(file_path, code_path);
	sp.read_syntax();
	sp.generate_clan();
	
	// vector<string> test1 = {"void", "标识符", "(","int", "标识符",")", "{", ";", "}"};
	// vector<string> test2 = {"标识符",";"};
	// cout << sp.parse(test1) << endl;
	// puts("------------------------------");
	// cout << sp.parse(test2) << endl;
	sp.parse_code();
	sp.end();
#ifdef m_
	fclose(stdin);
	fclose(stdout);
#endif
	return 0;
}

