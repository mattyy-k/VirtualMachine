#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <cassert>
#include <unordered_map>
#include <cctype>
#include <queue>
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
    PRINT,
    JUMP_IF_FALSE,
    JUMP,

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
    PRINT,
    ENDOF
};

struct Token {
    TokenType type;
    string lexeme;
    int line;
};
class Compiler;
struct Expr {
    virtual ~Expr() = default; // to enable polymorphism and consistent treatment of all expressions.
    virtual void compile(Compiler &c){};
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual void compile(Compiler& c){};
};

class Compiler {
    public:
    vector<int> bytecode;
    int nextLocalSlot = 0;
    
    Opcode opcodeFor(TokenType t) {
        switch (t) {
            case TokenType::PLUS: return Opcode::ADD;
            case TokenType::MINUS: return Opcode::SUB;
            case TokenType::MULTIPLY: return Opcode::MUL;
            case TokenType::DIVIDE: return Opcode::DIV;

            case TokenType::EQUAL_EQUAL: return Opcode::EQUAL;
            case TokenType::NOTEQUAL: return Opcode::NOTEQUAL;
            case TokenType::LESS_THAN: return Opcode::LESSTHAN;
            case TokenType::LESSEQUAL: return Opcode::LESSEQUAL;
            case TokenType::GRTR_THAN: return Opcode::GRTRTHAN;
            case TokenType::GRTREQL: return Opcode::GRTREQUAL;

            default: assert(false);
        }
    }
    unordered_map<string, int> varSlots;

    vector<int> compileProgram(vector<Stmt*> stmts){
        for (auto stmt : stmts){
            stmt->compile(*this);
        }
        bytecode.push_back((int)Opcode::HALT);
        return bytecode;
    }
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
    void compile(Compiler& c){
        int n = c.varSlots[name];
        c.bytecode.push_back((int)Opcode::GET_LOCAL);
        c.bytecode.push_back(n);
    }
};
struct ErrorExpr : Expr {
    int line;
    ErrorExpr(int l) : line(l) {}
};

struct ExprStmt : Stmt { // compile an expression
    Expr* expr;
    ExprStmt(Expr* e) : expr(e) {}
    void compile(Compiler& c){
        expr->compile(c);
        c.bytecode.push_back((int)Opcode::POP);
    }
};
struct VarDeclStmt : Stmt { // introduce a name and store a local value on the callstack
    string name;
    Expr* initializerExpr;
    VarDeclStmt(string n, Expr* e) : name(n), initializerExpr(e) {}

    void compile(Compiler& c){
        initializerExpr->compile(c); // push compiled value onto the stack
        c.varSlots[name] = c.nextLocalSlot;
        c.bytecode.push_back((int)Opcode::SET_LOCAL); // push setlocal back into bytecode
        c.bytecode.push_back(c.nextLocalSlot);
        c.nextLocalSlot++;
        c.bytecode.push_back((int)Opcode::POP); // pop compiled value from stack
    }
};
struct AssignmentStmt : Stmt { // modify an existing variable (variable must exist on the top of the callstack)
    string name;
    Expr* valueExpr;
    AssignmentStmt(string n, Expr* e) : name(n), valueExpr(e) {}
    void compile(Compiler& c){
        valueExpr->compile(c); //compile value
        c.bytecode.push_back((int)Opcode::SET_LOCAL); // emit set local
        c.bytecode.push_back(c.varSlots[name]); // emit index of variable in the locals vector
        c.bytecode.push_back((int)Opcode::POP);
    }
};
struct ErrorStmt : Stmt {
    int line;
    ErrorStmt(int l) : line(l) {
        assert(0 > 1);
    }
    void compile(Compiler& c){
        assert(0 > 1);
    }
};
struct AssignmentExpr : Expr {
    string name;
    Expr* valueExpr;
    AssignmentExpr(string n, Expr* e) : name(n), valueExpr(e) {}

    void compile(Compiler& c) {
        valueExpr->compile(c); // Pushes value to stack
        c.bytecode.push_back((int)Opcode::SET_LOCAL);
        c.bytecode.push_back(c.varSlots[name]); // value stays on the stack.
    }
};
struct PrintStmt : Stmt {
    Expr* expr;
    PrintStmt(Expr* e) : expr(e) {}

    void compile(Compiler& c) {
        expr->compile(c);
        c.bytecode.push_back((int)Opcode::PRINT);
    }
};
struct BlockStmt : Stmt {
    vector<Stmt*> stmts;
    BlockStmt(vector<Stmt*> v) : stmts(v) {}

    void compile(Compiler& c){
        for (auto stmt : stmts){
            stmt->compile(c);
        }
    }
};
struct IfStmt : Stmt {
    Expr* cond;
    Stmt* block;
    IfStmt(Expr* e, Stmt* b) : cond(e), block(b) {}

    int emitJump(Opcode jumptype, Compiler& c){
        c.bytecode.push_back((int)jumptype);
        return c.bytecode.size();
    }
    void patchJump(int jumpInd, Compiler& c){
        c.bytecode[jumpInd] = c.bytecode.size();
    }
    void compile(Compiler& c){
        cond->compile(c);
        int jumpIndex = emitJump(Opcode::JUMP_IF_FALSE, c);
        c.bytecode.push_back(0); //temporary value
        block->compile(c);
        patchJump(jumpIndex, c); // backpatch the jump index
    }
};
struct WhileStmt : Stmt {
    Expr* cond;
    Stmt* block;
    WhileStmt(Expr* e, Stmt* b) : cond(e), block(b) {}

    int emitJump(Opcode jumptype, Compiler& c){
        c.bytecode.push_back((int)jumptype);
        return c.bytecode.size();
    }
    void emitJump(Compiler& c, Opcode jumptype, int ls){
        c.bytecode.push_back((int)Opcode::JUMP);
        c.bytecode.push_back(ls);
    }
    void patchJump(int jumpInd, Compiler& c){
        c.bytecode[jumpInd] = c.bytecode.size();
    }
    void compile(Compiler& c){
        int loopStart = c.bytecode.size();
        cond->compile(c);
        int jumpIndex = emitJump(Opcode::JUMP_IF_FALSE, c);
        c.bytecode.push_back(0); //temporary value
        block->compile(c);
        emitJump(c, Opcode::JUMP, loopStart);
        patchJump(jumpIndex, c);
    }
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
        {"nil", TokenType::NIL},
        {"print", TokenType::PRINT}
    };
    
    vector<Token> tokens;
    int index = 0;

    // helper functions:
    char peek(){
        if (index + 1 < source.size()) return source[index + 1];
        else { index++; return '\0'; }
        index++;
        return '\0';
    }
    char advance(){
        if (index + 1 < source.size()) return source[index++];
        else { index++; return '\0'; }
        index++;
        return '\0';
    }
    bool match(char c){
        if (index + 1 < source.size()) {
            if (source[index + 1] == c){
                index++;
                return true;
            }
            else return false;
        }
        index++;
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
                    while (index < source.size() && (isalnum(peek()) || peek() == '_')) { index++; }
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

class Parser {
    public:
    vector<Token> tokens;
    int index;
    Parser(vector<Token> t) : tokens(t), index(0) {}

    // helper functions:
    Token peek (){
        if (index < tokens.size()) return tokens[index];
        Token temp;
        temp.type = TokenType::ENDOF;
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
    Token nextCheck(){
        if (index + 1 < tokens.size()) return tokens[index + 1];
        Token temp;
        temp.type = TokenType::ENDOF;
        return temp;
    }
    vector<Stmt*> parseProgram() {
        vector<Stmt*> stmts;
        while (peek().type != TokenType::ENDOF){
            stmts.push_back(parseStatement());
        }
        return stmts;
    }
    Stmt* parseStatement(){
        if (match(TokenType::PRINT)) {
            Expr* value = parseExpression();
            if (check(TokenType::SEMICOLON)) advance();
            return new PrintStmt(value);
        }
        if (peek().type == TokenType::IDENTIFIER) {
            if (nextCheck().type == TokenType::EQUAL){
                string temp = peek().lexeme;
                advance();
                advance();
                Stmt* stmt = new AssignmentStmt(temp, parseExpression());
                if (peek().type == TokenType::SEMICOLON) advance();
                return stmt;
            }
            else {
                Stmt* stmt = new ExprStmt(parseExpression());
                if (peek().type == TokenType::SEMICOLON) advance();
                return stmt;
            }
        }
        else if (match(TokenType::LET)){
            if (peek().type == TokenType::IDENTIFIER){
                string temp = peek().lexeme;
                advance();
                if (peek().type == TokenType::EQUAL) advance();
                Stmt* stmt = new VarDeclStmt(temp, parseExpression());
                if (peek().type == TokenType::SEMICOLON) advance();
                return stmt;
            }
        }
        else if (match(TokenType::IF)){
            if (match(TokenType::LEFT_PAREN)){
                Expr* cond = parseExpression();
                if (!match(TokenType::RIGHT_PAREN)) {
                    perror("Expected ')'");
                }
                
                Stmt* block = parseStatement();
                Stmt* ifstmt = new IfStmt(cond, block);
                return ifstmt;
            }
            //else compiler error
        }
        else if (match(TokenType::WHILE)){
            if (match(TokenType::LEFT_PAREN)){
                Expr* cond = parseExpression();
                if (!match(TokenType::RIGHT_PAREN)) {
                    perror("Expected ')'");
                }
                
                Stmt* block = parseStatement();
                Stmt* whstmt = new WhileStmt(cond, block);
                return whstmt;
            }
            // else compiler error
        }
        else if (match(TokenType::LEFT_BRACE)){
            vector<Stmt*> stmts;
            while (peek().type != TokenType::RIGHT_BRACE && peek().type != TokenType::ENDOF){
                stmts.push_back(parseStatement());
            }
            advance(); //consume right brace
            return new BlockStmt(stmts);
        }
        else //if (peek().type == TokenType::INTEGER) - WRONG, because statements can start with '-', '!', '(', etc, not just integers.
        {
            Stmt* stmt = new ExprStmt(parseExpression());
            if (peek().type == TokenType::SEMICOLON) advance();
            return stmt;
        }
        perror("Error: Unexpected token at statement");
        advance();
        Stmt* error = new ErrorStmt(peek().line);
        return error;

    }

    Expr* parseExpression() {
    return parseAssignment(); // New top of the chain
    }

    Expr* parseAssignment() {
        Expr* expr = parseEquality(); // Fall through to math

        if (match(TokenType::EQUAL)) {
            Token equals = tokens[index - 1];
            Expr* value = parseAssignment(); // Right-associative

            if (IdentifierExpr* i = dynamic_cast<IdentifierExpr*>(expr)) {
                return new AssignmentExpr(i->name, value); // Change this to an EXPR
            }
            perror("Invalid assignment target.");
        }

        return expr;
    }
    Expr* parseEquality(){
        Expr* left = parseComparison();
        while (check(TokenType::EQUAL_EQUAL) || check(TokenType::NOTEQUAL)){
            TokenType operand = peek().type;
            advance();
            Expr* right = parseComparison();

            left = new BinaryExpr(left, operand, right); // reassigning it to left to support chaining
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

            left = new BinaryExpr(left, operand, right); // again, reassigning it to left to support chaining
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
        advance();
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

queue<int> freedheap;
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
    for (const auto& val : vm.opst) {
        if (val.tag == ValueType::OBJECT) markObject(val.data.objectHandle, vm);
    }
    for (const auto& x : vm.callst){
        for (const auto& v : x.locals){
            if (v.tag == ValueType::OBJECT) markObject(v.data.objectHandle, vm); // marking locals on the callstack.
        }
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
            freedheap.push(i);
            if (vm.heap[i].type == HeapType::ARRAY) vm.heap[i].arr.erase(vm.heap[i].arr.begin(), vm.heap[i].arr.end());
            else vm.heap[i].st.erase();
            allocatedSinceLastGC--;
        }
        else vm.heap[i].marked = false;
    }
}

int main (){
    VM vm;
    ifstream file("program.vm");
    string src(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    ); // take the entire program as input string.
    Lexer lexer(src);
    vector<Token> tokens = lexer.scanTokens();
    Parser parser(tokens);
    vector<Stmt*> stmts = parser.parseProgram();
    Compiler c;
    vm.bc = c.compileProgram(stmts);
    vm.callst.push_back(callFrame(0, 0));
    for (auto x : c.bytecode) cout << x << " "; cout << "\n";

    bool running = true;
    while (running){
        assert(vm.ip >= 0 && vm.ip < vm.bc.size());
        Opcode oc = (Opcode) vm.bc[vm.ip];
        switch (oc){
            case Opcode::PUSH: {
                Value value = Value::Int(vm.bc[++vm.ip]);
                vm.opst.push_back(value);

                cout << "Pushed " << value.data.intVal << endl; // all such prints are for debugging purposes
                vm.ip++;
                continue;
            }
            case Opcode::POP:{
                assert(vm.opst.size() >= 1);
                vm.opst.pop_back();

                cout << "Popped" << endl;
                vm.ip++;
                continue;
            }
            case Opcode::ADD:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT || op2.tag == ValueType::BOOL); // as bool is implicitly convertible to int
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT || op1.tag == ValueType::BOOL);
                vm.opst.pop_back();
                op1.data.intVal = (op1.tag == ValueType::BOOL ? (int)op1.data.boolVal : op1.data.intVal);
                op2.data.intVal = (op2.tag == ValueType::BOOL ? (int)op2.data.boolVal : op2.data.intVal);
                vm.opst.push_back(Value::Int(op1.data.intVal+op2.data.intVal));

                cout << "Added " << op1.data.intVal << " and " << op2.data.intVal << endl;
                vm.ip++;
                continue;
            }
            case Opcode::SUB:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT || op2.tag == ValueType::BOOL);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT || op1.tag == ValueType::BOOL);
                vm.opst.pop_back();

                op1.data.intVal = (op1.tag == ValueType::BOOL ? (int)op1.data.boolVal : op1.data.intVal);
                op2.data.intVal = (op2.tag == ValueType::BOOL ? (int)op2.data.boolVal : op2.data.intVal);
                vm.opst.push_back(Value::Int(op1.data.intVal-op2.data.intVal));

                cout << "Subtracted " << op1.data.intVal << " and " << op2.data.intVal << endl;
                vm.ip++;
                continue;
            }
            case Opcode::MUL:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT || op2.tag == ValueType::BOOL);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT || op1.tag == ValueType::BOOL);
                vm.opst.pop_back();

                op1.data.intVal = (op1.tag == ValueType::BOOL ? (int)op1.data.boolVal : op1.data.intVal);
                op2.data.intVal = (op2.tag == ValueType::BOOL ? (int)op2.data.boolVal : op2.data.intVal);
                vm.opst.push_back(Value::Int(op1.data.intVal*op2.data.intVal));

                cout << "Multiplied " << op1.data.intVal << " and " << op2.data.intVal << endl;
                vm.ip++;
                continue;
            }
            case Opcode::DIV:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert(op2.tag == ValueType::INT || op2.tag == ValueType::BOOL);
                assert(op2.data.intVal != 0);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT || op1.tag == ValueType::BOOL);
                vm.opst.pop_back();

                op1.data.intVal = (op1.tag == ValueType::BOOL ? (int)op1.data.boolVal : op1.data.intVal);
                op2.data.intVal = (op2.tag == ValueType::BOOL ? (int)op2.data.boolVal : op2.data.intVal);
                vm.opst.push_back(Value::Int(op1.data.intVal/op2.data.intVal));

                cout << "Divided " << op1.data.intVal << " and " << op2.data.intVal << endl;
                vm.ip++;
                continue;
            }
            case Opcode::MOD:{
                assert(vm.opst.size() >= 2);
                Value op2 = vm.opst.back();
                assert((op2.tag == ValueType::INT || op2.tag == ValueType::BOOL));
                assert(op2.data.intVal != 0);
                vm.opst.pop_back();

                Value op1 = vm.opst.back();
                assert(op1.tag == ValueType::INT || op1.tag == ValueType::BOOL);
                vm.opst.pop_back();

                op1.data.intVal = (op1.tag == ValueType::BOOL ? (int)op1.data.boolVal : op1.data.intVal);
                op2.data.intVal = (op2.tag == ValueType::BOOL ? (int)op2.data.boolVal : op2.data.intVal);
                vm.opst.push_back(Value::Int(op1.data.intVal%op2.data.intVal));

                cout << "Mod " << op1.data.intVal << " and " << op2.data.intVal << endl;
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

                cout << "Called @ " << vm.ip << endl;
                continue;
            }
            case Opcode::ALLOC_STRING:{
                assert(vm.bc.size() > vm.ip + 1);
                int index = vm.bc[++vm.ip];
                string str = vm.constants[index]; //bytecode references strings in a constant pool, since it cannot pass strings on its own.
                int handle = (!freedheap.empty() ? freedheap.front() :  vm.heap.size());
                if (!freedheap.empty()) freedheap.pop();
                vm.heap.push_back(HeapObject::String(str));
                allocatedSinceLastGC++;
                if (allocatedSinceLastGC > 50) collectGarbage(vm);
                vm.opst.push_back(Value::Object(handle));

                cout << "Allocated string" << str << endl;
                continue;
            }
            case Opcode::ALLOC_ARRAY:{
                int n = vm.bc[++vm.ip];
                int handle = (!freedheap.empty() ? freedheap.front() :  vm.heap.size());
                if (!freedheap.empty()) freedheap.pop();
                vm.heap.push_back(HeapObject::Array(n));
                allocatedSinceLastGC++;
                if (allocatedSinceLastGC > 50) collectGarbage(vm);
                vm.opst.push_back(Value::Object(handle));

                cout << "Allocated array " << endl;
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

                cout << "Got heap value at index " << n.data.intVal << endl;
                vm.ip++;
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

                cout << "Set heap value at index " << index.data.intVal << endl;
                vm.ip++;
                continue;
            }
            case Opcode::GRTRTHAN:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal > right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));

                cout << "Compared > and pushed boolean " << ret << endl;
                vm.ip++;
                continue;
            }
            case Opcode::GRTREQUAL:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal >= right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));

                cout << "compared >= and pushed boolean " << ret << endl;
                vm.ip++;
                continue;
            }
            case Opcode::EQUAL:{
                assert(vm.opst.size() > 1);
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = (left.data.intVal == right.data.intVal);
                vm.opst.push_back(Value::Bool(ret));

                cout << "Compared == and pushed boolean " << ret << endl;
                vm.ip++;
                continue;
            }
            case Opcode::LESSTHAN:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal < right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));

                cout << "Compared < and pushed boolean " << ret << endl;
                vm.ip++;
                continue;
            }
            case Opcode::LESSEQUAL:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal <= right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));

                cout << "Compared <= and pushed boolean " << ret << endl;
                vm.ip++;
                continue;
            }
            case Opcode::NOTEQUAL:{
                Value right = vm.opst.back();
                vm.opst.pop_back();

                Value left = vm.opst.back();
                vm.opst.pop_back();

                bool ret = left.data.intVal != right.data.intVal;
                vm.opst.push_back(Value::Bool(ret));

                cout << "Compared != and pushed boolean " <<  ret << endl;
                vm.ip++;
                continue;
            }
            case Opcode::GET_LOCAL: {
                int n = vm.bc[++vm.ip];
                vm.opst.push_back(vm.callst.back().locals[n]);

                cout << "Pushed local @ " << n << endl;
                vm.ip++;
                continue;
            }
            case Opcode::SET_LOCAL:{
                int n = vm.bc[++vm.ip];
                Value val = vm.opst.back();
                //vm.opst.pop_back(); this line is incorrect and was a pretty frustrating bug; we already emit POP after SET_LOCAL, so the VM ends up popping twice-stack underflow
                callFrame &frame = vm.callst.back();
                
                if (n >= frame.locals.size()) frame.locals.resize(n + 1);
                vm.callst.back().locals[n] = val;

                cout << "Set local" << endl;
                vm.ip++;
                continue;
            }
            case Opcode::NEG:{
                assert(vm.opst.back().tag == ValueType::INT);
                int v = vm.opst.back().data.intVal;
                vm.opst.pop_back();
                vm.opst.push_back(Value::Int(-v));

                cout << "Negated and pushed integer" << -v << endl;
                vm.ip++;
                continue;
            }
            case Opcode::NOT:{
                assert(vm.opst.back().tag == ValueType::BOOL);
                bool v = vm.opst.back().data.boolVal;
                vm.opst.pop_back();
                vm.opst.push_back(Value::Bool(!v));

                cout << "Boolean negated and pushed boolean " << !v << endl;
                vm.ip++;
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

                cout << "Returned to: " << retIP << endl;
                continue;
            }
            case Opcode::PRINT: {
                Value v = vm.opst.back();
                vm.opst.pop_back();

                switch (v.tag) {
                    case ValueType::INT:
                        cout << v.data.intVal << "\n";
                        break;
                    case ValueType::BOOL:
                        cout << (v.data.boolVal ? "true" : "false") << "\n";
                        break;
                    case ValueType::NIL:
                        cout << "nil\n";
                        break;
                    default:
                        cout << "<object>\n";
                }

                vm.ip++;
                continue;
            }
            case Opcode::JUMP_IF_FALSE: {
                assert(vm.opst.size() > 0);
                Value val = vm.opst.back();
                vm.opst.pop_back();
                assert(val.tag == ValueType::BOOL);

                int n = vm.bc[++vm.ip];
                if (!val.data.boolVal) vm.ip = n;
                else vm.ip++;
                continue;
            }
            case Opcode::JUMP: {
                int n = vm.bc[++vm.ip];
                vm.ip = n;
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
