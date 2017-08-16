// Check if a list is palindrome
#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
};

void createList(ListNode **head) {
	(*head) = new ListNode(1);
	(*head)->next = new ListNode(2);
	(*head)->next->next = new ListNode(3);
	(*head)->next->next->next = new ListNode(2);
	(*head)->next->next->next->next = new ListNode(1);
}

 //Find if a list is palindrome or not
bool is_palindrome_list(ListNode* current, ListNode** late_head) {
	if(current == nullptr) {
		return true;
	}
	bool isPalin = is_palindrome_list(current->next, late_head);
	if(!isPalin)
		return false;
	if((*late_head)->val != current->val)
		return false;
	else {
		(*late_head) = (*late_head)->next;
		return true;
	}
}

int main() {
	ListNode *head = nullptr;
	createList(&head);
	ListNode *late_head = head;
	bool isPalin = is_palindrome_list(head, &late_head);
	cout<<"Is list Palindrome: "<<std::boolalpha<<isPalin<<endl;
	return 0;
}
