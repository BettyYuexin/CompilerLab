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
static int tempID = 0;
static int layer_cnt = 0;
static int cur_layer = 0;
static map<int, int> father;

static string get_label(string var_name, int layer) {
    return var_name + "_" + to_string(layer);
}

class VarInfo {
public:
    string var_name;
    int val;
    int layer;
    
    VarInfo (string label, int v) {
        var_name = label;
        val = v;
        layer = cur_layer;
    }
    VarInfo () {
        var_name = "";
        val = 0;
        layer = cur_layer;
    }
};

class SymbolTable {
    map<string, VarInfo> const_table;
    map<string, VarInfo> var_table;
public:
    string get_var_label(string var_name) {
        int t = cur_layer;
        while(t >= 0) {
            string label = var_name + "_" + to_string(t);
            if(exists(label)) {
                return label;
            }
            if(!father.count(t)) break;
            t = father[t];
        }
        return "%NotStored%";
    }

    bool exists(string label) {
        if(const_table.count(label) || var_table.count(label)) {
            return true;
        }
        return false;
    }

    bool is_const(string label) {
        return const_table.count(label);
    }

    bool is_var(string label) {
        return var_table.count(label);
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

class ComputeBaseAST : public BaseAST {
public:
    virtual void Compute() = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    unique_ptr<BaseAST> func_def;
    
    void Dump() override {
        func_def->Dump();
    }
};

class DeclAST : public BaseAST {
public:
    // 用智能指针管理对象
    unique_ptr<BaseAST> const_decl;
    unique_ptr<BaseAST> var_decl;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "DeclAST {\n";
        #endif
        if(!strcmp("const", parse_type.c_str())) {
            const_decl->Dump();
        }
        else if(!strcmp("var", parse_type.c_str())) {
            var_decl->Dump();
        }
        else {
            assert(false);
        }
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
    unique_ptr<ComputeBaseAST> const_init_val;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "ConstDefAST {\n";
        #endif

        const_init_val->Compute();
        variable_name = get_label(ident, cur_layer);
        val = const_init_val->val;
        symbolTable.insert(variable_name, val, true);
        
        #ifdef _DEBUG
        cout << "insert " << ident << " " << val << endl; 
        cout << "}\n";
        #endif
    }
};

class ConstInitValAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> const_exp;
    
    void Dump() override {
        const_exp->Dump();
    }
    void Compute() override {
        const_exp->Compute();
        val = const_exp->val;
    }
};

class VarDeclAST : public BaseAST {
public:
    unique_ptr<BaseAST> var_decl_rec;
    string type;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "VarDeclAST {\n";
        #endif

        var_decl_rec->Dump();

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class VarDeclRecAST : public BaseAST {
public:
    unique_ptr<BaseAST> var_decl_rec;
    unique_ptr<BaseAST> var_def;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "VarDeclRecAST {\n";
        #endif

        if(!strcmp(parse_type.c_str(), "varDef")) {
            var_def->Dump();
        }
        else if (!strcmp(parse_type.c_str(), "rec")) {
            var_decl_rec->Dump();
            var_def->Dump();
        }   
        else {
            assert(false);
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class VarDefAST : public BaseAST {
public:
    string ident;
    unique_ptr<ComputeBaseAST> init_val;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "VarDefAST {\n";
        #endif

        if(!strcmp(parse_type.c_str(), "ident")) {
            variable_name = get_label(ident, cur_layer);
            symbolTable.insert(variable_name, 0, false);
            cout << "\t@" << variable_name << " = alloc i32\n";
        }
        else if(!strcmp(parse_type.c_str(), "eq")) {
            init_val->Dump();
            init_val->Compute();
            variable_name = get_label(ident, cur_layer);
            val = init_val->val;
            symbolTable.insert(variable_name, val, false);
            cout << "\t@" << variable_name << " = alloc i32\n";
            cout << "\tstore " << init_val->variable_name << ", @" << variable_name << endl;
        }
        
        #ifdef _DEBUG
        cout << "insert " << ident << " " << val << endl; 
        cout << "}\n";
        #endif
    }
};

class InitValAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> exp;
    
    void Dump() override {
        exp->Dump();
        variable_name = exp->variable_name;
    }
    void Compute() override {
        exp->Compute();
        val = exp->val;
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
        cout << "%" << "entry:\n";
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

        layer_cnt++;
        father[layer_cnt] = cur_layer;
        cur_layer = layer_cnt;
        block_item_rec->Dump();
        cur_layer = father[cur_layer];

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
    unique_ptr<ComputeBaseAST> exp;
    unique_ptr<BaseAST> block;
    string lval;

    void Dump() override {
        #ifdef _DEBUG
        cout << "StmtAST {\n";
        #endif

        if(!strcmp(parse_type.c_str(), "ret")) {
            exp->Dump();
            string label = symbolTable.get_var_label(exp->variable_name);
            if(symbolTable.is_const(label)) {
                cout << "\tret " << symbolTable.getVal(label) << "\n";    
            }
            else {
                // ret %reg
                cout << "\tret " << exp->variable_name << "\n";
            }
        }
        else if(!strcmp(parse_type.c_str(), "lval")) {
            exp->Dump();
            string label = symbolTable.get_var_label(lval);
            cout << "\tstore " << exp->variable_name << ", @" << label << endl;
        }
        else if(!strcmp(parse_type.c_str(), "empty")) {}
        else if(!strcmp(parse_type.c_str(), "exp")) {
            exp->Dump();
        }
        else if(!strcmp(parse_type.c_str(), "block")) {
            block->Dump();
        }
        else if(!strcmp(parse_type.c_str(), "ret empty")) {
            cout << "\tret\n";
        }
        else {
            assert(false);
        }
        

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class ExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> l_or_exp;

    void Dump() override {
        #ifdef _DEBUG
        cout << "ExpAST {\n";
        #endif

        l_or_exp->Dump();
        variable_name = l_or_exp->variable_name;

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }

    void Compute() override {
        l_or_exp->Compute();
        val = l_or_exp->val;
    }
};

class MulExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> mul_exp;
    unique_ptr<ComputeBaseAST> unary_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "MulExpAST {\n";
        #endif
        
        if(!strcmp( parse_type.c_str(), "unaryExp")) {
            unary_exp->Dump();
            variable_name = unary_exp->variable_name;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            mul_exp->Dump();
            unary_exp->Dump();
            if(!strcmp(op.c_str(), "*")) {
                cout << "\t%" << tempID << " = mul " << mul_exp->variable_name << ", " << unary_exp->variable_name << "\n";
            }
            else if(!strcmp(op.c_str(), "/")) {
                cout << "\t%" << tempID << " = div " << mul_exp->variable_name << ", " << unary_exp->variable_name << "\n";
            }
            else if(!strcmp(op.c_str(), "%%")) {
                cout << "\t%" << tempID << " = mod " << mul_exp->variable_name << ", " << unary_exp->variable_name << "\n";
            }
            variable_name = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
    void Compute() override {
        if(!strcmp( parse_type.c_str(), "unaryExp")) {
            unary_exp->Compute();
            val = unary_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            mul_exp->Compute();
            unary_exp->Compute();
            if(!strcmp(op.c_str(), "*")) {
                val = mul_exp->val * unary_exp->val;
            }
            else if(!strcmp(op.c_str(), "/")) {
                val = mul_exp->val / unary_exp->val;
            }
            else if(!strcmp(op.c_str(), "%%")) {
                val = mul_exp->val % unary_exp->val;
            }
        }
    }
};

class AddExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> add_exp;
    unique_ptr<ComputeBaseAST> mul_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "AddExpAST {\n";
        #endif
        
        if(!strcmp( parse_type.c_str(), "multExp")) {
            mul_exp->Dump();
            variable_name = mul_exp->variable_name;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            add_exp->Dump();
            mul_exp->Dump();
            if(!strcmp(op.c_str(), "+")) {
                cout << "\t%" << tempID << " = add " << add_exp->variable_name << ", " << mul_exp->variable_name << "\n";
            }
            else if(!strcmp(op.c_str(), "-")) {
                cout << "\t%" << tempID << " = sub " << add_exp->variable_name << ", " << mul_exp->variable_name << "\n";
            }
            variable_name = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
    void Compute() override {
        if(!strcmp( parse_type.c_str(), "multExp")) {
            mul_exp->Compute();
            val = mul_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            add_exp->Compute();
            mul_exp->Compute();
            if(!strcmp(op.c_str(), "+")) {
                val = add_exp->val + mul_exp->val;
            }
            else if(!strcmp(op.c_str(), "-")) {
                val = add_exp->val - mul_exp->val;
            }
        }
    }
};

class PrimaryExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> exp;
    int number;
    string lval;

    void Dump() override {
        #ifdef _DEBUG
        cout << "PrimaryExpAST {\n";
        #endif
        
        if(!strcmp(parse_type.c_str(), "exp")) {
            exp->Dump();
            variable_name = exp->variable_name;
        }
        else if (!strcmp(parse_type.c_str(), "number")) {
            variable_name = to_string(number);
        }
        else if (!strcmp(parse_type.c_str(), "lval")) {
            string label = symbolTable.get_var_label(lval);
            if(symbolTable.is_var(label)) {
                cout << "\t%" << tempID << " = load @" << label << endl;
                variable_name = "%" + to_string(tempID);
                tempID++;
            }
            else if (symbolTable.is_const(label)) {
                variable_name = to_string(symbolTable.getVal(label));
            }
        }
        else {
            assert(false);
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
    void Compute() override {
        if(!strcmp(parse_type.c_str(), "exp")) {
            exp->Compute();
            val = exp->val;
        }
        else if (!strcmp(parse_type.c_str(), "number")) {
            val = number;
        }
        else if (!strcmp(parse_type.c_str(), "lval")) {
            string label = symbolTable.get_var_label(lval);
            val = symbolTable.getVal(label);
        }
    }
};

class UnaryExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> primary_exp;
    unique_ptr<ComputeBaseAST> unary_exp;
    string unary_op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "UnaryExpAST {\n";
        #endif
        
        if(!strcmp(parse_type.c_str(), "primary")) {
            primary_exp->Dump();
            variable_name = primary_exp->variable_name;
        }
        else if (!strcmp( parse_type.c_str(), "uop")) {
            unary_exp->Dump();
            if(!strcmp(unary_op.c_str(), "+")) {
                // + 不生成IR
                variable_name = unary_exp->variable_name;
            }
            else {
                if(!strcmp(unary_op.c_str(), "-")) {
                    cout << "\t%" << tempID << " = sub 0, " << unary_exp->variable_name << "\n";
                }
                else if(!strcmp(unary_op.c_str(), "!")) {
                    cout << "\t%" << tempID << " = eq " << unary_exp->variable_name << ", 0\n";
                }
                variable_name = "%" + to_string(tempID);
                tempID++;
            }
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
    void Compute() override {
        if(!strcmp(parse_type.c_str(), "primary")) {
            primary_exp->Compute();
            val = primary_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "uop")) {
            unary_exp->Compute();
            if(!strcmp(unary_op.c_str(), "+")) {
                val = unary_exp->val;
            }
            else {
                if(!strcmp(unary_op.c_str(), "-")) {
                    val = -(unary_exp->val);
                }
                else if(!strcmp(unary_op.c_str(), "!")) {
                    val = !(unary_exp->val);
                }
            }
        }
    }
};

class RelExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> add_exp;
    unique_ptr<ComputeBaseAST> rel_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "RelExpAST {\n";
        #endif
        
        if(!strcmp(parse_type.c_str(), "addExp")) {
            add_exp->Dump();
            variable_name = add_exp->variable_name;
        }
        else if (!strcmp(parse_type.c_str(), "bin")) {
            rel_exp->Dump();
            add_exp->Dump();
            if(!strcmp(op.c_str(), "<")) {
                cout << "\t%" << tempID << " = lt " << rel_exp->variable_name << ", " << add_exp->variable_name << "\n";
            }
            else if(!strcmp(op.c_str(), ">")) {
                cout << "\t%" << tempID << " = gt " << rel_exp->variable_name << ", " << add_exp->variable_name << "\n";
            }
            else if(!strcmp(op.c_str(), "<=")) {
                cout << "\t%" << tempID << " = le " << rel_exp->variable_name << ", " << add_exp->variable_name << "\n";
            }
            else if(!strcmp(op.c_str(), ">=")) {
                cout << "\t%" << tempID << " = ge " << rel_exp->variable_name << ", " << add_exp->variable_name << "\n";
            }
            variable_name = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
    void Compute() override {
        if(!strcmp(parse_type.c_str(), "addExp")) {
            add_exp->Compute();
            val = add_exp->val;
        }
        else if (!strcmp(parse_type.c_str(), "bin")) {
            rel_exp->Compute();
            add_exp->Compute();
            if(!strcmp(op.c_str(), "<")) {
                val = (rel_exp->val < add_exp->val);
            }
            else if(!strcmp(op.c_str(), ">")) { 
                val = (rel_exp->val > add_exp->val);
            }
            else if(!strcmp(op.c_str(), "<=")) {
                val = (rel_exp->val <= add_exp->val);
            }
            else if(!strcmp(op.c_str(), ">=")) {
                val = (rel_exp->val >= add_exp->val);
            }
        }
    }
};

class EqExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> rel_exp;
    unique_ptr<ComputeBaseAST> eq_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "EqExpAST {\n";
        #endif
        
        if(!strcmp(parse_type.c_str(), "relExp")) {
            rel_exp->Dump();
            variable_name = rel_exp->variable_name;
        }
        else if (!strcmp(parse_type.c_str(), "bin")) {
            eq_exp->Dump();
            rel_exp->Dump();
            if(!strcmp(op.c_str(), "==")) {
                cout << "\t%" << tempID << " = eq " << eq_exp->variable_name << ", " << rel_exp->variable_name << "\n";
            }
            else if(!strcmp(op.c_str(), "!=")) {
                cout << "\t%" << tempID << " = ne " << eq_exp->variable_name << ", " << rel_exp->variable_name << "\n";
            }
            variable_name = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
    void Compute() override {
        if(!strcmp(parse_type.c_str(), "relExp")) {
            rel_exp->Compute();
            val = rel_exp->val;
        }
        else if (!strcmp(parse_type.c_str(), "bin")) {
            eq_exp->Compute();
            rel_exp->Compute();
            if(!strcmp(op.c_str(), "==")) {
                val = (eq_exp->val == rel_exp->val);
            }
            else if(!strcmp(op.c_str(), "!=")) {
                val = (eq_exp->val != rel_exp->val);
            }
        }
    }
};

class LAndExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> eq_exp;
    unique_ptr<ComputeBaseAST> l_and_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "LAndExpAST {\n";
        #endif
        
        if(!strcmp( parse_type.c_str(), "eqExp")) {
            eq_exp->Dump();
            variable_name = eq_exp->variable_name;
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
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
    void Compute() override {
        if(!strcmp( parse_type.c_str(), "eqExp")) {
            eq_exp->Compute();
            val = eq_exp->val;
        }
        else if (!strcmp( parse_type.c_str(), "bin")) {
            l_and_exp->Compute();
            eq_exp->Compute();
            val = l_and_exp->val && eq_exp->val;
        }
    }
};

class LOrExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> l_and_exp;
    unique_ptr<ComputeBaseAST> l_or_exp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "LOrExpAST {\n";
        #endif
        
        if(!strcmp( parse_type.c_str(), "lAndExp")) {
            l_and_exp->Dump();
            variable_name = l_and_exp->variable_name;
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
            tempID++;
        }
        else {
            assert(false);
        }
    
        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
    void Compute() override {
        if(!strcmp(parse_type.c_str(), "lAndExp")) {
            l_and_exp->Compute();
            val = l_and_exp->val;
        }
        else if (!strcmp(parse_type.c_str(), "bin")) {
            l_or_exp->Compute();
            l_and_exp->Compute();
            val = l_or_exp->val || l_and_exp->val;
        }
        else {
            assert(false);
        }
    }
};

class ConstExpAST : public ComputeBaseAST {
public:
    unique_ptr<ComputeBaseAST> exp;

    void Dump() override {
        exp->Dump();
    }
    void Compute() override {
        exp->Compute();
        val = exp->val;
    }
};


