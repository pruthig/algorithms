// This program merges linked lists with alternate nodes...
#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int value;
  ListNode *next;
  ListNode(int x) : value(x), next(NULL) {}
};
 

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
	(*head_A)->next->next->next = new ListNode(9);
	(*head_A)->next->next->next->next = new ListNode(12);
	(*head_A)->next->next->next->next->next = new ListNode(15);
	(*head_A)->next->next->next->next->next->next = new ListNode(16);
	(*head_A)->next->next->next->next->next->next->next = new ListNode(89);
}

void createList_B(ListNode *(*head_B)) {
	(*head_B) = new ListNode(8);
	(*head_B)->next = new ListNode(9);
	(*head_B)->next->next = new ListNode(10);
	(*head_B)->next->next->next = new ListNode(22);
	(*head_B)->next->next->next->next = new ListNode(45);
	(*head_B)->next->next->next->next->next = new ListNode(78);
}

ListNode* merge_lists(ListNode *head_A, ListNode *head_B, bool _switch) {
	if(!head_A)
		return head_B;
	if(!head_B)
		return head_A;
	if(_switch) {
		head_A->next = merge_lists(head_A->next, head_B, false);
		return head_A;
	}
	else {
		head_B->next = merge_lists(head_A, head_B->next, true);
		return head_B;
	}
}

int main() {
	ListNode (*head_A) = nullptr, (*head_B) = nullptr, *merged_list = nullptr;
	createList_A(&head_A);
	createList_B(&head_B);
	
	merged_list = merge_lists(head_A, head_B, true);
	if(merged_list)
		printList(merged_list);
	return 0;
}

