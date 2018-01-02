// implementing trie to find unique prefix;
#include<iostream>
#include<vector>

using namespace std;

struct TrieNode {
	char c;
	int setChildCount;
	bool isEndOfWord;
	struct TrieNode *child[26];
	TrieNode(char ch):child(), c(ch), setChildCount(0), isEndOfWord(false) {
	}
};

namespace {
	TrieNode *root = NULL;
}

// This function will add a string to trie...assume all chars in lowercase
void addString(TrieNode *root, string s, int i) {
	if(root == NULL || s.length() == 0)
		return;
	
	if(i == s.length()) {
		root->isEndOfWord = true;
		++root->setChildCount;	
		return;
	}
	
	if(!root->child[s[i] - 'a']) {
		root->child[s[i] - 'a'] = new TrieNode(s[i]);
	}

	++root->setChildCount;
	
	addString(root->child[s[i] - 'a'], s, i+1);
}

void printTrieUtil(TrieNode *root) {
	static string temp = "";
	if(root == NULL) {
		return;
	}
	temp += root->c;
	if(root->setChildCount) {
		for(int i = 0; i < 26; ++i) {
			if(root->child[i])  
				printTrieUtil(root->child[i]);
		}
	}
	if(root->isEndOfWord)
		cout<<temp;
		
	temp.pop_back();
	cout<<endl;
}

// This function prints all strings under trie using recursion...
void printTrie(TrieNode *root) {
	if(root == NULL) {
		return;
	}
	
	// Traverse all childern
	for(int i = 0; i < 26; ++i)
		printTrieUtil(root->child[i]);
	return;
}


void findUniquePrefixUtil(TrieNode *root, string s, int i, string& t) {
		
	if(root == NULL || s.length() == 0 || i == s.length() || !root->setChildCount)
		return;
	
	if(root->setChildCount) {
		t += s[i];
		if(root->setChildCount == 1)
			return;
	}
		
	else
		return;
	
	++i;
	findUniquePrefixUtil(root->child[s[i]-'a'], s, i, t);
}
 	
// Returns the unique prefix of string which is already added to trie
// It calls util with proper child node
string findUniquePrefix(TrieNode *root, string s) {
	string t = "";
	findUniquePrefixUtil(root->child[s[0]-'a'], s, 0, t);
	return t;
}

int main() {
	// create head node
	root = new TrieNode('\0');
	vector<string> A { "poled", "polam"} ; //"ebra", "dog", "duck", "dove" };
	vector<string> res{};
	
	for(auto a : A)
		addString(root, a, 0);
		
	//printTrie(root);
	
	for(auto a: A) {
		res.push_back(findUniquePrefix(root, a));
	}
	// print result
	for(auto &b : res)
		cout<<b<<endl;
	return 0;
	
	
	
}
