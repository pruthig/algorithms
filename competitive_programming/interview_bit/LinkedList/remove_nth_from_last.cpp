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
ListNode* removeNthNode(ListNode* head, int element) {
	
	if(head == NULL)
		return nullptr;
	if(element == 0)
		return head;
		
	int size = 0;
	
	ListNode *tmp = head;
	while(tmp) {
		++size;
		tmp = tmp->next;
	}
	tmp = head;
	// Check if list size is less than element entered
	if(element >= size) {
		head = head->next;
		delete tmp;
		return head;
		
	}
	else {
		// traverse the list till dummyth element
		int dummy = size-element-1;
		while(dummy) {
			tmp = tmp->next;
			--dummy;
		}
		ListNode *todelete = tmp->next;
		tmp->next = todelete->next;
		delete todelete;
	}
		
		
	return head;
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


int main() {
	ListNode (*head_A) = nullptr;
	createList_A(&head_A);
	cout<<"Enter the index\n";
	int elem;
	cin>>elem;
	ListNode *ptr = removeNthNode(head_A, elem);
	cout<<"Modified list is: "<<endl;
	if(ptr)
		printList(ptr);
	return 0;
}
