%code requires {
  #include <memory>
  #include <string>
  #include "astDef.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "astDef.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  ComputeBaseAST *compute_ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> Decl ConstDecl ConstDeclRec ConstDef FuncDef 
%type <ast_val> FuncType Block BlockItemRec BlockItem Stmt
%type <ast_val> VarDecl VarDeclRec VarDef
%type <compute_ast_val> ConstInitVal UnaryExp AddExp MulExp RelExp 
%type <compute_ast_val> EqExp LAndExp LOrExp ConstExp Exp PrimaryExp
%type <compute_ast_val> InitVal
%type <int_val> Number
%type <str_val> UnaryOp BinaryOp1 BinaryOp2 RelOp EqOp BType LVal

%%


CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->const_decl = unique_ptr<BaseAST>($1);
    ast->parse_type = "const";
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->var_decl = unique_ptr<BaseAST>($1);
    ast->parse_type = "var";
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDeclRec ';' {
    auto ast = new ConstDeclAST();
    ast->type = *unique_ptr<string>($2);
    ast->const_decl_rec = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstDeclRec
  : ConstDef{
    auto ast = new ConstDeclRecAST();
    ast->const_def = unique_ptr<BaseAST>($1);
    ast->parse_type = "constDef";
    $$ = ast;
  }
  | ConstDeclRec ',' ConstDef {
    auto ast = new ConstDeclRecAST();
    ast->const_decl_rec = unique_ptr<BaseAST>($1);
    ast->const_def = unique_ptr<BaseAST>($3);
    ast->parse_type = "rec";
    $$ = ast;
  }
  ;

BType
  : INT {
    string * s = new string("int");
    $$ = s;
  };

// 定义常量
ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->const_init_val = unique_ptr<ComputeBaseAST>($3);
    $$ = ast;
  };

ConstInitVal
  : ConstExp{
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<ComputeBaseAST>($1);
    $$ = ast;
  };

VarDecl
  : BType VarDeclRec ';' {
    auto ast = new VarDeclAST();
    ast->type = *unique_ptr<string>($1);
    ast->var_decl_rec = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

VarDeclRec
  : VarDef {
    auto ast = new VarDeclRecAST();
    ast->var_def = unique_ptr<BaseAST>($1);
    ast->parse_type = "varDef";
    $$ = ast;
  }
  | VarDeclRec ',' VarDef {
    auto ast = new VarDeclRecAST();
    ast->var_decl_rec = unique_ptr<BaseAST>($1);
    ast->var_def = unique_ptr<BaseAST>($3);
    ast->parse_type = "rec";
    $$ = ast;
  }
  ;

// 定义变量，第一种方式中实际初始值未定义
VarDef 
  : IDENT {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->parse_type = "ident";
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->init_val = unique_ptr<ComputeBaseAST>($3);
    ast->parse_type = "eq";
    $$ = ast;
  }
  ;

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<ComputeBaseAST>($1);
    $$ = ast;
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = "i32";
    $$ = ast;
  }
  ;

// Block内不允许声明重名的变量或常量，在定义到此语句块尾的范围内有效
// 变量/常量的名字可以是main
Block
  : '{' BlockItemRec '}' {
    auto ast = new BlockAST();
    ast->block_item_rec = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

BlockItemRec
  : {
    auto ast = new BlockItemRecAST();
    ast->parse_type = "empty";
    $$ = ast;
  }
  | BlockItem {
    auto ast = new BlockItemRecAST();
    ast->block_item = unique_ptr<BaseAST>($1);
    ast->parse_type = "single";
    $$ = ast;
  }
  | BlockItemRec BlockItem {
    auto ast = new BlockItemRecAST();
    ast->block_item_rec = unique_ptr<BaseAST>($1);
    ast->block_item = unique_ptr<BaseAST>($2);
    ast->parse_type = "rec";
    $$ = ast;
  }
  ;

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<BaseAST>($1);
    ast->parse_type = "decl";
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    ast->parse_type = "stmt";
    $$ = ast;
  };

Stmt
  : LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->lval = *unique_ptr<string>($1);
    ast->exp = unique_ptr<ComputeBaseAST>($3);
    ast->parse_type = "lval";
    $$ = ast;
  }
  | RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<ComputeBaseAST>($2);
    ast->parse_type = "ret";
    $$ = ast;
  }
  ;

// Exp内出现的LVal必须是之前定义过的
Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->l_or_exp = unique_ptr<ComputeBaseAST>($1);
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    $$ = $1;
  };

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->parse_type = "exp";
    ast->exp = unique_ptr<ComputeBaseAST>($2);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->parse_type = "number";
    ast->number = $1;
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->parse_type = "lval";
    ast->lval = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->parse_type = "primary";
    ast->primary_exp = unique_ptr<ComputeBaseAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->parse_type = "uop";
    ast->unary_op = *unique_ptr<string>($1);
    ast->unary_exp = unique_ptr<ComputeBaseAST>($2);
    $$ = ast;
  }
  ;

UnaryOp
  : '+' {
    string * op = new string("+");
    $$ = op;
  }
  | '-' {
    string * op = new string("-");
    $$ = op;
  }
  | '!' {
    string * op = new string("!");
    $$ = op;
  }
  ;

MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->unary_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "unaryExp";
    $$ = ast;
  }
  | MulExp BinaryOp1 UnaryExp {
    auto ast = new MulExpAST();
    ast->mul_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "bin";
    ast->op = *unique_ptr<string>($2);
    ast->unary_exp = unique_ptr<ComputeBaseAST>($3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->mul_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "multExp";
    $$ = ast;
  }
  | AddExp BinaryOp2 MulExp {
    auto ast = new AddExpAST();
    ast->add_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "bin";
    ast->op = *unique_ptr<string>($2);
    ast->mul_exp = unique_ptr<ComputeBaseAST>($3);
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->add_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "addExp";
    $$ = ast;
  }
  | RelExp RelOp AddExp {
    auto ast = new RelExpAST();
    ast->rel_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "bin";
    ast->op = *unique_ptr<string>($2);
    ast->add_exp = unique_ptr<ComputeBaseAST>($3);
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->rel_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "relExp";
    $$ = ast;
  }
  | EqExp EqOp RelExp {
    auto ast = new EqExpAST();
    ast->eq_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "bin";
    ast->op = *unique_ptr<string>($2);
    ast->rel_exp = unique_ptr<ComputeBaseAST>($3);
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->eq_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "eqExp";
    $$ = ast;
  }
  | LAndExp '&' '&' EqExp {
    auto ast = new LAndExpAST();
    ast->l_and_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "bin";
    ast->op = "&&";
    ast->eq_exp = unique_ptr<ComputeBaseAST>($4);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->l_and_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "lAndExp";
    $$ = ast;
  }
  | LOrExp '|' '|' LAndExp {
    auto ast = new LOrExpAST();
    ast->l_or_exp = unique_ptr<ComputeBaseAST>($1);
    ast->parse_type = "bin";
    ast->op = "||";
    ast->l_and_exp = unique_ptr<ComputeBaseAST>($4);
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<ComputeBaseAST>($1);
    $$ = ast;
  }
  ;

BinaryOp1
  : '%' {
    string * op = new string("%%");
    $$ = op;
  }
  | '*' {
    string * op = new string("*");
    $$ = op;
  }
  | '/' {
    string * op = new string("/");
    $$ = op;
  }
  ;

BinaryOp2
  : '+' {
    string * op = new string("+");
    $$ = op;
  }
  | '-' {
    string * op = new string("-");
    $$ = op;
  }
  ;

RelOp
  : '<' {
    string * op = new string("<");
    $$ = op;
  }
  | '>' {
    string * op = new string(">");
    $$ = op;
  }
  | '<' '=' {
    string * op = new string("<=");
    $$ = op;
  }
  | '>' '=' {
    string * op = new string(">=");
    $$ = op;
  }
  ;

EqOp
  : '=' '=' {
    string * op = new string("==");
    $$ = op;
  }
  | '!' '=' {
    string * op = new string("!=");
    $$ = op;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
