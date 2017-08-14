#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int value;
  ListNode *next;
  ListNode(int x) : value(x), next(NULL) {}
};
 
 ListNode *head = nullptr;
 
//Reverse a list
ListNode* reverseList(ListNode *prev, ListNode* current) {
	if(current == nullptr)
		return prev;
	ListNode* ptr = reverseList(current, current->next);
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

void createList_A(ListNode *(*head_A)) {
	(*head_A) = new ListNode(1);
	(*head_A)->next = new ListNode(5);
	(*head_A)->next->next = new ListNode(6);
	(*head_A)->next->next->next = new ListNode(7);
	(*head_A)->next->next->next->next = new ListNode(9);
	(*head_A)->next->next->next->next->next = nullptr; 
}

ListNode* reverseListAux(ListNode *head_A){
	return reverseList(nullptr, head_A);
}
int main() {
	ListNode (*head_A) = nullptr;
	createList_A(&head_A);
	ListNode *ptr = reverseListAux(head_A);
	if(ptr)
		printList(ptr);
	return 0;
}
