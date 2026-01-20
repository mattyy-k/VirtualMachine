#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <cassert>
#include <unordered_map>
#include <cctype>
using namespace std;

enum class Opcode {
    PUSH,
    POP,
    NEG,
    NOT,
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

    GET_LOCAL,
    SET_LOCAL,
    GET_GLOBAL,
    SET_GLOBAL,

    LESSTHAN,
    LESSEQUAL,
    GRTRTHAN,
    GRTREQUAL,
    EQUAL,
    NOTEQUAL,

    HALT
};

enum class ValueType {
    INT,
    NIL,
    BOOL,
    OBJECT
};

enum class HeapType {
    STRING,
    ARRAY
};
int allocatedSinceLastGC = 0;

enum class TokenType {
    LEFT_PAREN, // (
    RIGHT_PAREN, // )
    LEFT_BRACE, // {
    RIGHT_BRACE, // }
    SEMICOLON, // ;
    LEFT_BRACKET, // [
    RIGHT_BRACKET, // ]

    COMMA, // ,
    PLUS, // +
    MINUS, // -
    MULTIPLY, // *
    DIVIDE, // /
    MOD, // %

    EQUAL, // =
    NOT, // !
    GRTR_THAN, // >
    LESS_THAN, // <
    GRTREQL, // >=
    LESSEQUAL, // <=
    NOTEQUAL, // !=
    EQUAL_EQUAL, // ==

    IDENTIFIER,
    INTEGER,
    STRING,

    TAB,
    CARR_RETURN,

    LET,
    FUN,
    IF,
    ELSE,
    RETURN,
    WHILE,
    NIL,
    ENDOF
};

struct Token {
    TokenType type;
    string lexeme;
    int line;
};

struct Value { //tagged union
    ValueType tag;
    union {
        int intVal;
        bool boolVal;
        int objectHandle;
        //later-> float, etc.
    } data;
    
    // static factory functions:
    
    static Value Int(int x){
        Value v;
        v.tag = ValueType::INT;
        v.data.intVal = x;
        return v;
    }

    static Value Bool(bool x){
        Value v;
        v.tag = ValueType::BOOL;
        v.data.boolVal = x;
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

class Lexer {
    public:
    int linenum;
    string source;

    Lexer(string src) : source(src), linenum(1) {}

    unordered_map<string, TokenType> keywords = {
        {"let", TokenType::LET},
        {"fun", TokenType::FUN},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"return", TokenType::RETURN},
        {"nil", TokenType::NIL}
    };
    
    vector<Token> tokens;
    int index = 0;

    // helper functions:
    char peek(){
        assert(index + 1 < source.size());
        if (index + 1 < source.size()) return source[index + 1];
        else return '\0';
        return '\0';
    }
    char advance(){
        assert(index + 1 < source.size());
        if (index + 1 < source.size()) return source[index++];
        else return '\0';
        return '\0';
    }
    bool match(char c){
        assert(index + 1 < source.size());
        if (index + 1 < source.size()) {
            if (source[index + 1] == c){
                index++;
                return true;
            }
            else return false;
        }
        return false;
    }

    vector<Token> scanTokens() {
        while (index < source.size()){
            if (source[index] == ' ') { index++; continue; }
            else if (source[index] == '\n') { linenum++; index++; continue; }
            else if (source[index] == '\t') { 
                Token temp;
                temp.line = linenum;
                temp.lexeme = '\t';
                temp.type = TokenType::TAB;
                tokens.push_back(temp);
                index++;
            }
            else if (source[index] == '\r') { 
                Token temp;
                temp.line = linenum;
                temp.lexeme = '\r';
                temp.type = TokenType::CARR_RETURN;
                tokens.push_back(temp);
                index++;
            }

            char c = source[index];
            string tem;

            if (c == '(') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '(';
                temp.type = TokenType::LEFT_PAREN;
                tokens.push_back(temp);
                index++;
            }
            else if (c == ')') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = ')';
                temp.type = TokenType::RIGHT_PAREN;
                tokens.push_back(temp);
                index++;
            }
            else if (c == ';') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = ';';
                temp.type = TokenType::SEMICOLON;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '{') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '{';
                temp.type = TokenType::LEFT_BRACE;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '}') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '}';
                temp.type = TokenType::RIGHT_BRACE;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '[') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '[';
                temp.type = TokenType::LEFT_BRACKET;
                tokens.push_back(temp);
                index++;
            }
            else if (c == ']') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = ']';
                temp.type = TokenType::RIGHT_BRACKET;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '+') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '+';
                temp.type = TokenType::PLUS;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '-') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '-';
                temp.type = TokenType::MINUS;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '*') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '*';
                temp.type = TokenType::MULTIPLY;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '/') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '/';
                temp.type = TokenType::DIVIDE;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '%') {
                Token temp;
                temp.line = linenum;
                temp.lexeme = '%';
                temp.type = TokenType::MOD;
                tokens.push_back(temp);
                index++;
            }
            else if (c == '!'){
                if (match('=')) {
                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = "!=";
                    temp.type = TokenType::NOTEQUAL;
                    tokens.push_back(temp);
                    index++;
                }
                else {
                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = "!";
                    temp.type = TokenType::NOT;
                    tokens.push_back(temp);
                    index++;
                }
            }
            else if (c == '>'){
                if (match('=')) {
                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = ">=";
                    temp.type = TokenType::GRTREQL;
                    tokens.push_back(temp);
                    index++;
                }
                else {
                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = ">";
                    temp.type = TokenType::GRTR_THAN;
                    tokens.push_back(temp);
                    index++;
                }
            }
            else if (c == '<'){
                if (match('=')) {
                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = "<=";
                    temp.type = TokenType::LESSEQUAL;
                    tokens.push_back(temp);
                    index++;
                }
                else {
                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = "<";
                    temp.type = TokenType::LESS_THAN;
                    tokens.push_back(temp);
                    index++;
                }
            }
            else if (c == '='){
                if (match('=')) {
                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = "==";
                    temp.type = TokenType::EQUAL_EQUAL;
                    tokens.push_back(temp);
                    index++;
                }
                else {
                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = "=";
                    temp.type = TokenType::EQUAL;
                    tokens.push_back(temp);
                    index++;
                }
            }
            else {
                if (isalpha(c) || c == '_'){
                    int start = index;
                    while (index < source.size() && (isalnum(peek()) || peek() == '_')) index++;
                    index++;

                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = source.substr(start, index-start);
                    temp.type = TokenType::IDENTIFIER;
                    if (keywords.count(temp.lexeme)) temp.type = keywords[temp.lexeme];
                    tokens.push_back(temp);
                }
                else if (isdigit(c)){
                    int start = index;
                    while (index < source.size() && isdigit(peek())) index++;
                    index++;

                    Token temp;
                    temp.line = linenum;
                    temp.lexeme = source.substr(start, index-start);
                    temp.type = TokenType::INTEGER;
                    tokens.push_back(temp);
                }
            }
        }
        Token temp;
        temp.line = linenum;
        temp.lexeme = "EOF";
        temp.type = TokenType::ENDOF;
        tokens.push_back(temp);

        return tokens;
    }
};

class Compiler {
    public:
    vector<int> bytecode;
    unordered_map<TokenType, Opcode> map {
        {TokenType::PLUS, Opcode::ADD},
        {TokenType::MINUS, Opcode::SUB},
        {TokenType::MULTIPLY, Opcode::MUL},
        {TokenType::LESS_THAN, Opcode::LESSTHAN}
    };
    Opcode opcodeFor(TokenType t){
        return map[t];
    }
};

struct Expr {
    virtual ~Expr() = default; // to enable polymorphism and consistent treatment of all expressions.
    virtual void compile(Compiler &c){};
};
struct LiteralExpr : Expr {
    Value value;

    void compile (Compiler &c){
        c.bytecode.push_back((int)Opcode::PUSH);
        c.bytecode.push_back(value.data.intVal);
    }

    LiteralExpr(Value v) : value(v) {}
};
struct BinaryExpr : Expr {
    Expr* left;
    TokenType op;
    Expr* right;

    void compile(Compiler& c) {
        left->compile(c);
        right->compile(c);
        c.bytecode.push_back((int)c.opcodeFor(op));
    }

    BinaryExpr(Expr* l, TokenType o, Expr* r) : left(l), right(r), op(o) {}
};
struct UnaryExpr : Expr {
    TokenType op;
    Expr* exp;

    void compile(Compiler &c){
        exp->compile(c);
        if (op == TokenType::MINUS) c.bytecode.push_back((int)Opcode::NEG);
        else if (op == TokenType::NOT) c.bytecode.push_back((int)Opcode::NOT);
    }

    UnaryExpr(TokenType o, Expr* e) : op(o), exp(e) {}
};
struct GroupingExpr : Expr {
    Expr* exp;

    void compile(Compiler &c){
        exp->compile(c);
    }

    GroupingExpr(Expr* e) : exp(e) {}
};
struct IdentifierExpr : Expr {
    string name;

    IdentifierExpr(string n) : name(n) {}
};
struct ErrorExpr : Expr {
    int line;
    ErrorExpr(int l) : line(l) {}
};

class Parser {
    public:
    vector<Token> tokens;
    int index;
    Parser(vector<Token> t) : tokens(t), index(0) {}

    // helper functions:
    Token peek (){
        if (index < tokens.size()) return tokens[index];
        Token temp;
        temp.type == TokenType::ENDOF;
        return temp;
    }
    void advance(){
        index++;
    }
    bool match(TokenType t){
        if (index < tokens.size() && tokens[index].type == t) { index++; return true; }
        return false;
    }
    bool check(TokenType t){
        if (index < tokens.size() && tokens[index].type == t) return true;
        return false;
    }

    Expr* parseExpression(){
        return parseEquality();
    }
    Expr* parseEquality(){
        Expr* left = parseComparison();
        while (check(TokenType::EQUAL_EQUAL) || check(TokenType::NOTEQUAL)){
            TokenType operand = peek().type;
            advance();
            Expr* right = parseComparison();

            left = new BinaryExpr(left, operand, right); // reassigning it to left to support chains
        }
        return left;
    }
    Expr* parseComparison(){
        Expr* left = parseTerm();
        while (check(TokenType::GRTR_THAN) || check(TokenType::GRTREQL) || check(TokenType::LESS_THAN) || check(TokenType::LESSEQUAL)){
            TokenType operand = peek().type;
            advance();
            Expr* right = parseTerm();

            left = new BinaryExpr(left, operand, right); // reassigning it to left to support chains
        }
        return left;
    }
    Expr* parseTerm(){
        Expr* left = parseFactor();
        while (check(TokenType::PLUS) || check(TokenType::MINUS)){
            TokenType operand = peek().type;
            advance();
            Expr* right = parseFactor();

            left = new BinaryExpr(left, operand, right);
        }
        
        return left;
    }
    Expr* parseFactor(){
        Expr* left = parseUnary();
        while (check(TokenType::MULTIPLY) || check(TokenType::DIVIDE) || check(TokenType::MOD)){
            TokenType operand = peek().type;
            advance();
            Expr* right = parseUnary();

            left = new BinaryExpr(left, operand, right);
        }
        return left;
    }
    Expr* parseUnary(){
        if (peek().type == TokenType::MINUS || peek().type == TokenType::NOT){
            Token temp = peek();
            advance();
            Expr* expr = parseUnary();
            Expr* exp = new UnaryExpr(temp.type, expr);
            return exp;
        }
        else return parsePrimary();
    }
    Expr* parsePrimary(){
        if (peek().type == TokenType::INTEGER){
            int val = stoi(peek().lexeme);
            advance();
            Value value;
            value.tag = ValueType::INT;
            value.data.intVal = val;

            Expr* expr = new LiteralExpr(value);
            return expr; // this is a leaf node
        }
        else if (peek().type == TokenType::IDENTIFIER){
            Token temp = peek();
            advance();
            Expr* expr = new IdentifierExpr(temp.lexeme);
            return expr; //another leaf node
        }
        else if (peek().type == TokenType::LEFT_PAREN){
            advance();
            Expr* expr = parseExpression();
            if (peek().type == TokenType::RIGHT_PAREN) { 
                advance();
                GroupingExpr* exp = new GroupingExpr(expr);
                return exp;
            }
        }
        int line = tokens[index].line;
        Expr* error = new ErrorExpr(line);
        cerr << "Expected Expression on Line " << line;
        return error;
    }
};

struct HeapObject {
    HeapType type;
    int size;
    string st;
    vector<Value> arr;
    bool marked; // has reference from root (false -> destroyed, true -> alive)
    bool free;

    static HeapObject String(string str){
        HeapObject ob;
        ob.type = HeapType::STRING;
        ob.st = str;
        ob.size = str.size();
        ob.marked = false;
        ob.free = false;
        return ob;
    }
    static HeapObject Array(int n){
        HeapObject ob;
        ob.type = HeapType::ARRAY;
        ob.arr = vector<Value>(n, Value::Nil());
        ob.size = n;
        ob.marked = false;
        ob.free = false;
        return ob;
    }
    private:
        HeapObject(){}
};

struct callFrame {
    int returnIP;
    int frameBase;
    vector<Value> locals;
    callFrame(int ip, int fb) : returnIP(ip), frameBase(fb) {}
};

struct VM {
    int ip;
    vector<Value> opst; // operand stack
    vector<callFrame> callst; //call stack

    vector<int> bc; //bytecode
    vector<string> constants; // constant pool (for now)

    vector<HeapObject> heap; //heap
    VM(){ 
        ip = 0;
        callst.push_back(callFrame(-1, 0));
    }
};

void markObject(int ob, VM &vm){
    assert(ob < vm.heap.size());
    assert(!vm.heap[ob].free);
    if (vm.heap[ob].marked) return;

    vm.heap[ob].marked = true;
    if (vm.heap[ob].type == HeapType::ARRAY){
        int n = vm.heap[ob].arr.size();
        for (int i = 0; i < n; i++){
            if (vm.heap[ob].arr[i].tag == ValueType::OBJECT) {
                assert(vm.heap[ob].arr[i].data.objectHandle < vm.heap.size());
                markObject(vm.heap[ob].arr[i].data.objectHandle, vm);
            }
        }
    }
}
void markRoots(VM &vm){
    int n = vm.opst.size();
    for (int i = 0; i < n; i++){
        if (vm.opst[i].tag == ValueType::OBJECT) markObject(vm.opst[i].data.objectHandle, vm);
    }
}
void collectGarbage(VM &vm){
    // Mark Phase (recursive):
    markRoots(vm);
    // Sweep Phase:
    int n = vm.heap.size();
    for (int i = 0; i < n; i++){
        if (!vm.heap[i].marked) {
            vm.heap[i].free = true;
            if (vm.heap[i].type == HeapType::ARRAY) vm.heap[i].arr.erase(vm.heap[i].arr.begin(), vm.heap[i].arr.end());
            else vm.heap[i].st.erase();
            allocatedSinceLastGC--;
        }
        else vm.heap[i].marked = false;
    }
}

int main (){
    VM vm;
    string src = "int x = 5;\nint b =(x +5)/2;"; // take the entire program as input string.
    Lexer lexer(src);
    vector<Token> tokens = lexer.scanTokens();
    Parser parser(tokens);
    Expr* ast = parser.parseExpression();
    Compiler c;
    ast->compile(c);
    vm.bc = c.bytecode;

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
                assert(vm.ip + 2 < vm.bc.size());
                callFrame cf(vm.ip + 2, vm.opst.size());
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
                allocatedSinceLastGC++;
                if (allocatedSinceLastGC > 50) collectGarbage(vm);
                vm.opst.push_back(Value::Object(handle));
                continue;
            }
            case Opcode::ALLOC_ARRAY:{
                int n = vm.bc[++vm.ip];
                int handle = vm.heap.size();
                vm.heap.push_back(HeapObject::Array(n));
                allocatedSinceLastGC++;
                if (allocatedSinceLastGC > 50) collectGarbage(vm);
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
            case Opcode::GRTRTHAN:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal > right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));
                continue;
            }
            case Opcode::GRTREQUAL:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal >= right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));
                continue;
            }
            case Opcode::LESSTHAN:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal < right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));
                continue;
            }
            case Opcode::LESSEQUAL:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal <= right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));
                continue;
            }
            case Opcode::NOTEQUAL:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal != right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));
                continue;
            }
            case Opcode::GET_LOCAL: {
                int n = vm.bc[++vm.ip];
                vm.opst.push_back(vm.callst.back().locals[n]);
                continue;
            }
            case Opcode::SET_LOCAL:{
                int n = vm.bc[++vm.ip];
                int val = vm.opst.back().data.intVal;
                vm.callst.back().locals[n] = Value::Int(val);
                continue;
            }
            case Opcode::NEG:{
                int v = vm.opst.back().data.intVal;
                vm.opst.pop_back();
                vm.opst.push_back(Value::Int(-1*v));
                continue;
            }
            case Opcode::NOT:{
                assert(vm.opst.back().tag == ValueType::BOOL);
                bool v = vm.opst.back().data.boolVal;
                vm.opst.pop_back();
                vm.opst.push_back(Value::Int(!v));
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
                running = false;
                break;
            }
        }
    }
}
