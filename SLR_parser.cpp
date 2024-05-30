#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <sstream>

using namespace std;

// Define a structure to represent a parsing action
enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };
struct Action {
    ActionType type;
    int state; // For SHIFT, this is the next state
    int production; // For REDUCE, this is the production number
};

// Define the CFG productions
vector<pair<string, vector<string>>> productions = {
    {"S'", {"CODE"}},
    {"CODE", {"VDECL", "CODE"}},
    {"CODE", {"FDECL", "CODE"}},
    {"CODE", {""}},
    {"VDECL", {"vtype", "id", "semi"}},
    {"VDECL", {"vtype", "ASSIGN", "semi"}},
    {"ASSIGN", {"id", "assign", "RHS"}},
    {"RHS", {"EXPR"}},
    {"RHS", {"literal"}},
    {"RHS", {"character"}},
    {"RHS", {"boolstr"}},
    {"EXPR", {"TERM", "EXPR_TAIL"}},
    {"EXPR_TAIL", {"addsub", "TERM", "EXPR_TAIL"}},
    {"EXPR_TAIL", {""}},
    {"TERM", {"FACTOR", "TERM_TAIL"}},
    {"TERM_TAIL", {"multdiv", "FACTOR", "TERM_TAIL"}},
    {"TERM_TAIL", {""}},
    {"FACTOR", {"lparen", "EXPR", "rparen"}},
    {"FACTOR", {"id"}},
    {"FACTOR", {"num"}},
    {"FDECL", {"vtype", "id", "lparen", "ARG", "rparen", "lbrace", "BLOCK", "RETURN", "rbrace"}},
    {"ARG", {"vtype", "id", "MOREARGS"}},
    {"ARG", {""}},
    {"MOREARGS", {"comma", "vtype", "id", "MOREARGS"}},
    {"MOREARGS", {""}},
    {"BLOCK", {"STMT", "BLOCK"}},
    {"BLOCK", {""}},
    {"STMT", {"VDECL"}},
    {"STMT", {"ASSIGN", "semi"}},
    {"STMT", {"IF"}},
    {"STMT", {"IFELSE"}},
    {"STMT", {"while", "lparen", "COND", "rparen", "lbrace", "BLOCK", "rbrace"}},
    {"IF", {"if", "lparen", "COND", "rparen", "lbrace", "BLOCK", "rbrace"}},
    {"IFELSE", {"if", "lparen", "COND", "rparen", "lbrace", "BLOCK", "rbrace", "else", "lbrace", "BLOCK", "rbrace"}},
    {"COND", {"SIMPLECOND", "COND_TAIL"}},
    {"SIMPLECOND", {"boolstr"}},
    {"SIMPLECOND", {"lparen", "COND", "rparen"}},
    {"COND_TAIL", {"comp", "SIMPLECOND", "COND_TAIL"}},
    {"COND_TAIL", {""}},
    {"RETURN", {"return", "RHS", "semi"}}
};

// ACTION and GOTO tables (simplified for demonstration)
map<int, map<string, Action>> ACTION;
map<int, map<string, int>> GOTO;

void initializeTables() {
    // Example: ACTION and GOTO table entries
    // State 0: S' -> .CODE
    ACTION[0]["vtype"] = { ActionType::SHIFT, 1, -1 }; // Shift to state 1 on 'vtype'
    ACTION[0]["$"] = { ActionType::ERROR, -1, -1 };

    // State 1: CODE -> vtype .id semi CODE | vtype .ASSIGN semi CODE
    ACTION[1]["id"] = { ActionType::SHIFT, 2, -1 };
    ACTION[1]["assign"] = { ActionType::SHIFT, 3, -1 };

    // Add all other required ACTION and GOTO entries similarly...

    // Example GOTO entries
    GOTO[0]["CODE"] = 4;
    GOTO[1]["VDECL"] = 5;
}

bool parse(vector<string> tokens, string& output) {
    stack<int> states;
    stack<string> symbols;
    states.push(0);

    int pos = 0;
    while (true) {
        int state = states.top();
        string token = (pos < tokens.size()) ? tokens[pos] : "$";

        if (ACTION[state].find(token) == ACTION[state].end()) {
            output += "Error: Unexpected token '" + token + "' at position " + to_string(pos) + "\n";
            return false;
        }

        Action action = ACTION[state][token];
        if (action.type == ActionType::SHIFT) {
            states.push(action.state);
            symbols.push(token);
            pos++;
        }
        else if (action.type == ActionType::REDUCE) {
            const auto& production = productions[action.production];
            int pop_count = production.second.size();
            while (pop_count--) {
                states.pop();
                symbols.pop();
            }

            state = states.top();
            states.push(GOTO[state][production.first]);
            symbols.push(production.first);

            // Append to parse tree output
            output += "Reduced using production: " + production.first + " -> ";
            for (const auto& sym : production.second) {
                output += sym + " ";
            }
            output += "\n";
        }
        else if (action.type == ActionType::ACCEPT) {
            output += "Parsing successful!\n";
            return true;
        }
        else {
            output += "Error: Parsing error at token '" + token + "' at position " + to_string(pos) + "\n";
            return false;
        }
    }
}

vector<string> readTokensFromFile(const string& filename) {
    ifstream infile(filename);
    vector<string> tokens;
    string token;

    while (infile >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

void writeOutputToFile(const string& filename, const string& content) {
    ofstream outfile(filename);
    outfile << content;
    outfile.close();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " input.txt output.txt" << endl;
        return 1;
    }

    string inputFilename = argv[1];
    string outputFilename = argv[2];

    initializeTables();

    vector<string> tokens = readTokensFromFile(inputFilename);
    string output;

    if (parse(tokens, output)) {
        writeOutputToFile(outputFilename, output);
    }
    else {
        writeOutputToFile(outputFilename, output);
    }

    return 0;
}
