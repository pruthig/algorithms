// reverse every 'K' nodes
#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int value;
  ListNode *next;
  ListNode(int x) : value(x), next(NULL) {}
};
 
//Reverse a list
ListNode* reverseList(ListNode *prev, ListNode* current, ListNode** next_block, int k) {
	if(current == nullptr || k == 0) {
		if(current)
			*next_block = current;
		return prev;
	}
	ListNode* ptr = reverseList(current, current->next, next_block, --k);
	current->next = prev;
	return ptr;
}

void printList(ListNode* head) {
	while(head) {
		cout<<head->value<<"->";
		head = head->next;
	}
	cout<<"nullptr";
}

void createList(ListNode **head) {
	(*head) = new ListNode(1);
	(*head)->next = new ListNode(2);
	(*head)->next->next = new ListNode(3);
	(*head)->next->next->next = new ListNode(4);
	(*head)->next->next->next->next = new ListNode(5);
	(*head)->next->next->next->next->next = new ListNode(6);
	(*head)->next->next->next->next->next->next = new ListNode(7);
	(*head)->next->next->next->next->next->next->next = new ListNode(8);
	(*head)->next->next->next->next->next->next->next->next = new ListNode(9);
}

ListNode* reverseListAux(ListNode *head, int k){
	if(!head)
		return nullptr;
	ListNode *next_block = nullptr;
	// reverse by k positions
	ListNode *new_ptr = reverseList(nullptr, head, &next_block, k);
	head->next = reverseListAux(next_block, k);
	return new_ptr;
}	                                         

int main() {
	ListNode *head = nullptr;
	createList(&head);
	ListNode* ptr = reverseListAux(head, 3);
	if(ptr)
		printList(ptr);
	return 0;
}
