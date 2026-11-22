#include <iostream>
#include <vector>
#include <string>
#include <cctype>
using namespace std;

vector<string> tokens;
size_t pos;

string peek() {
    return pos < tokens.size() ? tokens[pos] : "$";
}

void advance() {
    if (pos < tokens.size()) pos++;
}

bool tokenize(const string &s) {
    tokens.clear();
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = s[i];
        if (isspace(c)) {
            i++;
        } else if (isalpha(c) || c == '_') {
            size_t j = i;
            while (j < s.size() && (isalnum(static_cast<unsigned char>(s[j])) || s[j] == '_')) j++;
            tokens.push_back("id");
            i = j;
        } else if (isdigit(c)) {
            size_t j = i;
            while (j < s.size() && isdigit(static_cast<unsigned char>(s[j]))) j++;
            tokens.push_back("num");
            i = j;
        } else if (string("+-*/()").find(s[i]) != string::npos) {
            tokens.push_back(string(1, s[i]));
            i++;
        } else {
            cout << "Invalid character '" << s[i] << "'\n";
            return false;
        }
    }
    return true;
}

bool E();
bool EPrime();
bool T();
bool TPrime();
bool F();

bool E() {
    cout << "E  -> T E'\n";
    return T() && EPrime();
}

bool EPrime() {
    if (peek() == "+" || peek() == "-") {
        cout << "E' -> " << peek() << " T E'\n";
        advance();
        return T() && EPrime();
    }
    cout << "E' -> epsilon\n";
    return true;
}

bool T() {
    cout << "T  -> F T'\n";
    return F() && TPrime();
}

bool TPrime() {
    if (peek() == "*" || peek() == "/") {
        cout << "T' -> " << peek() << " F T'\n";
        advance();
        return F() && TPrime();
    }
    cout << "T' -> epsilon\n";
    return true;
}

bool F() {
    if (peek() == "id" || peek() == "num") {
        cout << "F  -> " << peek() << "\n";
        advance();
        return true;
    }
    if (peek() == "(") {
        cout << "F  -> ( E )\n";
        advance();
        if (!E()) return false;
        if (peek() == ")") {
            advance();
            return true;
        }
        return false;
    }
    return false;
}

int main() {
    cout << "Grammar:\n";
    cout << "  E  -> T E'\n";
    cout << "  E' -> + T E' | - T E' | epsilon\n";
    cout << "  T  -> F T'\n";
    cout << "  T' -> * F T' | / F T' | epsilon\n";
    cout << "  F  -> ( E ) | id | num\n\n";

    string line;
    while (true) {
        cout << "Enter expression (q to quit): ";
        if (!getline(cin, line) || line == "q") break;
        if (!tokenize(line)) continue;
        pos = 0;
        cout << "\nParsing steps:\n";
        bool ok = E();
        if (ok && pos == tokens.size())
            cout << "\nResult: string ACCEPTED\n\n";
        else
            cout << "\nResult: string REJECTED (error near token '" << peek() << "' at position " << pos + 1 << ")\n\n";
    }
    return 0;
}
