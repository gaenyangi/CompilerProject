#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <sstream>

using namespace std;

// Define a class to represent a parsing action
enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

class Action {
public:
    ActionType type;
    int state; // For SHIFT, this is the next state
    int production; // For REDUCE, this is the production number

    // Default constructor
    Action() : type(ActionType::ERROR), state(-1), production(-1) {}

    Action(ActionType type, int state = -1, int production = -1)
        : type(type), state(state), production(production) {}
};

// Define a class to represent a node in the parse tree
class TreeNode {
public:
    string symbol;
    vector<TreeNode*> children;

    TreeNode(const string& sym) : symbol(sym) {}
    ~TreeNode() {
        for (auto child : children) {
            delete child;
        }
    }

    void printTree(int depth = 0) const {
        for (int i = 0; i < depth; ++i) {
            cout << "  ";
        }
        cout << symbol << "\n";
        for (auto child : children) {
            child->printTree(depth + 1);
        }
    }

    void writeTreeToFile(ofstream& outfile, int depth = 0) const {
        for (int i = 0; i < depth; ++i) {
            outfile << "  ";
        }
        outfile << symbol << "\n";
        for (auto child : children) {
            child->writeTreeToFile(outfile, depth + 1);
        }
    }
};

class Parser {
private:
    vector<pair<string, vector<string>>> productions;
    map<int, map<string, Action>> ACTION;
    map<int, map<string, int>> GOTO;

    void initializeProductions() {
        productions = {
            {"S'", {"CODE"}},//0
            {"CODE", {"VDECL", "CODE"}},
            {"CODE", {"FDECL", "CODE"}},
            {"CODE", {""}},
            {"VDECL", {"vtype", "id", "semi"}},
            {"VDECL", {"vtype", "ASSIGN", "semi"}}, //5
            {"ASSIGN", {"id", "assign", "RHS"}},
            {"RHS", {"EXPR"}},
            {"RHS", {"literal"}},
            {"RHS", {"character"}},
            {"RHS", {"boolstr"}}, //10
            {"EXPR", {"TERM", "EXPR_TAIL"}},
            {"EXPR_TAIL", {"addsub", "TERM", "EXPR_TAIL"}},
            {"EXPR_TAIL", {""}},
            {"TERM", {"FACTOR", "TERM_TAIL"}},
            {"TERM_TAIL", {"multdiv", "FACTOR", "TERM_TAIL"}},//15
            {"TERM_TAIL", {""}},
            {"FACTOR", {"lparen", "EXPR", "rparen"}},
            {"FACTOR", {"id"}},
            {"FACTOR", {"num"}},
            {"FDECL", {"vtype", "id", "lparen", "ARG", "rparen", "lbrace", "BLOCK", "RETURN", "rbrace"}},//20
            {"ARG", {"vtype", "id", "MOREARGS"}},
            {"ARG", {""}},
            {"MOREARGS", {"comma", "vtype", "id", "MOREARGS"}},
            {"MOREARGS", {""}},
            {"BLOCK", {"STMT", "BLOCK"}},//25
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
    }

    void initializeTables() {
        ACTION[0]["vtype"] = { ActionType::SHIFT, 4 };
        ACTION[0]["$"] = { ActionType::REDUCE, 0, 3 };

        ACTION[1]["$"] = { ActionType::ACCEPT };

        ACTION[2]["vtype"] = { ActionType::SHIFT, 4 };
        ACTION[2]["$"] = { ActionType::REDUCE, 0, 3 };

        ACTION[3]["vtype"] = { ActionType::SHIFT, 4 };
        ACTION[3]["$"] = { ActionType::REDUCE, 0, 3 };

        ACTION[4]["id"] = { ActionType::SHIFT, 7 };

        ACTION[5]["$"] = { ActionType::REDUCE, 0, 1 };

        ACTION[6]["$"] = { ActionType::REDUCE, 0, 2 };

        ACTION[7]["semi"] = { ActionType::SHIFT, 9 };
        ACTION[7]["assign"] = { ActionType::SHIFT, 11 };
        ACTION[7]["lparen"] = { ActionType::SHIFT, 10 };

        ACTION[8]["semi"] = { ActionType::SHIFT, 12 };

        ACTION[9]["vtype"] = { ActionType::REDUCE, 0, 4 };
        ACTION[9]["id"] = { ActionType::REDUCE, 0, 4 };
        ACTION[9]["rbrace"] = { ActionType::REDUCE, 0, 4 };
        ACTION[9]["while"] = { ActionType::REDUCE, 0, 4 };
        ACTION[9]["if"] = { ActionType::REDUCE, 0, 4 };
        ACTION[9]["return"] = { ActionType::REDUCE, 0, 4 };
        ACTION[9]["$"] = { ActionType::REDUCE, 0, 4 };

        ACTION[10]["vtype"] = { ActionType::SHIFT, 14 };
        ACTION[10]["rparen"] = { ActionType::REDUCE, 0, 22 };

        ACTION[11]["id"] = { ActionType::SHIFT, 23 };
        ACTION[11]["literal"] = { ActionType::SHIFT, 17 };
        ACTION[11]["character"] = { ActionType::SHIFT, 18 };
        ACTION[11]["boolstr"] = { ActionType::SHIFT, 19 };
        ACTION[11]["lparen"] = { ActionType::SHIFT, 22 };
        ACTION[11]["num"] = { ActionType::SHIFT, 24 };

        ACTION[12]["vtype"] = { ActionType::REDUCE, 0, 5 };
        ACTION[12]["id"] = { ActionType::REDUCE, 0, 5 };
        ACTION[12]["rbrace"] = { ActionType::REDUCE, 0, 5 };
        ACTION[12]["while"] = { ActionType::REDUCE, 0, 5 };
        ACTION[12]["if"] = { ActionType::REDUCE, 0, 5 };
        ACTION[12]["return"] = { ActionType::REDUCE, 0, 5 };
        ACTION[12]["$"] = { ActionType::REDUCE, 0, 5 };

        ACTION[13]["rparen"] = { ActionType::SHIFT, 25 };

        ACTION[14]["id"] = { ActionType::SHIFT, 26 };

        ACTION[15]["semi"] = { ActionType::REDUCE, 0, 6 };

        ACTION[16]["semi"] = { ActionType::REDUCE, 0, 7 };

        ACTION[17]["semi"] = { ActionType::REDUCE, 0, 8 };

        ACTION[18]["semi"] = { ActionType::REDUCE, 0, 9 };

        ACTION[19]["semi"] = { ActionType::REDUCE, 0, 10 };

        ACTION[20]["semi"] = { ActionType::REDUCE, 0, 13 };
        ACTION[20]["rparen"] = { ActionType::REDUCE, 0, 13 };
        ACTION[20]["addsub"] = { ActionType::SHIFT, 28 };

        ACTION[21]["semi"] = { ActionType::REDUCE, 0, 16 };
        ACTION[21]["addsub"] = { ActionType::REDUCE, 0, 16 };
        ACTION[21]["rparen"] = { ActionType::REDUCE, 0, 16 };
        ACTION[21]["multdiv"] = { ActionType::SHIFT, 30 };

        ACTION[22]["id"] = { ActionType::SHIFT, 23 };
        ACTION[22]["lparen"] = { ActionType::SHIFT, 22 };
        ACTION[22]["num"] = { ActionType::SHIFT, 24 };

        ACTION[23]["semi"] = { ActionType::REDUCE, 0, 18 };
        ACTION[23]["addsub"] = { ActionType::REDUCE, 0, 18 };
        ACTION[23]["multdiv"] = { ActionType::REDUCE, 0, 18 };
        ACTION[23]["rparen"] = { ActionType::REDUCE, 0, 18 };

        ACTION[24]["semi"] = { ActionType::REDUCE, 0, 19 };
        ACTION[24]["addsub"] = { ActionType::REDUCE, 0, 19 };
        ACTION[24]["multdiv"] = { ActionType::REDUCE, 0, 19 };
        ACTION[24]["rparen"] = { ActionType::REDUCE, 0, 19 };

        ACTION[25]["lbrace"] = { ActionType::SHIFT, 32 };

        ACTION[26]["$"] = { ActionType::REDUCE, 0, 24 };
        ACTION[26]["comma"] = { ActionType::SHIFT, 34 };

        ACTION[27]["semi"] = { ActionType::REDUCE, 0, 11 };
        ACTION[27]["rparen"] = { ActionType::REDUCE, 0, 11 };

        ACTION[28]["id"] = { ActionType::SHIFT, 23 };
        ACTION[28]["lparen"] = { ActionType::SHIFT, 22 };
        ACTION[28]["num"] = { ActionType::SHIFT, 24 };

        ACTION[29]["semi"] = { ActionType::REDUCE, 0, 14 };
        ACTION[29]["addsub"] = { ActionType::REDUCE, 0, 14 };
        ACTION[29]["rparen"] = { ActionType::REDUCE, 0, 14 };

        ACTION[30]["id"] = { ActionType::SHIFT, 23 };
        ACTION[30]["lparen"] = { ActionType::SHIFT, 22 };
        ACTION[30]["num"] = { ActionType::SHIFT, 24 };

        ACTION[31]["rparen"] = { ActionType::SHIFT, 37 };

        ACTION[32]["vtype"] = { ActionType::SHIFT, 45 };
        ACTION[32]["id"] = { ActionType::SHIFT, 46 };
        ACTION[32]["while"] = { ActionType::SHIFT, 44 };
        ACTION[32]["if"] = { ActionType::SHIFT, 47 };
        ACTION[32]["rbrace"] = { ActionType::REDUCE, 0, 26 };
        ACTION[32]["return"] = { ActionType::REDUCE, 0, 26 };

        ACTION[33]["rparen"] = { ActionType::REDUCE, 0, 21 };

        ACTION[34]["vtype"] = { ActionType::SHIFT, 48 };

        ACTION[35]["semi"] = { ActionType::REDUCE, 0, 13 };
        ACTION[35]["addsub"] = { ActionType::SHIFT, 28 };
        ACTION[35]["rparen"] = { ActionType::REDUCE, 0, 13 };

        ACTION[36]["semi"] = { ActionType::REDUCE, 0, 16 };
        ACTION[36]["addsub"] = { ActionType::REDUCE, 0, 16 };
        ACTION[36]["multdiv"] = { ActionType::SHIFT, 30 };
        ACTION[36]["rparen"] = { ActionType::REDUCE, 0, 16 };

        ACTION[37]["semi"] = { ActionType::REDUCE, 0, 17 };
        ACTION[37]["addsub"] = { ActionType::REDUCE, 0, 17 };
        ACTION[37]["multdiv"] = { ActionType::REDUCE, 0, 17 };
        ACTION[37]["rparen"] = { ActionType::REDUCE, 0, 17 };

        ACTION[38]["return"] = { ActionType::SHIFT, 52 };

        ACTION[39]["vtype"] = { ActionType::SHIFT, 45 };
        ACTION[39]["id"] = { ActionType::SHIFT, 46 };
        ACTION[39]["while"] = { ActionType::SHIFT, 44 };
        ACTION[39]["if"] = { ActionType::SHIFT, 47 };
        ACTION[39]["rbrace"] = { ActionType::REDUCE, 0, 26 };
        ACTION[39]["return"] = { ActionType::REDUCE, 0, 26 };

        ACTION[40]["vtype"] = { ActionType::REDUCE, 0, 27 };
        ACTION[40]["id"] = { ActionType::REDUCE, 0, 27 };
        ACTION[40]["rbrace"] = { ActionType::REDUCE, 0, 27 };
        ACTION[40]["while"] = { ActionType::REDUCE, 0, 27 };
        ACTION[40]["if"] = { ActionType::REDUCE, 0, 27 };
        ACTION[40]["return"] = { ActionType::REDUCE, 0, 27 };

        ACTION[41]["semi"] = { ActionType::SHIFT, 54 };

        ACTION[42]["vtype"] = { ActionType::REDUCE, 0, 29 };
        ACTION[42]["id"] = { ActionType::REDUCE, 0, 29 };
        ACTION[42]["rbrace"] = { ActionType::REDUCE, 0, 29 };
        ACTION[42]["while"] = { ActionType::REDUCE, 0, 29 };
        ACTION[42]["if"] = { ActionType::REDUCE, 0, 29 };
        ACTION[42]["return"] = { ActionType::REDUCE, 0, 29 };

        ACTION[43]["vtype"] = { ActionType::REDUCE, 0, 30 };
        ACTION[43]["id"] = { ActionType::REDUCE, 0, 30 };
        ACTION[43]["rbrace"] = { ActionType::REDUCE, 0, 30 };
        ACTION[43]["while"] = { ActionType::REDUCE, 0, 30 };
        ACTION[43]["if"] = { ActionType::REDUCE, 0, 30 };
        ACTION[43]["return"] = { ActionType::REDUCE, 0, 30 };

        ACTION[44]["lparen"] = { ActionType::SHIFT, 55 };

        ACTION[45]["id"] = { ActionType::SHIFT, 56 };

        ACTION[46]["assign"] = { ActionType::SHIFT, 11 };

        ACTION[47]["lparen"] = { ActionType::SHIFT, 57 };

        ACTION[48]["id"] = { ActionType::SHIFT, 58 };

        ACTION[49]["semi"] = { ActionType::REDUCE, 0, 12 };
        ACTION[49]["rparen"] = { ActionType::REDUCE, 0, 15 };

        ACTION[50]["semi"] = { ActionType::REDUCE, 0, 15 };
        ACTION[50]["addsub"] = { ActionType::REDUCE, 0, 15 };
        ACTION[50]["rparen"] = { ActionType::REDUCE, 0, 15 };

        ACTION[51]["rbrace"] = { ActionType::SHIFT, 59 };

        ACTION[52]["id"] = { ActionType::SHIFT, 23 };
        ACTION[52]["literal"] = { ActionType::SHIFT, 17 };
        ACTION[52]["character"] = { ActionType::SHIFT, 18 };
        ACTION[52]["boolstr"] = { ActionType::SHIFT, 19 };
        ACTION[52]["lparen"] = { ActionType::SHIFT, 22 };
        ACTION[52]["num"] = { ActionType::SHIFT, 24 };

        ACTION[53]["rbrace"] = { ActionType::REDUCE, 0, 25 };
        ACTION[53]["return"] = { ActionType::REDUCE, 0, 25 };

        ACTION[54]["vtype"] = { ActionType::REDUCE, 0, 28 };
        ACTION[54]["id"] = { ActionType::REDUCE, 0, 28 };
        ACTION[54]["rbrace"] = { ActionType::REDUCE, 0, 28 };
        ACTION[54]["while"] = { ActionType::REDUCE, 0, 28 };
        ACTION[54]["if"] = { ActionType::REDUCE, 0, 28 };
        ACTION[54]["return"] = { ActionType::REDUCE, 0, 28 };

        ACTION[55]["boolstr"] = { ActionType::SHIFT, 63 };
        ACTION[55]["lparen"] = { ActionType::SHIFT, 64 };

        ACTION[56]["semi"] = { ActionType::SHIFT, 9 };
        ACTION[56]["assign"] = { ActionType::SHIFT, 11 };

        ACTION[57]["boolstr"] = { ActionType::SHIFT, 63 };
        ACTION[57]["lparen"] = { ActionType::SHIFT, 64 };

        ACTION[58]["rparen"] = { ActionType::REDUCE, 0, 24 };
        ACTION[58]["comma"] = { ActionType::SHIFT, 34 };

        ACTION[59]["vtype"] = { ActionType::REDUCE, 0, 20 };
        ACTION[59]["$"] = { ActionType::REDUCE, 0, 20 };

        ACTION[60]["semi"] = { ActionType::SHIFT, 67 };

        ACTION[61]["rparen"] = { ActionType::SHIFT, 68 };

        ACTION[62]["rparen"] = { ActionType::REDUCE, 0, 38 };
        ACTION[62]["comp"] = { ActionType::SHIFT, 70 };

        ACTION[63]["rparen"] = { ActionType::REDUCE, 0, 35 };
        ACTION[63]["comp"] = { ActionType::REDUCE, 0, 35 };

        ACTION[64]["boolstr"] = { ActionType::SHIFT, 63 };
        ACTION[64]["lparen"] = { ActionType::SHIFT, 64 };

        ACTION[65]["rparen"] = { ActionType::SHIFT, 72 };

        ACTION[66]["rparen"] = { ActionType::REDUCE, 0, 23 };

        ACTION[67]["rbrace"] = { ActionType::REDUCE, 0, 39 };

        ACTION[68]["lbrace"] = { ActionType::SHIFT, 73 };

        ACTION[69]["rparen"] = { ActionType::REDUCE, 0, 34 };

        ACTION[70]["boolstr"] = { ActionType::SHIFT, 63 };
        ACTION[70]["lparen"] = { ActionType::SHIFT, 64 };

        ACTION[71]["rparen"] = { ActionType::SHIFT, 75 };

        ACTION[72]["rbrace"] = { ActionType::SHIFT, 76 };

        ACTION[73]["vtype"] = { ActionType::SHIFT, 45 };
        ACTION[73]["id"] = { ActionType::SHIFT, 46 };
        ACTION[73]["while"] = { ActionType::SHIFT, 44 };
        ACTION[73]["if"] = { ActionType::SHIFT, 47 };
        ACTION[73]["rbrace"] = { ActionType::REDUCE, 0, 26 };
        ACTION[73]["return"] = { ActionType::REDUCE, 0, 26 };

        ACTION[74]["rparen"] = { ActionType::REDUCE, 0, 38 };
        ACTION[74]["comp"] = { ActionType::SHIFT, 70 };

        ACTION[75]["rparen"] = { ActionType::REDUCE, 0, 36 };
        ACTION[75]["comp"] = { ActionType::REDUCE, 0, 36 };

        ACTION[76]["vtype"] = { ActionType::SHIFT, 45 };
        ACTION[76]["id"] = { ActionType::SHIFT, 46 };
        ACTION[76]["while"] = { ActionType::SHIFT, 44 };
        ACTION[76]["if"] = { ActionType::SHIFT, 47 };
        ACTION[76]["rbrace"] = { ActionType::REDUCE, 0, 26 };
        ACTION[76]["return"] = { ActionType::REDUCE, 0, 26 };

        ACTION[77]["rbrace"] = { ActionType::SHIFT, 80 };

        ACTION[78]["rparen"] = { ActionType::REDUCE, 0, 37 };

        ACTION[79]["rbrace"] = { ActionType::SHIFT, 81 };

        ACTION[80]["vtype"] = { ActionType::REDUCE, 0, 31 };
        ACTION[80]["id"] = { ActionType::REDUCE, 0, 31 };
        ACTION[80]["rbrace"] = { ActionType::REDUCE, 0, 31 };
        ACTION[80]["while"] = { ActionType::REDUCE, 0, 31 };
        ACTION[80]["if"] = { ActionType::REDUCE, 0, 31 };
        ACTION[80]["return"] = { ActionType::REDUCE, 0, 31 };

        ACTION[81]["vtype"] = { ActionType::REDUCE, 0, 32 };
        ACTION[81]["id"] = { ActionType::REDUCE, 0, 32 };
        ACTION[81]["rbrace"] = { ActionType::REDUCE, 0, 32 };
        ACTION[81]["while"] = { ActionType::REDUCE, 0, 32 };
        ACTION[81]["if"] = { ActionType::REDUCE, 0, 32 };
        ACTION[81]["return"] = { ActionType::REDUCE, 0, 32 };
        ACTION[81]["else"] = { ActionType::SHIFT, 82 };

        ACTION[82]["lbrace"] = { ActionType::SHIFT, 83 };

        ACTION[83]["vtype"] = { ActionType::SHIFT, 45 };
        ACTION[83]["id"] = { ActionType::SHIFT, 46 };
        ACTION[83]["while"] = { ActionType::SHIFT, 44 };
        ACTION[83]["if"] = { ActionType::SHIFT, 47 };
        ACTION[83]["rbrace"] = { ActionType::REDUCE, 0, 26 };
        ACTION[83]["return"] = { ActionType::REDUCE, 0, 26 };

        ACTION[84]["rbrace"] = { ActionType::SHIFT, 85 };

        ACTION[85]["vtype"] = { ActionType::REDUCE, 0, 33 };
        ACTION[85]["id"] = { ActionType::REDUCE, 0, 33 };
        ACTION[85]["rbrace"] = { ActionType::REDUCE, 0, 33 };
        ACTION[85]["while"] = { ActionType::REDUCE, 0, 33 };
        ACTION[85]["if"] = { ActionType::REDUCE, 0, 33 };
        ACTION[85]["return"] = { ActionType::REDUCE, 0, 33 };

        // Initialize GOTO table
        GOTO[0]["CODE"] = 1;
        GOTO[0]["VDECL"] = 2;
        GOTO[0]["FDECL"] = 3;

        GOTO[2]["CODE"] = 5;
        GOTO[2]["VDECL"] = 2;
        GOTO[2]["FDECL"] = 3;

        GOTO[3]["CODE"] = 6;
        GOTO[3]["VDECL"] = 2;
        GOTO[3]["FDECL"] = 3;

        GOTO[4]["ASSIGN"] = 8;

        GOTO[10]["ARG"] = 13;

        GOTO[11]["RHS"] = 15;
        GOTO[11]["EXPR"] = 16;
        GOTO[11]["TERM"] = 20;
        GOTO[11]["FACTOR"] = 21;

        GOTO[20]["EXPR_TAIL"] = 27;

        GOTO[21]["TERM_TAIL"] = 29;

        GOTO[22]["EXPR"] = 31;
        GOTO[22]["TERM"] = 20;
        GOTO[22]["FACTOR"] = 21;

        GOTO[26]["MOREARGS"] = 33;

        GOTO[28]["TERM"] = 35;
        GOTO[28]["FACTOR"] = 21;

        GOTO[30]["FACTOR"] = 36;

        GOTO[32]["VDECL"] = 40;
        GOTO[32]["ASSIGN"] = 41;
        GOTO[32]["BLOCK"] = 38;
        GOTO[32]["STMT"] = 39;
        GOTO[32]["IF"] = 42;
        GOTO[32]["IFELSE"] = 43;

        GOTO[35]["EXPR_TAIL"] = 49;

        GOTO[36]["TERM_TAIL"] = 50;

        GOTO[46]["RETURN"] = 51;

        GOTO[39]["VDECL"] = 40;
        GOTO[39]["ASSIGN"] = 41;
        GOTO[39]["BLOCK"] = 38;
        GOTO[39]["STMT"] = 39;
        GOTO[39]["IF"] = 42;
        GOTO[39]["IFELSE"] = 43;

        GOTO[45]["ASSIGN"] = 8;

        GOTO[52]["RHS"] = 60;
        GOTO[52]["EXPR"] = 16;
        GOTO[52]["TERM"] = 20;
        GOTO[52]["FACTOR"] = 21;

        GOTO[55]["COND"] = 61;
        GOTO[55]["SIMPLECOND"] = 62;

        GOTO[57]["COND"] = 65;
        GOTO[57]["SIMPLECOND"] = 62;

        GOTO[58]["MOREARGS"] = 66;

        GOTO[62]["COND_TAIL"] = 69;

        GOTO[64]["COND"] = 71;
        GOTO[64]["SIMPLECOND"] = 62;

        GOTO[70]["SIMPLECOND"] = 74;

        GOTO[73]["VDECL"] = 40;
        GOTO[73]["ASSIGN"] = 41;
        GOTO[73]["BLOCK"] = 77;
        GOTO[73]["STMT"] = 39;
        GOTO[73]["IF"] = 42;
        GOTO[73]["IFELSE"] = 43;

        GOTO[74]["COND_TAIL"] = 78;

        GOTO[76]["VDECL"] = 40;
        GOTO[76]["ASSIGN"] = 41;
        GOTO[76]["BLOCK"] = 79;
        GOTO[76]["STMT"] = 39;
        GOTO[76]["IF"] = 42;
        GOTO[76]["IFELSE"] = 43;

        GOTO[83]["VDECL"] = 40;
        GOTO[83]["ASSIGN"] = 41;
        GOTO[83]["BLOCK"] = 84;
        GOTO[83]["STMT"] = 39;
        GOTO[83]["IF"] = 42;
        GOTO[83]["IFELSE"] = 43;
    }

public:
    Parser() {
        initializeProductions();
        initializeTables();
    }

    bool parse(const vector<string>& tokens, string& output, TreeNode*& parseTree) {
        stack<int> states;
        stack<TreeNode*> parseStack;
        states.push(0);

        int pos = 0;
        while (true) {
            int state = states.top();
            string token = (pos < tokens.size()) ? tokens[pos] : "$";

            // Log current state and token
            cout << "State: " << state << ", Token: " << token << endl;

            if (ACTION[state].find(token) == ACTION[state].end()) {
                output = "Error: Unexpected token '" + token + "' at position " + to_string(pos) + "\n";
                output += "Error: No ACTION entry for state " + to_string(state) + " and token '" + token + "'\n";
                return false;
            }

            Action action = ACTION[state][token];
            if (action.type == ActionType::SHIFT) {
                states.push(action.state);
                parseStack.push(new TreeNode(token));
                pos++;
            }
            else if (action.type == ActionType::REDUCE) {
                const auto& production = productions[action.production];
                int pop_count = production.second.size();
                TreeNode* node = new TreeNode(production.first);
                while (pop_count--) {
                    states.pop();
                    node->children.insert(node->children.begin(), parseStack.top());
                    parseStack.pop();
                }
                state = states.top();

                // Log the production used for reduction
                cout << "Reducing using production: " << production.first << " -> ";
                for (const auto& sym : production.second) {
                    cout << sym << " ";
                }
                cout << endl;

                if (GOTO[state].find(production.first) == GOTO[state].end()) {
                    output = "Error: No GOTO for production '" + production.first + "' from state " + to_string(state) + "\n";
                    output += "Error: Unexpected token '" + token + "' at position " + to_string(pos) + "\n";
                    return false;
                }
                states.push(GOTO[state][production.first]);
                parseStack.push(node);
            }
            else if (action.type == ActionType::ACCEPT) {
                output = "Parsing successful!\n";
                if (!parseStack.empty()) {
                    parseTree = parseStack.top();
                }
                return true;
            }
            else {
                output = "Error: Parsing error at token '" + token + "' at position " + to_string(pos) + "\n";
                return false;
            }
        }
    }




};

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

    Parser parser;

    vector<string> tokens = readTokensFromFile(inputFilename);
    string output;
    TreeNode* parseTree = nullptr;

    if (parser.parse(tokens, output, parseTree)) {
        ofstream outfile(outputFilename);
        outfile << output;
        if (parseTree) {
            parseTree->writeTreeToFile(outfile);
        }
        outfile.close();
    }
    else {
        writeOutputToFile(outputFilename, output);
    }

    delete parseTree;
    return 0;
}
