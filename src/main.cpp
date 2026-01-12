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
    ALLOC_STRING,
    ALLOC_ARRAY,
    GET_INDEX,
    SET_INDEX,
    HALT
};

enum class ValueType {
    INT,
    NIL,
    OBJECT
};

enum class HeapType {
    STRING,
    ARRAY
};

struct Value { //tagged union
    ValueType tag;
    union {
        int intVal;
        int objectHandle;
        //later-> float, bool, etc.
    } data;
    
    // static factory functions:
    
    static Value Int(int x){
        Value v;
        v.tag = ValueType::INT;
        v.data.intVal = x;
        return v;
    }

    static Value Nil(){
        Value v;
        v.tag = ValueType::NIL;
        return v;
    }

    static Value Object(int handle){
        Value v;
        v.tag = ValueType::OBJECT;
        v.data.objectHandle = handle;
        return v;
    }
    Value(){ tag = ValueType::NIL; } // to prevent accidental default construction.
};

struct HeapObject {
    HeapType type;
    int size;
    string st;
    vector<Value> arr;

    static HeapObject String(string str){
        HeapObject ob;
        ob.type = HeapType::STRING;
        ob.st = str;
        ob.size = str.size();
        return ob;
    }
    static HeapObject Array(int n){
        HeapObject ob;
        ob.type = HeapType::ARRAY;
        ob.arr = vector<Value>(n, Value::Nil());
        ob.size = n;
        return ob;
    }
    private:
        HeapObject(){}
};

struct callFrame {
    int returnIP;
    int frameBase;
};

struct VM {
    int ip;
    vector<Value> opst; // operand stack
    vector<callFrame> callst; //call stack

    vector<int> bc; //bytecode
    vector<string> constants; // constant pool (for now)

    vector<HeapObject> heap; //heap
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
        assert(vm.ip >= 0 && vm.ip < vm.bc.size());
        Opcode oc = (Opcode) vm.bc[vm.ip];
        switch (oc){
            case Opcode::PUSH: {
                Value value = Value::Int(vm.bc[++vm.ip]);
                vm.opst.push_back(value);
                vm.ip++;
                continue;
            }
            case Opcode::POP:{
                assert(vm.opst.size() >= 1);
                vm.opst.pop_back();
                vm.ip++;
                continue;
            }
            case Opcode::ADD:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT);
                vm.opst.pop_back();

                vm.opst.push_back(Value::Int(op1.data.intVal+op2.data.intVal));
                vm.ip++;
                continue;
            }
            case Opcode::SUB:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT);
                vm.opst.pop_back();

                vm.opst.push_back(Value::Int(op1.data.intVal-op2.data.intVal));
                vm.ip++;
                continue;
            }
            case Opcode::MUL:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT);
                vm.opst.pop_back();

                vm.opst.push_back(Value::Int(op1.data.intVal*op2.data.intVal));
                vm.ip++;
                continue;
            }
            case Opcode::DIV:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT);
                assert(op2.data.intVal != 0);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT);
                vm.opst.pop_back();

                vm.opst.push_back(Value::Int(op1.data.intVal/op2.data.intVal));
                vm.ip++;
                continue;
            }
            case Opcode::MOD:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT);
                assert(op2.data.intVal != 0);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT);
                vm.opst.pop_back();

                vm.opst.push_back(Value::Int(op1.data.intVal%op2.data.intVal));
                vm.ip++;
                continue;
            }
            case Opcode::HALT:{
                running = false;
                break;
            }
            case Opcode::CALL:{
                callFrame cf;
                cf.frameBase = vm.opst.size();
                assert(vm.ip + 1 < vm.bc.size());
                cf.returnIP = vm.ip + 2;
                vm.callst.push_back(cf);
                vm.ip = vm.bc[vm.ip + 1];
                continue;
            }
            case Opcode::ALLOC_STRING:{
                assert(vm.bc.size() > vm.ip + 1);
                int index = vm.bc[++vm.ip];
                string str = vm.constants[index]; //bytecode references strings in a constant pool, since it cannot pass strings on its own.
                int handle = vm.heap.size();
                vm.heap.push_back(HeapObject::String(str));
                vm.opst.push_back(Value::Object(handle));
                continue;
            }
            case Opcode::ALLOC_ARRAY:{
                int n = vm.bc[++vm.ip];
                int handle = vm.heap.size();
                vm.heap.push_back(HeapObject::Array(n));
                vm.opst.push_back(Value::Object(handle));
                continue;
            }
            case Opcode::GET_INDEX:{
                Value n = vm.opst.back();
                assert(n.tag == ValueType::INT);
                vm.opst.pop_back();
                
                Value ref = vm.opst.back();
                assert(ref.tag == ValueType::OBJECT);
                vm.opst.pop_back();
                assert(n.data.intVal < vm.heap[ref.data.objectHandle].arr.size() && n.data.intVal >= 0 
                && vm.heap[ref.data.objectHandle].type == HeapType::ARRAY);

                Value fetch = vm.heap[ref.data.objectHandle].arr[n.data.intVal];
                vm.opst.push_back(fetch);
                continue;
            }
            case Opcode::SET_INDEX:{
                Value value = vm.opst.back();
                vm.opst.pop_back();

                Value index = vm.opst.back();
                assert(index.tag == ValueType::INT);
                vm.opst.pop_back();

                Value ref = vm.opst.back();
                assert(ref.tag == ValueType::OBJECT);
                assert(index.data.intVal < vm.heap[ref.data.objectHandle].arr.size() && index.data.intVal >= 0 
                && vm.heap[ref.data.objectHandle].type == HeapType::ARRAY);
                vm.opst.pop_back();

                vm.heap[ref.data.objectHandle].arr[index.data.intVal] = value;
                continue;
            }
            case Opcode::RET:{
                assert(!vm.callst.empty());
                callFrame temp = vm.callst.back();
                int base = temp.frameBase;
                int retIP = temp.returnIP;
                
                bool hasReturn = vm.opst.size() > base;
                if (hasReturn) {
                    Value returnValue = vm.opst.back(); // only initialise a value if it will correspond to a real runtime value.
                    vm.opst.resize(base);
                    vm.opst.push_back(returnValue);
                } else {
                    vm.opst.resize(base);
                }

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
