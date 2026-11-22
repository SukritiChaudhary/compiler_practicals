#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

const char EPS = '#';

map<char, vector<string>> G;
vector<char> order_;
set<char> NT;
map<char, set<char>> FIRST, FOLLOW;
char START;

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

void printSet(const string &name, char A, const set<char> &s) {
    cout << "  " << name << "(" << A << ") = { ";
    bool first = true;
    for (char x : s) {
        if (!first) cout << ", ";
        cout << x;
        first = false;
    }
    cout << " }\n";
}

int main() {
    cout << "Enter number of productions: ";
    int n;
    cin >> n;
    cin.ignore();
    cout << "Enter productions like E->TA or A->+TA|# (# = epsilon, uppercase = non-terminal):\n";

    for (int i = 0; i < n; i++) {
        string line;
        getline(cin, line);
        string clean;
        for (char c : line)
            if (c != ' ') clean += c;
        size_t arrow = clean.find("->");
        if (arrow == string::npos || arrow != 1) {
            cout << "Invalid production: " << line << "\n";
            return 1;
        }
        char lhs = clean[0];
        if (i == 0) START = lhs;
        if (!G.count(lhs)) order_.push_back(lhs);
        NT.insert(lhs);
        stringstream ss(clean.substr(arrow + 2));
        string alt;
        while (getline(ss, alt, '|'))
            if (!alt.empty()) G[lhs].push_back(alt);
    }

    computeFirst();
    computeFollow();

    cout << "\nFIRST sets\n";
    for (char A : order_) printSet("FIRST", A, FIRST[A]);

    cout << "\nFOLLOW sets\n";
    for (char A : order_) printSet("FOLLOW", A, FOLLOW[A]);
    return 0;
}
