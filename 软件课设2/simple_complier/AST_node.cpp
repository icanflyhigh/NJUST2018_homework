#include "AST_node.h"
#include "Type.h"



int AST_node::temp_node_cnt = 0;
int TAC::tag_cnt = 0;



void AST_node::show_code()
{
	puts("");
	for (auto& t : code)
	{
		t.show();
	}
}
// 因为实现的原因，需要在dfs刚到的时候用，一个比较危险的函数
int AST_node::get_declared(AST_node& tn, valueType vt, vector<AST_node> & T, bool isleft)
{
	// 如果是标识符
	if (tn.syb && tn.syb->oType == VARIABLE && isleft)
	{
#ifdef _DEBUG_
		//printf("declare %s type %d\n", tn.name.c_str(), vt);
#endif // _DEBUG_
		if (tn.syb->is_declare) 
		{ 
			printf("重复声明%s。\n", tn.name.c_str());
			tn.type = node_error; return -1;
		}
		else
		{
			tn.syb->is_declare = true;
			tn.syb->vType = vt;
		}
	}
	// 很不负责的一句话，因为bug有点不好修复
	tn.dType = vt;
	for (int j = 0; j < tn.op.size(); j++)
	{
#ifdef _DEBUG_
		//printf("to %d\n", tn.op[j]);
#endif // _DEBUG_

		if (tn.type == node_assign && !j || isleft)
		{
			if (get_declared(T[tn.op[j]], vt, T, true))
			{
				return -1;
			}
		}
		else if (get_declared(T[tn.op[j]], vt, T))
		{
			return -1;
		}
	}
	return 0;
}

int AST_node::__declare__(vector<AST_node> & T)
{
	for (auto idx : op)
	{
		code.insert(code.end(),
			T[idx].code.begin(), T[idx].code.end());
	}

	if (!type_check(T))
	{
		type = node_error;
		printf("类型不匹配 %s\n", name.c_str());
		return -1;
	}	
	dType = T[0].dType;
	return 0;
}

int AST_node::__function__(vector<AST_node> & T)
{
	TAC c;
	c.type = TAG;
	c.tag = "tag: " + name;
	code.push_back(c);
	code.insert(code.end(),
			T[op[3]].code.begin(), T[op[3]].code.end());
	dType = T[T[op[0]].op[0]].dType;
	syb->vType = dType;
	return 0;
}



int AST_node::__if__(vector<AST_node> & T)
{
	name = T[op[0]].name;
	TAC c;
	string false_tag = "tag: " + to_string(TAC::tag_cnt++);
	code.insert(code.end(), T[op[0]].code.begin(), T[op[0]].code.end());//if判断块

	c.type = BEQ;
	c.op.push_back(T[op[0]].name);
	c.op.push_back("0");
	c.op.push_back(false_tag);
	code.push_back(c); // 跳转

#ifdef _DEBUG_
	//printf("if insert  ");
	//T[op[0]].show_code();
	//T[op[1]].show_code();
#endif // _DEBUG_

	code.insert(code.end(), T[op[1]].code.begin(), T[op[1]].code.end()); //if块
	
	c.type = TAG;
	c.op.clear();
	c.tag = false_tag;// if块的出口
	code.push_back(c);
	return 0;
}

int AST_node::__while__(vector<AST_node> & T)
{
	// 这个name要不要赋值不是很清楚
	name = T[op[0]].name;
	TAC c;
	string while_tag = "tag: " + to_string(TAC::tag_cnt++),
		false_tag = "tag: " + to_string(TAC::tag_cnt++);
	c.type = TAG;
	c.tag = while_tag;// if块内的入口
	code.push_back(c);

	code.insert(code.end(), T[op[0]].code.begin(), T[op[0]].code.end());//if判断块

	c.type = BEQ;
	c.op.push_back(T[op[0]].name);
	c.op.push_back("0");
	c.op.push_back(false_tag);
	code.push_back(c); // 跳转

	code.insert(code.end(), T[op[1]].code.begin(), T[op[1]].code.end()); //if块
	
	c.type = GOTO;
	c.op.clear();
	c.op.push_back(while_tag);// 跳回到第一句判断
	code.push_back(c);


	c.type = TAG;
	c.tag = false_tag;
	c.op.clear();
	code.push_back(c);
	return 0;
}

int AST_node::__return__(vector<AST_node>& T)
{
	name = "return";
	for (auto idx : op)
	{
		code.insert(code.end(),
			T[idx].code.begin(), T[idx].code.end());
	}
	TAC c;
	c.type = RETURN;
	c.op.push_back(T[op[0]].name);
	code.push_back(c);
	return 0;
}

int AST_node::__call__(vector<AST_node>& T)
{
	name = T[op[0]].name + to_string(temp_node_cnt++);
	for (auto idx : op)
	{
		code.insert(code.end(),
			T[idx].code.begin(), T[idx].code.end());
	}
	AST_node & tn = T[op[1]];
	TAC c;
	c.type = CALL;
	c.op.push_back(name);
	while (1)
	{
		if (tn.op.size() == 2)
		{
			c.op.push_back(T[tn.op[1]].name);
			tn = T[tn.op[0]];
		}
		else if(tn.op.size() == 1)
		{
			c.op.push_back(T[tn.op[0]].name);
			break;
		}
		else
		{
			break;
		}
	}
	code.push_back(c);
	
	return 0;
}









int AST_node::__cal_op__(vector<AST_node> & T, node_type nt)
{
	for (auto idx : op)
	{
		code.insert(code.end(),
			T[idx].code.begin(), T[idx].code.end());
	}
	if (!syb)name = "$" + to_string(temp_node_cnt++);// 没有指向syb的都是中间temp节点
	if (!type_check(T))
	{
		type = node_error;
		puts("类型不匹配");
		return -1;
	}
	TAC c;
	if(nt == node_add)c.type = ADD;
	else if (nt == node_sub) c.type = SUB;
	else if (nt == node_mul) c.type = MUL;
	else if (nt == node_div) c.type = DIV;
	else if (nt == node_greater) c.type = GREATER;
#ifdef _DEBUG_
	else { puts("程序错误"); return -100; }
#endif // _DEBUG_
	c.op.push_back(name);
	c.op.push_back(T[op[0]].name);
	c.op.push_back(T[op[1]].name);
	code.push_back(c);
	return 0;
}

int AST_node::__nop__(vector<AST_node>& T)
{
	for (auto idx : op)
	{
		code.insert(code.end(),
			T[idx].code.begin(), T[idx].code.end());
	}

	// 没有操作的直接传值
	if(op.size())name = T[op[0]].name;
#ifdef _DEBUG_
	//puts("nop end");
	//printf("op size %d \n", op.size());
#endif // _DEBUG_

	return 0;
}

int AST_node::__assign__(vector<AST_node>& T)
{
	for (auto idx : op)
	{
		code.insert(code.end(),
			T[idx].code.begin(), T[idx].code.end());
	}
	if (!syb)
	{
		// 没有指向syb的都是中间temp节点
		name = "$" + to_string(temp_node_cnt++);
	}
	if (!type_check(T))
	{
		type = node_error;
		puts("类型不匹配");
		return -1;
	}
	TAC c;
	c.type = ASSIGN;
	c.op.push_back(T[op[0]].name);
	c.op.push_back(T[op[1]].name);
	code.push_back(c);
	c.op[0] = name;
	// 加入自己的assign
	code.push_back(c);
	return 0;
}

// 访问该节点产生TAC
void AST_node::call(vector<AST_node> & T)
{
	// 第一步都是将op的代码合在一起
#ifdef _DEBUG_
	//printf("%d\n", type);
#endif // _DEBUG_

	if (op.size())
		dType = !T[op[0]].syb ? T[op[0]].dType :
		T[op[0]].syb->vType;
	else if (syb) dType = syb->vType;

	switch (type)
	{
	case node_nop:
		__nop__(T);
		break;
	case node_assign:
		__assign__(T);
		break;
	case node_greater:
	case node_add:
	case node_sub:
	case node_mul:
	case node_div:
		__cal_op__(T, type);
		break;
	case node_if:
		__if__(T);
		break;
	case node_while:
		__while__(T);
		break;
	case node_declare:
		__declare__(T);
		break;
	case node_func:
		__function__(T);
		break;
	case node_error:
		puts("ERROR NODE");
		break;
	case node_return:
		__return__(T);
		break;
	case node_call:
		__call__(T);
	default:
		break;
	}
//#ifdef _DEBUG_
//	puts("call end");
//	printf("%d\n", syb->vType);
//#endif // _DEBUG_


}

// 检查AST_node的类型
bool AST_node::type_check(vector<AST_node>& T)
{
	valueType vt;
	string nm;
	if (op.size())
	{
		 vt = !T[op[0]].syb ? T[op[0]].dType :
			T[op[0]].syb->vType;
		 nm = T[op[0]].name;
	}
	for (size_t i = 1; i < op.size(); i++)
	{
		valueType vit = !T[op[i]].syb ? T[op[i]].dType :
			T[op[i]].syb->vType;
		if (vit != vt)
		{

			printf("%s type:%d  %s type:%d ", nm.c_str(),vt,  T[op[i]].name.c_str(), vit);
			return false;
		}
	}
	return true;
}


int AST_tree::add_node(AST_node & node)
{
	int ret = tree.size();
	tree.push_back(node);
	return ret;
}


