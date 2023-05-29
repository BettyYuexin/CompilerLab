#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
// #include <map>


// std::map <std::string, std::string> type2IRtype = {
//     {"int", "i32"}
// };

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        func_def->Dump();
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        std::cout << "func @" << ident << "(): ";
        func_type->Dump();
        std::cout << "{ ";
        block->Dump();
        std::cout << " }";
    }
};

class FuncTypeAST : public BaseAST {
public:
    std::string type;

    void Dump() const override {
        std::cout << type;
    }
};

class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << "%%entry: ";
        stmt->Dump();
    }
};

class StmtAST : public BaseAST {
public:
    int number;

    void Dump() const override {
        std::cout << "ret " << number;
    }
};



