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
    HALT
};

struct VM {
    int ip;
    vector<int> st; // stack
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
    };

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
            default:{
                perror("Wrong opcode");
            }
        }
    }
}
