#include "sematic_parser.h"
#include "Type.h"
// author: M@
 //#define _DEBUG_

/*
语义分析器
*/

void sematic_parser::test()
{
	read_syntax();
	generate_clan();
	parse_code();
	//output();
	AST2TAC();
}


sematic_parser::sematic_parser(string syntax_path, string code_path)
{
	f1.open(syntax_path, std::ios::in);
	f2.open(code_path, std::ios::in);
}

//
//void sematic_parser::_print_quadra_tuple_(const quadra_tuple  & q) {
//	switch (q.func) {
//	case enum_add:
//		printf("%s %s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(),
//			id_table[q.op2].name.c_str(), id_table[q.dst].name.c_str());
//		break;
//	case enum_sub:
//		printf("%s %s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(),
//			id_table[q.op2].name.c_str(), id_table[q.dst].name.c_str());
//		break;
//	case enum_mul:
//		printf("%s %s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(),
//			id_table[q.op2].name.c_str(), id_table[q.dst].name.c_str());
//		break;
//	case enum_div:
//		printf("%s %s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(),
//			id_table[q.op2].name.c_str(), id_table[q.dst].name.c_str());
//		break;
//	case enum_assign:
//		printf("%s %s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str(),
//			id_table[q.op2].name.c_str());
//		break;
//	case enum_print:
//		printf("%s %s\n", atom_op[(int)q.func].c_str(), id_table[q.op1].name.c_str());
//		break;
//	}
//
//}
//
//struct node {
//	id_type type;
//	int val, op1, op2;
//	bool is_leaf;
//	vector<int> to;
//	operation come_op;
//	node(id_type to_type, int to_val = 0) :
//		type(to_type), val(to_val) {}
//	node(id_type to_type, operation to_come_op, int to_op1 = -1, int to_op2 = -1, int to_val = 0) :
//		type(to_type), come_op(to_come_op), val(to_val), op1(to_op1), op2(to_op2) {}
//};

void sematic_parser::output()
{
	puts("-----* output *-----");

	for (int i = 0; i < form_list.size(); i++)
	{
		cout << V_list[form_list[i].left] << " ";
		for (auto j : form_list[i].right)
		{
			cout << V_list[j] << " ";
		}
		cout << endl;
	}

	puts("Form_List");
	for (int i = 0; i < form_list.size(); i++) {
		form temp = form_list[i];
		printf("%s->", V_list[temp.left].c_str());
		for (int j = 0; j < temp.right.size(); j++) {
			printf("%s", V_list[temp.right[j]].c_str());
		}
		printf("  $  ");
		for (auto j : temp.ops) {
			printf("%d  ", j);
		}
		for (auto j : temp.var) {
			int jv = j;
			if (jv == -1) {// 是左式
				printf("(L)%s ", V_list[temp.left].c_str());
			}
			else {// 是右式
					printf("(R)%s ", V_list[temp.right[j]].c_str());
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
	for (auto i : to_epsilon)
	{
		cout << V_list[i] << "  ";
	}
	puts("\n\n");

	puts("first_set");
	for (auto & s : first_set)
	{
		cout << V_list[s.first] << ": ";
		for (auto i : s.second)
		{
			cout << V_list[i] << " ";
		}
		puts("");
	}

	puts("item_clan");
	for (int i = 0; i < item_clan.size(); i++)
	{
		printf("item%d : \n", i);
		for (auto & j : item_clan[i])
		{
			cout << V_list[form_list[j.f].left] << "->";
			for (auto k : form_list[j.f].right)
			{
				cout << V_list[k];
			}
			cout << "  ";
			cout << " " << j.p << "  " << V_list[j.fs] << endl;
		}
		cout << endl;
	}

	puts("go & reverse");
	for (int i = 0; i < go.size(); i++)
	{
		printf("item%d:\n", i);
		puts("go:");
		for (auto & j : go[i])
		{
			cout << i << "  " << V_list[j.first] << "  " << j.second << endl;
		}
		puts("reverse:");
		for (auto & j : reverse[i])
		{
			cout << i << "  " << V_list[j.first] << "  ";
			printf("%s->", V_list[form_list[j.second].left].c_str());
			for (int i : form_list[j.second].right)
			{
				cout << V_list[i];
			}
			cout << endl;
		}
		cout << endl;
	}

	puts("DAG");
	int i = 0;
	for (auto &n : dag) {
		printf("[%d]", i);
		cout << "type:" << (n.type >= 0 && n.type <= 3 ? (id_type)n.type : -1) << "  come_op:" << n.come_op;
		printf(" is_leaf: %d, op1: %d, op2: %d, val: %d\n", (int)n.is_leaf, n.op1, n.op2, n.val);
		i++;
	}
	cout << endl;

	puts("id2node");
	for (auto i : id2node) {
		printf("[%d] -> %d\n", i.first, i.second);
	}

	cout << endl;
}



// 搜索s是否在V_LIST中，若不在则加入，返回s在V_LSIT中的编号
int sematic_parser::V_idx(const string & s, bool is_push)
{
	int i = 0;
	for (i = 0; i < V_list.size(); i++)
	{
		if (V_list[i] == s)
		{
			break;
		}
	}
	if (i == V_list.size() && is_push)
	{
		V_list.push_back(s);
		// 如果左右为<>则认为是VN
		if (s[0] == '<' &&s[max(int(s.size()) - 1, 0)] == '>')
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
void sematic_parser::find_epsilon()
{
	bool change = false;
	while (1)
	{
		change = false;
		for (int i = 0; i < V_list.size(); i++)
		{
			if (is_VT[i] == true || (is_VT[i] == false && to_epsilon.find(i) != to_epsilon.end()))
			{
				continue;
			}
			for (auto f : form_map[i])
			{
				bool flag = false;
				for (int j = 0; j < form_list[f].right.size(); j++)
				{
					//如果是Vt或者不指向epsilon的Vt跳过
					int idx = form_list[f].right[j];
					if (is_VT[idx] == true || (is_VT[idx] == false && to_epsilon.find(idx) == to_epsilon.end()))
					{
						break;
					}
					if (j == form_list[f].right.size() - 1)
					{
						flag = change = true;
						to_epsilon.insert(i);
					}

				}
				if (flag == true)
				{
					break;
				}
			}

		}
		if (change == false)
		{
			break;
		}

	}
}

// p 指向字符串读取数字的第一个位置
inline int sematic_parser::read_int(const string & s, int & p) {
	int ret = 0;
	bool t = false;
	while (s[p] < '0' || s[p] > '9') {
		p++;
	}
	if (s[max(p - 1, 0)] == '-')t = true;
	while (s[p] >= '0' && s[p] <= '9') {
		ret = ret * 10 + s[p++] - '0';
	}
	return t ? -ret : ret;
}

inline int sematic_parser::chk_dot(const string & s) {
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == '.')return i;
	}
	return -1;
}


// p 指向字符串读取数字的第一个位置
inline double sematic_parser::read_double(const string & s, int p) {
	bool t = true;
	double z = 0, b = 0.1;
	if (s[p] == '-') {
		t = false;
		p++;
	}
	while (s[p] >= '0' && s[p] <= '9') {
		z = z * 10 + s[p++] - '0';
	}
	p++;
	while (s[p] >= '0' && s[p] <= '9') {
		z += (s[p++] - '0') * b;
		b *= 0.1;
	}
	return t ? z : -z;
}

int sematic_parser::str2Num(const string  s)
{
	int ret = 0, p = 0;
	while (s[p] != 0)
	{
		ret *= 10;
		ret += s[p] - '0';
		p++;
	}
	return ret;
}

node_type sematic_parser::check_node_type(const string & s) 
{
	auto iter = node_type_map.find(s);
	//map<string, node_type>::iterator iter = node_type_map.find(s);
	if ( iter == node_type_map.end())return node_nop;
	return (*iter).second;
}

// 读取语法
void sematic_parser::read_syntax()
{ 
	string buf;
	// 预处理
	V_idx(end_s);
	V_idx(start_s);
	form_list.push_back(form(V_idx(start_s), { V_idx("<S>") }));

	while (getline(f1, buf)) {
		if (buf.size() == 0) {
			continue;
		}
		if (!buf[0] || buf[0] == '/' && buf[1] == '/')continue;
		int p = 0, pp = 0;
		int left;
		vector<int> right;
		//读取左部
		while (buf[p] != ' ' && buf[p]) {
			p++;
		}
		string temp = buf.substr(pp, p - pp);
		left = V_idx(temp);
		// 读取右部
		while (buf[p] != '$' && buf[p])
		{
			while (buf[p] == ' ')
			{
				p++;
			}
			pp = p;
			while (buf[p] != ' ' && buf[p] && buf[p] != '$')
			{
				p++;
			}
			if (p - pp == 0)
			{
				continue;
			}
			string temp = buf.substr(pp, p - pp);
			right.push_back(V_idx(temp));
			while (buf[p] == ' ')
			{
				p++;
			}
		}
		// 默认语法中的产生式不会重复
		// 如果右部没有东西，则为epsilon
		if (right.size() == 0)
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
		if (!buf[p]) continue;
		p++;
		int idx = form_list.size() - 1;
		bool clr_temp = true; // 忽略第一个操作符的返回值
		// 循环读取表达式
		while (1) {
			while (buf[p] == ' ')p++;
			if (!buf[p])break;
			int pp = p;
			while (buf[p] != ' ' && buf[p] != 0)p++;
			string temp = buf.substr(pp, p - pp); // 得到操作的字符串
			auto iter = node_type_map.find(temp);
			// 如果节点
			if (iter == node_type_map.end())
			{
				int _pointer = 0;
				for (_pointer = 0; temp[_pointer] != 0 && temp[_pointer] != '_'; _pointer++);
				if (_pointer == temp.size()) { //该V没有下划线
					if (temp == V_list[form_list[idx].left]) {
						form_list[idx].var.push_back(-1);
					}
					else {
						for (int i = 0; i < form_list[idx].right.size(); i++) {
							if (V_list[form_list[idx].right[i]] == temp) {
								form_list[idx].var.push_back(i);
								break;
							}
						}
					}
				}
				{ // 有下划线的情况
					int pp = _pointer + 1;
					string v1 = temp.substr(0, _pointer);
					if (v1[0] == '<') {
						v1 += string(">");
					}
					int num = read_int(temp, _pointer);
					for (int i = 0; i < form_list[idx].right.size(); i++) {
						if (v1 == V_list[form_list[idx].right[i]]) {
							num--;
							if (num == 0) {
								form_list[idx].var.push_back(i);
								break;
							}
						}
					}
				}
			}
			else form_list[idx].ops.push_back((*iter).second);
		}
	}


	find_epsilon();
	gen_first();
}

set<int> sematic_parser::union_first(const set<int> & a, const set<int> & b)
{
	set<int> c;
	set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(c, c.begin()));
	return c;
}

// 存在迭代，高危代码段
set<int> sematic_parser::form_first_set(int idx, set<int> & vis)
{
	set<int> ret;
	int t = form_list[idx].right[0];
	int p = 0, len = form_list[idx].right.size();
	if (is_VT[t] == true)
	{
		return set<int>{t};
	}
	else if (to_epsilon.find(t) != to_epsilon.end())
	{

		for (; p < len; p++)
		{
			if (form_list[idx].right[p] != 0 && to_epsilon.find(form_list[idx].right[p]) == to_epsilon.end())
			{
				break;
			}
		}
		if (p == len)
		{
			return set<int>{0};
		}
		else
		{
			t = form_list[idx].right[p];
			if (is_VT[t])
			{
				return set<int>{t};
			}
		}
	}

	// 接下来的这个t是一个不为空的Vn
	if (first_set[t].size() != 0)
	{
		ret = union_first(ret, first_set[t]);
	}
	else if (vis.find(t) == vis.end())
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
void sematic_parser::gen_first()
{
	//对每个Vn求firstset
	// first_set[1].insert(0); 说实话我看不懂为什么要写这句话
	for (int i = 0; i < V_list.size(); i++)
	{
		if (is_VT[i] == true)
		{
			continue;
		}
		set<int> temp;
		for (auto f : form_map[i])
		{
			//设置vis是为了切断循环 first
			set<int> vis{ i };
			temp = union_first(temp, form_first_set(f, vis));
		}
		first_set[i] = temp;

	}

}

// 求项目集
int sematic_parser::closure(vector<item> & vi, int fa)
{
	int p = 0;
	set<item> chk(vi.begin(), vi.end());
	// 对每个项目扩展
	while (p < vi.size())
	{
		// 当前项目的form
		form tf = form_list[vi[p].f];
		// 如果当前项目的.指向最后则continue
		if (vi[p].p == tf.right.size())
		{
			p++;
			continue;
		}
		// 当前原点右边的V
		int vv = tf.right[vi[p].p];
		// 如果是VT继续
		if (is_VT[vv] == true)
		{
			p++;
			continue;
		}
		// 当前.所指的位置
		int tp = vi[p].p;
		// 如果是Vn则要扩展
		// 提前计算出前向搜索符
		vector<int> fss;
		while (1)
		{
			tp++;
			if (tp == tf.right.size())
			{
				break;
			}
			if (is_VT[tf.right[tp]])
			{
				// 读取到VT则退出
				fss.push_back(tf.right[tp]);
				break;
			}
			bool to_e = false;
			for (auto first_c : first_set[tf.right[tp]])
			{
				if (first_c != 0)
				{
					fss.push_back(first_c);
				}
				else
				{
					to_e = true;
				}
			}
			if (to_e == false)
			{
				break;
			}
		}
		if (fss.size() == 0)
		{
			fss.push_back(vi[p].fs);
		}
		// 对Vn左部的form f
		for (auto f : form_map[vv])
		{
			for (auto first_c : fss)
			{
				item ti(f, 0, first_c);
				if (chk.find(ti) == chk.end())
				{
					chk.insert(ti);
					vi.push_back(ti);
				}

			}
		}
		p++;
	}
	bool flag = false;
	if (fa != -1)
	{
		int i = 0;
		for (i = 0; i < item_clan.size(); i++)
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
			if (flag == false)
			{
				return i;
			}
		}
	}
	item_clan.push_back(vi);
	return item_clan.size() - 1;


}

// action——goto表与项目集族一起产生
void sematic_parser::generate_clan()
{
	// 初始化
	item it0 = item(0, 0, 0);// <start>->.<S>, #加入项目集
	vector<item> vi{ it0 };
	closure(vi, -1);
	int p = 0;
	while (p < item_clan.size())
	{
		go.push_back(map<int, int>{});
		reverse.push_back(map<int, int>{});
		// 先得到规约
		for (auto & f : item_clan[p])
		{

			if (f.p == form_list[f.f].right.size() || form_list[f.f].right[0] == 0)
			{
				(reverse[p])[f.fs] = f.f;
			}
		}
		// 再得到go
		for (int i = 1; i < V_list.size(); i++)
		{
			vector<item> it;
			vector<int> goi;
			for (auto & f : item_clan[p])
			{
				if (f.p < form_list[f.f].right.size())
				{
					if (form_list[f.f].right[f.p] == i)
					{
						goi.push_back(i);
						it.push_back(item(f.f, f.p + 1, f.fs));
					}
				}
			}
			if (it.size() > 0)
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

void sematic_parser::parse_begin()
{
	a_stack.clear();
	s_stack.clear();
	V_stack.clear();
	s_stack.push_back(0);
	V_stack.push_back(0);
	symbolTable.clear();

}


 //TODO: 产生AST_NODE
AST_node sematic_parser::deal_expression(int id_num, int form_idx) 
{
	vector<AST_node> rf = vector<AST_node>(a_stack.end() - id_num, a_stack.end());

	 //将表达式输出为4元式
	AST_node tast;
	const form &tf = form_list[form_idx];
	int opp = tf.ops.size(), varp = tf.var.size();
	if(opp > 0)tast.type = tf.ops[0];
	else tast.type = node_nop;

	if (tast.type == node_func)
	{

		tast.name = rf[tf.var[1]].name;
		tast.dType = ASTTree.tree[rf[tf.var[0]].op[0]].dType;
		tast.syb = rf[tf.var[1]].syb;
		tast.syb->vType = tast.dType;
		tast.syb->hold_domain = symbolTable.prev_table;
#ifdef _DEBUG_
		//printf("%s type: %d \n", tast.name.c_str(), tast.dType);
#endif // _DEBUG_
		int fa = symbolTable.cur_table;
		// 将形参表中的标识符加入子领域
		// 这些东西写得特别死
		AST_node& tn = rf[tf.var[2]];
		if (tn.op.size())
		{
			tn = ASTTree.tree[tn.op[0]];
			while (1)
			{
				if (tn.op.size() == 3)
				{
					string& name = ASTTree.tree[tn.op[2]].name;
					symbol & tsb = (*(symbolTable.local_tables[fa]))[name];
					(*(symbolTable.local_tables[tast.syb->hold_domain]))[name] = tsb;
					tsb.not_in = symbolTable.cur_table;
					tsb.is_declare = true;
					tsb.vType = ASTTree.tree[ASTTree.tree[tn.op[1]].op[0]].dType;
					tn = ASTTree.tree[tn.op[0]];
				}
				else if (tn.op.size() == 2)
				{
					
					string& name = ASTTree.tree[tn.op[1]].name;
					symbol & tsb = (*(symbolTable.local_tables[fa]))[name];
					(*(symbolTable.local_tables[tast.syb->hold_domain]))[name] = tsb;
					tsb.not_in = symbolTable.cur_table;
					tsb.is_declare = true;
					tsb.vType = ASTTree.tree[ASTTree.tree[tn.op[0]].op[0]].dType; 
#ifdef _DEBUG_
					//printf("type trans : %d\n", tsb.vType);
#endif // _DEBUG_
					break;
				}
				else
				{
					break;
				}
			}
		}

	}
#ifdef _DEBUG_
	//for (auto & m : symbolTable.local_tables)
	//{
	//	symbol_table & mp = *m;
	//	printf("table size %d  ", mp.size());
	//	for (auto & p : mp)
	//	{
	//		for (auto & t : ASTTree.tree)
	//		{
	//			if (t.syb == &p.second) {
	//				printf("1_"); break;
	//			}
	//		}
	//		printf("%s ", p.first.c_str());
	//	}
	//	puts("");
	//}
	//for (auto idx : tf.var)
	//{	
	//	if (rf[idx].syb)
	//		cout << rf[idx].syb->data.ival << "  " ;
	//	printf("tf.var %s  ", rf[idx].name);
	//}
	//puts("");
#endif //_DEBUG_


	for (auto idx : tf.var)
	{
		tast.op.push_back(ASTTree.add_node(rf[idx]));
	}

	return tast;
}

symbol sematic_parser::read_COSNT(const string & s)
{
	symbol sb;
	sb.oType = CONSTANT;
	int p = 0;
	while(s[p] && s[p] != '.')p++;
	if (!s[p])
	{
		sb.vType = INT_TYPE;
		p = 0;
		sb.data.ival = read_int(s, p);
	}
	else
	{
		sb.vType = DOUBLE_TYPE;
		sb.data.dval = read_double(s, 0);
	}

	return sb;
}
symbol sematic_parser::read_VARIABLE(const string & s)
{
	symbol sb;
	sb.oType = VARIABLE;
	return sb;
}

// TODO处理parse，1.产生参数表 2.语法树 3.特殊处理某些输入
// 分析序列 返回错误类型，或者接受 -1程序错误 0接受 1不识别的Vt 2拒绝 3处理中 4标识符类型错误
int sematic_parser::parse_Vt(string  s, string Vt, string type)
{


	int idx = V_idx(s, false);
	if (idx == V_list.size()) // 未识别Vt
	{
		return 1;
	}
	int back = s_stack.back();
	// 如果是{则需要加深一层symbol_table
	if (Vt == "{")
	{
#ifdef _DEBUG_
		//printf("{  %lld\n", symbolTable.cur_table);
#endif // _DEBUG_
		symbolTable.add_table(symbolTable.cur_table);
	}
	// 程序可读性极差
	if (go[back].find(idx) != go[back].end()) // 转移
	{
		
		// 加入symbol_table
		AST_node ta;
		if (type == "常量")
		{
			symbol sb = read_COSNT(Vt);
			ta.name = Vt;
			ta.syb = symbolTable.add_symbol(Vt, sb);
		}
		else if (type == "标识符")
		{
			symbol sb = read_VARIABLE(Vt);
			ta.name = Vt;
			ta.syb = symbolTable.add_symbol(Vt, sb);
		}
		else if (Vt == "int") // 在最初将dtype加上
		{
			ta.dType = INT_TYPE;
		}
		else if (Vt == "double")
		{
			ta.dType = DOUBLE_TYPE;
		}
		else if (Vt == "void")
		{
			ta.dType = VOID_TYPE;
		}
		a_stack.push_back(ta);
		V_stack.push_back(idx);
		s_stack.push_back(go[back][idx]);
		return 3;
	}
	else if (reverse[back].find(idx) != reverse[back].end()) // 规约
	{


		int form_idx = reverse[back][idx];
		int cnt = form_list[form_idx].right.size();
		// 特判空产生式
		if (form_list[reverse[back][idx]].right[0] == 0)
		{
			cnt = 0;
		}
		AST_node tast = deal_expression(cnt, form_idx); 
		while (cnt--)
		{
			a_stack.pop_back();
			s_stack.pop_back();
			V_stack.pop_back();
		}
		a_stack.push_back(tast);
		V_stack.push_back(form_list[form_idx].left); 
		if (V_stack.size() == 2 && V_stack.back() == 1)
		{
			return 0;
		}
		int tl = s_stack.size();
		s_stack.push_back(go[s_stack[tl - 1]][form_list[form_idx].left]);
#ifdef _DEBUG_
		//for (auto & s : s_stack)
		//{
		//	cout << s;
		//}
		//puts("");
		//for (auto & v : V_stack)
		//{
		//	cout << V_list[v];
		//}
		//puts("\n");
#endif
		int ret = parse_Vt(s, Vt, type);
		// 规约不会消耗下一个符号
		return ret;
	}
	else // 输入错误，拒绝接受
	{
		return 2;
	}
	return -1;
}

// 读取TASK1的输出
// TODO给出错误的原因
int sematic_parser::parse_code()
{
	string buf;
	// 开始处理，初始化，感觉有点类似openGL
	parse_begin();
	int result;
	bool have_main = false;
	int line_idx;
	string type, Vt;
	while (getline(f2, buf))
	{
		int bp = 0, pp = 0;
		// 读取行号
		while (buf[bp] != ' ')
		{
			bp++;
		}
		line_idx = str2Num(buf.substr(pp, bp - pp));

		// 读取Vt
		pp = ++bp;
		while (buf[bp] != ' ')
		{
			bp++;
		}
		Vt = buf.substr(pp, bp - pp);
		// 读取词性
		pp = ++bp;
		while (buf[bp] != 0)
		{
			bp++;
		}
		type = buf.substr(pp, bp - pp);
		// 如果Vt是关键字，op，界符，限定词，则将原来的符号扔进去
		if (type == "关键词" || type == "界符")
		{
			if (Vt == "main")
			{
				if (have_main == true)
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
				if (Vt == "}")
				{
#ifdef _DEBUG_
					//printf("} %lld, %lld\n", symbolTable.cur_table, symbolTable.fa[symbolTable.cur_table]);
#endif // _DEBUG_

					symbolTable.prev_table = symbolTable.cur_table;
					symbolTable.cur_table = symbolTable.fa[symbolTable.cur_table];
				}
			}

		}
		else if (type == "标识符")
		{
			result = parse_Vt(type, Vt, type);
		}
		else  if (type == "操作符")
		{
			result = parse_Vt(Vt, Vt, type);

		}
		else  if (type == "常量" || type == "限定符")// 如果是常数 标识符 则把type扔进去
		{
			result = parse_Vt(type, Vt, type);
		}
		else
		{
			result = 1;
		}

		if (result != 3)
		{
			if (result == 1)
			{
				printf("未识别的符号:%s\n", Vt.c_str());
			}
			break;
		}
#ifdef _DEBUG_
		//cout << line_idx << "  " << result << endl;
		//for (auto & s : s_stack)
		//{
		//	cout << s;
		//}
		//puts("");
		//for (auto & v : V_stack)
		//{
		//	cout << V_list[v];
		//}
		//puts("\n");
#endif

	}
	// printf("111\n");
	if (result != 3) 
	{
		cout << "NO:\nline:" << line_idx << "  " << Vt << endl;
		if (result != 1) 
		{
			if (result == 4)
			{
				puts("多个main");
			}
			printf("可能的错误: %s或%s\n", V_list[V_stack.back()].c_str(), Vt.c_str());
		}
		
	}
	else {
		result = parse_Vt(end_s, end_s, "end_type");
		if (result != 0) 
		{
			cout << "NO:\nline:" << line_idx << "  " << Vt << endl;
		}
		else
		{
			printf("YES!\n");
		}
		
	}

	cout << endl;
	
#ifdef _DEBUG_
	//for (auto & s : s_stack)
	//{
	//	cout << s;
	//}
	//puts("");
	//for (auto & v : V_stack)
	//{
	//	cout << V_list[v];
	//}
	//puts("\n");
#endif
	// cout << result << endl;
	return result;
}

// 产生TAC的过程极度暴力
// TODO DEBUG dsf_AST
int sematic_parser::dfs_AST(int idx)
{
#ifdef _DEBUG_
	//printf("dfs_ast: idx: %d\n", idx);
#endif // _DEBUG_

	AST_node & tn = ASTTree.tree[idx];


	int ret_flag = 0;
	if (tn.type == node_declare)
	{
		valueType vt = ASTTree.tree[ASTTree.tree[tn.op[0]].op[0]].dType;
#ifdef _DEBUG_
		//printf("declare_type: %d\n", vt);
#endif // _DEBUG_
		ret_flag = AST_node::get_declared(tn, vt, ASTTree.tree);
	}
	// 遍历子树
	if (!ret_flag)
	{
		for (auto to : tn.op)
		{
			ret_flag = dfs_AST(to);
			if (ret_flag)break;
		}
		// 如果syb不为空
		tn.call(ASTTree.tree);
	}
	


	return ret_flag;
}



// 将抽象语法树转化为四元式
int sematic_parser::AST2TAC()
{
	int mp = 0, cnt = 0, ret = 0;
	// 找到main函数入口
	for(;mp < ASTTree.tree.size(); mp++)
	{ 

#ifdef _DEBUG_
		//if (ASTTree.tree[mp].name.size())
		//	printf("%d %s %d\n", mp, ASTTree.tree[mp].name.c_str(), ASTTree.tree[mp].type);
#endif // _DEBUG_
		//if (ASTTree.tree[mp].name == "main" && ASTTree.tree[mp].type == node_func)break;
		if (ASTTree.tree[mp].type == node_func)
		{
			if (ASTTree.tree[mp].name == "main")
			{
				cnt++;
			}
			ret = dfs_AST(mp);
			ASTTree.tree[mp].show_code();
		}

	}

#ifdef _DEBUG_
	//printf("tree size %d\n", ASTTree.tree.size());
	//printf("ASTTree.tree[%d].name %s\n", mp, ASTTree.tree[mp].name.c_str());

	//for (auto & m : symbolTable.local_tables)
	//{
	//	symbol_table & mp = *m;
	//	printf("table size %d  ", mp.size());
	//	for (auto & p : mp)
	//	{
	//		for (auto & t : ASTTree.tree)
	//		{
	//			if (t.syb == &p.second) {
	//				printf("1_"); break;
	//			}
	//		}
	//		printf("%s ", p.first.c_str());
	//	}
	//	puts("");
	//}
	//assert(mp != ASTTree.tree.size());
#endif // _DEBUG_

	//if (mp == ASTTree.tree.size())
	//{
	//	ret = dfs_AST(mp);
	//	ASTTree.tree[mp].show_code();
	//}
	if(!cnt)
	{
		puts("缺少main函数作为入口");
		return -1;
	}
	else if (cnt > 1)
	{
		puts("存在多个main函数作为入口");
	}
#ifdef _DEBUG_
	puts("AST2TAC end");
#endif // _DEBUG_
	return ret;
}


//void sematic_parser::end()
//{
//	f1.close();
//	f2.close();
//}
//
//int sematic_parser::_add_const_var_leaf_(int val) {
//	node tn = node(const_var, val);
//	tn.is_leaf = true;
//	int idx = _chk_in_dag_(tn);
//	if (idx == -1) {
//		idx = dag.size();
//		dag.push_back(tn);
//	}
//	return idx;
//}
//
//int sematic_parser::_chk_in_dag_(node n) {
//	if (n.type == const_var) {
//		for (int i = 0; i < dag.size(); i++) {
//			if (dag[i].type == const_var && dag[i].val == n.val) {
//				return i;
//			}
//		}
//	}
//	// 如果是变量的话 直接忽略
//	return -1;
//}
//
//int sematic_parser::_add_ident_leaf(int idx) {
//	int ret = -1;
//	if (id2node.find(idx) == id2node.end()) {// 是之前没有出现过的变量
//		id2node[idx] = ret = dag.size();
//		node tn = node(ident, enum_assign, idx);
//		tn.is_leaf = true;
//		dag.push_back(tn);
//	}
//	else {
//		ret = id2node[idx];
//	}
//	return ret;
//}
//
//int sematic_parser::_add_ident_node_(node n) {
//	int idx = 0;
//	for (auto & tn : dag) {
//		if (tn.come_op == n.come_op && tn.op1 == n.op1 && tn.op2 == n.op2) {
//			break;
//		}
//		idx++;
//	}
//	if (idx == dag.size()) {
//		n.is_leaf = false;
//		dag.push_back(n);
//		if (n.op1 != -1)dag[n.op1].to.push_back(idx); // 加入to，方便之后的拓扑排序
//		if (n.op2 != -1)dag[n.op2].to.push_back(idx);
//	}
//	return idx;
//}
//
//void sematic_parser::_optim_assign_(const quadra_tuple & q) {
//	int idx1 = id_table[q.op1].type == ident ? _add_ident_leaf(q.op1) :
//		_add_const_var_leaf_(id_table[q.op1].val);
//	id2node[q.op2] = idx1;
//}
//
//void sematic_parser::_optim_op_(const quadra_tuple & q) {
//	int idx1 = id_table[q.op1].type == ident ? _add_ident_leaf(q.op1) :
//		_add_const_var_leaf_(id_table[q.op1].val)
//		, idx2 = id_table[q.op1].type == ident ? _add_ident_leaf(q.op2) :
//		_add_const_var_leaf_(id_table[q.op2].val);
//	if (dag[idx1].type == const_var && dag[idx2].type == const_var) {
//		//如果op1和op2都是常量, dst一定是变量，所以直接拉到add的结果节点上
//		int val;
//		if (q.func == enum_add)val = dag[idx1].val + dag[idx2].val;
//		else if (q.func == enum_sub)val = dag[idx1].val - dag[idx2].val;
//		else if (q.func == enum_mul)val = dag[idx1].val * dag[idx2].val;
//		else val = dag[idx1].val / dag[idx2].val;
//		int dst = _add_const_var_leaf_(val);
//		id2node[q.dst] = dst;
//	}
//	else {
//		node tn = node(ident, q.func, idx1, idx2);
//		int idx2 = _add_ident_node_(tn);
//		id2node[q.dst] = idx2; // 将dst加入映射
//	}
//}
//
//void sematic_parser::_optim_print_(const quadra_tuple & q) {
//	// 无需优化，直接加入即可
//	int idx1 = id_table[q.op1].type == ident ? _add_ident_leaf(q.op1) :
//		_add_const_var_leaf_(id_table[q.op1].val);
//	node tn = node(ident, enum_print, idx1);
//	_add_ident_node_(tn);
//}
//
//// 优化函数(main)
//void sematic_parser::optim() {
//	for (int i = 0; i < quadra_tuple_list.size(); i++) {
//		switch (quadra_tuple_list[i].func) {
//		case enum_assign:
//			_optim_assign_(quadra_tuple_list[i]);
//			break;
//		case enum_add:
//		case enum_sub:
//		case enum_mul:
//		case enum_div:
//			_optim_op_(quadra_tuple_list[i]);
//			break;
//		case enum_print:
//			_optim_print_(quadra_tuple_list[i]);
//			break;
//		}
//	}
//	gen_optimized_quadra_tuple_list();
//}
//
//int sematic_parser::add_const_id(int val) {
//	string name = num2str(val);
//	int ret = add_id(name, const_var);
//	return ret == -1 ? id_table.size() - 1 : ret;
//}
//
//inline string sematic_parser::num2str(int num) {
//	string temp;
//	stringstream ss;
//	ss << num;
//	ss >> temp;
//	return temp;
//}
//
//int sematic_parser::get_node_id(int node_idx, vector<vector<int>> & node2id) {// 查询idx号node对应的id的值
//	int ret = -1;
//	if (node2id[node_idx].size() == 0) {// 如果该节点还没有加入id_table, 则加入,而且这一定是const_var
//		if (dag[node_idx].type == const_var) {
//			ret = add_const_id(dag[node_idx].val);
//			node2id[node_idx].push_back(ret);
//		}
//		else {// 叶子节点的标识符
//		//TODO 解决叶子结点标识符的问题，以及temp标识符的问题
//			ret = dag[node_idx].op1;
//		}
//		node2id[node_idx].push_back(ret);
//	}
//	else {
//		// 如果是常量，则输出常量
//		// 这个常量还是放在最后一个的
//		// 是变量型节点, 将最后一个取出来当做是该节点的
//		//合并这两种情况得
//		ret = node2id[node_idx].back();
//	}
//	return ret;
//}
//
//// 将单个节点转化为4元组
//void sematic_parser::node2quadra_tuple(int idx, vector<vector<int>> & node2id) {
//	const node &tn = dag[idx];
//	if (node2id[idx].size() == 0 && dag[idx].type == const_var) { // 该节点没有标识符，是常量节点，则不管
//	}
//	else if (tn.type == const_var) { // 如果是常量且有标识符的节点，则可以认为是赋值
//		int idx2 = add_const_id(tn.val);
//		for (auto i : node2id[idx]) {
//			// 通过赋值，将标识符赋值
//			optimized_quadra_tuple_list.push_back(quadra_tuple(enum_assign, i, idx2));
//		}
//		node2id[idx].push_back(idx2);
//	}
//	// 如果是非叶子结点的标识符
//	// else if(tn.is_leaf == false && tn.type == ident){
//	else if (tn.type == ident) {
//		// 这里不会为两个常量，因为常量在之前已近合并了
//		switch (tn.come_op) {
//		case enum_add:
//		case enum_sub:
//		case enum_mul:
//		case enum_div: {
//			int node_idx1 = tn.op1, node_idx2 = tn.op2;
//			int op1 = get_node_id(node_idx1, node2id), op2 = get_node_id(node_idx2, node2id);
//			int dst = 0;
//			if (node2id[idx].size() == 0) {
//				node2id[idx].push_back(add_temp());
//			}
//			optimized_quadra_tuple_list.push_back(quadra_tuple(tn.come_op, op1, op2, node2id[idx][dst++]));// 就第一个执行运算，其他均为赋值操作
//			for (; dst < node2id[idx].size(); dst++) {
//				optimized_quadra_tuple_list.push_back(quadra_tuple(enum_assign, node2id[idx][0], node2id[idx][dst++]));
//			}
//			break;
//		}
//		default: // 没有操作符的就是外来的标识符
//		case enum_assign: {
//			// 这里认为不可能是$temp，因为$temp没有被赋值过，不能作为建立node的基础
//			int op1 = tn.op1;
//			int op2 = 0;
//			for (; op2 < node2id[idx].size(); op2++) {
//				if (node2id[idx][op2] != op1)
//					optimized_quadra_tuple_list.push_back(quadra_tuple(enum_assign, op1, node2id[idx][op2++]));
//			}
//			break;
//		}
//		case enum_print: {
//			int op1 = get_node_id(tn.op1, node2id);
//			optimized_quadra_tuple_list.push_back(quadra_tuple(enum_print, op1));
//			break;
//		}
//		}
//	}
//}
//
//// TODO优化之后使用拓扑排序直接输出优化后的TAC
//void sematic_parser::sematic_parser::gen_optimized_quadra_tuple_list() {
//	queue<int> que;
//	bool * vis = new bool[dag.size()];
//	vector<vector<int>>  node2id(dag.size());
//	Vn_reg_cnt = 0; // 借用add_temp函数
//	for (auto i : id2node) {
//		if (id_table[i.first].name[0] != '$')
//			node2id[i.second].push_back(i.first);
//	}
//	for (int i = 0; i < dag.size(); i++) {
//		if (dag[i].is_leaf)que.push(i);
//		vis[i] = false;
//	}
//	while (que.empty() == false) {
//		int idx = que.front(); que.pop();
//		if (vis[idx])continue;
//		vis[idx] = true;
//		for (auto i : dag[idx].to) {
//			if (vis[i] == false) {
//				que.push(i);
//			}
//		}
//		node2quadra_tuple(idx, node2id);
//	}
//	puts("optimzed_quadra_tuple_list");
//	print_quadra_tuple_list(optimized_quadra_tuple_list);
//	delete[]vis;
//}
//

	
	
	
	
	
	
//
//int main()
//{
//
//#ifdef m_
//	// freopen("output.txt","w",stdout);
//	// freopen("rjks/task1/token_grammar.txt", "r", stdin);
//	// freopen("input.txt","r",stdin);
//	freopen("output.txt", "w", stdout);
//#endif
//	string file_path = "rjks/task3/sematic.txt";
//	// string file_path = "input.txt";
//	string code_path = "rjks/task3/token_list.txt";
//	sematic_parser sp(file_path, code_path);
//	clock_t t1, t2, t3, t4;
//	// t1 = clock();
//	sp.read_syntax();
//	// t2 = clock();
//	sp.generate_clan();
//	// t3 = clock();
//	// sp.output();
//
//	sp.parse_code();
//	sp.optim();
//	// t4 = clock();
//	sp.output();
//	// sp.end();
//#ifdef m_
//	fclose(stdin);
//	fclose(stdout);
//#endif
//	return 0;
//}

