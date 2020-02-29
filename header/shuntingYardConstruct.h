#ifndef SHUNTINGYARDCONSTRUCT_H
#define SHUNTINGYARDCONSTRUCT_H

#include <iostream>
#include <vector>
#include <deque>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "rshellclasses.h"
#include "rshellutils.h"
#include "splitOnChar.h"

int findClose(const string& targetString, int start, char targetClose) {
	// Given this:
	//              |
	// mgoiejnoaijt ( kteopajtio []gea][]g[a((()))(()) )
	// Where the | represents the start char pos, and targetclose = ')',
	// then findClose will return:
	//                                                 |
	// mgoiejnoaijt ( kteopajtio []gea][]g[a((()))(()) )
	//
	// This allows for the range of ParenthesisToken to be defined.
	int depth = 0;
	char open = targetString.at(start);
	for (int i = start+1; i < targetString.size(); i++) {
		char currChar = targetString.at(i);
		if (currChar == open) {
			depth += 1;
		} else if ((currChar == targetClose) && (depth > 0)) {
			depth -= 1;
		} else if ((currChar == targetClose) && (depth == 0)) {
			return i;
		}
	}

	return -1;
}

deque<Token*> shuntingYardConstruct(string commandString) {
	// Want to go from input string to final deque to execute on
	// echo a || (( false && echo c ))
	// becomes
	// {Subcommand: {"echo", "a"},
	//  OrToken:    {"||"},
	//  ParenToken: leftChild->ParenToken,
	//  ParenToken: 

	unordered_set<string> operators = {"||", "&&", ";"};
	unordered_map<char, char> openToClose = {
											{'(',')'},  
											{'[', ']'}
											};
	// The following deque is used for checking whether an operator has been read
	deque<char> backlog;
	int maxbacklog = 2; // Defined by the longest operator in operators; update as needed
	vector<int> allowed_lengths = {2, 1}; // Defined by length of each operator...

	// The following vector is used for flushes to Subcommand
	vector<char> buffer;

	// Shunting yard deque/stack
	stack<Token*> shuntingSouth;
	deque<Token*> outputQueue;

	int lastFlushed = 0; // TODO (maybe)
	int currPos = 0;
	for (auto it = commandString.begin(); it != commandString.end(); it++) {
		// Check if currently on something in "operators"
		char c = *it;
		backlog.push_back(c);
		if (backlog.size() > maxbacklog) {
			backlog.pop_front();
		}
		string accepted = " ";

		for (int i : allowed_lengths) {
			string query(backlog.begin()+(maxbacklog-i), backlog.end());
			if (operators.count(query) > 0) {
				accepted = query;
				break;
			}
		}

		if (accepted != " ") {
			// Will need to double check whether this kind of pointer arithmetic is valid
			string subcstring(buffer.begin(), buffer.end());
			buffer.clear();
			vector<string> subcvect = splitOnChar(subcstring, ' '); // strip() and split(' ')!

			lastFlushed = currPos + 1;

			Subcommand* subcobj = new Subcommand(subcvect);

			// Construct specific type of Token
			Token* myToken;
			if (accepted == "||") {
				myToken = new OrToken({"||"});
			} else if (accepted == "&&") {
				myToken = new AndToken({"&&"});
			} else if (accepted == ";") {
				myToken = new SemiToken({";"});
			}

			outputQueue.push_back(subcobj);

			// In shunting yard, pop operators when a new one is added.
			while (shuntingSouth.size() > 0) {
				Token* myOp = shuntingSouth.top();
				shuntingSouth.pop();
				outputQueue.push_back(myOp);
			}
			shuntingSouth.push(myToken);

		} else if (openToClose.count(c) > 0) { // It's something in the form (  ) or [   ]
			if (c == '[') {
				int closepos = findClose(commandString, currPos, openToClose[c]);
				string teststring(commandString.begin()+currPos+1, commandString.begin()+closepos);
			
			
			} else if (c == '(') {

				// Recursion time!
				// Why?
				// Shunting yard doesn't handle conditional execution for parentheses properly.
				// Example:
				// echo a || ( echo b && echo c )
				// Shunting yard would output this deque:
				// [echo a, echo b, echo c, &&, ||]
				// When thrown into execution,"d = echo b && echo c" would run first, and then "echo a || d".
				// While this is correct order of operations if this were algebra, we never wanted the stuff
				//     inside the parentheses to execute in the first place.
				// Therefore, I pair (   ) delimited stuff into a "ParenthesisToken", which causes Shunting Yard to produce something like this:
				// [echo a, [echo b, echo c, &&], ||]
				// This WILL be properly handled during execution, as opposed to the other deque.

				int closepos = findClose(commandString, currPos, openToClose[c]);
				string substring(commandString.begin()+currPos+1, commandString.begin()+closepos);

				deque<Token*> retq = shuntingYardConstruct(substring);
				ParenthesisToken* stuffInside = new ParenthesisToken(retq);

				outputQueue.push_back(stuffInside);
			}
		} else {
			buffer.push_back(c);
		}
		currPos += 1;
	}
	return outputQueue;
}

#endif
