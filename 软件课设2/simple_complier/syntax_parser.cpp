#include "syntax_parser.h"
// author: M@
#define _DEBUG_
#include<iostream>
#include <iterator>
#include<cstdlib>
#include<cstdio>
#include<cstdlib>
#include<string.h>
#include<queue>
#include<vector>
#include<map>
#include<algorithm>
#include<set>
#include<time.h>
#include<string>
#include<assert.h>
using namespace std;
/*

�﷨������

*/

syntax_parser::syntax_parser(string syntax_path, string code_path)
{
	f1.open(syntax_path, std::ios::in);
	f2.open(code_path, std::ios::in);
}

void syntax_parser::output()
{
	puts("-----* output *-----");

	for (int i = 0; i < form_list.size(); i++)
	{
		cout << V_list[form_list[i].left] << " ";
		for (auto j : form_list[i].right)
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

	cout << endl;
}

// ����s�Ƿ���V_LIST�У�����������룬����s��V_LSIT�еı��
int syntax_parser::V_idx(const string & s, bool is_push)
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
		// �������Ϊ<>����Ϊ��VN
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

// Ѱ�Ҳ����յ�Vn
void syntax_parser::find_epsilon()
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
					//�����Vt���߲�ָ��epsilon��Vt����
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

// ��ȡ�﷨
void syntax_parser::read_syntax()
{
	string buf;
	// Ԥ����
	V_idx(end_s);
	V_idx(start_s);
	form_list.push_back(form(V_idx(start_s), { V_idx("<S>") }));

	while (getline(f1, buf))
	{
		if (buf.size() == 0)
		{
			continue;
		}
		int p = 0, pp = 0;
		int left;
		vector<int> right;
		//��ȡ��
		while (buf[p] != ' ' &&buf[p] != 0)
		{
			p++;
		}
		string temp = buf.substr(pp, p - pp);
		left = V_idx(temp);
		// ��ȡ�Ҳ�
		while (buf[p] != 0)
		{
			while (buf[p] == ' ')
			{
				p++;
			}
			pp = p;
			while (buf[p] != ' ' && buf[p] != 0)
			{
				p++;
			}
			if (p - pp == 0)
			{
				continue;
			}
			string temp = buf.substr(pp, p - pp);
			right.push_back(V_idx(temp));
		}
		// Ĭ���﷨�еĲ���ʽ�����ظ�
		// ����Ҳ�û�ж�������Ϊepsilon
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


	}


	find_epsilon();
	gen_first();
}

set<int> syntax_parser::union_first(const set<int> & a, const set<int> & b)
{
	set<int> c;
	set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(c, c.begin()));
	return c;
}

// ���ڵ�������Σ�����
set<int> syntax_parser::form_first_set(int idx, set<int> & vis)
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

	// �����������t��һ����Ϊ�յ�Vn
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

// ��FIRST
void syntax_parser::gen_first()
{
	//��ÿ��Vn��firstset
	// first_set[1].insert(0); ˵ʵ���ҿ�����ΪʲôҪд��仰
	for (int i = 0; i < V_list.size(); i++)
	{
		if (is_VT[i] == true)
		{
			continue;
		}
		set<int> temp;
		for (auto f : form_map[i])
		{
			//����vis��Ϊ���ж�ѭ�� first
			set<int> vis{ i };
			temp = union_first(temp, form_first_set(f, vis));
		}
		first_set[i] = temp;

	}

}

// ����Ŀ��
int syntax_parser::closure(vector<item> & vi, int fa)
{
	int p = 0;
	set<item> chk(vi.begin(), vi.end());
	// ��ÿ����Ŀ��չ
	while (p < vi.size())
	{
		// ��ǰ��Ŀ��form
		form tf = form_list[vi[p].f];
		// �����ǰ��Ŀ��.ָ�������continue
		if (vi[p].p == tf.right.size())
		{
			p++;
			continue;
		}
		// ��ǰԭ���ұߵ�V
		int vv = tf.right[vi[p].p];
		// �����VT����
		if (is_VT[vv] == true)
		{
			p++;
			continue;
		}
		// ��ǰ.��ָ��λ��
		int tp = vi[p].p;
		// �����Vn��Ҫ��չ
		// ��ǰ�����ǰ��������
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
				// ��ȡ��VT���˳�
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
		// ��Vn�󲿵�form f
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

// action����goto������Ŀ����һ�����
void syntax_parser::generate_clan()
{
	// ��ʼ��
	item it0 = item(0, 0, 0);// <start>->.<S>, #������Ŀ��
	vector<item> vi{ it0 };
	closure(vi, -1);
	int p = 0;
	while (p < item_clan.size())
	{
		go.push_back(map<int, int>{});
		reverse.push_back(map<int, int>{});
		// �ȵõ���Լ
		for (auto & f : item_clan[p])
		{

			if (f.p == form_list[f.f].right.size() || form_list[f.f].right[0] == 0)
			{
				(reverse[p])[f.fs] = f.f;
			}
		}
		// û����epsilon
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

// �������� ���ش������ͣ����߽��� 0���� 1��ʶ���Vt 2�ܾ�
int syntax_parser::parse(vector<string> & s)
{
	s.push_back(V_list[0]);
	s_stack.clear();
	V_stack.clear();
	s_stack.push_back(0);
	V_stack.push_back(0);
	int p = 0;
	while (p < s.size())
	{
		int idx = V_idx(s[p], false);
		if (idx == V_list.size()) // δʶ��Vt
		{
			return 1;
		}
		int back = s_stack.back();
		if (go[back].find(idx) != go[back].end())
		{
			V_stack.push_back(idx);
			s_stack.push_back(go[back][idx]);
			p++;
		}
		else if (reverse[back].find(idx) != reverse[back].end())
		{

			int form_idx = reverse[back][idx];
			int cnt = form_list[form_idx].right.size();
			// ���пղ���ʽ
			if (form_list[reverse[back][idx]].right[0] == 0)
			{
				cnt = 0;
			}
			while (cnt--)
			{
				s_stack.pop_back();
				V_stack.pop_back();
			}
			V_stack.push_back(form_list[form_idx].left);
			if (V_stack.size() == 2 && V_stack.back() == 1)
			{
				return 0;
			}
			int tl = s_stack.size();
			s_stack.push_back(go[s_stack[tl - 1]][form_list[form_idx].left]);
		}
		else // ������󣬾ܾ�����
		{
			return 2;
		}
#ifdef _DEBUG_
		for (auto & s : s_stack)
		{
			cout << s;
		}
		puts("");
		for (auto & v : V_stack)
		{
			cout << V_list[v];
		}
		puts("\n");
#endif
	}

	return 0;
}

void syntax_parser::parse_begin()
{
	s_stack.clear();
	V_stack.clear();
	s_stack.push_back(0);
	V_stack.push_back(0);
	Variable.clear();
}
// �������� ���ش������ͣ����߽��� -1������� 0���� 1��ʶ���Vt 2�ܾ� 3������ 
int syntax_parser::parse_Vt(string  s)
{
	int idx = V_idx(s, false);
	if (idx == V_list.size()) // δʶ��Vt
	{
		return 1;
	}
	int back = s_stack.back();
	if (go[back].find(idx) != go[back].end())
	{
		V_stack.push_back(idx);
		s_stack.push_back(go[back][idx]);
		return 3;
	}
	else if (reverse[back].find(idx) != reverse[back].end())
	{

		int form_idx = reverse[back][idx];
		int cnt = form_list[form_idx].right.size();
		// ���пղ���ʽ
		if (form_list[reverse[back][idx]].right[0] == 0)
		{
			cnt = 0;
		}
		while (cnt--)
		{
			s_stack.pop_back();
			V_stack.pop_back();
		}
		V_stack.push_back(form_list[form_idx].left);
		if (V_stack.size() == 2 && V_stack.back() == 1)
		{
			return 0;
		}
		int tl = s_stack.size();
		s_stack.push_back(go[s_stack[tl - 1]][form_list[form_idx].left]);
		// ��Լ����������һ������
		return parse_Vt(s);
	}
	else // ������󣬾ܾ�����
	{
		return 2;
	}
	return -1;
}

int syntax_parser::str2Num(const string  s)
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

// ��ȡTASK1�����
// TODO���������ԭ��
void syntax_parser::parse_code()
{
	string buf;
	// ��ʼ������ʼ�����о��е�����openGL
	parse_begin();
	int result;
	bool have_main = false;
	int line_idx;
	string type, Vt;
	while (getline(f2, buf))
	{
#ifdef _DEBUG_
		printf("%s\n", buf.c_str());
#endif // _DEBUG_

		int bp = 0, pp = 0;
		// ��ȡ�к�
		while (buf[bp] != ' ')
		{
			bp++;
		}
		line_idx = str2Num(buf.substr(pp, bp - pp));

		// ��ȡVt
		pp = ++bp;
		while (buf[bp] != ' ')
		{
			bp++;
		}
		Vt = buf.substr(pp, bp - pp);
		// ��ȡ����
		pp = ++bp;
		while (buf[bp] != 0)
		{
			bp++;
		}
		type = buf.substr(pp, bp - pp);
		// ���Vt�ǹؼ��֣�op��������޶��ʣ���ԭ���ķ����ӽ�ȥ
		if (type == "�ؼ���" || type == "���")
		{
			if (Vt == "main")
			{
				if (have_main == true)
				{
					// 4���ֶ��main
					result = 4;
				}
				else
				{
					have_main = true;
					result = parse_Vt("��ʶ��");
				}
			}
			else
			{
				result = parse_Vt(Vt);
			}
		}
		else if (type == "��ʶ��")
		{
			result = parse_Vt(type);
		}
		else  if (type == "������")
		{

			result = parse_Vt(Vt);


		}
		else  if (type == "����" || type == "�޶���")// ����ǳ��� ��ʶ�� ���type�ӽ�ȥ
		{
			result = parse_Vt(type);
		}
		else
		{
			result = 1;
		}

		if (result == 2)
		{
			break;
		}
#ifdef _DEBUG_
		cout << line_idx << "  " << result << endl;
		for (auto & s : s_stack)
		{
			cout << s;
		}
		puts("");
		for (auto & v : V_stack)
		{
			cout << V_list[v];
		}
		puts("\n");
#endif

	}
	result = parse_Vt(end_s);
	cout << result << endl;
	if (result != 0)
	{
		cout << "ERROR:\n line:" << line_idx << "  " << Vt << endl;
	}
#ifdef _DEBUG_
	for (auto & s : s_stack)
	{
		cout << s;
	}
	puts("");
	for (auto & v : V_stack)
	{
		cout << V_list[v];
	}
	puts("\n");
#endif

	// cout << result << endl;
}

void syntax_parser::end()
{
	f1.close();
	f2.close();
}


