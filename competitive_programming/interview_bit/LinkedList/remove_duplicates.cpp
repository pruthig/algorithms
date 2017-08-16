// delete duplicates from linked list
#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int value;
  ListNode *next;
  ListNode(int x) : value(x), next(NULL) {}
};
 
//Remove duplicates from a list
ListNode* removeDuplicates(ListNode* node) {
	ListNode *head = node, *prevNode = node, *head_node = node;
	if(!node)
		return nullptr;
	
	int prev_data = node->value;
	node = node->next;
	while(node) {
		if(node->value != prev_data) {
			prevNode->next = node;
			prev_data = node->value;
			prevNode = node;
		}
		node = node->next;
		
	}
	prevNode->next = nullptr;
	return head_node;
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
	(*head_A)->next->next = new ListNode(5);
	(*head_A)->next->next->next = new ListNode(5);
	(*head_A)->next->next->next->next = new ListNode(5);
	(*head_A)->next->next->next->next->next = nullptr; 
}

int main() {
	ListNode *head_A = nullptr;
	createList_A(&head_A);
	ListNode *ptr = removeDuplicates(head_A);
	if(ptr)
		printList(ptr);
	return 0;
}
