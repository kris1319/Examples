#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <queue>
using namespace std;

bool file = false;

/*
   + 1.  Пофиксить баг с переводами строк между блоком if и блоком else
   + 2.  Удалить чекер на функцию с(...) + all func
    3.  Интерпретатор должен не все '\n' воспринимать как разделители
   + 4.  Написать ПОЛИЗ
   + 5.  Отладить ПОЛИЗ
    6.  Интерпретация
   + 7.  Таблицы для констант строковых и числовых
   8.  Реализация нужных структур данных
*/
enum type_of_lex {
    LEX_NULL, 
    LEX_RNULL, LEX_FALSE, LEX_TRUE, LEX_IF, LEX_ELSE, LEX_C, LEX_LEN, LEX_MODE, LEX_MTX, LEX_TMTX, LEX_ISVEC, LEX_ISMTX,
    LEX_FIN, LEX_FUNCASG, LEX_SEMICOLON, LEX_BYROW, LEX_NROW, LEX_NEXT, LEX_TIME, LEX_CONST, 
    LEX_ASSIGN, LEX_COLON, LEX_COMMA, LEX_EQ, LEX_LESS, LEX_GRT, LEX_LEQ, LEX_GEQ, LEX_NEQ, LEX_NEG, 
    LEX_AND, LEX_OR, LEX_MINUS, LEX_PLUS, LEX_MUL, LEX_DIV, LEX_MTXMUL, LEX_NCOL, LEX_UPLUS, LEX_UMINUS, 
    LEX_LBRACKET, LEX_RBRACKET, LEX_LPAREN, LEX_RPAREN, LEX_LBRACE, LEX_RBRACE,
    LEX_REAL, LEX_CHAR, LEX_ID, LEX_LOGIC, LEX_FUNC, LEX_DATA, LEX_ENTMTX, LEX_ENTVEC,
    POLIZ_FGO, POLIZ_GO, POLIZ_LABEL, POLIZ_IDENT, POLIZ_DEREF, POLIZ_EIND, POLIZ_PLUS
};
class Lex {
    public:
    type_of_lex t_lex;
    int v_lex;
public:
    Lex(type_of_lex t = LEX_NULL, int v = 0) {
        t_lex = t;
        v_lex = v;
    }

    type_of_lex get_type() {
        return t_lex;
    }
    int get_value() {
        return v_lex;
    }
    friend ostream& operator<< (ostream &out, Lex l);
};
class Ident {
public:
    char *name;//char *name;
    bool declare;
    type_of_lex type;
    vector <double> *num;
    vector <string> *str;
    vector <bool> *log;
    int r, c;
    int ind_base;
    vector <int> inds;
public:
    Ident(int x = 0, int y = 0, type_of_lex t = LEX_RNULL) : declare(false), ind_base(-1), name(NULL) {
        r = x;
        c = y;
        type = t;

        if (x) {
            if (t == LEX_REAL) num = new vector <double> [x];
            if (t == LEX_LOGIC) log = new vector <bool> [x];
            if (t == LEX_CHAR) str = new vector <string> [x];
        }
    }
    Ident(const Ident &a) {
        declare = a.declare;
        type = a.type;
        r = a.r; c = a.c;
        ind_base = a.ind_base;
        //inds = a.inds;
        //log = a.log; 
        if (type == LEX_LOGIC) {
            log = new vector <bool> [r];
            for (int i = 0; i < r; i++)
                for (int j = 0; j < c; j++)
                    log[i].push_back(a.log[i][j]);
        }
        if (type == LEX_REAL) {
            num = new vector <double> [r];
            for (int i = 0; i < r; i++)
                for (int j = 0; j < c; j++)
                    num[i].push_back(a.num[i][j]);
        }
        if (type == LEX_CHAR) {
            str = new vector <string> [r];
            for (int i = 0; i < r; i++)
                for (int j = 0; j < c; j++)
                    str[i].push_back(a.str[i][j]);
        }
        unsigned k = a.inds.size();
        if (k)
            for (unsigned i = 0; i < k; i++)
                inds.push_back(a.inds[i]);
        //num = a.num; str = a.str;
        if (a.name) {
            name = new char [strlen(a.name) + 1];
            strcpy(name, a.name);
        } else 
            name = NULL;
    }
    /*~Ident() {
        delete []name;
        delete []num;
        delete []log;
        delete []str;
    }*/

    char *get_name() {
        return name;
    }
    void put_name(const char *new_name) {
        int l = strlen(new_name);
        name = new char [l + 1];
        strcpy(name, new_name);
    }
    bool get_declare() {
        return declare;
    }
    void put_declare() {
        declare = true;
    }
    type_of_lex get_type() {
        return type;
    }
    void put_type(type_of_lex t) {
        type = t;
    }
    void erase() {
        //delete []name;
        //if (type == LEX_REAL) delete []num;
        //if (type == LEX_LOGIC) delete []log;
        //if (type == LEX_CHAR) delete []str;
    }
};
class tabl_ident {
    vector <Ident> p;
    int top;
public:
    tabl_ident() {
        top = 0;
    }

    Ident& operator[] (int k) {
        return p[k];
    }
    int put(char *str) {
        for (int i = 0; i < top; i++)
            if (p[i].get_name() && !strcmp(str, p[i].get_name()))
                return i;

        Ident tmp;
        tmp.put_name(str);
        p.push_back(tmp);
        top++;

        return top - 1;
    }
    int push(Ident &x) {
        p.push_back(x);
        top++;
        return top - 1;
    }
    void erase(int i) {
        p[i].erase();
    }
};
class Scanner {
    enum state {
        H, IDENT, NUM, NREAL, CHAR, COM, CMP, EQ, LASG, MMUL, DELIM, INUM
    };

    static char *TW[];
    static char *TD[];
    static char *TF[];
    static char *TA[];
    static type_of_lex words[];
    static type_of_lex dlms[];
    static type_of_lex funcs[];
    static type_of_lex args[];
    state CS;
    char c, buf[101];
    int buf_top;
    FILE *fp;

    void clear() {
        for (int i = 0; i < 101; i++)
            buf[i] = '\0';
        buf_top = 0;
    }
    void add() {
        if (buf_top == 100)
            throw c;
        buf[buf_top++] = c;
    }
    int look(const char *buf, char **table) {
        int i = 0;
        while (table[i] && strcmp(table[i], buf))
            i++;
        return (table[i] ? i : 0);
    }
    void gc() {
        c = !file ? fgetc(fp) : getchar();
    }

public:
    Lex get_lex();
    Scanner(const char *program) {
        if (!program)
            file = true;
        else 
            fp = fopen(program, "r");
        buf_top = 0;
        CS = H;
        clear();
        if (file)
            cout << ">  ";
        gc();
    }
};
template <class T> class Stack {
    vector <T> st;
    int size;
public:
    Stack() : size(0) { }
    void push(T type) {
        st.push_back(type);
        size++;
    }
    T pop() {
        T tmp = st[size - 1];
        st.pop_back();
        size--;
        return tmp;
    }
    void clear() {
        size = 0;
        st.clear();
    }
    bool isempty() {
        return st.empty();
    }
    int get_size() {
        return size;
    }

    T& operator[] (int i) {
        return st[i];
    }
};
class Poliz {
    vector <Lex> p;
    int size;
public:
    Poliz () {
        size = 0;
    }

    void put_lex(Lex l) {
        p.push_back(l);
        size++;
    }
    void put_lex(Lex l, int place) {
        if (place >= size)
            throw place;
        p[place] = l;
    }
    void put_space() {
        p.push_back(Lex());
        size++;
    }
    int get_size() {
        return size;
    }
    void print() {
        for (int i = 0; i < size; i++) {
            cout << p[i].get_type() << endl;
        }
    }
    
    Lex& operator[] (int i) {
        if (i >= size)
            throw i;
        return p[i];
    }
};
class Parser {
public:         /////////////////////////////////
    Lex curr_lex;
    type_of_lex c_type;
    int c_val;
    Scanner scan;
    Poliz prog;

    int c_block;
    bool st, mtx_st, if_el;
    Stack <type_of_lex> st_lex;
    Stack <int> st_id;
    int count_of_args;

    void Program();
    void Exp();  void Exp1();  void Exp2();  void Exp3();  void Exp4();  void Exp5();  void Exp6();  void Exp7();
    void Un();  void Const();  void Mtx_ArglistElem();  void Cond();  void C_Call();  void Mtx_Call();  void T_Call();
    void Len_Call();  void Mode_Call();  void IsVec_Call();  void IsMtx_Call();
    /*
    void dec();
    void check_id();
    void check_upm();
    void check_op();
    void check_not();
    void check_ind();
    void check_c();
    void check_mtx();
    void check_if();
    */
    void gl() {
        curr_lex = scan.get_lex();
        c_type = curr_lex.get_type();
        c_val = curr_lex.get_value();
    }
    void ngl() {
        do {
            gl();
            if (file)
                cout << "+  ";
        } while (c_type == LEX_NEXT);
    }

public:
    Parser(const char *Program) : scan(Program), c_block(0), if_el(false) { 
    }
    void analyze() {
        gl();
        Program();
    }
};
class Executer {
    Lex item;
public:
    void neg();
    void neq();
    void upm(bool);
    void pm_md(int);
    void mtxmul();
    void leqg(int);
    void comma();
    void colon();
    void c(int);
    void length();
    void mode();
    void t();
    void is_vec_or_mtx(bool);
    void rbrace();
    void assign();
    void deref();

    Stack <Lex> args;
    Lex tmp, tmp2;
    int size;
    int ind;
    void execute(Poliz &prog);
    void copy_id(Ident &a, Ident &b);
    void cget_mem(Ident &id, int r, int c, type_of_lex t);
    bool func_cond(int t, Ident &a, Ident &b, int k, int l, int j);
    void log_to_double(Ident &id, Ident &id2);
    void double_to_log(Ident &id, Ident &id2);
    void log_to_string(Ident &id, Ident &id2);
    void double_to_string(Ident &id, Ident &id2);
};
class Interpretator {
    Parser pars;
    Executer exe;
public:
    Interpretator(const char *prog) : pars(prog) {
        if (!prog)
            file = true;
    }
    void interpretation() {        
        pars.analyze();
        if (!file) 
            exe.execute(pars.prog);
    }
};

Executer glob;
tabl_ident TID;
string str;
double d = 0;

char * Scanner::TW[] = {
    "", "NULL", "TRUE", "FALSE", "if", "else", NULL
};
char * Scanner::TD[] = {
    "", ",", ":", ";", "=", "\n", 
    "+", "-", "*", "/", "%*%", "!", "&", "|", ">", "<", ">=", "<=", "==", "!=", "<-", 
    "(", ")", "[", "]", "{", "}", NULL
};
char * Scanner::TF[] = {
    "", "c", "matrix", "t", "length", "mode", "is.vector", "is.matrix", NULL
};
char * Scanner::TA[] = {
    "", "nrow", "ncol", "byrow", "data", NULL
};

type_of_lex Scanner::words[] = {
    LEX_NULL, LEX_RNULL, LEX_TRUE, LEX_FALSE, LEX_IF, LEX_ELSE, LEX_NULL
};
type_of_lex Scanner::dlms[] = {
    LEX_NULL, LEX_COMMA, LEX_COLON, LEX_SEMICOLON, LEX_FUNCASG, LEX_NEXT, 
    LEX_PLUS, LEX_MINUS, LEX_MUL, LEX_DIV, LEX_MTXMUL, LEX_NEG, LEX_AND, LEX_OR,
    LEX_GRT, LEX_LESS, LEX_GEQ, LEX_LEQ, LEX_EQ, LEX_NEQ, LEX_ASSIGN, 
    LEX_LPAREN, LEX_RPAREN, LEX_LBRACKET, LEX_RBRACKET, LEX_LBRACE, LEX_RBRACE, LEX_NULL
};
type_of_lex Scanner::funcs[] = {
    LEX_NULL, LEX_C, LEX_MTX, LEX_TMTX, LEX_LEN, LEX_MODE, LEX_ISVEC, LEX_ISMTX, LEX_NULL
};
type_of_lex Scanner::args[] = {
    LEX_NULL, LEX_NROW, LEX_NCOL, LEX_BYROW, LEX_DATA, LEX_NULL
};

Lex Scanner::get_lex() {
    int i;
    bool id = false;
    double n = 1e-1;
    CS = H;
    do {
        switch (CS) {
        case H:
            if (isalpha(c) || c == '_') {
                clear();
                add();
                gc();
                CS = IDENT;
            } else if (c == '.') { 
                clear();
                add();
                gc();
                CS = INUM;
            } else if(isdigit(c)) {
                d = c - '0';
                gc();
                CS = NUM;
            } else if (c == '"') {
                str.clear();
                //str.push_back(c);
                gc();
                CS = CHAR;
            } else if (c == '#') {
                gc();
                CS = COM;
            } else if (c == '>' || c == '!') {
                clear();
                add();
                gc();
                CS = CMP;
            } else if (c == '=') {
                clear();
                add();
                gc();
                CS = EQ;
            } else if (c == '<') {
                clear();
                add();
                gc();
                CS = LASG;
            } else if (c == '%') {
                clear();
                add();
                gc();
                CS = MMUL;
            } else if (c == EOF)
                return Lex(LEX_FIN);
            else if (c != '\n' && isspace(c))
                gc();
            else
                CS = DELIM;
            break;

        case INUM:
            if (isdigit(c)) {
                d = 0;
                CS = NREAL;
            } else
                CS = IDENT;
            break;
        case IDENT:
            if (!id && (isalpha(c) || isdigit(c) || c == '.' || c == '_')) {
                add();
                gc();
            } else if (isspace(c) && c != '\n') {
                if ((i = look(buf, TW)))
                    return Lex(words[i], i);
                id = true;
                gc();
            } else if (c == '(') {
                    if ((i = look(buf, TF)))
                        return Lex(funcs[i], i);
                    i = TID.put(buf);
                    return Lex(LEX_ID, i);
            } else if (c == '=') {
                if ((i = look(buf, TA)))
                        return Lex(args[i], i);
                    i = TID.put(buf);
                    return Lex(LEX_ID, i);
            } else {
                    if ((i = look(buf, TW)))
                        return Lex(words[i], i);
                    i = TID.put(buf);
                    return Lex(LEX_ID, i);
            }
            break;

        case NUM:
            if (isdigit(c)) {
                d = 10 * d + (c - '0');
                gc();
            } else if (c == '.') {
                gc();
                CS = NREAL;
            } else if (isalpha(c))
                throw c;
            else {
                Ident tmp(1, 1, LEX_REAL);
                //tmp.put_type(LEX_CONST);
                tmp.num[0].push_back(d);
                return Lex(LEX_REAL, TID.push(tmp));
            }
            break;

        case NREAL:
            if (isdigit(c)) {
                d += n * (c - '0');
                n /= 10;
                gc();
            } else if (isalpha(c))
                throw c;
            else  {
                Ident tmp(1, 1, LEX_REAL);
                //tmp.put_type(LEX_CONST);
                tmp.num[0].push_back(d);
                return Lex(LEX_REAL, TID.push(tmp));
            }
            break;
        
        case CHAR:
            if (c == '"') {
                //str.push_back(c);
                gc();
                Ident tmp(1, 1, LEX_CHAR);
                //tmp.put_type(LEX_CONST);
                tmp.str[0].push_back(str);
                return Lex(LEX_CHAR, TID.push(tmp));
            } else if (c == EOF)
                throw c;
            else {
                str.push_back(c);
                gc();
            }
            break;

        case COM:
            if (c == '\n') {
                //gc();
                CS = DELIM;
            } else if (c == EOF)
                return Lex(LEX_FIN);
            else
                gc();
            break;

        case CMP:
            if (c == '=') {
                add();
                gc();
                i = look(buf, TD);
                return Lex(dlms[i], i);
            } else {
                i = look(buf, TD);
                return Lex(dlms[i], i);
            }
            break;

        case EQ:
            if (c == '=') {
                add();
                gc();
                i = look(buf, TD);
                return Lex(dlms[i], i);
            } else {
                i = look(buf, TD);
                return Lex(dlms[i], i);
            }
            break;
        
        case LASG:
            if (c == '=' || c == '-') {
                add();
                gc();
                i = look(buf, TD);
                return Lex(dlms[i], i);
            } else {
                i = look(buf, TD);
                return Lex(dlms[i], i);
            }
            break;

        case MMUL:
            if (c == '*') {
                add();
                gc();
                if (c == '%') {
                    add();
                    gc();
                    i = look(buf, TD);
                    return Lex(dlms[i], i);
                } else
                    throw c;
            } else 
                throw c;

        case DELIM:
            clear();
            add();
            if (c == '\n' && file)
                cout << ">  ";
            gc();
            if (!(i = look(buf, TD)))
                throw c;
            return Lex(dlms[i], i);
            break;
    
        default:
            return Lex(LEX_FIN);
        }
    } while (true);
}

void Parser::Program() {
    if (c_type != LEX_FIN || c_type != LEX_RBRACE) {
        Exp();
        prog.put_lex(Lex(LEX_SEMICOLON));
        if (file) {
            glob.execute(prog);
            if (c_type == LEX_NEXT)
                cout << ">  ";
        }
        if (!if_el && c_type != LEX_FIN && c_type != LEX_RBRACE && c_type != LEX_SEMICOLON && c_type != LEX_NEXT)
            throw curr_lex;
        while (c_type == LEX_SEMICOLON || c_type == LEX_NEXT || if_el) {
            //prog.put_lex(Lex(LEX_SEMICOLON));
            
            if (!if_el)
                ngl();
            else
                if_el = false;
            if (c_type == LEX_RBRACE)
                break;
            if (c_type != LEX_SEMICOLON && c_type != LEX_NEXT) {
                Exp();
                prog.put_lex(Lex(LEX_SEMICOLON));
                if (file) {
                    glob.execute(prog);
                    if (c_type == LEX_NEXT)
                        cout << ">  ";
                }
            }
        }
        if (c_type != LEX_FIN && c_type != LEX_RBRACE)
            throw curr_lex;
        if (c_type == LEX_RBRACE && !c_block)
                throw curr_lex;
        //prog.put_lex(Lex(LEX_SEMICOLON));
    }
}
void Parser::Exp() {
    if (c_type == LEX_FIN || c_type == LEX_NEXT || (c_type == LEX_RBRACE && c_block))
        return;
    st = false;
    if (c_type == LEX_ID) {
        prog.put_lex(Lex(LEX_ID, c_val));  //  opz
        gl();
        if (c_type == LEX_LBRACKET) {
            ngl();
            if (c_type == LEX_COMMA) {
                prog.put_lex(Lex(POLIZ_EIND));  //  opz
                ngl();
                Exp();
                //prog.put_lex(Lex(LEX_COMMA));  //  opz
            } else {
                Exp();
                if (c_type == LEX_COMMA) {
                    ngl();
                    if (c_type != LEX_RBRACKET) {
                        Exp();
                    } else
                        prog.put_lex(Lex(POLIZ_EIND));  //  opz
                    //prog.put_lex(Lex(LEX_COMMA));  //  opz
                }
            }
            if (c_type != LEX_RBRACKET)
                throw curr_lex;
            //prog.put_lex(Lex(POLIZ_PLUS));  //  opz
            gl();
            prog.put_lex(Lex(POLIZ_DEREF));  //  opz
        }
        
        if (c_type == LEX_ASSIGN) {
            ngl();
            Exp();
            prog.put_lex(Lex(LEX_ASSIGN));  //  opz
        } else {
            st = true;
            Exp1();
        }
    } else
        Exp1();
}
void Parser::Exp1() {
    Exp2();
    while (c_type == LEX_OR || c_type == LEX_AND) {
        type_of_lex tmp = c_type;
        ngl();
        Exp2();
        prog.put_lex(Lex(tmp));  //  opz
    }
}
void Parser::Exp2() {
    bool f = false;
    if (!st && c_type == LEX_NEG) {
        f = true;
        ngl();
    }
    Exp3();
    if (f)
        prog.put_lex(Lex(LEX_NEG));  //  opz
}
void Parser::Exp3() {
    Exp4();
    if (c_type == LEX_GRT || c_type == LEX_LESS || c_type == LEX_GEQ || c_type == LEX_LEQ || c_type == LEX_EQ || c_type == LEX_NEQ) {
        type_of_lex tmp = c_type;
        ngl();
        Exp4();
        prog.put_lex(Lex(tmp));  //  opz
    }
}
void Parser::Exp4() {
    Exp5();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS) {
        type_of_lex tmp = c_type;
        ngl();
        Exp5();
        prog.put_lex(Lex(tmp));  //  opz
    }
}
void Parser::Exp5() {
    Exp6();
    while (c_type == LEX_MUL || c_type == LEX_DIV || c_type == LEX_MTXMUL) {
        type_of_lex tmp = c_type;
        ngl();
        Exp6();
        prog.put_lex(Lex(tmp));  //  opz
    }
}
void Parser::Exp6() {
    if (!st)
        Un();
    else {
        st = false;
    }
    while (c_type == LEX_COLON) {
        ngl();
        Un();
        prog.put_lex(Lex(LEX_COLON));  //  opz
    }
}
void Parser::Un() {
    bool f = false;
    type_of_lex tmp = c_type;
    if (c_type == LEX_MINUS || c_type == LEX_PLUS) {
        f = true;
        ngl();
    }
    Exp7();
    if (f) {
        if (tmp == LEX_MINUS)
            prog.put_lex(Lex(LEX_UMINUS));  //  opz
        else
            prog.put_lex(Lex(LEX_UPLUS));  //  opz
    }
}
void Parser::Exp7() {
    if (c_type == LEX_LPAREN) {
        ngl();
        Exp();
        if (c_type != LEX_RPAREN)
            throw curr_lex;
        gl();
    } else if (c_type == LEX_LBRACE) {
        prog.put_lex(Lex(LEX_LBRACE));  //  opz
        c_block++;
        ngl();
        Program();
        if (c_type != LEX_RBRACE)
            throw curr_lex;
        else 
            c_block--;
        prog.put_lex(Lex(LEX_RBRACE));  //  opz
        gl();
    } else if (c_type == LEX_ID) {
        prog.put_lex(Lex(LEX_ID, c_val));  //  opz
        gl();
        if (c_type == LEX_LBRACKET) {
            ngl();
            if (c_type == LEX_COMMA) {
                prog.put_lex(Lex(POLIZ_EIND));  //  opz
                ngl();
                Exp();
                //prog.put_lex(Lex(LEX_COMMA));  //  opz
            } else {
                Exp();
                if (c_type == LEX_COMMA) {
                    ngl();
                    if (c_type != LEX_RBRACKET) {
                        Exp();
                    } else 
                        prog.put_lex(Lex(POLIZ_EIND));  //  opz
                    //prog.put_lex(Lex(LEX_COMMA));  //  opz
                }
            }
            if (c_type != LEX_RBRACKET)
                throw curr_lex;
            //prog.put_lex(Lex(POLIZ_PLUS));  //  opz
            gl();
            prog.put_lex(Lex(POLIZ_DEREF));  //  opz
        }
    } else if (c_type == LEX_C)
        C_Call();
    else if (c_type == LEX_MTX)
        Mtx_Call();
    else if (c_type == LEX_TMTX)
        T_Call();
    else if (c_type == LEX_ISVEC) {
        IsVec_Call();
    } else if (c_type == LEX_ISMTX) {
        IsMtx_Call();
    } else if (c_type == LEX_LEN) {
        Len_Call();
    } else if (c_type == LEX_MODE) {
        Mode_Call();
    } else if (c_type == LEX_IF) 
        Cond();
    else 
        Const();
}
void Parser::C_Call() {
    gl();
    if (c_type != LEX_LPAREN)
        throw curr_lex;
    ngl();
    if (c_type == LEX_RPAREN)
        throw "bad args in function C";

    Exp();
    while (c_type == LEX_COMMA) {
        ngl();
        Exp();
        prog.put_lex(Lex(LEX_COMMA));  //  opz
    }
    
    /*bool f = false;
    do {
        if (f)
            ngl();
        else
            f = true;
        Exp();
    } while (c_type == LEX_COMMA);*/
    
    if (c_type != LEX_RPAREN)
        throw curr_lex;
    prog.put_lex(Lex(LEX_C));  //  opz
    gl();
}
void Parser::Len_Call() {
    gl();
    if (c_type != LEX_LPAREN)
        throw curr_lex;
    ngl();
    if (c_type == LEX_RPAREN)
        throw "bad args in function Length";
    Exp();
    if (c_type != LEX_RPAREN)
        throw "bad args in function Length";
    prog.put_lex(Lex(LEX_LEN));  //  opz
    gl();
}
void Parser::Mode_Call() {
    gl();
    if (c_type != LEX_LPAREN)
        throw curr_lex;
    ngl();
    if (c_type == LEX_RPAREN)
        throw "bad args in function Mode";
    Exp();
    if (c_type != LEX_RPAREN)
        throw "bad args in function Mode";
    prog.put_lex(Lex(LEX_MODE));  //  opz
    gl();
}
void Parser::Mtx_Call() {
    mtx_st = false;
    bool f = false, g = false;
    gl();
    if (c_type != LEX_LPAREN)
        throw curr_lex;
    ngl();
    if (c_type == LEX_RPAREN) {
        gl();
        return;
    }
    
    Mtx_ArglistElem();
    while (c_type == LEX_COMMA) {
        ngl();
        Mtx_ArglistElem();
        prog.put_lex(Lex(LEX_COMMA));  //  opz
    }

    if (c_type != LEX_RPAREN)
        throw curr_lex;
    prog.put_lex(Lex(LEX_MTX));  //  opz
    gl();
}
void Parser::Mtx_ArglistElem() { 
    bool f = false;
    if (c_type == LEX_DATA || c_type == LEX_BYROW || c_type == LEX_NROW || c_type == LEX_NCOL) {
        if (c_type != LEX_DATA || count_of_args != 1)
            mtx_st = true;
        prog.put_lex(Lex(c_type));  //  opz
        ngl();
        if (c_type != LEX_FUNCASG) 
            throw curr_lex;
        f = true;
        ngl();
    } else if (mtx_st)
        throw "bad sequence of args";

    Exp();
    if (f)
        prog.put_lex(Lex(LEX_FUNCASG));  //  opz
}
void Parser::IsVec_Call() {
    gl();
    if (c_type != LEX_LPAREN)
        throw curr_lex;
    ngl();
    if (c_type == LEX_RPAREN)
        throw "bad args in function Is.vector";
    Exp();
    if (c_type != LEX_RPAREN)
        throw "bad args in function Is.vector";
    prog.put_lex(Lex(LEX_ISVEC));  //  opz
    gl();
}
void Parser::IsMtx_Call() {
    gl();
    if (c_type != LEX_LPAREN)
        throw curr_lex;
    ngl();
    if (c_type == LEX_RPAREN)
        throw "bad args in function Is.matrix";
    Exp();
    if (c_type != LEX_RPAREN)
        throw "bad args in function Is.matrix";
    prog.put_lex(Lex(LEX_ISMTX));  //  opz
    gl();
}
void Parser::T_Call() {
    gl();
    if (c_type != LEX_LPAREN)
        throw curr_lex;
    ngl();
    if (c_type == LEX_RPAREN)
        throw "bad args in function T";
    Exp();
    if (c_type != LEX_RPAREN)
        throw "bad args in function T";
    prog.put_lex(Lex(LEX_TMTX));  //  opz
    gl();
}
void Parser::Cond() {
    ngl();
    if (c_type != LEX_LPAREN)
        throw curr_lex;
    ngl();
    Exp();
    if (c_type != LEX_RPAREN)
        throw curr_lex;
    int p1 = prog.get_size();
    prog.put_space();
    prog.put_lex(Lex(POLIZ_FGO));  //  opz
    ngl();
    Exp();

    int p2 = prog.get_size();
    prog.put_space();
    prog.put_lex(Lex(POLIZ_GO));  //  opz
    prog.put_lex(Lex(POLIZ_LABEL, prog.get_size()), p1);  //  opz
    if (c_type == LEX_NEXT)
        ngl();
    if (c_type == LEX_ELSE) {
        ngl();
        Exp();
    } else {
        prog.put_lex(Lex(LEX_RNULL));  //  opz
        if_el = true;
    }
    prog.put_lex(Lex(POLIZ_LABEL, prog.get_size()), p2);  //  opz
}
void Parser::Const() {
    if (c_type != LEX_REAL && c_type != LEX_CHAR && c_type != LEX_TRUE && c_type != LEX_FALSE && c_type != LEX_RNULL)
        throw curr_lex;
    prog.put_lex(Lex(c_type, c_val));  //  opz
    gl();
}

/*
    1. Добавить RNULL как тип вектора
    2. Мониторить конечные NULL-элементы
    3. Уничтожать все ненужные временные объекты
*/

void Executer::cget_mem (Ident &id, int r, int c, type_of_lex t) {
    id.r = r;
    id.c = c;
    id.type = t;
    if (t == LEX_REAL)
        id.num = new vector <double> [r];
    else if (t == LEX_LOGIC)
        id.log = new vector <bool> [r];
    if (t == LEX_CHAR)
        id.str = new vector <string> [r];
}
void Executer::log_to_double(Ident &id, Ident &id2) {
    id.type = LEX_REAL;
    cget_mem (id, id2.r, id2.c, LEX_REAL);
    for (int k = 0; k < id.r; k++)
        for (int l = 0; l < id.c; l++)
            id.num[k].push_back(id2.log[k][l] ? 1.0 : 0);
}
void Executer::log_to_string(Ident &id, Ident &id2) {
    id.type = LEX_CHAR;
    cget_mem (id, id2.r, id2.c, LEX_CHAR);
    for (int k = 0; k < id.r; k++)
        for (int l = 0; l < id.c; l++)
            id.str[k].push_back(id2.log[k][l] ? "TRUE" : "FALSE");
}
void Executer::double_to_log(Ident &id, Ident &id2) {
    id.type = LEX_LOGIC;
    cget_mem (id, id2.r, id2.c, LEX_LOGIC);
    for (int k = 0; k < id.r; k++) 
        for (int l = 0; l < id.c; l++)
            id.log[k].push_back(id2.num[k][l] ? 1 : 0);
}
void Executer::double_to_string(Ident &id, Ident &id2) {
    id.type = LEX_CHAR;
    cget_mem (id, id2.r, id2.c, LEX_CHAR);
    for (int k = 0; k < id.r; k++)
        for (int l = 0; l < id.c; l++)
            id.str[k].push_back(to_string(id2.num[k][l]));
}

bool Executer::func_cond(int t, Ident &a, Ident &b, int k, int l, int j) {
    switch (a.type) {
    case LEX_REAL:
        switch (t) {
        case 1: return a.num[k][l] < b.num[k][j];
        case 2: return a.num[k][l] == b.num[k][j];
        case 3: return a.num[k][l] > b.num[k][j];
        case 4: return a.num[k][l] != b.num[k][j];
        case 5: return a.num[k][l] <= b.num[k][j];
        case 6: return a.num[k][l] >= b.num[k][j];
        }
    case LEX_CHAR:
        switch (t) {
        case 1: return a.str[k][l] < b.str[k][j];
        case 2: return a.str[k][l] == b.str[k][j];
        case 3: return a.str[k][l] > b.str[k][j];
        case 4: return a.str[k][l] != b.str[k][j];
        case 5: return a.str[k][l] <= b.str[k][j];
        case 6: return a.str[k][l] >= b.str[k][j];
        }
    default:
        return false;
    }
}
void Executer::copy_id(Ident &a, Ident &b) {
    a.erase();
    a.type = b.type;
    a.r = b.r;
    a.c = b.c;
    switch (b.type) {
    case LEX_REAL:
        a.num = new vector <double> [b.r];
        for (int i = 0; i < b.r; i++)
            for (int j = 0; j < b.c; j++)
                a.num[i].push_back(b.num[i][j]);
        break;
    case LEX_LOGIC:
        a.log = new vector <bool> [b.r];
        for (int i = 0; i < b.r; i++)
            for (int j = 0; j < b.c; j++)
                a.log[i].push_back(b.log[i][j]);
        break;
    case LEX_CHAR:
        a.str = new vector <string> [b.r];
        for (int i = 0; i < b.r; i++)
            for (int j = 0; j < b.c; j++)
                a.str[i].push_back(b.str[i][j]);
    default:    break;
    }
}

void Executer::neg() {
    tmp = args.pop();
    type_of_lex t = tmp.t_lex;
    if (t == LEX_ID && !TID[tmp.v_lex].declare)
        throw "non-declarated identifier";
    if (t == LEX_CONST && TID[tmp.v_lex].type == LEX_RNULL) {
        args.push(tmp);
        return;
    }

    Ident res;
    Ident &id = TID[tmp.v_lex];
    if (id.type == LEX_CHAR)
        throw "bad arg";
    if (id.type == LEX_REAL) {
        double_to_log(res, id);
        for (int k = 0; k < res.r; k++)
            for (int l = 0; l < res.c; l++)
                res.log[k][l] = !res.log[k][l];
    }
    else {
        res.type = LEX_LOGIC;
        cget_mem (res, id.r, id.c, LEX_LOGIC);
        for (int k = 0; k < res.r; k++)
            for (int l = 0; l < res.c; l++)
                res.log[k].push_back(!id.log[k][l]);
    }
    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::upm(bool pm) {
    tmp = args.pop();
    type_of_lex t = tmp.t_lex;
    if (t == LEX_ID && !TID[tmp.v_lex].declare)
        throw "non-declarated identifier";
    if (t == LEX_CONST && TID[tmp.v_lex].type == LEX_RNULL) {
        args.push(tmp);
        return;
    }

    Ident res;
    Ident &id = TID[tmp.v_lex];
    if (id.type == LEX_CHAR)
        throw "bad arg";
    if (id.type == LEX_LOGIC) {
        log_to_double(res, id);
        if (pm)
            for (int k = 0; k < id.r; k++)
                for (int l = 0; l < id.c; l++)
                    res.num[k][l] = -res.num[k][l];
    } else {
        res.type = LEX_REAL;
        cget_mem (res, id.r, id.c, LEX_REAL);
        for (int k = 0; k < id.r; k++)
            for (int l = 0; l < id.c; l++) {
                if (pm) res.num[k].push_back(-id.num[k][l]);
                else res.num[k].push_back(id.num[k][l]);
            }
    }

    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::pm_md(int t) {
    tmp2 = args.pop();
    tmp = args.pop();
    type_of_lex t1 = tmp.t_lex;
    type_of_lex t2 = tmp2.t_lex;
    if (t1 == LEX_CONST && TID[tmp.v_lex].type == LEX_RNULL) {
        args.push(tmp);
        return;
    }
    if (t2 == LEX_CONST && TID[tmp2.v_lex].type == LEX_RNULL) {
        args.push(tmp2);
        return;
    }

    Ident &id1 = TID[tmp.v_lex];
    Ident &id2 = TID[tmp2.v_lex];
    Ident tid1, tid2;

    if (t1 == LEX_ID && !id1.declare)
        throw "non-declarated identifier";
    if (t2 == LEX_ID && !id2.declare)
        throw "non-declarated identifier";
    if (id1.type == LEX_CHAR || id2.type == LEX_CHAR)
        throw "bad type";
    if (id1.r == id2.r && id1.r != 1 && id1.c != id2.c)
        throw "bad pair of args";

    bool f = false, g = false;
    if (id1.type == LEX_LOGIC) {
        log_to_double(tid1, id1);
        f = true;
    }
    Ident &a = !f ? id1 : tid1;
    if (id2.type == LEX_LOGIC) {
        log_to_double(tid2, id2);
        g = true;
    }
    Ident &b = !g ? id2 : tid2;

    Ident res;
    res.type = LEX_REAL;
    if (a.c > b.c) {
        int j = 0;
        cget_mem(res, a.r, a.c, LEX_REAL);
        for (int k = 0; k < a.r; k++)
            for (int l = 0, j = 0; l < a.c; l++, j++) {
                if (j >= b.c)   j = 0;

                switch (t) {
                case 1: res.num[k].push_back(a.num[k][l] + b.num[k][j]);
                    break;
                case 2: res.num[k].push_back(a.num[k][l] - b.num[k][j]);
                    break;
                case 3: res.num[k].push_back(a.num[k][l] * b.num[k][j]);
                    break;
                case 4: res.num[k].push_back(a.num[k][l] / b.num[k][j]);
                    break;
                }
            }
    } else {
        int j = 0;
        cget_mem(res, b.r, b.c, LEX_REAL);
        for (int k = 0; k < a.r; k++)
            for (int l = 0, j = 0; l < b.c; l++, j++) {
                if (j >= a.c)   j = 0;

                switch (t) {
                case 1: res.num[k].push_back(a.num[k][j] + b.num[k][l]);
                    break;
                case 2: res.num[k].push_back(a.num[k][j] - b.num[k][l]);
                    break;
                case 3: res.num[k].push_back(a.num[k][j] * b.num[k][l]);
                    break;
                case 4: res.num[k].push_back(a.num[k][j] / b.num[k][l]);
                    break;
                }
            }
    }

    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    if (tmp2.t_lex == LEX_TIME || tmp2.t_lex == LEX_CONST)
        TID.erase(tmp2.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::mtxmul() {
    tmp2 = args.pop();
    tmp = args.pop();
    type_of_lex t1 = tmp.t_lex;
    type_of_lex t2 = tmp2.t_lex;
    Ident &id1 = TID[tmp.v_lex];
    Ident &id2 = TID[tmp2.v_lex];
    Ident tid1, tid2;

    if (t1 == LEX_ID && !id1.declare)
        throw "non-declarated identifier";
    if (t2 == LEX_ID && !id2.declare)
        throw "non-declarated identifier";
    if (id1.type == LEX_CHAR || id2.type == LEX_CHAR)
        throw "bad type";
    if (t1 == LEX_CONST && TID[tmp.v_lex].type == LEX_RNULL) {
        args.push(tmp);
        return;
    }
    if (t2 == LEX_CONST && TID[tmp2.v_lex].type == LEX_RNULL) {
        args.push(tmp2);
        return;
    }
    if (id1.c != id2.r)
        throw "bad pair of args";

    bool f = false;
    if (id1.type == LEX_LOGIC) {
        log_to_double(tid1, id1);
        f = true;
    }
    Ident &a = f ? id1 : tid1;
    f = false;
    if (id2.type == LEX_LOGIC) {
        log_to_double(tid2, id2);
        f = true;
    }
    Ident &b = f ? id2 : tid2;

    Ident res;
    res.type = LEX_REAL;
    cget_mem(res, a.r, b.c, LEX_REAL);
    for (int k = 0; k < a.r; k++)
        for (int l = 0; l < b.c; l++) {
            double ans = 0;
            for (int i = 0; i < a.r; i++)
                ans += a.num[k][i] * b.num[i][l];
            res.num[k].push_back(ans);
        }

    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    if (tmp2.t_lex == LEX_TIME || tmp2.t_lex == LEX_CONST)
        TID.erase(tmp2.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::leqg(int t) {
    tmp2 = args.pop();
    tmp = args.pop();
    type_of_lex t1 = tmp.t_lex;
    type_of_lex t2 = tmp2.t_lex;
    Ident &id1 = TID[tmp.v_lex];
    Ident &id2 = TID[tmp2.v_lex];
    Ident tid1, tid2;

    if (t1 == LEX_ID && !id1.declare)
        throw "non-declarated identifier";
    if (t2 == LEX_ID && !id2.declare)
        throw "non-declarated identifier";
    if (t1 == LEX_CONST && TID[tmp.v_lex].type == LEX_RNULL || t2 == LEX_CONST && TID[tmp2.v_lex].type == LEX_RNULL) {
        Ident res(1, 1, LEX_LOGIC);
        res.log[0][0] = false;
        tmp.v_lex = TID.push(res);
        tmp.t_lex = LEX_CONST;
        args.push(tmp);
        return;
    }
    if (id1.r == id2.r && id1.r != 1 && id1.c != id2.c)
        throw "bad pair of args";

    bool f = false, g = false;
    if (id1.type == LEX_CHAR) {
        if (id2.type != LEX_CHAR) {
            g = true;
            id2.type == LEX_LOGIC ? log_to_string(tid2, id2) : double_to_string(tid2, id2);
        }
    } else if (id2.type == LEX_CHAR) {
        if (id1.type != LEX_CHAR) {
            f = true;
            id1.type == LEX_LOGIC ? log_to_string(tid1, id1) : double_to_string(tid1, id1);
        }
    } else {
        if (id1.type == LEX_LOGIC) {
            log_to_double(tid1, id1);
            f = true;
        }
        if (id2.type == LEX_LOGIC) {
            log_to_double(tid2, id2);
            g = true;
        }
    }
       
    Ident &a = f ? tid1 : id1;
    Ident &b = g ? tid2 : id2;

    Ident res;
    res.type = LEX_LOGIC;
    if (a.c > b.c) {
        int j = 0;
        cget_mem(res, a.r, a.c, LEX_LOGIC);
        for (int k = 0; k < a.r; k++)
            for (int l = 0, j = 0; l < a.c; l++, j++) {
                if (j >= b.c)   j = 0;
                res.log[0].push_back(func_cond(t, a, b, k, l, j));
            }
    } else {
        int j = 0;
        cget_mem(res, b.r, b.c, LEX_LOGIC);
        for (int k = 0; k < a.r; k++)
            for (int l = 0, j = 0; l < b.c; l++, j++) {
                if (j >= a.c)   j = 0;
                res.log[0].push_back(func_cond(t, a, b, k, j, l));
            }
    }

    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    if (tmp2.t_lex == LEX_TIME || tmp2.t_lex == LEX_CONST)
        TID.erase(tmp2.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::colon() {
    tmp2 = args.pop();
    tmp = args.pop();
    type_of_lex t1 = tmp.t_lex;
    type_of_lex t2 = tmp2.t_lex;
    if (t1 == LEX_RNULL || t2 == LEX_RNULL) {
        args.push(tmp);
        return;
    }
    Ident &id1 = TID[tmp.v_lex];
    Ident &id2 = TID[tmp2.v_lex];
    Ident tid1, tid2;

    if (t1 == LEX_ID && !id1.declare)
        throw "non-declarated identifier";
    if (t2 == LEX_ID && !id2.declare)
        throw "non-declarated identifier";
    if (id1.type == LEX_CHAR || id2.type == LEX_CHAR)
        throw "bad type";
    if (t1 == LEX_CONST && TID[tmp.v_lex].type == LEX_RNULL) {
        args.push(tmp);
        return;
    }
    if (t2 == LEX_CONST && TID[tmp2.v_lex].type == LEX_RNULL) {
        args.push(tmp2);
        return;
    }
    if (id1.c == 0 || id2.c == 0 || id1.r == 0 || id2.r == 0)
        throw "bad size";

    double a = id1.type == LEX_LOGIC ? (double)id1.log[0][0] : id1.num[0][0];
    double b = id2.type == LEX_LOGIC ? (double)id2.log[0][0] : id2.num[0][0];
    Ident res(1, 0, LEX_REAL);
    if (a <= b) 
        for ( ; a <= b; a += 1) {
            res.num[0].push_back(a);
            res.c++;
        }
    else 
        for ( ; a >= b; a -= 1) {
            res.num[0].push_back(a);
            res.c++;
        }
    
    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    if (tmp2.t_lex == LEX_TIME || tmp2.t_lex == LEX_CONST)
        TID.erase(tmp2.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::c(int k) {
    tmp = args.pop();
    if (tmp.t_lex != LEX_CONST)
        throw "bad arg in function c(...)";

    Ident &id = TID[tmp.v_lex];
    //res.type = id.type;
    int cnull = 0;
    Ident res(1, 1, id.type);
    if (id.type == LEX_RNULL)
        cnull++;
    if (id.type == LEX_REAL) 
        res.num[0].push_back(id.num[0][0]);
    else if (id.type == LEX_LOGIC) 
        res.log[0].push_back(id.log[0][0]);
    else 
        res.str[0].push_back(id.str[0][0]);
    TID.erase(tmp.v_lex);
    
    while (k > 0) {
        k--;
        tmp = args.pop();
        if (tmp.t_lex != LEX_CONST)
            throw "bad arg in function c(...)";
        Ident &p = TID[tmp.v_lex];
        if (p.type == LEX_RNULL) 
            cnull++;

        if (cnull && res.type == LEX_RNULL && p.type == LEX_CHAR) {
            res.type = LEX_CHAR;
        }

        if (p.type == LEX_CHAR && res.type == LEX_CHAR)
            res.str[0].push_back(p.str[0][0]);
        else if (p.type == LEX_REAL && res.type == LEX_REAL)
            res.num[0].push_back(p.num[0][0]);
        else if (p.type == LEX_LOGIC && res.type == LEX_LOGIC)
            res.log[0].push_back(p.log[0][0]);
        else if (p.type == LEX_CHAR && res.type != LEX_CHAR) {
            Ident temp;
            res.type == LEX_REAL ? double_to_string(temp, res) : log_to_string(temp, res);
            copy_id(res, temp);
            res.str[0].push_back(p.str[0][0]);
        }
        else if (p.type != LEX_CHAR && res.type == LEX_CHAR) {
            if (p.type == LEX_REAL)
                res.str[0].push_back(to_string(p.num[0][0]));
            else
                res.str[0].push_back(p.log[0][0] ? "TRUE" : "FALSE");
        }
        else if (p.type == LEX_REAL && res.type == LEX_LOGIC) {
            Ident temp;
            log_to_double(temp, res);
            copy_id(res, temp);
            res.num[0].push_back(p.num[0][0]);
        }
        else if (p.type == LEX_LOGIC && res.type == LEX_REAL)
            res.num[0].push_back(p.log[0][0] ? 1.0 : 0);

        TID.erase(tmp.v_lex);
        res.c++;
    }

    int i = 0;
    while (i < res.c / 2) {
        switch (res.type) {
        case LEX_REAL:
            swap(res.num[0][i], res.num[0][res.c - i - 1]);
            break;
        case LEX_LOGIC:
            swap(res.log[0][i], res.log[0][res.c - i - 1]);
            break;
        case LEX_CHAR:
            swap(res.str[0][i], res.str[0][res.c - i - 1]);
            break;
        }
        i++;
    }

    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::t() {
    tmp = args.pop();
    Ident &id = TID[tmp.v_lex];
    
    if (tmp.t_lex == LEX_ID && !id.declare)
        throw "non-declarated identifier";

    for (int i = 0; i < id.r; i++)
        for (int j = 0; j < id.c; j++) {
            switch (id.type) {
            case LEX_REAL:  id.num[j][i] = id.num[i][j];
                break;
            case LEX_LOGIC: id.log[j][i] = id.log[i][j];
                break;
            case LEX_CHAR:  id.str[j][i] = id.str[i][j];
                break;
            }
        }
    args.push(tmp);
}
void Executer::mode() {
    tmp = args.pop();
    type_of_lex t = tmp.t_lex;
    Ident &id = TID[tmp.v_lex];
    if (t == LEX_ID && !id.declare)
        throw "non-declarated identifier";
    
    Ident res(1, 1, LEX_CHAR);
    switch (id.type) {
    case LEX_REAL:  res.str[0].push_back("numeric");
        break;
    case LEX_LOGIC:  res.str[0].push_back("logical");
        break;
    case LEX_CHAR:  res.str[0].push_back("character");
        break;
    case LEX_RNULL:  res.str[0].push_back("NULL");
        break;
    }

    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::length() {
    tmp = args.pop();
    type_of_lex t = tmp.t_lex;
    Ident &id = TID[tmp.v_lex];
    if (t == LEX_ID && !id.declare)
        throw "non-declarated identifier";
    
    Ident res(1, 1, LEX_REAL);
    res.num[0].push_back(id.c * id.r);

    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::is_vec_or_mtx(bool t) {
    tmp = args.pop();
    Ident &id = TID[tmp.v_lex];
    if (tmp.t_lex == LEX_ID && !id.declare)
        throw "non-declarated identifier";
    
    Ident res(1, 1, LEX_LOGIC);
    res.log[0].push_back(!t ? id.r == 1 : id.r > 1);

    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);
}
void Executer::rbrace() {
    Lex top = args.pop();
    tmp = top;
    while (tmp.t_lex != LEX_LBRACE) {
        tmp = args.pop();
        if (tmp.t_lex == LEX_CONST || tmp.t_lex == LEX_TIME)
            TID.erase(tmp.v_lex);
    }

    if (top.t_lex != LEX_LBRACE)
        args.push(top);
    else 
        args.push(Lex(LEX_RNULL));
}
void Executer::assign() {
    tmp2 = args.pop();
    tmp = args.pop();
    Ident &id1 = TID[tmp.v_lex];
    Ident &id2 = TID[tmp2.v_lex];

    if (tmp.t_lex != LEX_ID && (tmp.t_lex != LEX_TIME || id1.ind_base == -1))
        throw "bad assignment";

    if (tmp.t_lex == LEX_ID) {
        id1.declare = true;
        copy_id(id1, id2);

        if (tmp2.t_lex == LEX_CONST || tmp2.t_lex == LEX_TIME)
            TID.erase(tmp2.v_lex);
        args.push(tmp);
        return;
    }

    unsigned s = id1.inds.size();
    Ident &cur = TID[id1.ind_base];
    int j = 0;
    for (unsigned i = 0; i < s; i++, j++) {
        if (j >= id2.c) j = 0;
        switch (cur.type) {
        case LEX_REAL:
            cur.num[0][id1.inds[i]] = id2.num[0][j];
            break;
        case LEX_LOGIC:
            cur.log[0][id1.inds[i]] = id2.log[0][j];
            break;
        case LEX_CHAR:
            cur.str[0][id1.inds[i]] = id2.str[0][j];
            break;
        }
    }
    TID.erase(tmp.v_lex);
    if (tmp2.t_lex == LEX_CONST || tmp2.t_lex == LEX_TIME)
            TID.erase(tmp2.v_lex);
    tmp.v_lex = id1.ind_base;
    tmp.t_lex = LEX_ID;
    args.push(tmp);
}
void Executer::deref() {
    tmp = args.pop();
    if (tmp.t_lex == LEX_ID && !TID[tmp.v_lex].declare)
        throw "non-declarated identifier";
    Ident &id = TID[tmp.v_lex];
    if (id.type == LEX_CHAR)
        throw "bad arg";
    if (id.r > 1)
        throw "bad index";
   
    Ident res;
    Lex cur = args.pop();
    res.ind_base = cur.v_lex;
    Ident &cid = TID[cur.v_lex];
    if (id.type == LEX_REAL) {
        cget_mem(res, 1, id.c, cid.type);
        //res.inds[0].push_back(1);
        for (int i = 0; i < id.c; i++) {
            int k = static_cast <int> (floor(id.num[0][i])) - 1;
            switch (res.type) {
            case LEX_REAL:
                res.num[0].push_back(cid.num[0][k]);
                break;
            case LEX_LOGIC:
                res.log[0].push_back(cid.log[0][k]);
                break;
            case LEX_CHAR:
                res.str[0].push_back(cid.str[0][k]);
                break;
            }
            res.inds.push_back(k);
        }
    } else  {
        int s = 0, y = 0;
        for (int i = 0; i < cid.c; i++, y++) {
            if (y >= id.c)  y = 0;
            if (id.log[0][y])   s++;
        }
        cget_mem(res, 1, s, cid.type);
        res.inds.push_back(1);
        
        y = 0;
        for (int i = 0; i < cid.c; i++, y++) {
            if (y >= id.c)  y = 0;
            if (id.log[0][y]) {
                switch (res.type) {
                case LEX_REAL:
                    res.num[0].push_back(cid.num[0][i]);
                    break;
                case LEX_LOGIC:
                    res.log[0].push_back(cid.log[0][i]);
                    break;
                case LEX_CHAR:
                    res.str[0].push_back(cid.str[0][i]);
                    break;
                }
                res.inds.push_back(i);
            }
        }
    }

    if (tmp.t_lex == LEX_TIME || tmp.t_lex == LEX_CONST)
        TID.erase(tmp.v_lex);
    tmp.v_lex = TID.push(res);
    tmp.t_lex = LEX_TIME;
    args.push(tmp);

    /*tmp2 = args.pop();
    tmp = args.pop();
    if (tmp.t_lex == LEX_ID && !TID[tmp.v_lex].declare)
        throw "non-declarated identifier";
    if (tmp2.t_lex == LEX_ID && !TID[tmp2.v_lex].declare)
        throw "non-declarated identifier";

    Ident &id1 = TID[tmp.v_lex];
    Ident &id2 = TID[tmp2.v_lex];
    if (id1.type == LEX_CHAR || id2.type == LEX_CHAR)
        throw "bad arg";
    if (id1.r > 1 || id2.r > 1)
        throw "bad index";
    
    Ident res;
    Lex cur = args.pop();
    res.ind_base = cur.v_lex;
    Ident &cid = TID[cur.v_lex];
    if (id1.type == LEX_REAL) {
        cget_mem(res, id1.c, 0, cid.type);
        if (id2.type == LEX_REAL) {
            for (int i = 0; i < id1.c; i++) {
                int k = static_cast <int> (floor(id1.num[0][i]));
                for (int j = 0; j < id2.c; j++) {
                    int l = static_cast <int> (floor(id2.num[0][j]));
                    switch (res.type) {
                    case LEX_REAL:
                        res.num[i].push_back(cid.num[k][l]);
                        break;
                    case LEX_LOGIC:
                        res.log[i].push_back(cid.log[k][l]);
                        break;
                    case LEX_CHAR:
                        res.str[i].push_back(cid.str[k][l]);
                        break;
                    }
                    res.inds[1].push_back(l);
                    res.c++;
                }
                res.inds[0].push_back(k);
            }
        } else {
            for (int i = 0; i < id1.c; i++) {
                int k = static_cast <int> (floor(id1.num[0][i]));
                int y = 0;
                for (int j = 0; j < cid.c; j++, y++) {
                    if (y >= id2.c) y = 0;
                    if (id2.log[0][y]) {
                        switch (res.type) {
                        case LEX_REAL:
                            res.num[i].push_back(cid.num[k][j]);
                            break;
                        case LEX_LOGIC:
                            res.log[i].push_back(cid.log[k][j]);
                            break;
                        case LEX_CHAR:
                            res.str[i].push_back(cid.str[k][j]);
                            break;
                        }
                        res.inds[1].push_back(j);
                        res.c++;
                    }
                }
                res.inds[0].push_back(k);
            }
        }
    } else {
        int s = 0, q = 0;
        for (int i = 0; i < cid.r; i++, q++) {
            if (q >= id1.c) q = 0;
            if (id2.log[0][q])
                s++;
        }
        cget_mem(res, s, 0, cid.type);
        if (id2.type == LEX_REAL) {
            int y = 0, p = 0;
            for (int i = 0; i < cid.r; i++, y++) {
                for (int j = 0; j < cid.c; j++, y++) {
                    if (y >= id2.c) y = 0;
                    if (id2.log[0][y]) {
                        switch (res.type) {
                        case LEX_REAL:
                            res.num[i].push_back(cid.num[k][j]);
                            break;
                        case LEX_LOGIC:
                            res.log[i].push_back(cid.log[k][j]);
                            break;
                        case LEX_CHAR:
                            res.str[i].push_back(cid.str[k][j]);
                            break;
                        }
                        res.inds[1].push_back(j);
                        res.c++;
                    }
                }
                res.inds[0].push_back(k);
            }
        } else {
        }
    }*/
}

void Executer::execute(Poliz &prog) {
    size = prog.get_size();
    ind = 0;
    bool asgn = false;
    int k = 0, i, block = 0;
    Ident q(1, 1, LEX_LOGIC), rnull(1, 1, LEX_RNULL), cond;
    q.log[0].push_back(true);

    while (ind < size) {
        //asgn = false;
        item = prog[ind];
        if (asgn && item.t_lex != LEX_ASSIGN) {
            args.pop();
            asgn = false;
        }
        switch (item.t_lex) {
        case LEX_TRUE:     
            q.log[0][0] = true;
            item.t_lex = LEX_CONST;
            item.v_lex = TID.push(q);
            args.push(item);
            break;

        case LEX_FALSE:
            q.log[0][0] = false;
            item.t_lex = LEX_CONST;
            item.v_lex = TID.push(q);
            args.push(item);
            break;

        case POLIZ_DEREF:
            deref();
            break;

        case LEX_REAL:
        case LEX_CHAR:
            item.t_lex = LEX_CONST;
            args.push(item);
            break;

        case LEX_LBRACE:
            block++;
        case LEX_RNULL:
            item.v_lex = TID.push(rnull);
            item.t_lex = LEX_CONST;
            args.push(item);
            break;
        case LEX_ID:
        case POLIZ_LABEL:
            args.push(item);
            break;
        
        case LEX_COMMA: k++;
            break;

        case LEX_NEG:   neg();
            break;

        case LEX_UMINUS:    upm(true);
            break;
        case LEX_UPLUS: upm(false);
            break;

        case LEX_PLUS: pm_md(1);
            break;
        case LEX_MINUS:  pm_md(2);
            break;
        case LEX_MUL:   pm_md(3);
            break;
        case LEX_DIV:   pm_md(4);
            break;

        case LEX_MTXMUL:    mtxmul();
            break;

        case LEX_LESS:  leqg(1);
            break;
        case LEX_EQ:    leqg(2);
            break;
        case LEX_GRT:   leqg(3);
            break;
        case LEX_NEQ:   leqg(4);
            break;
        case LEX_LEQ:   leqg(5);
            break;
        case LEX_GEQ:   leqg(6);
            break;

        case LEX_COLON: colon();
            break;

        case LEX_LEN:   length();
            break;

        case LEX_MODE:  mode();
            break;

        case LEX_ISVEC: is_vec_or_mtx(false);
            break;
        case LEX_ISMTX: is_vec_or_mtx(true);
            break;

        case LEX_TMTX:  t();
            break;

        case LEX_RBRACE:    rbrace();
            block--;
            break;

        case LEX_ASSIGN:    assign();
            asgn = true;
            break;

        case LEX_C: c(k);
            k = 0;
            break;

        case POLIZ_GO:
            ind = args.pop().v_lex - 1;
            break;
        case POLIZ_FGO:
            i = args.pop().v_lex;
            cond = TID[args.pop().v_lex];
            if (cond.type == LEX_CHAR)
                throw "bad type of conditional arg";
            if (!(cond.type == LEX_REAL ? cond.num[0][0] : cond.log[0][0]))
                ind = i - 1;
            break;

        case LEX_SEMICOLON:
            /*if (asgn) {
                args.pop();
                asgn = false;
            }*/
            if (block)  break;
            if (args.get_size()) {
                Lex ans = args.pop();
                Ident &ai = TID[ans.v_lex];
                switch (ai.type) {
                case LEX_REAL:
                    for (int i = 0; i < ai.c; i++)
                        cout << ai.num[0][i] << " ";
                    cout << endl;
                    break;
                case LEX_LOGIC:
                    for (int i = 0; i < ai.c; i++)
                        cout << ai.log[0][i] << " ";
                    cout << endl;
                    break;
                case LEX_CHAR:
                    for (int i = 0; i < ai.c; i++)
                        cout << ai.str[0][i] << " ";
                    cout << endl;
                    break;
                default:
                    break;
                }
            }
            break;
        }
        ind++;
    }
}

ostream& operator<< (ostream &out, Lex l) {
    out << '(' << l.t_lex << ", " << l.v_lex << ");";
    return out;
}

int main() {
    //freopen("p.in", "r", stdin);
    //freopen("p.out", "w", stdout);

    //char *buf = "x";
    //TID.put(buf);

    try {
        Interpretator I("p.in");
        I.interpretation();
        char q;
        cin >> q;
        return 0;
    } catch (char c) {
        cout << "unexpected symbol " << c << endl;
        return 1;
    } catch (Lex l) {
        cout << "unexpected lexeme " << l << endl;
        return 1;
    } catch (const char *str) {
        cout << str << endl;
        return 1;
    }

    return 0;
}