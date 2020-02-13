#ifndef RSHELLCLASSES_H
#define RSHELLCLASSES_H

#include <vector>
#include <string>
#include <stack>

using namespace std;


class Token {
    public:
        Token() {};

        vector<string> getContent()          { return content; }
        void setContent(vector<string> V)    { content = V; }

        virtual int execute() = 0;

        void setLeft(Token* t) { leftChild = t; }
        void setRight(Token* t) { rightChild = t; }
        bool hasChildren() { return ((leftChild != nullptr) || (rightChild != nullptr)); }
		bool operator==(Token const &rhs) const {
			return (this->content == rhs.content);
		}
        string stringify() { return joinVector(content, ' '); }

        // Member variables
        vector<string> content;
        Token* leftChild;
        Token* rightChild;
        int status = -1; // Current exit status of Token
};

class Subcommand : public Token {
    public:
        Subcommand(vector<string> V) { content = V; }
        int execute() { // TODO: To be implemented
		return 0;
	}
};

class Operator : public Token {
    public:
        Operator(vector<string> V) { content = V; }
        void makeStatus(int a, int b) {
            if (b == -1) {
                if (a == 0) {
                    this->status = 0;
                } else {
                    this->status = 1;
                }
            } else {
                if ((a == 0) || (b == 0)) {
                    this->status = 0;
                } else {
                    this->status = 1;
                }
            }
        }
        int execute() {
            int statusLeft, statusRight = -1;
            statusLeft = leftChild->execute();
            
            // Always run ;
            if (content[0] == ";") {
                statusRight = rightChild->execute();
            }
            
            // Conditionally run && and ||
            if ((statusLeft == 0) && (content[0] == "&&")) {
                statusRight = rightChild->execute();
            }
            if ((statusLeft != 0) && (content[0] == "||")) {
                statusRight = rightChild->execute();
            }

            makeStatus(statusLeft, statusRight);
        }
};

class CommandTree {
    public:
        CommandTree() : head(nullptr) {}
//        ~CommandTree() {
//            // Delete nodes using BFS
//            if (head != nullptr) {
//                stack<Token*> s;
//                s.push(head);
//                while (!(s.empty())) {
//                    Token* currNode = s.top();
//                    s.pop();
//                    if (currNode->leftChild != nullptr) {
//                        s.push(currNode->leftChild);
//                    }
//                    if (currNode->rightChild != nullptr) {
//                        s.push(currNode->rightChild);
//                    }
//                    delete currNode;
//                }
//            }
//        }
//
        void setHead(Token* t) { head = t; }
        Token* getHead() { return head; }
        string stringify() {
            // Initialize stack
            stack<pair<Token*,int>> s; // Stores token and number of spaces
            vector<string> output;
            pair<Token*, int> startelt;
            startelt.first = head;
            startelt.second = 0;
            s.push(startelt);
            while (!(s.empty()) ) {
                // Get top element
                pair<Token*, int> topelt = s.top();
                Token* curr = topelt.first;
                int numSpaces = topelt.second;
                s.pop();

                string spaces(numSpaces, ' ');
                if (curr->hasChildren()) {
                    output.push_back(spaces);
                    output.push_back(curr->stringify());
                    output.push_back(" : {");
                    
                    // add right, then left so stack order prints properly

                    if (curr->rightChild != nullptr) {
                        pair<Token*, int> add_to_stack;
                        add_to_stack.first = curr->rightChild;
                        add_to_stack.second = numSpaces + 2;
                        s.push(add_to_stack);
                    }
                    
                    if (curr->leftChild != nullptr) {
                        pair<Token*, int> add_to_stack;
                        add_to_stack.first = curr->leftChild;
                        add_to_stack.second = numSpaces + 2;
                        s.push(add_to_stack);
                    }
                    
                } else {
                    output.push_back(spaces);
                    output.push_back(curr->stringify());
                    output.push_back("\n");
                    if (numSpaces >= 2) {
                        numSpaces -= 2;
                    }
                    string lessSpaces(numSpaces, ' ');
                    output.push_back(lessSpaces);
                    output.push_back("}");
                }
                output.push_back("\n");
            }
            return joinVector(output);
        }
    protected:
        Token* head;
};

#endif
