#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

typedef set<tuple<int, int, char>> Items;

const char EPS = '#';

vector<pair<char, string>> P;
set<char> NT;
vector<char> terminals, nonTerminals;
map<char, set<char>> FIRST;
vector<Items> C;
map<pair<int, char>, int> GO;
map<pair<int, char>, set<string>> ACT;
bool conflict = false;

bool readGrammar() {
    int n;
    cout << "Enter number of productions: ";
    cin >> n;
    cin.ignore();
    cout << "Enter productions like S->CC or C->cC|d (# = epsilon, uppercase = non-terminal, first one is the start symbol):\n";
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

set<char> firstOfString(const string &s) {
    set<char> result;
    bool allEps = true;
    for (char c : s) {
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
        for (const auto &p : P)
            for (char x : firstOfString(p.second))
                if (FIRST[p.first].insert(x).second) changed = true;
    }
}

Items closure(Items I) {
    bool changed = true;
    while (changed) {
        changed = false;
        Items add;
        for (const auto &it : I) {
            int p = get<0>(it), d = get<1>(it);
            char la = get<2>(it);
            const string &r = P[p].second;
            if (d < static_cast<int>(r.size()) && NT.count(r[d])) {
                char B = r[d];
                set<char> f = firstOfString(r.substr(d + 1) + string(1, la));
                for (int k = 0; k < static_cast<int>(P.size()); k++) {
                    if (P[k].first != B) continue;
                    for (char b : f) {
                        if (b == EPS) continue;
                        auto ni = make_tuple(k, 0, b);
                        if (!I.count(ni)) add.insert(ni);
                    }
                }
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
        int p = get<0>(it), d = get<1>(it);
        const string &r = P[p].second;
        if (d < static_cast<int>(r.size()) && r[d] == X)
            J.insert(make_tuple(p, d + 1, get<2>(it)));
    }
    return closure(J);
}

void buildStates() {
    Items start;
    start.insert(make_tuple(0, 0, '$'));
    C.push_back(closure(start));
    for (size_t i = 0; i < C.size(); i++) {
        set<char> syms;
        for (const auto &it : C[i]) {
            const string &r = P[get<0>(it)].second;
            if (get<1>(it) < static_cast<int>(r.size())) syms.insert(r[get<1>(it)]);
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

void mergeStates() {
    size_t before = C.size();
    map<set<pair<int, int>>, int> index;
    vector<Items> merged;
    vector<int> newId(C.size());
    vector<vector<int>> members;
    for (size_t i = 0; i < C.size(); i++) {
        set<pair<int, int>> core;
        for (const auto &it : C[i]) core.insert({get<0>(it), get<1>(it)});
        auto f = index.find(core);
        if (f == index.end()) {
            index[core] = static_cast<int>(merged.size());
            merged.push_back(Items());
            members.push_back(vector<int>());
            f = index.find(core);
        }
        newId[i] = f->second;
        merged[f->second].insert(C[i].begin(), C[i].end());
        members[f->second].push_back(static_cast<int>(i));
    }
    map<pair<int, char>, int> go2;
    for (const auto &kv : GO)
        go2[{newId[kv.first.first], kv.first.second}] = newId[kv.second];

    cout << "\nLR(1) states: " << before << "  ->  LALR(1) states: " << merged.size() << "\n";
    for (size_t k = 0; k < members.size(); k++) {
        if (members[k].size() > 1) {
            cout << "  merged LR(1) states";
            for (int m : members[k]) cout << " I" << m;
            cout << " into I" << k << "\n";
        }
    }
    C = merged;
    GO = go2;
}

void buildTable() {
    for (size_t i = 0; i < C.size(); i++) {
        int s = static_cast<int>(i);
        for (const auto &it : C[i]) {
            int p = get<0>(it), d = get<1>(it);
            char la = get<2>(it);
            const string &r = P[p].second;
            if (d < static_cast<int>(r.size())) {
                char X = r[d];
                if (!NT.count(X)) ACT[{s, X}].insert("s" + to_string(GO[{s, X}]));
            } else if (p == 0) {
                ACT[{s, '$'}].insert("acc");
            } else {
                ACT[{s, la}].insert("r" + to_string(p));
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
    cout << "\nLALR(1) collection (after merging states with the same core)\n";
    for (size_t i = 0; i < C.size(); i++) {
        cout << "I" << i << ":\n";
        map<pair<int, int>, set<char>> grouped;
        for (const auto &it : C[i]) grouped[{get<0>(it), get<1>(it)}].insert(get<2>(it));
        for (const auto &g : grouped) {
            string r = P[g.first.first].second;
            r.insert(g.first.second, ".");
            string las;
            for (char a : g.second) las += (las.empty() ? "" : "/") + string(1, a);
            cout << "    " << P[g.first.first].first << " -> " << r << " , " << las << "\n";
        }
    }
}

void printTable() {
    cout << "\nLALR(1) parsing table\n" << setw(8) << "STATE";
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

    computeFirst();
    buildStates();
    mergeStates();
    buildTable();
    printItems();
    printTable();

    if (conflict) {
        cout << "\nGrammar is not LALR(1).\n";
        return 0;
    }
    cout << "\nGrammar is LALR(1).\n";

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
