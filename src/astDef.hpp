#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <sstream>
// #define _DEBUG


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
        #ifdef _DEBUG
        std::cout << "CompUnitAST {\n";
        #endif

        func_def->Dump();

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        #ifdef _DEBUG
        std::cout << "FuncDefAST {\n";
        #endif

        std::cout << "fun @" << ident << "(): ";
        func_type->Dump();
        std::cout << "{\n";
        block->Dump();
        std::cout << "}\n";

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

class FuncTypeAST : public BaseAST {
public:
    std::string type;

    void Dump() const override {
        #ifdef _DEBUG
        std::cout << "FuncTypeAST {\n";
        #endif

        std::cout << type;

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        #ifdef _DEBUG
        std::cout << "BlockAST {\n";
        #endif

        std::cout << "%" << "entry:\n";
        stmt->Dump();

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

class StmtAST : public BaseAST {
public:
    int number;

    void Dump() const override {
        #ifdef _DEBUG
        std::cout << "StmtAST {\n";
        #endif

        std::cout << "ret " << number << std::endl;

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};



