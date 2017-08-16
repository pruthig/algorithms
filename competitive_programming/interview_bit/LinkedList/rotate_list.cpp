// This program rotates the linked list
#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
};

void printList(ListNode* head) {
	cout<<"Rotated list is: \n";
	while(head) {
		cout<<head->val<<"->";
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

int get_size_of_list(ListNode * cur) {
	int count = 0;
    while(cur) {
        ++count;
        cur = cur->next;
    }
    return count;
}

 // This functions rotates the list by k positions
ListNode* rotate_list(ListNode* head, int k) {
	ListNode *new_head = nullptr;
	if(head == nullptr) {
		return nullptr;
	}
	int size = get_size_of_list(head);
	
	if(k > size)
		k %= size;

	if(k == 0 || k == size)
		return head;

		
	if(!size)
		return nullptr;
	ListNode *cursor = head;
	int to_trav = size-k-1;
	while(to_trav) {
		cursor = cursor->next;
		--to_trav;
	}
	
	new_head = cursor->next;
	cursor->next = nullptr;
	ListNode *tmp = new_head;
	while(tmp && tmp->next)
		tmp = tmp->next;
	
	tmp->next = head;
	return new_head;
	
}

int main() {
	ListNode *head = nullptr;
	int elem = 0;
	createList(&head);
	cout<<"Enter the number of elements by which list needs to be rotated\n";
	cin>>elem;
	ListNode *new_list = rotate_list(head, elem);
	printList(new_list);
	return 0;
}
