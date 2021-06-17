#include<iostream>

using namespace std;

#define ALPHABET_SIZE 26

// trie node
struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
  
    bool isLeaf; // used to denote if given node is End of Word.
};

// Returns true if root has no children, else false
bool isEmpty(TrieNode* root)
{
    for (int i = 0; i < ALPHABET_SIZE; i++)
        if (root->children[i])
            return false;
    return true;
}

TrieNode *createTrieNode() {
    TrieNode *newNode = new TrieNode;
    for(int i=0;i<26;++i)
        newNode->children[i] = nullptr;
    return newNode;
}

void insertTrieUtil(struct TrieNode *root, string key, int index) {
    if(index == key.size()-1) {
        if(!root->children[key[index]-'a'])
            root->children[key[index]-'a'] = createTrieNode();
        root->children[key[index]-'a']->isLeaf = true;
        return;
    }
    if(root->children[key[index]-'a'] == nullptr) {
        root->children[key[index]-'a'] = createTrieNode();
    }
    insertTrieUtil(root->children[key[index]-'a'], key, index+1);
}
// root : root node of the trie
// key : string to be inserted into the trie
// If not present, inserts key into trie
// If the key is prefix of trie node, just marks leaf node

void insert(struct TrieNode *root, string key) {
    insertTrieUtil(root, key, 0);
}

// root : root node of the trie
// key : string to search in the trie
// Returns true if key presents in trie, else false
bool searchTrieUtil(struct TrieNode *root, string key, int index) {
    if(!root)
        return false;
    if(index == key.size()-1) {
        if(root->children[key[index]-'a'] != nullptr &&
            root->children[key[index]-'a']->isLeaf == true)
            return true;
        else
            return false;
    }
    if(root->children[key[index]-'a'] == nullptr) {
        return false;;
    }
    
    bool b = searchTrieUtil(root->children[key[index]-'a'], key, index+1);
    if(!b)
        return false;
    return true;
}

bool search(struct TrieNode *root, string key) {
    //cout<<"Searching"<<endl;
    return searchTrieUtil(root, key, 0);
}

// Recursive function to delete a key from given Trie
TrieNode* remove(TrieNode* root, string key, int depth = 0)
{
    // If tree is empty
    if (!root)
        return NULL;
 
    // If last character of key is being processed
    if (depth == key.size()) {
 
        // This node is no more end of word after
        // removal of given key
        if (root->isLeaf)
            root->isLeaf = false;
 
        // If given is not prefix of any other word
        if (isEmpty(root)) {
            delete (root);
            root = NULL;
        }
 
        return root;
    }
 
    // If not last character, recur for the child
    // obtained using ASCII value
    int index = key[depth] - 'a';
    root->children[index] =
          remove(root->children[index], key, depth + 1);
 
    // If root does not have any child (its only child got
    // deleted), and it is not end of another word.
    if (isEmpty(root) && root->isLeaf == false) {
        delete (root);
        root = NULL;
    }
    return root;
}

int main() {
    
    string keys[] = {"the", "a", "there",
                    "answer", "any", "by",
                     "bye", "their" };
    int n = sizeof(keys)/sizeof(keys[0]);
  
    struct TrieNode *root = createTrieNode();
  
    // Construct trie
    for (int i = 0; i < n; i++)
        insert(root, keys[i]);
  
    // Search for different keys
    search(root, "the")? cout << "Yes\n" :
                         cout << "No\n";
    search(root, "these")? cout << "Yes\n" :
                           cout << "No\n";
    return 0;
}
    