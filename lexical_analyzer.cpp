#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <cctype>
#include <iomanip>
using namespace std;

struct Token {
    string type;
    string lexeme;
    int line;
};

const set<string> keywords = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof",
    "static", "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while", "bool", "class", "public", "private", "protected",
    "new", "delete", "namespace", "using", "template", "true", "false"
};

const vector<string> operators = {
    "<<=", ">>=", "...",
    "++", "--", "->", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||",
    "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "::",
    "+", "-", "*", "/", "%", "=", "<", ">", "!", "&", "|", "^", "~",
    "?", ":", "."
};

const string delimiters = "(){}[];,#";

string readSource(int argc, char **argv) {
    stringstream ss;
    if (argc > 1) {
        ifstream file(argv[1]);
        if (!file) {
            cerr << "Cannot open file: " << argv[1] << "\n";
            exit(1);
        }
        ss << file.rdbuf();
    } else {
        cout << "Enter source code (finish with Ctrl+D on Linux/Mac or Ctrl+Z then Enter on Windows):\n";
        ss << cin.rdbuf();
    }
    return ss.str();
}

int main(int argc, char **argv) {
    string src = readSource(argc, argv);
    size_t n = src.size();
    size_t i = 0;
    int line = 1;
    vector<Token> tokens;
    vector<string> errors;

    while (i < n) {
        char c = src[i];

        if (c == '\n') {
            line++;
            i++;
            continue;
        }
        if (isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            while (i < n && src[i] != '\n') i++;
            continue;
        }

        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            int startLine = line;
            bool closed = false;
            i += 2;
            while (i < n) {
                if (src[i] == '*' && i + 1 < n && src[i + 1] == '/') {
                    i += 2;
                    closed = true;
                    break;
                }
                if (src[i] == '\n') line++;
                i++;
            }
            if (!closed)
                errors.push_back("Line " + to_string(startLine) + ": unterminated comment");
            continue;
        }

        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t j = i;
            while (j < n && (isalnum(static_cast<unsigned char>(src[j])) || src[j] == '_')) j++;
            string word = src.substr(i, j - i);
            tokens.push_back({keywords.count(word) ? "KEYWORD" : "IDENTIFIER", word, line});
            i = j;
            continue;
        }

        if (isdigit(static_cast<unsigned char>(c)) ||
            (c == '.' && i + 1 < n && isdigit(static_cast<unsigned char>(src[i + 1])))) {
            size_t j = i;
            bool isFloat = false;
            while (j < n && isdigit(static_cast<unsigned char>(src[j]))) j++;
            if (j < n && src[j] == '.') {
                isFloat = true;
                j++;
                while (j < n && isdigit(static_cast<unsigned char>(src[j]))) j++;
            }
            if (j < n && (src[j] == 'e' || src[j] == 'E')) {
                size_t k = j + 1;
                if (k < n && (src[k] == '+' || src[k] == '-')) k++;
                if (k < n && isdigit(static_cast<unsigned char>(src[k]))) {
                    isFloat = true;
                    while (k < n && isdigit(static_cast<unsigned char>(src[k]))) k++;
                    j = k;
                }
            }
            if (j < n && (isalpha(static_cast<unsigned char>(src[j])) || src[j] == '_')) {
                while (j < n && (isalnum(static_cast<unsigned char>(src[j])) || src[j] == '_')) j++;
                errors.push_back("Line " + to_string(line) + ": invalid token '" + src.substr(i, j - i) + "'");
            } else {
                tokens.push_back({isFloat ? "FLOAT" : "INTEGER", src.substr(i, j - i), line});
            }
            i = j;
            continue;
        }

        if (c == '"' || c == '\'') {
            char quote = c;
            size_t j = i + 1;
            while (j < n && src[j] != quote && src[j] != '\n') {
                if (src[j] == '\\' && j + 1 < n) j++;
                j++;
            }
            if (j < n && src[j] == quote) {
                tokens.push_back({quote == '"' ? "STRING" : "CHARACTER", src.substr(i, j - i + 1), line});
                i = j + 1;
            } else {
                errors.push_back("Line " + to_string(line) + ": unterminated " +
                                 string(quote == '"' ? "string" : "character") + " literal");
                i = j;
            }
            continue;
        }

        bool matched = false;
        for (const string &op : operators) {
            if (src.compare(i, op.size(), op) == 0) {
                tokens.push_back({"OPERATOR", op, line});
                i += op.size();
                matched = true;
                break;
            }
        }
        if (matched) continue;

        if (delimiters.find(c) != string::npos) {
            tokens.push_back({"DELIMITER", string(1, c), line});
            i++;
            continue;
        }

        errors.push_back("Line " + to_string(line) + ": unknown character '" + string(1, c) + "'");
        i++;
    }

    cout << "\n" << left << setw(8) << "LINE" << setw(14) << "TOKEN TYPE" << "LEXEME\n";
    cout << string(40, '-') << "\n";
    map<string, int> count;
    for (const Token &t : tokens) {
        cout << left << setw(8) << t.line << setw(14) << t.type << t.lexeme << "\n";
        count[t.type]++;
    }

    cout << "\nToken summary\n" << string(40, '-') << "\n";
    for (const auto &p : count)
        cout << left << setw(14) << p.first << p.second << "\n";
    cout << left << setw(14) << "TOTAL" << tokens.size() << "\n";

    if (!errors.empty()) {
        cout << "\nLexical errors\n" << string(40, '-') << "\n";
        for (const string &e : errors) cout << e << "\n";
    } else {
        cout << "\nNo lexical errors found.\n";
    }
    return 0;
}
