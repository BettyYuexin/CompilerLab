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
#define REG_CNT 15

static string tempRegName[REG_CNT] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
static map<long, int> insts_table;
static map<long, string> var_table;
static bool regVisited[REG_CNT] = {};
static long stack_shift = 0;

static string epilogue;

// 函数声明
void Visit(const koopa_raw_program_t&);
void Visit(const koopa_raw_slice_t&);
void Visit(const koopa_raw_function_t&);
void Visit(const koopa_raw_basic_block_t&);
void Visit(const koopa_raw_value_t&);
void Visit(const koopa_raw_return_t&);
void Visit(const koopa_raw_integer_t&);
void Visit(const koopa_raw_binary_t&);
void VisitLocalAlloc(const koopa_raw_global_alloc_t&);
void Visit(const koopa_raw_store_t&);
void Visit(const koopa_raw_load_t&);

string getStoredAddr(long);
string getNextReg();
void releaseReg(string);
string getRegName(const koopa_raw_value_t&);

string getNextReg() {
  for(int i = 0; i < REG_CNT; i++) {
    if(!regVisited[i]) {
      regVisited[i] = true;
      return tempRegName[i];
    }
  }
  assert(false);
  return "Wrong!";
}

void releaseReg(string reg_name) {
  if(!strcmp(reg_name.c_str(), "x0")) 
    return;
  for(int i = 0; i < REG_CNT; i++) {
    if(!strcmp(reg_name.c_str(), tempRegName[i].c_str()) && regVisited[i]) {
      regVisited[i] = false;
      return;
    }
  }
  cout << reg_name << "not allocated" << endl;
  assert(false);
}

string getStoredAddr(long ptr) {
  string regName = "Wrong\n";
  if(var_table.count(ptr)) {
    regName = var_table[ptr];
  }
  else {
    regName = to_string(stack_shift) + "(sp)";
    var_table[ptr] = regName;
    stack_shift += 4;
  }
  return regName;
}

// 获取临时使用的整数，或者是之前用过的指令结果存储的地方
string getRegName(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;
  string regName = "Wrong\n";
  switch (kind.tag) {
    case KOOPA_RVT_INTEGER: {
      if (kind.data.integer.value) {
        regName = getNextReg();
        cout << "\tli\t\t" << regName  << ",\t" << kind.data.integer.value << endl;
      }
      else {
        regName = "x0";
      }
      break;
    }
    case KOOPA_RVT_ALLOC:
    case KOOPA_RVT_LOAD:
    case KOOPA_RVT_BINARY: {
      regName = getNextReg();
      string varName = getStoredAddr((long)(&kind.data));
      cout << "\tlw\t\t" << regName << ",\t" << varName << endl;
      break;
    }
    default:
      cout << "false val!!!" << kind.tag << endl;
      assert(false);
  }
  return regName;
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
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);

  #ifdef _DEBUG
  cout << " }\n";
  #endif
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    #ifdef _DEBUG
    cout << "slice " << i << ": " << slice.kind << endl;
    #endif
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
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  // 访问所有基本块
  #ifdef _DEBUG
  cout << "visit func:\n";
  Visit(func->bbs);
  #endif
  const char * funcName = func->name;
  string nameOut(funcName);
  cout << "\t.globl " << nameOut.substr(1) << endl;
  cout << nameOut.substr(1) << ":\n";
  
  // TODO 计算栈帧大小
  // size_t func_len = (func->bbs.len) * 4;
  int func_len = 256;
  // 16字节对齐
  // func_len = int((func_len + 15) / 16) * 16;
  
  // 更新栈帧
  cout << "\taddi\tsp,\tsp,\t" << -func_len << endl;

  epilogue = "\taddi\tsp,\tsp,\t"+to_string(func_len)+"\n";

  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // 访问所有指令
  #ifdef _DEBUG
  cout << "visit basic block: \n";
  Visit(bb->insts);
  #endif

  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  #ifdef _DEBUG
  cout << "visit value: " << value->kind.tag << "\n";
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
      Visit(kind.data.binary);
      break;
    }
    case KOOPA_RVT_ALLOC: {
      VisitLocalAlloc(kind.data.global_alloc);
      break;
    }
    case KOOPA_RVT_STORE: {
      Visit(kind.data.store);
      break;
    }
    case KOOPA_RVT_LOAD: {
      Visit(kind.data.load);
      break;
    }
    default:
      // cout << "false val!!!" << kind.tag << endl;
      assert(false);
  }
}

void Visit(const koopa_raw_load_t &load) {
  #ifdef _DEBUG
  cout << "visit load, loaded " << load.src->name <<  "\n";
  cout << "visit load, loaded " << load.src->kind.tag <<  "\n";
  #endif
  string var_addr = getStoredAddr(long(&(load.src->kind.data)));
  string regName = getNextReg();
  string dest_addr = getStoredAddr(long(&load));
  cout << "\tlw\t\t" << regName << ",\t" << var_addr << endl;
  cout << "\tsw\t\t" << regName << ",\t" << dest_addr << endl;
  releaseReg(regName);
}

void Visit(const koopa_raw_store_t &store) {
  #ifdef _DEBUG
  cout << "visit store, stored " << store.dest->name <<  "\n";
  #endif
  string var_addr = getStoredAddr(long(&(store.dest->kind.data)));
  string regName = getRegName(store.value);
  cout << "\tsw\t\t" << regName << ",\t" << var_addr << endl;
  releaseReg(regName);
}

void VisitLocalAlloc(const koopa_raw_global_alloc_t &alloc) {
  #ifdef _DEBUG
  cout << "visit alloc" << endl;
  #endif
  // getStoredAddr()
}

void Visit(const koopa_raw_return_t &ret) {
  #ifdef _DEBUG
  cout << "visit ret:\n";
  #endif
  if (&(ret.value->ty) == 0){
    cout << "  ret" << endl;
    return;
  }
  auto kind = ret.value->kind;
  switch(kind.tag) {
    case KOOPA_RVT_INTEGER:{
      cout << "\tli\ta0,\t" << ret.value->kind.data.integer.value << endl;
      cout << "  ret" << endl;
      break;
    }
    default: {
      string regName = getRegName(ret.value);
      cout << "\tmv\t\ta0, " << regName << endl;
      cout << epilogue;
      cout << "\tret\n";
      releaseReg(regName);
    }
  }

}

void Visit(const koopa_raw_integer_t &int_val) {
  #ifdef _DEBUG
  cout << "visit int_val:" << int_val.value << endl;
  #endif
  cout << "visit int_val:" << int_val.value << endl;
  // cout << "\tli\t" << regName  << ",\t" << int_val.value << endl;
}

void Visit(const koopa_raw_binary_t &bin_val) {
  #ifdef _DEBUG
  cout << "visit bin_val:" << endl;
  #endif

  string lhsName, rhsName, regName;

  lhsName = getRegName(bin_val.lhs);
  rhsName = getRegName(bin_val.rhs);
  regName = getNextReg();

  switch(bin_val.op) {
    case KOOPA_RBO_NOT_EQ: {
      cout << "\txor\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      cout << "\tsnez\t" << regName << ",\t" << regName << endl;
      break;
    }
    case KOOPA_RBO_EQ: {
      cout << "\txor\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      cout << "\tseqz\t" << regName << ",\t" << regName << endl;
      break;
    }
    case KOOPA_RBO_GT: {
      cout << "\tsgt\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_LT: {
      cout << "\tslt\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_GE: {
      cout << "\tslt\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      cout << "\tseqz\t" << regName << ",\t" << regName << endl;
      break;
    }
    case KOOPA_RBO_LE: {
      cout << "\tsgt\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      cout << "\tseqz\t" << regName << ",\t" << regName << endl;
      break;
    }
    case KOOPA_RBO_ADD: {
      cout << "\tadd\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl; 
      break;
    }
    case KOOPA_RBO_SUB: {
      cout << "\tsub\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_MUL: {
      cout << "\tmul\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_DIV: {
      cout << "\tdiv\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_MOD: { 
      cout << "\trem\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_AND: {
      cout << "\tand\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
      break;
    }
    case KOOPA_RBO_OR: {
      cout << "\tor\t\t" << regName << ",\t" << lhsName << ",\t" << rhsName << endl;
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

  string save_addr = getStoredAddr(long(&bin_val));
  cout << "\tsw\t\t" << regName << ",\t" << save_addr << endl;
  releaseReg(lhsName);
  releaseReg(rhsName);
  releaseReg(regName);
}
