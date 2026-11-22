#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

map<char, vector<string>> G;
vector<char> order_;
set<char> NT;
set<char> nullable;
string target;
set<string> failed;
vector<string> steps;
int attempts = 0;
size_t limit = 0;
char START;

bool readGrammar() {
    int n;
    cout << "Enter number of productions: ";
    cin >> n;
    cin.ignore();
    cout << "Enter productions like S->cAd or A->ab|a (# = epsilon, uppercase = non-terminal, first one is the start symbol):\n";
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
        if (i == 0) START = lhs;
        if (!NT.count(lhs)) order_.push_back(lhs);
        NT.insert(lhs);
        stringstream ss(clean.substr(3));
        string alt;
        while (getline(ss, alt, '|')) {
            if (alt.empty()) continue;
            if (alt == "#") alt = "";
            G[lhs].push_back(alt);
        }
    }
    return true;
}

void computeNullable() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (char A : order_) {
            if (nullable.count(A)) continue;
            for (const string &p : G[A]) {
                bool all = true;
                for (char c : p)
                    if (!nullable.count(c)) {
                        all = false;
                        break;
                    }
                if (all) {
                    nullable.insert(A);
                    changed = true;
                    break;
                }
            }
        }
    }
}

size_t minLength(const string &form) {
    size_t count = 0;
    for (char c : form)
        if (!NT.count(c) || !nullable.count(c)) count++;
    return count;
}

string show(const string &s) {
    return s.empty() ? "epsilon" : s;
}

bool dfs(const string &form, int depth) {
    size_t i = 0;
    while (i < form.size() && !NT.count(form[i])) i++;
    if (i > target.size() || target.compare(0, i, form, 0, i) != 0) return false;
    if (i == form.size()) return form == target;
    if (minLength(form) > target.size() || form.size() > limit) return false;
    if (failed.count(form)) return false;

    char A = form[i];
    for (const string &p : G[A]) {
        string next = form.substr(0, i) + p + form.substr(i + 1);
        attempts++;
        cout << string(depth * 2, ' ') << "try " << A << " -> " << (p.empty() ? "#" : p) << "  =>  " << show(next) << "\n";
        steps.push_back(next);
        if (dfs(next, depth + 1)) return true;
        steps.pop_back();
        cout << string(depth * 2, ' ') << "backtrack\n";
    }
    failed.insert(form);
    return false;
}

int main() {
    if (!readGrammar()) return 1;
    computeNullable();

    cout << "\nGrammar\n";
    for (char A : order_) {
        cout << "  " << A << " -> ";
        for (size_t k = 0; k < G[A].size(); k++)
            cout << (k ? " | " : "") << (G[A][k].empty() ? "#" : G[A][k]);
        cout << "\n";
    }

    string line;
    while (true) {
        cout << "\nEnter input string (q to quit): ";
        if (!getline(cin, line) || line == "q") break;
        target.clear();
        for (char c : line)
            if (c != ' ') target += c;

        failed.clear();
        steps.clear();
        attempts = 0;
        limit = 2 * target.size() + 5;

        cout << "\nSearch (leftmost derivation, trying productions in order)\n";
        string startForm(1, START);
        bool ok = dfs(startForm, 0);

        if (ok) {
            cout << "\nDerivation\n  " << START;
            for (const string &s : steps) cout << "\n  => " << show(s);
            cout << "\n\nProductions tried: " << attempts << "\nResult: string ACCEPTED\n";
        } else {
            cout << "\nProductions tried: " << attempts << "\nResult: string REJECTED\n";
        }
    }
    return 0;
}
