// remove duplicated nodes
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

void createList(ListNode **head) {
	(*head) = new ListNode(1);
	(*head)->next = new ListNode(1);
	(*head)->next->next = new ListNode(2);
	(*head)->next->next->next = new ListNode(3);
	(*head)->next->next->next->next = new ListNode(3);
	(*head)->next->next->next->next->next = new ListNode(6);
	(*head)->next->next->next->next->next->next = new ListNode(7);
	(*head)->next->next->next->next->next->next->next = new ListNode(8);
	(*head)->next->next->next->next->next->next->next->next = new ListNode(9);
}

 // This functions remove all the nodes which have a duplicate present in the list.
ListNode* unique_list(ListNode* head) {
	if(head == nullptr) {
		return nullptr;
	}
	ListNode *cursor = head;
	ListNode *new_list = nullptr, *new_head = nullptr;
	
	while(cursor) {
		if(cursor->next && cursor->val == cursor->next->val) {
			ListNode *tmp = cursor;
			while(tmp && tmp->val == cursor->val)
				tmp = tmp->next;
			cursor = tmp;
			continue;
		}
		
		if(!new_list){
			new_list = cursor;
			new_head = new_list;
		}
			
		else {
			new_list->next = cursor; 
			new_list = new_list->next;
		}
		
		cursor = cursor->next;
	}
	if(new_list)
		new_list->next = nullptr;
	return new_head;
}

int main() {
	ListNode *head = nullptr;
	createList(&head);
	ListNode *new_list = unique_list(head);
	printList(new_list);
	return 0;
}
