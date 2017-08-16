// This program will partition the list in 2 halves such that elements less than given element comes
// before the element that are larger than given element
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

void createList(ListNode **head) {
	(*head) = new ListNode(3);
	(*head)->next = new ListNode(7);
	/*(*head)->next->next = new ListNode(12);
	(*head)->next->next->next = new ListNode(6);
	(*head)->next->next->next->next = new ListNode(8);
	(*head)->next->next->next->next->next = new ListNode(21);
	(*head)->next->next->next->next->next->next = new ListNode(3);
	(*head)->next->next->next->next->next->next->next = new ListNode(6);
	(*head)->next->next->next->next->next->next->next->next = new ListNode(10);
	(*head)->next->next->next->next->next->next->next->next->next = new ListNode(4);
	*/
}

void segregate(ListNode **head, int element) {
	ListNode *larger = nullptr, *smaller = nullptr, *larger_head = nullptr, *smaller_head = nullptr;
	ListNode *cursor = *head;
	while(cursor) {
		if(cursor->value < element) {
			if(!smaller)
				smaller_head = cursor;
			if(smaller)
				smaller->next = cursor;
			smaller = cursor;
			cout<<"Added in Smaller: "<<smaller->value<<endl;
		}
		else {
			if(!larger)
				larger_head = cursor;
			if(larger)
				larger->next = cursor;
			larger = cursor;
			cout<<"Added in larger: "<<larger->value<<endl;
		}
		cursor = cursor->next;
	}
	if(larger)
		larger->next = nullptr;
	if(smaller)
		smaller->next = nullptr;
	// End of loop
	if(!smaller_head)
		*head = larger_head;
	else {
		*head = smaller_head;
		if(smaller)
			smaller->next = larger_head;
	}
}

int main() {
	// Create list
	ListNode *head = nullptr;
	createList(&head);
	int element;
	cout<<"Enter the element\n";
	cin>>element;
	segregate(&head, element);
	printList(head);
	return 0;
}
