// Subtract and store result of (last-first) to first n/2 nodes
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
//	(*head)->next = new ListNode(2);
//	(*head)->next->next = new ListNode(3);
//	(*head)->next->next->next = new ListNode(4);
//	(*head)->next->next->next->next = new ListNode(5);
}

int get_size_of_list(ListNode * cur) {
    int count = 0;
    while(cur) {
        ++count;
        cur = cur->next;
    }   
    return count;
}

void printList(ListNode* head) {
    cout<<endl;
    while(head) {
        cout<<head->val<<"->";
        head = head->next;
    }
    cout<<"nullptr";
}

 //Find if a list is palindrome or not
void alternate_subtract(ListNode* current, ListNode** late_head, int* count, int size) {
	if(current == nullptr) {
		return;
	}
	alternate_subtract(current->next, late_head, count, size);

    if(*count == size/2)
        return;
	(*late_head)->val = current->val - (*late_head)->val;
    *count = *count+1;
	(*late_head) = (*late_head)->next;
}

int main() {
	ListNode *head = nullptr;
	createList(&head);
    int size = get_size_of_list(head), count = 0;
	ListNode *late_head = head;
	alternate_subtract(head, &late_head, &count, size);
    cout<<endl;
    printList(head);
	return 0;
}
