// This program finds where the cycles begins
// reverse every 'K' nodes
#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
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
	(*head)->next->next->next->next->next->next->next = (*head)->next->next->next;
	//(*head)->next->next->next->next->next->next->next->next = (*head)->next->next->next->next;
}               

bool findCycle(ListNode *head, ListNode **detectionpoint) {
	ListNode *cursor = head;
	if(cursor->next == nullptr)
		return false;
	ListNode *slow = head->next;
	ListNode *fast = head->next->next;
	
	while(slow && fast) {
		if(slow == fast) {
			(*detectionpoint) = slow;
			return true;
		}
			
		slow = slow->next;
		if(!fast->next)
			break;
		fast = fast->next->next;
	}
	return false;
	
}

ListNode* findHit(ListNode *head, ListNode *detectionpoint) {
	ListNode *cursor = head;
	while(cursor != detectionpoint) {
		cursor = cursor->next;
		detectionpoint = detectionpoint->next;
	}
	return cursor;
}

int main() {
	ListNode *head = nullptr, *detectionpoint = nullptr;
	createList(&head);
	bool doesCycleExists = findCycle(head, &detectionpoint);
	if(doesCycleExists) {
		cout<<"Value is: "<<findHit(head, detectionpoint)->val;
	}
	else
		cout<<"nullptr"<<endl;
	return 0;
}
