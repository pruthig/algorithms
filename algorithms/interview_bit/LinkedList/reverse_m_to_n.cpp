// reverse nodes from position 'm to 'n'
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

ListNode* reverseListAux(ListNode **head, int m, int n){
	if(!(*head))
		return nullptr;
	ListNode *cursor = (*head);
	ListNode *new_ptr = nullptr;
	if(m) {
		while(m-1) {
			cursor = cursor->next;
			--m;
		} // Head after loop points to the node before the first node
	}
	ListNode *next_block = nullptr;
	// reverse by k positions
	if(m == 0)
		new_ptr = reverseList(nullptr, cursor, &next_block, n-m+1);
	else
		new_ptr = reverseList(nullptr, cursor->next, &next_block, n-m+1);
		
	if(!m) {
		cursor->next = next_block;
		(*head) = new_ptr;
	}
	else {
		cursor->next->next = next_block;
		cursor->next = new_ptr;
	}
	
	return (*head);
}	                                         

int main() {
	ListNode *head = nullptr;
	int m = 0, n = 0;
	cout<<"Enter the position\n";
	cin>>m>>n;
	createList(&head);
	ListNode* ptr = reverseListAux(&head, m-1, n-1);
	if(ptr)
		printList(ptr);
	return 0;
}
