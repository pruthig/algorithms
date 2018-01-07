// A RandomListNode list is given such that each node contains an additional random pointer which could point to any node in the list or NULL.
// Return a deep copy of the list.
#include<iostream>
#include<unordered_map>

using namespace std;

struct RandomListNode {
    int label;
    RandomListNode *next, *random;
    RandomListNode(int x) : label(x), next(NULL), random(NULL) {}
};

namespace {
    unordered_map<  RandomListNode *,  RandomListNode * > mp;
    RandomListNode *head = nullptr;
};
    
void createNewFromOld(RandomListNode *old_list) {
    if(old_list == nullptr)
        return;
    RandomListNode *cur_old = old_list, *cur_new = nullptr;
    
    while(cur_old) {
	    if(!head) {
		    head = new RandomListNode(cur_old->label);
		    cur_new = head;
		    mp[cur_old] = cur_new;
		    cur_old = cur_old->next;
		    continue;
	    }
	    cur_new->next = new RandomListNode(cur_old->label);
	    mp[cur_old] = cur_new->next;
	    cur_old = cur_old->next;
	    cur_new = cur_new->next;
    }
}

void updateRandomPointers(RandomListNode  *old_list) {
    if(old_list == nullptr)
        return; // nullptr;
    RandomListNode *cur_old = old_list, *cur_new = head;
    
    while(cur_old && cur_new) {
        if(cur_old->random)
    	    cur_new->random = mp.find(cur_old->random)->second;
        cur_old = cur_old->next;
        cur_new = cur_new->next;
    }
}
	
void printList(RandomListNode* head) {
    while(head) {
        cout<<"value: "<<head->label;
        if(head->random) cout<<"Random: "<<head->random->label;

        head = head->next;
        cout<<endl;
    }
    cout<<"nullptr";
}

int main() {
    RandomListNode *old_list = nullptr;
    old_list = new RandomListNode(1);
    old_list->next =   new RandomListNode(2);
    old_list->next->random = old_list;
    printList(old_list);

    createNewFromOld(old_list);
    //if(head == nullptr) return nullptr;
    updateRandomPointers(old_list);
    return 0;
}
    

