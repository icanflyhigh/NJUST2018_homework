// author: M@
# define m_
#define _CRT_SECURE_NO_WARNINGS
// #define _DEBUG_
#pragma comment(linker, "/STACK:102400000,102400000") 
#include<iostream>
#include<stdlib.h>
#include<fstream>
#include<istream>
#include<sstream>
#include<cstdio>
#include<cstdlib>
#include<string.h>
#include<queue>
#include<vector>
#include<map>
#include<algorithm>
#include<cmath>
#include<stack>
#include<list>
#include<set>
#include<time.h>
#include<string>
#include<assert.h>
#include<time.h>
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
const vector<string> atom_op = {"print", "+", "-", "*", "/", ":="};
const vector<string> property = {"val", "type"};
vector<string> splitEx(const string &src, string separate_character);

enum id_type
{
	const_var, ident, type_other
};

enum operation
{
	// 主义排列顺序与atom_op的顺序相同，为了更好的映射
	enum_print, enum_add, enum_sub, enum_mul, enum_div, enum_assign
};



// 返回s在atom_op中的标号
inline int atom_op_idx(const string & s){
	for(int i = 0; i <= atom_op.size(); i++){
		if(atom_op[i] == s){
			return i;
		}
	}
	return -1;
}
class type_base{
};

inline int property_idx(const string & s){
	for(int i = 0; i <= property.size(); i++){
		if(property[i] == s)return i;
	}
	return -1;
}

class quadra_tuple{
	public:
		int  op1, op2, dst;
		operation func;
		quadra_tuple(operation to_func, int to_op1=0, int to_op2=0, int to_dst=0):
		op1(to_op1), op2(to_op2), dst(to_dst), func(to_func){}

};

class syntax_parser
{
	public:
	syntax_parser(string syntax_path, string code_path)
	{
		f1.open(syntax_path, std::ios::in);
		f2.open(code_path, std::ios::in);
	}

	struct operand{
		int v, property; //v指向产生式的第几个V -1 表示左式
		operand(int a=0, int b=0): v(a), property(b){
		}
	};

	struct form
	{
		int left;// 产生式左部
		vector<int> right;// 产生式右部
		vector<int> op;// 通过模拟栈操作来实现符号的处理
		vector<operand> var;// 通过模拟栈操作来实现符号的处理
		form(int a, vector<int> b):left(a), right(b){}
	};

	// 分析栈中的元素
	struct Identifier{
		string name;
		int val, addr, size, level;
		id_type type;
		Identifier(string a, int to_val=0, id_type to_type=(id_type)0, int to_addr=0, int to_size=0, int to_level=0):
		name(a), val(to_val), addr(to_addr), size(to_size), level(to_level), type(to_type){}
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


	void _print_quadra_tuple_(const quadra_tuple  & q){
		switch(q.func){
			case enum_add:
				printf("%s %s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(), 
										id_table[q.op2].name.c_str(), id_table[q.dst].name.c_str());
				break;
			case enum_sub:
				printf("%s %s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(), 
										id_table[q.op2].name.c_str(), id_table[q.dst].name.c_str());
				break;
			case enum_mul:
				printf("%s %s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(), 
										id_table[q.op2].name.c_str(), id_table[q.dst].name.c_str());
				break;
			case enum_div:
				printf("%s %s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(), 
										id_table[q.op2].name.c_str(), id_table[q.dst].name.c_str());
				break;
			case enum_assign:
				printf("%s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(), 
										id_table[q.op2].name.c_str());
				break;
			case enum_print:
				printf("%s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str());
				break;
		}




				
	}
	void print_quadra_tuple_list(){
		for(const auto & q: quadra_tuple_list){	_print_quadra_tuple_(q);}
	}
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
	vector<map<int, int>> go; // 转移 第一个int表示符号，第二个表示转移到的状态
	vector<map<int, int>> reverse;// 规约，第一个int表示符号，第二个表示规约的状态数
	map<int, set<int>> first_set; // first集

	// 处理token时会随着读取变化的变量
	vector<quadra_tuple> quadra_tuple_list;
	vector<Identifier> id_table; // 变量表
	map<int, int> int_val; // 储存int型变量的值
	map<int, double> double_map;// 储存double型变量的值
	vector<int> s_stack; // 状态栈
	vector<int> V_stack; // 符号栈
	vector<int> id_stack;// 操作id分配reg的stack
	int Vn_reg_cnt = 0; // 给杂项分配id_table的计数器

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
			cout << endl;
		}

		puts("Form_List");
		for(int i = 0; i < form_list.size(); i++){
			form temp = form_list[i];
			printf("%s->", V_list[temp.left].c_str());
			for(int j = 0; j < temp.right.size(); j++){
				printf("%s", V_list[temp.right[j]].c_str());
			}
			printf("  $  ");
			for(auto j : temp.op){
				printf("%s", atom_op[j].c_str());
			}
			for(auto j : temp.var){
				int jv = j.v;
				if(jv == -1){// 是左式
					printf("(L)%s.%s ", V_list[temp.left].c_str(), property[j.property].c_str());
				}
				else{// 是右式
					j.v == -2 ? 
					printf("(temp.%s ", property[j.property].c_str()) :
					printf("(R)%s.%s ", V_list[temp.right[j.v]].c_str(), property[j.property].c_str());
				}
				
			}
			cout << endl;
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

	// p 指向字符串读取数字的第一个位置
	inline int read_int(const string & s, int & p){
		int ret = 0;
		bool t = false;
		while(s[p] < '0' || s[p] > '9'){
			p++;
		}
		if(s[max(p - 1, 0)] == '-')t = true; 
		while(s[p] >='0' && s[p] <='9'){
			ret = ret * 10 + s[p++] - '0';
		}
		return t ? -ret : ret;
	}

	inline int chk_dot(const string & s){
		for(int i = 0; i < s.size(); i++){
			if(s[i] == '.')return i;
		}
		return -1;
	}


	// p 指向字符串读取数字的第一个位置
	inline double read_double(const string & s, int p){
		bool t = false;
		double z = 0, b = 0.1;
		if(s[p] == '-'){
			t = false;
			p++;
		}
		while(s[p] >='0' && s[p] <='9'){
			z = z * 10 + s[p++] - '0';
		}
		p++;
		while(s[p] >='0' && s[p] <='9'){
			z +=  (s[p++] - '0') * b;
			b *= 0.1;
		}
		return t ? -z : z;
	}

	// 读取语法
	void read_syntax()
	{
		string buf;
		// 预处理
		V_idx(end_s);
		V_idx(start_s);
		form_list.push_back(form(V_idx(start_s), {V_idx("<S>")}));
		
		while(getline(f1, buf)){
			if(buf.size() == 0){
				continue;
			}
			int p = 0, pp = 0;
			int left;
			vector<int> right;
			//读取左部
			while(buf[p] != ' ' &&buf[p] != 0){
				p++;
			}
			string temp = buf.substr(pp, p - pp);
			left = V_idx(temp);
			// 读取右部
			while(buf[p] != '$')
			{
				while(buf[p] == ' ')
				{
					p++;
				}
				pp = p;
				while(buf[p] != ' ' && buf[p] != 0 && buf[p] != '$')
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
			// 读取表达式 ,当前p指向$
			p++;
			int idx = form_list.size() - 1;
			int len = read_int(buf, p);
			bool clr_temp = true; // 忽略第一个操作符的返回值
			while(len){
				while(buf[p] == ' ')p++;
				if(buf[p] == 0)break;
				int pp = p;
				while(buf[p] != ' ' && buf[p] != 0)p++;
				string temp = buf.substr(pp, p - pp);
				int pos = chk_dot(temp);
				if(temp == ";"){
					clr_temp = true;
					continue;
				}
				if(pos == -1){ // 如果操作符
					form_list[idx].op.push_back(atom_op_idx(temp));
					if(clr_temp == false){
						form_list[idx].var.push_back(operand(-2, 0)); // 暂存结果用
					}
					else{
						clr_temp = false;
					}
				}
				else{// 如果是属性
					string ls, rs;
					ls = buf.substr(pp, pos);
					rs = buf.substr(pp + pos + 1, p - pos - 1 - pp);
					//将E_1这二种类型转化为v在产生式中的位置，左部不用_，右部标示第几个
					//从下标0开始
					int _pointer = 0;
					for (_pointer = 0; ls[_pointer] != 0 && ls[_pointer] != '_'; _pointer++){}
					if(_pointer == ls.size()){ //该V没有下划线
						if(ls == V_list[form_list[idx].left]){
							form_list[idx].var.push_back(operand(-1, property_idx(rs)));
						}
						else{
							for (int i = 0; i < form_list[idx].right.size(); i++){
								if(V_list[form_list[idx].right[i]] == ls){
									form_list[idx].var.push_back(operand(i, property_idx(rs))); 
									break;
								}
							}	
						}
					}
					else{ // 有下划线的情况
						int pp = _pointer + 1;
						string v1 = ls.substr(0, _pointer);
						if(v1[0] ==  '<'){
							v1 += string(">");
						}
						int num = read_int(ls, _pointer);
						for (int i = 0; i < form_list[idx].right.size(); i++){
							if(v1 == V_list[form_list[idx].right[i]]){
								num--;
								if(num == 0){
									form_list[idx].var.push_back(operand(i, property_idx(rs))); 
									break;
								}
							}
						}
					}
					// form_list[idx].var.push_back(operand(V_idx(ls, false), property_idx(rs)));
				}

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
			// 再得到go
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

	// 返回相加结果的变量的编号
	int _add_(int idx1, int idx2, int dst){ // 1号操作add
		quadra_tuple_list.push_back(quadra_tuple(enum_add, idx1, idx2, dst));
		return 0;
	}

	int _mul_(int idx1, int idx2, int dst){
		quadra_tuple_list.push_back(quadra_tuple(enum_mul, idx1, idx2, dst));
		return 0;
	}

	int _sub_(int idx1, int idx2, int dst){
		quadra_tuple_list.push_back(quadra_tuple(enum_sub, idx1, idx2, dst));
		return 0;
	}

	int _div_(int idx1, int idx2, int dst){
		quadra_tuple_list.push_back(quadra_tuple(enum_div, idx1, idx2, dst));
		return 0;
	}

	int _assign_(int idx1, int idx2){
		quadra_tuple_list.push_back(quadra_tuple(enum_assign, idx2, idx1));
		return 0;
	}

	int _error_(){
		printf("func ERROR\n");
		return 0;
	}


	// 打印idx对应元素的name(string)
	int _print_(int idx){
		quadra_tuple_list.push_back(quadra_tuple(enum_print, idx));
		return 0;
	}
	
	// 根据输入数字来调用函数
	void num2func(operation opn, int idx1, int idx2=0, int dst=0){
		switch(opn){
			case enum_add:
				_add_(idx1, idx2, dst);
				break;
			case enum_sub:
				_sub_(idx1, idx2, dst);
				break;
			case enum_mul:
				_mul_(idx1, idx2, dst);
				break;
			case enum_div:
				_div_(idx1, idx2, dst);
				break;
			case enum_print:
				_print_(idx1);
				break;
			case enum_assign:
				_assign_(idx1, idx2);
				break;
			default:
				_error_();
		}
		

	}

	int add_id(const string & name, id_type type, int val=0,  int addr=0, int size=0, int level=0){
		int i = 0;
		if(name[0] == '$'){ // 暂存数据

		}
		else{
			vector<Identifier>::iterator iter = id_table.begin();
			for(;iter != id_table.end(); i++,iter++){
				if(name == (*iter).name){
					if(type != (*iter).type){
						return -2;// -2表示元素类型错误
					}
					else{
						return i;
					}
				}
			}
		}
		
		id_table.push_back(Identifier(name, val, type, addr, size, level));
		return -1;// -1表示末尾添加
	}

	int add_temp(){
		stringstream ss;
		ss << Vn_reg_cnt++;
		string temps;
		ss >> temps;
		temps = string("$") + temps;
		add_id(temps, type_other);
		return id_table.size() - 1;
	}

	void parse_begin()
	{
		s_stack.clear();
		V_stack.clear();
		s_stack.push_back(0);
		V_stack.push_back(0);
		
	}

	
	
	
	void optimize_quadra_tuple_list(){
		vector<quadra_tuple>::iterator iter = quadra_tuple_list.begin();
		map<string, string> ms;
	}

	// TODO需要处理将表达式转化为4元式
	// id_stack留着在原来函数里面pop
	// 这里只处理表达式
	int deal_expression(int id_num, int form_idx, int lf){
		vector<int> rf = vector<int>(id_stack.end() - id_num, id_stack.end());
		// 将表达式输出为4元式
		const form &tf = form_list[form_idx];
		int opp = tf.op.size(), 
		    varp = tf.var.size() ;
		while(opp > 0){
			operation to = (operation)tf.op[--opp];
			int top;
			switch(to){// 从操作组栈弹出一个元素
				case enum_print:{ // 弹出一个操作数
					int tv = tf.var[--varp].v, idx1;
					if(tv == -2) idx1 = add_temp(), varp++;
					else idx1 = tv == -1 ? lf : rf[tv];
					num2func(enum_print, idx1);
					break;
				}
					
				case enum_add:{
					int idx1 = tf.var[--varp].v == -1 ? lf : tf.var[varp].v == -2 ? top :rf[tf.var[varp].v];;
					int idx2 = tf.var[--varp].v == -1 ? lf : rf[tf.var[varp].v];
					int tv = tf.var[--varp].v, dst;
					if(tv == -2){
						top = dst = add_temp(), varp++;
					}
					else dst = tv == -1 ? lf : rf[tf.var[varp].v];
					num2func(enum_add, idx1, idx2, dst);
					break;
				}
				case enum_sub:{
					int idx1 = tf.var[--varp].v == -1 ? lf : rf[tf.var[varp].v];
					int idx2 = tf.var[--varp].v == -1 ? lf : rf[tf.var[varp].v];
					int tv = tf.var[--varp].v, dst;
					if(tv == -2)top = dst = add_temp(), varp++;
					else dst = tv == -1 ? lf : tv;
					num2func(enum_sub, idx1, idx2, dst);
					break;
				}
				case enum_mul:{
					int idx1 = tf.var[--varp].v == -1 ? lf : tf.var[varp].v == -2 ? top :rf[tf.var[varp].v];;
					int idx2 = tf.var[--varp].v == -1 ? lf : rf[tf.var[varp].v];
					int tv = tf.var[--varp].v, dst;
					if(tv == -2)top = dst = add_temp(), varp++;
					else dst = tv == -1 ? lf : rf[tv];
					num2func(enum_mul, idx1, idx2, dst);
					break;
				}
				case enum_div:{
					int idx1 = tf.var[--varp].v == -1 ? lf : tf.var[varp].v == -2 ? top :rf[tf.var[varp].v];;
					int idx2 = tf.var[--varp].v == -1 ? lf : rf[tf.var[varp].v];
					int tv = tf.var[--varp].v, dst;
					if(tv == -2)top = dst = add_temp(), varp++;
					else dst = tv == -1 ? lf : rf[tf.var[varp].v];
					num2func(enum_div, idx1, idx2, dst);
					break;
				}
				case enum_assign:{
					int idx1 = tf.var[--varp].v == -1 ? lf : tf.var[varp].v == -2 ? top :rf[tf.var[varp].v];
					int tv = tf.var[--varp].v, idx2;
					if(tv == -2)idx2 = add_temp(), varp++;
					else top = idx2 = tv == -1 ? lf : rf[tf.var[varp].v];
					num2func(enum_assign, idx1, idx2);
					break;
				}
			}
		}
		

		return 0;
	}

	// TODO处理parse，1.产生参数表 2.语法树 3.特殊处理某些输入
	// 分析序列 返回错误类型，或者接受 -1程序错误 0接受 1不识别的Vt 2拒绝 3处理中 4标识符类型错误
	int parse_Vt(string  s, string Vt, string type)
	{
		int idx = V_idx(s, false);
		if(idx == V_list.size()) // 未识别Vt
		{
			return 1;
		}
		int back = s_stack.back();
		if(go[back ].find(idx) != go[back ].end()) // 转移
		{
			int ret = -1;
			if(type == "标识符"){
				ret = add_id(Vt, ident); // 在送入语法之前先加入table
			}
			else if(type == "常量"){
				int p = 0;
				int val = read_int(Vt, p);
				ret = add_id(Vt, const_var, val); // 在送入语法之前先加入table
			}
			else{ // 处理除标识符和常量之外的杂项
				ret = add_temp();
			}
			if(ret == -1){
				ret = id_table.size() - 1;
			}
			id_stack.push_back(ret);
			V_stack.push_back(idx);
			s_stack.push_back(go[back ][idx]);
			return 3;
		}
		else if(reverse[back].find(idx) != reverse[back].end()) // 规约
		{
			int form_idx = reverse[back][idx];
			int cnt = form_list[form_idx].right.size();
			// int ret = add_id(string("$") + V_list[form_list[form_idx].left], type_other) == -1 ? id_table.size() - 1 : ret;
			int ret = add_temp();
			// 特判空产生式
			if(form_list[reverse[back][idx]].right[0] == 0)
			{
				cnt = 0;
			}
			deal_expression(cnt, form_idx, ret);
			while(cnt--)
			{
				s_stack.pop_back();
				V_stack.pop_back();
				id_stack.pop_back();
			}
			V_stack.push_back(form_list[form_idx].left);
			
			id_stack.push_back(ret);
			if(V_stack.size() == 2 && V_stack.back() == 1)
			{
				return 0;
			}
			int tl = s_stack.size();
			s_stack.push_back(go[s_stack[tl - 1]][form_list[form_idx].left]);
			// 规约不会消耗下一个符号
			return parse_Vt(s, Vt, type);
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
	// TODO给出错误的原因
	void parse_code()
	{
		string buf;
		// 开始处理，初始化，感觉有点类似openGL
		parse_begin();
		int result;
		bool have_main = false;
		int line_idx;
		string type, Vt;
		while(getline(f2, buf))
		{
			int bp = 0, pp = 0;
			// 读取行号
			while(buf[bp] != ' ')
			{
				bp++;
			}
			line_idx = str2Num(buf.substr(pp, bp - pp));
			
			// 读取Vt
			pp = ++bp;
			while(buf[bp] != ' ')
			{
				bp++;
			}
			Vt = buf.substr(pp, bp - pp);
			// 读取词性
			pp = ++bp;
			while(buf[bp] != 0)
			{
				bp++;
			}
			type = buf.substr(pp, bp - pp);
			// 如果Vt是关键字，op，界符，限定词，则将原来的符号扔进去
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
						result = parse_Vt("标识符", Vt, "标识符");
					}
				}
				else
				{
					result = parse_Vt(Vt, Vt, type);
				}
			}
			else if(type == "标识符")
			{
				
				result = parse_Vt(type, Vt, type);
			}
			else  if(type == "操作符")
			{
				result = parse_Vt(Vt, Vt, type);
			}
			else  if(type == "常量" || type == "限定符")// 如果是常数 标识符 则把type扔进去
			{

				result = parse_Vt(type, Vt, type);
			}
			else
			{
				result = 1;
			}
			
			if(result == 2)
			{
				break;
			}
			#ifdef _DEBUG_
			cout << line_idx << "  " << result << endl;
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
		// printf("111\n");
		if(result != 3){
			cout << "NO:\n line:"<<line_idx << "  "<<Vt<<endl;
			printf("可能的错误: %s或%s\n", V_list[V_stack.back()].c_str(), Vt.c_str());
		}
		else{
			result = parse_Vt(end_s, end_s, "end_type");
			if(result != 0){
				cout << "NO:\n line:"<<line_idx << "  "<<Vt<<endl;
			}
			printf("YES!\n");
		}
		puts("quadra_tuple_list:");
		print_quadra_tuple_list();
		cout << endl;
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
	// freopen("rjks/task1/token_grammar.txt", "r", stdin);
	// freopen("input.txt","r",stdin);
	freopen("output.txt","w",stdout);
#endif
	string file_path = "rjks/task3/sematic.txt";
	// string file_path = "input.txt";
	string code_path = "rjks/task3/token_list.txt";
	syntax_parser sp(file_path, code_path);
	clock_t t1, t2, t3, t4;
	// t1 = clock();
	sp.read_syntax();
	// t2 = clock();
	// cout << "gogogo" << endl;
	sp.generate_clan();
	// t3 = clock();
	// sp.output();
	
	sp.parse_code();
	// t4 = clock();
	// sp.output();
	// sp.end();
#ifdef m_
	fclose(stdin);
	fclose(stdout);
#endif
	return 0;
}

vector<string> splitEx(const string &src, string separate_character)
{
	vector<string> strs;

	int separate_characterLen = separate_character.size(); // 分割字符串的长度,这样就可以支持如“,,”多字符串的分隔符
	int lastPosition = 0, index = -1;
	while (-1 != (index = src.find(separate_character, lastPosition)))
    {
		strs.push_back(src.substr(lastPosition, index - lastPosition));
		lastPosition = index + separate_characterLen;
	}
	string lastString = src.substr(lastPosition); // 截取最后一个分隔符后的内容
	if (!lastString.empty())
		strs.push_back(lastString); // 如果最后一个分隔符后还有内容就入队
	return strs;
}

