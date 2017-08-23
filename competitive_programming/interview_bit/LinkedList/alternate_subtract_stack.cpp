// Subtract and store result of (last-first) to first n/2 nodes
// This program uses stack
#include<iostream>
#include<vector>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
};

namespace {
vector<int> vec;
}

void createList(ListNode **head) {
	(*head) = new ListNode(1);
	(*head)->next = new ListNode(2);
    (*head)->next->next = new ListNode(3);
	(*head)->next->next->next = new ListNode(4);
    (*head)->next->next->next->next = new ListNode(5);
}

int get_size_of_list(ListNode * cur) {
    int count = 0;
    while(cur) {
        ++count;
        cur = cur->next;
    }   
    return count;
}

void printList(ListNode* head) {
    cout<<endl;
    while(head) {
        cout<<head->val<<"->";
        head = head->next;
    }
    cout<<"nullptr";
    cout<<endl;
}

 //subtract 1-n
void alternate_subtract(ListNode** current, int size) {
	if(current == nullptr || size <= 1) {
		return;
	}
    ListNode *temp = *current;
    int trav_count  = 0;
    if(size%2)
        trav_count = size/2 +1;
    else
        trav_count = size/2;

    while(temp && trav_count >= 0) {
        temp = temp->next;
        --trav_count;
    }

    while(temp) {
        vec.push_back(temp->val);
        temp = temp->next;
    }
    // stack half full, now use it
    temp = *current;
    trav_count = vec.size();

    if(trav_count && !vec.empty()) {
        temp->val = vec.back() - temp->val;
        vec.pop_back();
        --trav_count;
        temp = temp->next;
    }    
}

int main() {
	ListNode *head = nullptr;
	createList(&head);
    vec.clear();
    int size = get_size_of_list(head);
	alternate_subtract(&head, size);
    cout<<endl;
    printList(head);
	return 0;
}
