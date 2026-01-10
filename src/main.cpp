#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <cassert>
using namespace std;

enum class Opcode {
    PUSH,
    POP,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    CALL,
    RET,
    HALT
};

struct callFrame {
    int returnIP;
    int frameBase;
};

struct VM {
    int ip;
    vector<int> st; // operand stack
    vector<callFrame> callst; //call stack
    vector<int> bc;
    VM(){ ip = 0; }
};

int main (){
    VM vm;
    vm.bc = {
        (int)Opcode::PUSH, 2,
        (int)Opcode::PUSH, 3,
        (int)Opcode::ADD,
        (int)Opcode::HALT
    }; //hard-coded bytecode for testing

    bool running = true;
    while (running){
        Opcode oc = (Opcode) vm.bc[vm.ip];
        switch (oc){
            case Opcode::PUSH: {
                int value = vm.bc[++vm.ip];
                vm.st.push_back(value);
                vm.ip++;
                continue;
            }
            case Opcode::POP:{
                assert(vm.st.size() >= 1);
                vm.st.pop_back();
                vm.ip++;
                continue;
            }
            case Opcode::ADD:{
                assert(vm.st.size() >= 2);
                int op2 = vm.st.back();
                vm.st.pop_back();
                int op1 = vm.st.back();
                vm.st.pop_back();
                vm.st.push_back(op1+op2);
                vm.ip++;
                continue;
            }
            case Opcode::SUB:{
                assert(vm.st.size() >= 2);
                int op2 = vm.st.back();
                vm.st.pop_back();
                int op1 = vm.st.back();
                vm.st.pop_back();
                vm.st.push_back(op1-op2);
                vm.ip++;
                continue;
            }
            case Opcode::MUL:{
                assert(vm.st.size() >= 2);
                int op2 = vm.st.back();
                vm.st.pop_back();
                int op1 = vm.st.back();
                vm.st.pop_back();
                vm.st.push_back(op1*op2);
                vm.ip++;
                continue;
            }
            case Opcode::DIV:{
                assert(vm.st.size() >= 2);
                int op2 = vm.st.back();
                assert(op2 != 0);
                vm.st.pop_back();
                int op1 = vm.st.back();
                vm.st.pop_back();
                vm.st.push_back(op1/op2);
                vm.ip++;
                continue;
            }
            case Opcode::MOD:{
                assert(vm.st.size() >= 2);
                int op2 = vm.st.back();
                assert(op2 != 0);
                vm.st.pop_back();
                int op1 = vm.st.back();
                vm.st.pop_back();
                vm.st.push_back(op1%op2);
                vm.ip++;
                continue;
            }
            case Opcode::HALT:{
                running = false;
                break;
            }
            case Opcode::CALL:{
                callFrame cf;
                cf.frameBase = vm.st.size();
                cf.returnIP = vm.ip + 2;
                vm.callst.push_back(cf);
                vm.ip = vm.bc[vm.ip + 1];
                continue;
            }
            case Opcode::RET:{
                assert(!vm.callst.empty());
                callFrame temp = vm.callst.back();
                int base = temp.frameBase;
                int retIP = temp.returnIP;

                int t;
                bool hasReturn = vm.st.size() > base;

                // if a return value exists, push it back on TOS after cleanup:
                if (hasReturn) { t = vm.st.back(); }
                vm.st.resize(base);
                if (hasReturn) vm.st.push_back(t); // operand stack cleanup

                vm.callst.pop_back(); // call stack cleanup
                vm.ip = retIP;
                continue;
            }
            default:{
                perror("Wrong opcode");
            }
        }
    }
}
