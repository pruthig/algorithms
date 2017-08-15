// insertion sort
// this program runs insertion sort on linked list
#include<iostream>
#include<algorithm>

using namespace std;

struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
};

void printList(ListNode* head) {
	cout<<"Sorted list is: \n";
	while(head) {
		cout<<head->val<<"->";
		head = head->next;
	}
	cout<<"nullptr";
}


void createList(ListNode **head) {
	(*head) = new ListNode(9);
	(*head)->next = new ListNode(7);
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

// Assumed k >= 0, will deal in terms of indices only
int& getKthNode(ListNode *head, int k) {
	while(k-- && head) {
		head = head->next;
	}
	return head->val;
}

ListNode *insertion_sort(ListNode *head) {
	
	int size = get_size_of_list(head);
	
	for(int i = 1; i < size; ++i) {
		if(getKthNode(head, i) < getKthNode(head, i-1)) {
			int j = i;
			while(j > 0 && getKthNode(head, j) < getKthNode(head, j-1)) {
				swap(getKthNode(head, j-1) , getKthNode(head, j));
				--j;
			}
		}
	}
	return head;
}


int main() {
	ListNode *head = nullptr;
	int elem = 0;
	createList(&head);
	ListNode *new_list = insertion_sort(head);
	printList(new_list);
	return 0;
}
