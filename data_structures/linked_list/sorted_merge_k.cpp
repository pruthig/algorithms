// This program merges k sorted linked lists
#include<iostream>
#include<vector>

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
	(*head_A)->next = new ListNode(4);
	(*head_A)->next->next = new ListNode(6);
	(*head_A)->next->next->next = new ListNode(7);
}

void createList_B(ListNode *(*head_B)) {
	(*head_B) = new ListNode(8);
	(*head_B)->next = new ListNode(9);
	(*head_B)->next->next = new ListNode(10);
	(*head_B)->next->next->next = new ListNode(22);
}

void createList_C(ListNode *(*head_C)) {
	(*head_C) = new ListNode(7);
	(*head_C)->next = new ListNode(11);
	(*head_C)->next->next = new ListNode(13);
	(*head_C)->next->next->next = new ListNode(78);
}


ListNode* merge_lists(ListNode *head_A, ListNode *head_B) {
	if(!head_A)
		return head_B;
	if(!head_B)
		return head_A;
	if(head_A->value < head_B->value) {
		head_A->next = merge_lists(head_A->next, head_B);
		return head_A;
	}
	else {
		head_B->next = merge_lists(head_A, head_B->next);
		return head_B;
	}
}

int main() {
	ListNode (*head_A) = nullptr, (*head_B) = nullptr, (*head_C) = nullptr, *merged_list = nullptr;
	createList_A(&head_A);
	createList_B(&head_B);
	createList_C(&head_C);
	vector<ListNode*> A { head_A, head_B, head_C};
	if(A.size() == 0)
		return 0; //nullptr
	else if(A.size() == 1)
		return 0; //A[0];
	else {
		merged_list = A[0];
		for(int i = 1; i < A.size(); ++i)
			merged_list = merge_lists(merged_list, A[i]);
	}
	//printing the list
	if(merged_list)
		printList(merged_list);
	return 0;
}
