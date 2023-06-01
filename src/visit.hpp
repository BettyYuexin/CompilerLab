#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <sstream>
#include <map>
#include "koopa.h"
using namespace std;
// #define _DEBUG


string tempRegName[14] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
static map<long, string> insts_table;
static int regCnt = 0;

// 函数声明
void Visit(const koopa_raw_program_t&);
void Visit(const koopa_raw_slice_t&);
void Visit(const koopa_raw_function_t&);
void Visit(const koopa_raw_basic_block_t&);
void Visit(const koopa_raw_value_t&);
void Visit(const koopa_raw_return_t&);
void Visit(const koopa_raw_integer_t&);
void Visit(const koopa_raw_binary_t&);
string getVarName(const koopa_raw_value_t&);

string allocInstsTable(long);
string getVarName(long);
bool visitedInst(long);

string allocInstsTable(long ptr) {
  string regName = tempRegName[regCnt++];
  insts_table.insert(make_pair(ptr, regName));
  return regName;
}

string getVarName(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;
  string regName = "Wrong\n";
  switch (kind.tag) {
    case KOOPA_RVT_INTEGER: {
      if (kind.data.integer.value) {
        regName = allocInstsTable((long)(&kind.data.integer));
        cout << "\tli\t\t" << regName  << ",\t" << kind.data.integer.value << endl;
      }
      else {
        regName = "x0";
      }
      break;
    }
    case KOOPA_RVT_BINARY: {
      if(visitedInst((long)(&kind.data.binary))) {
        regName = insts_table[(long)(&kind.data.binary)];
      }
      else {
        assert(false);
      }
      break;
    }
    default:
      // cout << "false val!!!" << kind.tag << endl;
      assert(false);
  }
  return regName;
}

bool visitedInst(long ptr) {
  return insts_table.count(ptr);
}


// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  #ifdef _DEBUG
  cout << "visit program { \n";
  #endif
  cout << "\t.text\n";
  cout << "\t.globl main\n";
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);

  #ifdef _DEBUG
  cout << " }\n";
  #endif
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  #ifdef _DEBUG
  cout << "visit slice { \n";
  #endif

  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // cout << "false slice val!!!" << slice.kind << endl;
        assert(false);
    }

    #ifdef _DEBUG
    cout << " }\n";
    #endif
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有基本块
  #ifdef _DEBUG
  cout << "visit raw function { \n";
  #endif
  const char * funcName = func->name;
  string nameOut(funcName);
  cout << nameOut.substr(1) << ":\n";
  Visit(func->bbs);

  #ifdef _DEBUG
  cout << " }\n";
  #endif
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  #ifdef _DEBUG
  cout << "visit basic block { \n";
  #endif

  Visit(bb->insts);

  #ifdef _DEBUG
  cout << " }\n";
  #endif
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  #ifdef _DEBUG
  cout << "visit value { \n";
  #endif

  const auto &kind = value->kind;

  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER: {
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    }
    case KOOPA_RVT_BINARY: {
      allocInstsTable((long)value);
      Visit(kind.data.binary);
      break;
    }
    default:
      // cout << "false val!!!" << kind.tag << endl;
      assert(false);
  }

  #ifdef _DEBUG
  cout << " }\n";
  #endif
}

void Visit(const koopa_raw_return_t &ret) {
  #ifdef _DEBUG
  cout << "visit ret { \n";
  #endif

  string regName = getVarName(ret.value);
  cout << "\tmv\t\ta0, " << regName << endl;;
  cout << "\tret\n";

  #ifdef _DEBUG
  cout << " }\n";
  #endif
}

void Visit(const koopa_raw_integer_t &int_val) {
  #ifdef _DEBUG
  cout << "visit int_val:" << int_val.value << endl;
  #endif
  string regName = allocInstsTable((long)(&int_val));
  cout << "\tli " << regName  << "," << int_val.value << endl;
}

void Visit(const koopa_raw_binary_t &bin_val) {
  #ifdef _DEBUG
  cout << "visit bin_val:" << endl;
  #endif

  string lhsName, rhsName, saveReg;

  lhsName = getVarName(bin_val.lhs);
  rhsName = getVarName(bin_val.rhs);
  saveReg = allocInstsTable((long)(&bin_val));

  switch(bin_val.op) {
    case KOOPA_RBO_NOT_EQ: {
      cout << "ne\n";
      break;
    }
    case KOOPA_RBO_EQ: {
      cout << "\txor\t\t" << saveReg << ",\t" << lhsName << ",\t" << rhsName << endl;
      cout << "\tseqz\t" << saveReg << ",\t" << saveReg << endl;
      break;
    }
    case KOOPA_RBO_GT: {
      cout << "gt"; 
      break;
    }
    case KOOPA_RBO_LT: {
      cout << "lt"; 
      break;
    }
    case KOOPA_RBO_GE: {
      cout << "ge"; 
      break;
    }
    case KOOPA_RBO_LE: {
      cout << "le"; 
      break;
    }
    case KOOPA_RBO_ADD: {
      cout << "\tadd\t\t" << saveReg << ",\t" << lhsName << ",\t" << rhsName << endl; 
      break;
    }
    case KOOPA_RBO_SUB: {
      cout << "\tsub\t\t" << saveReg << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_MUL: {
      cout << "\tmul\t\t" << saveReg << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_DIV: {
      cout << "\tdiv\t\t" << saveReg << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_MOD: { 
      cout << "\tmod\t\t" << saveReg << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_AND: {
      cout << "and"; 
      break;
    }
    case KOOPA_RBO_OR: {
      cout << "or"; 
      break;
    }
    case KOOPA_RBO_XOR: {
      cout << "xor"; 
      break;
    }
    case KOOPA_RBO_SHL: {
      cout << "shl"; 
      break;
    }
    case KOOPA_RBO_SHR: {
      cout << "shr"; 
      break;
    }
    case KOOPA_RBO_SAR: {
      cout << "sar"; 
      break;
    }
    default:
      // cout << "false op!!!" << bin_op << endl;
      assert(false);
  }

}
