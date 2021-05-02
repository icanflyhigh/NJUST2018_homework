#include "symbol.h"
using namespace std;

// TODO:�޸����е�symbol*
symtab::symtab():cur_table(0), global_table(0)
{
	//local_tables.push_back(symbol_table());
}

symbol* symtab::add_symbol(string & str, symbol &sb, int dom)
{
#ifdef _DEBUG_
	//for (auto f : fa)
	//{
	//	cout << f.first << "  " << f.second << endl;
	//}
#endif // _DEBUG_

	int  tidx =  cur_table;
	// ���Ѱ�ұ�־
	int cnt = 0;
	if (dom == -1)
	{
		while (1)
		{
			symbol_table & st = *local_tables[tidx];
			auto iter = st.find(str);
			if (iter != st.end() && iter->second.not_in != tidx)
			{
				return &((*iter).second);
			}
			cnt++;
			auto fiter = fa.find(tidx);
			// ���û�и��ף���Ϊ���ڵ�
			if (fiter == fa.end())break;
		
			tidx = (fiter->second);
			if (cnt > 100 ) 
			{
				cout << fiter->second << endl;
				for (auto f : fa)
				{
					cout << f.first << "  " << f.second << endl;
				}
				puts("Ѱ�ҷ��ű����");
				//system("pause");

			}
		}
	}
	(*local_tables[cur_table])[str] = sb;
	(*local_tables[cur_table])[str].domain = cur_table;
	(*local_tables[cur_table])[str].level = cnt;
	return &(*local_tables[cur_table])[str];
	
}

void symtab::add_table(int fa_table)
{
	int pst = local_tables.size();
	local_tables.push_back(new symbol_table());
	fa[pst] = fa_table;
	prev_table = cur_table;
	cur_table = pst;


}

void symtab::clear()
{
	global_table = 0;
	cur_table = 0;
	// ��ȷ�������᲻�����ڴ�й©
	for (auto & p : local_tables)
	{
		delete p;
	}
	fa.clear();
	local_tables.clear();
	local_tables.push_back(new symbol_table());
}
