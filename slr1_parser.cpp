#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <iomanip>
using namespace std;

typedef set<pair<int, int>> Items;

vector<pair<char, string>> P = {
    {'Z', "E"}, {'E', "E+T"}, {'E', "T"}, {'T', "T*F"},
    {'T', "F"}, {'F', "(E)"}, {'F', "i"}
};
set<char> NT = {'Z', 'E', 'T', 'F'};
vector<char> terminals = {'i', '+', '*', '(', ')', '$'};
vector<char> nonTerminals = {'E', 'T', 'F'};

map<char, set<char>> FIRSTS, FOLLOW;
vector<Items> C;
map<pair<int, char>, int> GO;
map<pair<int, char>, string> ACTION;
bool conflict = false;

Items closure(Items I) {
    bool changed = true;
    while (changed) {
        changed = false;
        Items add;
        for (const auto &it : I) {
            const string &r = P[it.first].second;
            if (it.second < static_cast<int>(r.size()) && NT.count(r[it.second])) {
                char B = r[it.second];
                for (int k = 0; k < static_cast<int>(P.size()); k++)
                    if (P[k].first == B && !I.count({k, 0})) add.insert({k, 0});
            }
        }
        if (!add.empty()) {
            I.insert(add.begin(), add.end());
            changed = true;
        }
    }
    return I;
}

Items gotoFn(const Items &I, char X) {
    Items J;
    for (const auto &it : I) {
        const string &r = P[it.first].second;
        if (it.second < static_cast<int>(r.size()) && r[it.second] == X)
            J.insert({it.first, it.second + 1});
    }
    return closure(J);
}

void buildStates() {
    Items start;
    start.insert({0, 0});
    C.push_back(closure(start));
    for (size_t i = 0; i < C.size(); i++) {
        set<char> syms;
        for (const auto &it : C[i]) {
            const string &r = P[it.first].second;
            if (it.second < static_cast<int>(r.size())) syms.insert(r[it.second]);
        }
        for (char X : syms) {
            Items J = gotoFn(C[i], X);
            int idx = -1;
            for (size_t k = 0; k < C.size(); k++)
                if (C[k] == J) {
                    idx = static_cast<int>(k);
                    break;
                }
            if (idx == -1) {
                C.push_back(J);
                idx = static_cast<int>(C.size()) - 1;
            }
            GO[{static_cast<int>(i), X}] = idx;
        }
    }
}

void computeFirst() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &p : P) {
            char c = p.second[0];
            set<char> s;
            if (NT.count(c)) s = FIRSTS[c];
            else s.insert(c);
            for (char x : s)
                if (FIRSTS[p.first].insert(x).second) changed = true;
        }
    }
}

void computeFollow() {
    FOLLOW['Z'].insert('$');
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &p : P) {
            const string &r = p.second;
            for (size_t i = 0; i < r.size(); i++) {
                if (!NT.count(r[i])) continue;
                set<char> s;
                if (i + 1 < r.size()) {
                    char n = r[i + 1];
                    if (NT.count(n)) s = FIRSTS[n];
                    else s.insert(n);
                } else {
                    s = FOLLOW[p.first];
                }
                for (char x : s)
                    if (FOLLOW[r[i]].insert(x).second) changed = true;
            }
        }
    }
}

void setAction(int s, char a, const string &v) {
    auto key = make_pair(s, a);
    auto it = ACTION.find(key);
    if (it != ACTION.end() && it->second != v) {
        cout << "Conflict in state " << s << " on '" << a << "': " << it->second << " / " << v << "\n";
        conflict = true;
    }
    ACTION[key] = v;
}

void buildTable() {
    for (size_t i = 0; i < C.size(); i++) {
        int s = static_cast<int>(i);
        for (const auto &it : C[i]) {
            const string &r = P[it.first].second;
            if (it.second < static_cast<int>(r.size())) {
                char X = r[it.second];
                if (!NT.count(X)) setAction(s, X, "s" + to_string(GO[{s, X}]));
            } else if (it.first == 0) {
                setAction(s, '$', "acc");
            } else {
                for (char a : FOLLOW[P[it.first].first])
                    setAction(s, a, "r" + to_string(it.first));
            }
        }
    }
}

void printItems() {
    cout << "\nCanonical LR(0) collection\n";
    for (size_t i = 0; i < C.size(); i++) {
        cout << "I" << i << ":\n";
        for (const auto &it : C[i]) {
            string r = P[it.first].second;
            r.insert(it.second, ".");
            cout << "    " << P[it.first].first << " -> " << r << "\n";
        }
    }
}

void printSets() {
    cout << "\nFOLLOW sets\n";
    for (char A : nonTerminals) {
        cout << "  FOLLOW(" << A << ") = { ";
        for (char x : FOLLOW[A]) cout << x << " ";
        cout << "}\n";
    }
}

void printTable() {
    cout << "\nSLR parsing table\n" << setw(8) << "STATE";
    for (char t : terminals) cout << setw(6) << t;
    for (char n : nonTerminals) cout << setw(6) << n;
    cout << "\n" << string(8 + 6 * (terminals.size() + nonTerminals.size()), '-') << "\n";
    for (size_t i = 0; i < C.size(); i++) {
        int s = static_cast<int>(i);
        cout << setw(8) << s;
        for (char t : terminals) {
            auto it = ACTION.find({s, t});
            cout << setw(6) << (it == ACTION.end() ? "" : it->second);
        }
        for (char n : nonTerminals) {
            auto it = GO.find({s, n});
            cout << setw(6) << (it == GO.end() ? "" : to_string(it->second));
        }
        cout << "\n";
    }
    cout << "\nProductions: ";
    for (size_t k = 1; k < P.size(); k++)
        cout << "(" << k << ") " << P[k].first << "->" << P[k].second << "  ";
    cout << "\n";
}

string stackString(const vector<int> &st, const string &sym) {
    string out = to_string(st[0]);
    for (size_t k = 0; k < sym.size(); k++) {
        out += sym[k];
        out += to_string(st[k + 1]);
    }
    return out;
}

void parse(const string &input) {
    string in = input + "$";
    vector<int> st = {0};
    string sym;
    size_t ip = 0;

    cout << "\n" << left << setw(24) << "STACK" << setw(16) << "INPUT" << "ACTION\n" << string(60, '-') << "\n";

    while (true) {
        cout << left << setw(24) << stackString(st, sym) << setw(16) << in.substr(ip);
        auto it = ACTION.find({st.back(), in[ip]});
        if (it == ACTION.end()) {
            cout << "Error\n";
            cout << "\nResult: string REJECTED\n";
            return;
        }
        const string &act = it->second;
        if (act == "acc") {
            cout << "Accept\n";
            cout << "\nResult: string ACCEPTED\n";
            return;
        }
        int num = stoi(act.substr(1));
        if (act[0] == 's') {
            cout << "Shift " << in[ip] << " -> state " << num << "\n";
            sym += in[ip];
            st.push_back(num);
            ip++;
        } else {
            size_t len = P[num].second.size();
            for (size_t k = 0; k < len; k++) {
                st.pop_back();
                sym.pop_back();
            }
            sym += P[num].first;
            st.push_back(GO[{st.back(), P[num].first}]);
            cout << "Reduce " << P[num].first << " -> " << P[num].second << "\n";
        }
    }
}

int main() {
    cout << "Augmented grammar (i = identifier)\n";
    cout << "  Z -> E\n  E -> E + T | T\n  T -> T * F | F\n  F -> ( E ) | i\n";

    buildStates();
    computeFirst();
    computeFollow();
    buildTable();
    printItems();
    printSets();
    printTable();

    if (conflict) {
        cout << "\nGrammar is not SLR(1).\n";
        return 0;
    }
    cout << "\nGrammar is SLR(1).\n";

    string line;
    while (true) {
        cout << "\nEnter string using i + * ( ) (q to quit): ";
        if (!getline(cin, line) || line == "q") break;
        string s;
        bool valid = true;
        for (char c : line) {
            if (c == ' ') continue;
            if (string("i+*()").find(c) == string::npos) {
                cout << "Invalid symbol '" << c << "'\n";
                valid = false;
                break;
            }
            s += c;
        }
        if (valid) parse(s);
    }
    return 0;
}
