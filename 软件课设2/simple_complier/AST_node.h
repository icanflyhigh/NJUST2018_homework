#pragma once
#include <vector>
#include <string>
#include "TAC.h"
#include "Type.h"
using namespace std;



class AST_node
{
public:
static int temp_node_cnt;
	node_type type; // 节点类型
	vector<int> op; // 子节点的指针
	vector<TAC> code; // 代码
	string name=""; // 节点的名字
	symData tempData;// 暂存数据
	valueType dType;
	symbol * syb = NULL;
	static int get_declared(AST_node& tn, valueType vt, vector<AST_node> & T, bool isleft=false);
	int __function__(vector<AST_node> & T);
	int __nop__(vector<AST_node> & T);
	int __assign__(vector<AST_node> & T);
	int __cal_op__(vector<AST_node> & T, node_type nt);
	int __declare__(vector<AST_node> & T);
	int __if__(vector<AST_node> & T);
	int __while__(vector<AST_node> & T);
	void call(vector<AST_node>&);
	bool type_check(vector<AST_node>&);

	void show_code();
};

class AST_tree
{
public:
	
	vector<AST_node> tree;
	int root=-1;
	int add_node(AST_node &); // 将节点加入树中
};

