#include <iostream>
#include <map>
#include <string>
#include <iomanip>
using namespace std;

map<char, map<char, char>> REL;
const string symbols = "i+*()$";

void buildRelations() {
    const char *rows[6] = {
        "->>->>",
        "<><<>>",
        "<>><>>",
        "<<<<=-",
        "->>->>",
        "<<<<-A"
    };
    const char *cols = "i+*()$";
    const char *names = "i+*()$";
    for (int r = 0; r < 6; r++)
        for (int c = 0; c < 6; c++)
            REL[names[r]][cols[c]] = rows[r][c];
}

void printRelations() {
    cout << "Operator precedence table\n" << setw(4) << "";
    for (char c : symbols) cout << setw(4) << c;
    cout << "\n" << string(4 + 4 * symbols.size(), '-') << "\n";
    for (char r : symbols) {
        cout << setw(4) << r;
        for (char c : symbols) {
            char v = REL[r][c];
            cout << setw(4) << (v == '-' ? ' ' : v);
        }
        cout << "\n";
    }
    cout << "(< yields, > takes, = equal, A accept)\n";
}

char topTerminal(const string &st) {
    for (int k = static_cast<int>(st.size()) - 1; k >= 0; k--)
        if (st[k] != 'E') return st[k];
    return '$';
}

bool validHandle(const string &h) {
    return h == "i" || h == "E+E" || h == "E*E" || h == "(E)";
}

void parse(const string &input) {
    string in = input + "$";
    string st = "$";
    size_t ip = 0;

    cout << "\n" << left << setw(16) << "STACK" << setw(16) << "INPUT" << "ACTION\n" << string(52, '-') << "\n";

    while (true) {
        cout << left << setw(16) << st << setw(16) << in.substr(ip);
        char a = topTerminal(st);
        char b = in[ip];
        char rel = REL[a][b];

        if (rel == 'A' && st == "$E") {
            cout << "Accept\n";
            cout << "\nResult: string ACCEPTED\n";
            return;
        }
        if (rel == '<' || rel == '=') {
            cout << "Shift " << b << "\n";
            st += b;
            ip++;
        } else if (rel == '>') {
            string handle;
            char lastT = 0;
            while (!st.empty()) {
                char c = st.back();
                st.pop_back();
                handle = c + handle;
                if (c != 'E') lastT = c;
                if (lastT != 0 && REL[topTerminal(st)][lastT] == '<') break;
            }
            if (!st.empty() && st.back() == 'E') {
                handle = "E" + handle;
                st.pop_back();
            }
            if (!validHandle(handle)) {
                cout << "Error: invalid handle " << handle << "\n";
                cout << "\nResult: string REJECTED\n";
                return;
            }
            st += 'E';
            cout << "Reduce E -> " << handle << "\n";
        } else {
            cout << "Error\n";
            cout << "\nResult: string REJECTED\n";
            return;
        }
    }
}

int main() {
    buildRelations();
    cout << "Grammar (i = identifier)\n";
    cout << "  E -> E + E | E * E | ( E ) | i\n\n";
    printRelations();

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
