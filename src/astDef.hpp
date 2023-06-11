#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <map>
// #define _DEBUG
using namespace std;

// map <string, string> type2IRtype = {
//     {"int", "i32"}
// };
class VarInfo {
public:
    string var_name;
    int val;
    
    VarInfo (string label, int v) {
        var_name = label;
        val = v;
    }
    VarInfo () {
        var_name = "";
        val = 0;
    }
};
static int tempID = 0;

class SymbolTable {
    map<string, VarInfo> const_table;
    map<string, VarInfo> var_table;
public:
    bool exists(string label) {
        if(const_table.count(label) || var_table.count(label)) {
            return true;
        }
        return false;
    }
    void insert(string label, int val, bool is_const) {
        if(exists(label)) {
            cout << "redefination of " << label << endl;
            assert(false);
        }
        if(is_const) {
            const_table[label] = VarInfo(label, val);
        }
        else {
            var_table[label] = VarInfo(label, val);
        }
    }
    int getVal(string label) {
        if(!exists(label)) {
            cout << label << " does not exits" << endl;
            assert(false);
        }
        if(const_table.count(label)) {
            return const_table[label].val;
        }
        return var_table[label].val;
    }
};

static SymbolTable symbolTable;

// 所有 AST 的基类
class BaseAST {
public:
    string parse_type;
    string variable_name;
    int val;

    virtual ~BaseAST() = default;

    virtual void Dump() = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    unique_ptr<BaseAST> func_def;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "CompUnitAST {\n";
        #endif

        func_def->Dump();

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class DeclAST : public BaseAST {
public:
    // 用智能指针管理对象
    unique_ptr<BaseAST> const_decl;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "DeclAST {\n";
        #endif

        const_decl->Dump();

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class ConstDeclAST : public BaseAST {
public:
    unique_ptr<BaseAST> const_decl_rec;
    string type;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "ConstDeclAST {\n";
        #endif

        const_decl_rec->Dump();

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class ConstDeclRecAST : public BaseAST {
public:
    unique_ptr<BaseAST> const_decl_rec;
    unique_ptr<BaseAST> const_def;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "ConstDeclRecAST {\n";
        #endif

        if(!strcmp(parse_type.c_str(), "constDef")) {
            const_def->Dump();
        }
        else if (!strcmp(parse_type.c_str(), "rec")) {
            const_decl_rec->Dump();
            const_def->Dump();
        }   
        else {
            assert(false);
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};


class ConstDefAST : public BaseAST {
public:
    string ident;
    unique_ptr<BaseAST> const_init_val;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "ConstDefAST {\n";
        #endif

        const_init_val->Dump();
        variable_name = ident;
        val = const_init_val->val;
        symbolTable.insert(variable_name, val, true);

        #ifdef _DEBUG
        cout << "insert " << variable_name << " " << val << endl; 
        cout << "}\n";
        #endif
    }
};

class ConstInitValAST : public BaseAST {
public:
    unique_ptr<BaseAST> const_exp;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "ConstInitValAST {\n";
        #endif

        const_exp->Dump();
        variable_name = const_exp->variable_name;
        val = const_exp->val;

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};


// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    unique_ptr<BaseAST> func_type;
    string ident;
    unique_ptr<BaseAST> block;

    void Dump() override {
        #ifdef _DEBUG
        cout << "FuncDefAST {\n";
        #endif

        cout << "fun @" << ident << "(): ";
        func_type->Dump();
        cout << "{\n";
        block->Dump();
        cout << "}\n";

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class FuncTypeAST : public BaseAST {
public:
    string type;

    void Dump() override {
        #ifdef _DEBUG
        cout << "FuncTypeAST {\n";
        #endif

        cout << type;

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class BlockAST : public BaseAST {
public:
    unique_ptr<BaseAST> block_item_rec;

    void Dump() override {
        #ifdef _DEBUG
        cout << "BlockAST {\n";
        #endif

        cout << "%" << "entry:\n";
        block_item_rec->Dump();

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class BlockItemRecAST : public BaseAST {
public:
    unique_ptr<BaseAST> block_item;
    unique_ptr<BaseAST> block_item_rec;

    void Dump() override {
        #ifdef _DEBUG
        cout << "BlockItemRecAST {\n";
        #endif

        if(!strcmp( parse_type.c_str(), "rec")) {
            block_item_rec->Dump();
            block_item->Dump();
        }
        else if(!strcmp( parse_type.c_str(), "single")) {
            block_item->Dump();
        }
        else if(!strcmp( parse_type.c_str(), "empty")) {
        }
        else {
            assert(false);
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class BlockItemAST : public BaseAST {
public:
    unique_ptr<BaseAST> stmt;
    unique_ptr<BaseAST> decl;

    void Dump() override {
        #ifdef _DEBUG
        cout << "BlockItemAST {\n";
        #endif

        if(!strcmp( parse_type.c_str(), "stmt")) {
            stmt->Dump();
        }
        else if(!strcmp( parse_type.c_str(), "decl")) {
            decl->Dump();
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class StmtAST : public BaseAST {
public:
    unique_ptr<BaseAST> exp;

    void Dump() override {
        #ifdef _DEBUG
        cout << "StmtAST {\n";
        #endif

        exp->Dump();
        if(symbolTable.exists(exp->variable_name)) {
            cout << "\tret " << exp->val << "\n";    
        }
        else {
            cout << "\tret " << exp->variable_name << "\n";
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class ExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> l_or_exp;

    void Dump() override {
        #ifdef _DEBUG
        cout << "ExpAST {\n";
        #endif

        l_or_exp->Dump();
        variable_name = l_or_exp->variable_name;
        val = l_or_exp->val;

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class MulExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> mul_exp;
    unique_ptr<BaseAST> unary_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "MulExpAST {\n";
        #endif
        
        if(!strcmp( parse_type.c_str(), "unaryExp")) {
            unary_exp->Dump();
            variable_name = unary_exp->variable_name;
            val = unary_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            mul_exp->Dump();
            unary_exp->Dump();
            if(!strcmp(op.c_str(), "*")) {
                cout << "\t%" << tempID << " = mul " << mul_exp->variable_name << ", " << unary_exp->variable_name << "\n";
                val = mul_exp->val * unary_exp->val;
            }
            else if(!strcmp(op.c_str(), "/")) {
                cout << "\t%" << tempID << " = div " << mul_exp->variable_name << ", " << unary_exp->variable_name << "\n";
                val = mul_exp->val / unary_exp->val;
            }
            else if(!strcmp(op.c_str(), "%%")) {
                cout << "\t%" << tempID << " = mod " << mul_exp->variable_name << ", " << unary_exp->variable_name << "\n";
                val = mul_exp->val % unary_exp->val;
            }
            variable_name = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class AddExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> add_exp;
    unique_ptr<BaseAST> mul_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "AddExpAST {\n";
        #endif
        
        if(!strcmp( parse_type.c_str(), "multExp")) {
            mul_exp->Dump();
            variable_name = mul_exp->variable_name;
            val = mul_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            add_exp->Dump();
            mul_exp->Dump();
            if(!strcmp(op.c_str(), "+")) {
                cout << "\t%" << tempID << " = add " << add_exp->variable_name << ", " << mul_exp->variable_name << "\n";
                val = add_exp->val + mul_exp->val;
            }
            else if(!strcmp(op.c_str(), "-")) {
                cout << "\t%" << tempID << " = sub " << add_exp->variable_name << ", " << mul_exp->variable_name << "\n";
                val = add_exp->val - mul_exp->val;
            }
            variable_name = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class PrimaryExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> exp;
    int number;
    string lval;

    void Dump() override {
        #ifdef _DEBUG
        cout << "PrimaryExpAST {\n";
        #endif
        
        if(!strcmp(parse_type.c_str(), "exp")) {
            exp->Dump();
            variable_name = exp->variable_name;
            val = exp->val;
        }
        else if (!strcmp(parse_type.c_str(), "number")) {
            variable_name = to_string(number);
            val = number;
        }
        else if (!strcmp(parse_type.c_str(), "lval")) {
            variable_name = lval;
            val = symbolTable.getVal(variable_name);
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class UnaryExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> primary_exp;
    unique_ptr<BaseAST> unary_exp;
    string unary_op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "UnaryExpAST {\n";
        #endif
        
        if(!strcmp(parse_type.c_str(), "primary")) {
            primary_exp->Dump();
            variable_name = primary_exp->variable_name;
            val = primary_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "uop")) {
            unary_exp->Dump();
            if(!strcmp(unary_op.c_str(), "+")) {
                // + 不生成IR
                variable_name = unary_exp->variable_name;
                val = unary_exp->val;
            }
            else {
                if(!strcmp(unary_op.c_str(), "-")) {
                    cout << "\t%" << tempID << " = sub 0, " << unary_exp->variable_name << "\n";
                    val = -(unary_exp->val);
                }
                else if(!strcmp(unary_op.c_str(), "!")) {
                    cout << "\t%" << tempID << " = eq " << unary_exp->variable_name << ", 0\n";
                    val = !(unary_exp->val);
                }
                variable_name = "%" + to_string(tempID);
                tempID++;
            }
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class RelExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> add_exp;
    unique_ptr<BaseAST> rel_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "RelExpAST {\n";
        #endif
        
        if(!strcmp(parse_type.c_str(), "addExp")) {
            add_exp->Dump();
            variable_name = add_exp->variable_name;
            val = add_exp->val;
        }
        else if (!strcmp(parse_type.c_str(), "bin")) {
            rel_exp->Dump();
            add_exp->Dump();
            if(!strcmp(op.c_str(), "<")) {
                cout << "\t%" << tempID << " = lt " << rel_exp->variable_name << ", " << add_exp->variable_name << "\n";
                val = (rel_exp->val < add_exp->val);
            }
            else if(!strcmp(op.c_str(), ">")) {
                cout << "\t%" << tempID << " = gt " << rel_exp->variable_name << ", " << add_exp->variable_name << "\n";
                val = (rel_exp->val > add_exp->val);
            }
            else if(!strcmp(op.c_str(), "<=")) {
                cout << "\t%" << tempID << " = le " << rel_exp->variable_name << ", " << add_exp->variable_name << "\n";
                val = (rel_exp->val <= add_exp->val);
            }
            else if(!strcmp(op.c_str(), ">=")) {
                cout << "\t%" << tempID << " = ge " << rel_exp->variable_name << ", " << add_exp->variable_name << "\n";
                val = (rel_exp->val >= add_exp->val);
            }
            variable_name = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class EqExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> rel_exp;
    unique_ptr<BaseAST> eq_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "EqExpAST {\n";
        #endif
        
        if(!strcmp(parse_type.c_str(), "relExp")) {
            rel_exp->Dump();
            variable_name = rel_exp->variable_name;
            val = rel_exp->val;
        }
        else if (!strcmp(parse_type.c_str(), "bin")) {
            eq_exp->Dump();
            rel_exp->Dump();
            if(!strcmp(op.c_str(), "==")) {
                cout << "\t%" << tempID << " = eq " << eq_exp->variable_name << ", " << rel_exp->variable_name << "\n";
                val = (eq_exp->val == rel_exp->val);
            }
            else if(!strcmp(op.c_str(), "!=")) {
                cout << "\t%" << tempID << " = ne " << eq_exp->variable_name << ", " << rel_exp->variable_name << "\n";
                val = (eq_exp->val != rel_exp->val);
            }
            variable_name = "%" + to_string(tempID);

            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class LAndExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> eq_exp;
    unique_ptr<BaseAST> l_and_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "LAndExpAST {\n";
        #endif
        
        if(!strcmp( parse_type.c_str(), "eqExp")) {
            eq_exp->Dump();
            variable_name = eq_exp->variable_name;
            val = eq_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            l_and_exp->Dump();
            eq_exp->Dump();
            if(!strcmp(op.c_str(), "&&")) {
                cout << "\t%" << tempID << " = ne " << l_and_exp->variable_name << ", 0\n";
                tempID++;
                cout << "\t%" << tempID << " = ne " << eq_exp->variable_name << ", 0\n";
                tempID++;
                cout << "\t%" << tempID << " = and %" << tempID - 2 << ", %" << tempID - 1 << "\n";
            }
            else {
                assert(false);
            }
            variable_name = "%" + to_string(tempID);
            val = l_and_exp->val && eq_exp->val;
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class LOrExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> l_and_exp;
    unique_ptr<BaseAST> l_or_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "LOrExpAST {\n";
        #endif
        
        if(!strcmp( parse_type.c_str(), "lAndExp")) {
            l_and_exp->Dump();
            variable_name = l_and_exp->variable_name;
            val = l_and_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            l_or_exp->Dump();
            l_and_exp->Dump();
            if(!strcmp(op.c_str(), "||")) {
                cout << "\t%" << tempID << " = or " << l_or_exp->variable_name << ", " << l_and_exp->variable_name << "\n";
                tempID++;
                cout << "\t%" << tempID << " = ne %" << tempID - 1 << ", 0\n";
            }
            else {
                assert(false);
            }
            variable_name = "%" + to_string(tempID);
            val = l_or_exp->val || l_and_exp->val;
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class ConstExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> exp;

    void Dump() override {
        #ifdef _DEBUG
        cout << "PrimaryExpAST {\n";
        #endif
        
        exp->Dump();
        variable_name = exp->variable_name;
        val = exp->val;


        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};


