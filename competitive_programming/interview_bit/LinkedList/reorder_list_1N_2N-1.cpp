// This program re-orders the given linked list so that it looks like this:
// L0 -> LN -> L1 -> L(N-1) -> ...
#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
};


void printList(ListNode* head) {
	while(head) {
		cout<<head->val<<"->";
		head = head->next;
	}
	cout<<"nullptr";
}
 
//Reverse a list
ListNode* reverseList(ListNode *prev, ListNode* current) {
	if(current == nullptr)
		return prev;
	ListNode* ptr = reverseList(current, current->next);
	current->next = prev;
	return ptr;
}


void createList(ListNode **head) {
	(*head) = new ListNode(1);
	(*head)->next = new ListNode(2);
	(*head)->next->next = new ListNode(3);
	(*head)->next->next->next = new ListNode(4);
	(*head)->next->next->next->next = new ListNode(5);
	(*head)->next->next->next->next->next = new ListNode(6);
	//(*head)->next->next->next->next->next->next = new ListNode(7);
	//(*head)->next->next->next->next->next->next->next = (*head)->next->next->next;
	//(*head)->next->next->next->next->next->next->next->next = (*head)->next->next->next->next;
}               

int get_size_of_list(ListNode * cur) {
	int count = 0;
    while(cur) {
        ++count;
        cur = cur->next;
    }
    return count;
}


ListNode* reorder(ListNode *head) {
	if(head == nullptr)
		return nullptr;
	// find size of list and 
	int size = get_size_of_list(head);
	if(size == 1 || size == 2)
		return nullptr;
		
	--size; // change it in terms of index
	// start reversing from mid+1;
	int mid = size/2;
	// traverse to mid
	ListNode *cur = head;
	while(mid--)
		cur = cur->next;
	// Now cur points to mid
	ListNode *new_node = cur->next;
	cur->next = nullptr; // first list terminated
	new_node = reverseList(nullptr, new_node); // Now new_node points to head
	// Combine both lists
	cur = head;
	ListNode *nxt = cur->next;
	ListNode *new_nxt = new_node->next;
		
	// and new_node
	//head - new_node
	while(cur && new_node) {
		
		cur->next = new_node;
		cur = nxt;
		if(cur)
			nxt = cur->next;
		
		new_node->next = cur;
		new_node = new_nxt;
		if(new_node)
			new_nxt = new_node->next;
	}
	return head;
}

int main() {
	ListNode *head = nullptr, *detectionpoint = nullptr;
	createList(&head);
	ListNode *hdr = reorder(head);
	printList(hdr);
	return 0;
}
