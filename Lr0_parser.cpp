#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

typedef set<pair<int, int>> Items;

vector<pair<char, string>> P;
set<char> NT;
vector<char> terminals, nonTerminals;
vector<Items> C;
map<pair<int, char>, int> GO;
map<pair<int, char>, set<string>> ACT;
bool conflict = false;

bool readGrammar() {
    int n;
    cout << "Enter number of productions: ";
    cin >> n;
    cin.ignore();
    cout << "Enter productions like S->(S)|a (# = epsilon, uppercase = non-terminal, first one is the start symbol):\n";
    vector<pair<char, string>> prods;
    vector<char> lhsOrder;
    char start = 0;
    for (int i = 0; i < n; i++) {
        string line, clean;
        getline(cin, line);
        for (char c : line)
            if (c != ' ') clean += c;
        if (clean.size() < 3 || clean[1] != '-' || clean[2] != '>') {
            cout << "Invalid production: " << line << "\n";
            return false;
        }
        char lhs = clean[0];
        if (i == 0) start = lhs;
        if (!NT.count(lhs)) lhsOrder.push_back(lhs);
        NT.insert(lhs);
        stringstream ss(clean.substr(3));
        string alt;
        while (getline(ss, alt, '|')) {
            if (alt.empty()) continue;
            if (alt == "#") alt = "";
            prods.push_back({lhs, alt});
        }
    }
    P.push_back({'@', string(1, start)});
    NT.insert('@');
    for (auto &p : prods) P.push_back(p);
    set<char> ts;
    for (auto &p : P)
        for (char c : p.second)
            if (!NT.count(c)) ts.insert(c);
    terminals.assign(ts.begin(), ts.end());
    terminals.push_back('$');
    nonTerminals = lhsOrder;
    return true;
}

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

void buildTable() {
    for (size_t i = 0; i < C.size(); i++) {
        int s = static_cast<int>(i);
        for (const auto &it : C[i]) {
            const string &r = P[it.first].second;
            if (it.second < static_cast<int>(r.size())) {
                char X = r[it.second];
                if (!NT.count(X)) ACT[{s, X}].insert("s" + to_string(GO[{s, X}]));
            } else if (it.first == 0) {
                ACT[{s, '$'}].insert("acc");
            } else {
                for (char a : terminals) ACT[{s, a}].insert("r" + to_string(it.first));
            }
        }
    }
    for (const auto &kv : ACT) {
        if (kv.second.size() > 1) {
            conflict = true;
            cout << "Conflict in state " << kv.first.first << " on '" << kv.first.second << "':";
            for (const string &a : kv.second) cout << " " << a;
            cout << "\n";
        }
    }
}

string join(const set<string> &s) {
    string out;
    for (const string &x : s) out += (out.empty() ? "" : "/") + x;
    return out;
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

void printTable() {
    cout << "\nLR(0) parsing table\n" << setw(8) << "STATE";
    for (char t : terminals) cout << setw(8) << t;
    for (char n : nonTerminals) cout << setw(8) << n;
    cout << "\n" << string(8 + 8 * (terminals.size() + nonTerminals.size()), '-') << "\n";
    for (size_t i = 0; i < C.size(); i++) {
        int s = static_cast<int>(i);
        cout << setw(8) << s;
        for (char t : terminals) {
            auto it = ACT.find({s, t});
            cout << setw(8) << (it == ACT.end() ? "" : join(it->second));
        }
        for (char n : nonTerminals) {
            auto it = GO.find({s, n});
            cout << setw(8) << (it == GO.end() ? "" : to_string(it->second));
        }
        cout << "\n";
    }
    cout << "\nProductions: ";
    for (size_t k = 1; k < P.size(); k++)
        cout << "(" << k << ") " << P[k].first << "->" << (P[k].second.empty() ? "#" : P[k].second) << "  ";
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
        auto it = ACT.find({st.back(), in[ip]});
        if (it == ACT.end() || it->second.size() != 1) {
            cout << "Error\n";
            cout << "\nResult: string REJECTED\n";
            return;
        }
        string act = *it->second.begin();
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
            auto g = GO.find({st.back(), P[num].first});
            if (g == GO.end()) {
                cout << "Error\n";
                cout << "\nResult: string REJECTED\n";
                return;
            }
            st.push_back(g->second);
            cout << "Reduce " << P[num].first << " -> " << (P[num].second.empty() ? "#" : P[num].second) << "\n";
        }
    }
}

int main() {
    if (!readGrammar()) return 1;

    cout << "\nAugmented grammar\n";
    for (size_t k = 0; k < P.size(); k++)
        cout << "  (" << k << ") " << P[k].first << " -> " << (P[k].second.empty() ? "#" : P[k].second) << "\n";

    buildStates();
    buildTable();
    printItems();
    printTable();

    if (conflict) {
        cout << "\nGrammar is not LR(0).\n";
        return 0;
    }
    cout << "\nGrammar is LR(0).\n";

    string line;
    while (true) {
        cout << "\nEnter input string (q to quit): ";
        if (!getline(cin, line) || line == "q") break;
        string s;
        bool valid = true;
        for (char c : line) {
            if (c == ' ') continue;
            bool known = false;
            for (char t : terminals)
                if (t == c && c != '$') known = true;
            if (!known) {
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
