#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

char current;       // current char
char nextcar;       // next char for lookahead

void ReadChar(void) {
    current = nextcar;
    while (cin.get(nextcar) && (nextcar == ' ' || nextcar == '\t' || nextcar == '\n'));
}

void Error(string s) {
    cerr << "Erreur: " << s << endl;
    exit(-1);
}

void AdditiveOperator(void) {
    if (current == '+' || current == '-') {
        ReadChar();
    } else {
        Error("Opérateur additif attendu");
    }
}

void MultiplicativeOperator(void) {
    if (current == '*' || current == '/' || current == '%') {
        ReadChar();
    } else if (current == '&' && nextcar == '&') {
        ReadChar(); ReadChar(); // consume both &
    } else {
        Error("Opérateur multiplicatif attendu");
    }
}

void RelationalOperator(string& op) {
    if (current == '=')
        op = "==", ReadChar();
    else if (current == '<' && nextcar == '>') 
        op = "!=", ReadChar(), ReadChar();
    else if (current == '<' && nextcar == '=') 
        op = "<=", ReadChar(), ReadChar();
    else if (current == '>' && nextcar == '=') 
        op = ">=", ReadChar(), ReadChar();
    else if (current == '<')
        op = "<", ReadChar();
    else if (current == '>')
        op = ">", ReadChar();
    else
        Error("Opérateur relationnel attendu");
}

void Number(void) {
    unsigned long long number = 0;
    if (current < '0' || current > '9')
        Error("Chiffre attendu");

    while (current >= '0' && current <= '9') {
        number = number * 10 + (current - '0');
        ReadChar();
    }
    cout << "\tpush $" << number << endl;
}

void Factor(void);

void Term(void) {
    Factor();
    while (current == '*' || current == '/' || current == '%' || (current == '&' && nextcar == '&')) {
        char op = current;
        MultiplicativeOperator();
        Factor();
        cout << "\tpop %rbx" << endl;
        cout << "\tpop %rax" << endl;

        switch (op) {
            case '*':
                cout << "\timul %rbx, %rax" << endl;
                break;
            case '/':
                cout << "\tmovq $0, %rdx" << endl;
                cout << "\tidiv %rbx" << endl;
                break;
            case '%':
                cout << "\tmovq $0, %rdx" << endl;
                cout << "\tidiv %rbx" << endl;
                cout << "\tmovq %rdx, %rax" << endl;
                break;
            case '&':
                cout << "\tand %rbx, %rax" << endl;
                break;
        }
        cout << "\tpush %rax" << endl;
    }
}

void SimpleExpression(void) {
    Term();
    while (current == '+' || current == '-') {
        char op = current;
        AdditiveOperator();
        Term();
        cout << "\tpop %rbx" << endl;
        cout << "\tpop %rax" << endl;
        if (op == '+')
            cout << "\tadd %rbx, %rax" << endl;
        else
            cout << "\tsub %rbx, %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
void Expression(void) {
    SimpleExpression();
    if (current == '=' || current == '<' || current == '>') {
        string op;
        RelationalOperator(op);
        SimpleExpression();
        cout << "\tpop %rbx" << endl;
        cout << "\tpop %rax" << endl;
        cout << "\tcmp %rbx, %rax" << endl;

        if (op == "==") cout << "\tsete %al" << endl;
        else if (op == "!=") cout << "\tsetne %al" << endl;
        else if (op == "<") cout << "\tsetl %al" << endl;
        else if (op == "<=") cout << "\tsetle %al" << endl;
        else if (op == ">") cout << "\tsetg %al" << endl;
        else if (op == ">=") cout << "\tsetge %al" << endl;

        cout << "\tmovzbq %al, %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
}

void Factor(void) {
    if (current == '(') {
        ReadChar();
        Expression();
        if (current != ')')
            Error("')' attendu");
        ReadChar();
    } else if (current >= '0' && current <= '9') {
        Number();
    } else {
        Error("Facteur invalide");
    }
}

int main(void) {
    cout << "\t\t\t# This code was produced by the CERI Compiler" << endl;
    cout << "\t.text" << endl;
    cout << "\t.globl main" << endl;
    cout << "main:" << endl;
    cout << "\tmovq %rsp, %rbp" << endl;

    // Initial read to set current and nextcar
    while (cin.get(current) && (current == ' ' || current == '\t' || current == '\n'));
    while (cin.get(nextcar) && (nextcar == ' ' || nextcar == '\t' || nextcar == '\n'));
    Expression();
    if (nextcar != '\0' && !cin.eof()) {
        cerr << "Caractères en trop à la fin du programme : [" << nextcar << "]" << endl;
        Error(".");
    }

    cout << "\tmovq %rbp, %rsp" << endl;
    cout << "\tret" << endl;

    return 0;
}
