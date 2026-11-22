#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;

const char EPS = '#';
const char START = 'E';

map<char, vector<string>> G;
vector<char> order_;
set<char> NT;
set<char> terminals;
map<char, set<char>> FIRST, FOLLOW;
map<pair<char, char>, string> TABLE;

set<char> firstOfString(const string &s) {
    set<char> result;
    bool allEps = true;
    for (char c : s) {
        if (c == EPS) continue;
        if (!NT.count(c)) {
            result.insert(c);
            allEps = false;
            break;
        }
        bool hasEps = false;
        for (char x : FIRST[c]) {
            if (x == EPS) hasEps = true;
            else result.insert(x);
        }
        if (!hasEps) {
            allEps = false;
            break;
        }
    }
    if (allEps) result.insert(EPS);
    return result;
}

void computeFirst() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (char A : order_)
            for (const string &p : G[A])
                for (char x : firstOfString(p))
                    if (FIRST[A].insert(x).second) changed = true;
    }
}

void computeFollow() {
    FOLLOW[START].insert('$');
    bool changed = true;
    while (changed) {
        changed = false;
        for (char A : order_) {
            for (const string &p : G[A]) {
                for (size_t i = 0; i < p.size(); i++) {
                    if (!NT.count(p[i])) continue;
                    set<char> f = firstOfString(p.substr(i + 1));
                    for (char x : f)
                        if (x != EPS && FOLLOW[p[i]].insert(x).second) changed = true;
                    if (f.count(EPS)) {
                        set<char> fa = FOLLOW[A];
                        for (char x : fa)
                            if (FOLLOW[p[i]].insert(x).second) changed = true;
                    }
                }
            }
        }
    }
}

bool buildTable() {
    bool ll1 = true;
    for (char A : order_) {
        for (const string &p : G[A]) {
            set<char> f = firstOfString(p);
            for (char t : f) {
                if (t == EPS) continue;
                if (TABLE.count({A, t}) && TABLE[{A, t}] != p) ll1 = false;
                TABLE[{A, t}] = p;
            }
            if (f.count(EPS)) {
                for (char b : FOLLOW[A]) {
                    if (TABLE.count({A, b}) && TABLE[{A, b}] != p) ll1 = false;
                    TABLE[{A, b}] = p;
                }
            }
        }
    }
    return ll1;
}

void printSets() {
    cout << "FIRST sets\n";
    for (char A : order_) {
        cout << "  FIRST(" << A << ") = { ";
        for (char x : FIRST[A]) cout << x << " ";
        cout << "}\n";
    }
    cout << "\nFOLLOW sets\n";
    for (char A : order_) {
        cout << "  FOLLOW(" << A << ") = { ";
        for (char x : FOLLOW[A]) cout << x << " ";
        cout << "}\n";
    }
}

void printTable() {
    vector<char> cols(terminals.begin(), terminals.end());
    cols.push_back('$');
    cout << "\nParsing table\n" << setw(6) << "";
    for (char t : cols) cout << setw(10) << t;
    cout << "\n" << string(6 + 10 * cols.size(), '-') << "\n";
    for (char A : order_) {
        cout << setw(6) << A;
        for (char t : cols) {
            auto it = TABLE.find({A, t});
            string cell = it == TABLE.end() ? "" : string(1, A) + "->" + it->second;
            cout << setw(10) << cell;
        }
        cout << "\n";
    }
}

void parse(string input) {
    input += '$';
    string st = "$";
    st = string(1, START) + st;
    size_t ip = 0;
    cout << "\n" << left << setw(16) << "STACK" << setw(16) << "INPUT" << "ACTION\n" << string(52, '-') << "\n";
    while (true) {
        cout << left << setw(16) << st << setw(16) << input.substr(ip);
        char top = st[0];
        char a = input[ip];
        if (top == '$' && a == '$') {
            cout << "Accept\n";
            cout << "\nResult: string ACCEPTED\n";
            return;
        }
        if (!NT.count(top)) {
            if (top == a) {
                cout << "Match " << a << "\n";
                st.erase(0, 1);
                ip++;
            } else {
                cout << "Error: expected '" << top << "'\n";
                cout << "\nResult: string REJECTED\n";
                return;
            }
        } else {
            auto it = TABLE.find({top, a});
            if (it == TABLE.end()) {
                cout << "Error: no rule for [" << top << ", " << a << "]\n";
                cout << "\nResult: string REJECTED\n";
                return;
            }
            string p = it->second;
            cout << top << " -> " << p << "\n";
            st.erase(0, 1);
            if (p != string(1, EPS)) st = p + st;
        }
    }
}

int main() {
    G['E'] = {"TA"};
    G['A'] = {"+TA", "#"};
    G['T'] = {"FB"};
    G['B'] = {"*FB", "#"};
    G['F'] = {"(E)", "i"};
    order_ = {'E', 'A', 'T', 'B', 'F'};
    for (char A : order_) NT.insert(A);
    for (char A : order_)
        for (const string &p : G[A])
            for (char c : p)
                if (!NT.count(c) && c != EPS) terminals.insert(c);

    cout << "Grammar (# = epsilon, i = identifier)\n";
    for (char A : order_) {
        cout << "  " << A << " -> ";
        for (size_t k = 0; k < G[A].size(); k++) cout << (k ? " | " : "") << G[A][k];
        cout << "\n";
    }
    cout << "\n";

    computeFirst();
    computeFollow();
    printSets();
    bool ll1 = buildTable();
    printTable();
    if (!ll1) {
        cout << "\nGrammar is not LL(1).\n";
        return 0;
    }
    cout << "\nGrammar is LL(1).\n";

    string line;
    while (true) {
        cout << "\nEnter string using i + * ( ) (q to quit): ";
        if (!getline(cin, line) || line == "q") break;
        string s;
        for (char c : line)
            if (c != ' ') s += c;
        parse(s);
    }
    return 0;
}
