// author: M@
#include "lex_parser.h"
#include "lex_generater.h"
#include "syntax_parser.h"
#include "sematic_parser.h"
#include "Type.h"

using namespace std;

void gen_lex(){
	lex_generater lgen("lex.txt");
	lgen.gen();
}

void lex_parse(string lex_path){
	All_lex_parser  lexParser("lex.txt", "token_list.txt");
	lexParser.parse_all_grammar();
	lexParser.check_code("code.txt");
}

void syntax_parse(string file_path, string token_path){
	syntax_parser sp(file_path, token_path);
	sp.read_syntax();
	sp.generate_clan();
	sp.parse_code();
	sp.end();
}


void sematic_parse(string file_path, string token_path) {
	sematic_parser sp(file_path, token_path);
	sp.read_syntax();
	sp.generate_clan();
	int ret = sp.parse_code();
	//sp.output();
	if(!ret)
	sp.AST2TAC();
	//sp.read_syntax();
	//sp.generate_clan();
	// sp.output();
	//sp.parse_code();
	//sp.optim();
}
int main()
{

	string lex_path = "lex.txt", syntax_path = "syntax.txt", sematic_path = "sematic.txt";
	//gen_lex();
	lex_parse(lex_path);
	//syntax_parse(syntax_path, "token_list.txt");
	sematic_parse(sematic_path, "token_list.txt");
	system("pause");
}

