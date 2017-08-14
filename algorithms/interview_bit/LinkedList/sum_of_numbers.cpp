#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(NULL) {}
};
 
ListNode* addTwoNumbers(ListNode* A, ListNode* B) {
	
    
    ListNode *a_node = A, *b_node = B;
    ListNode *new_node = nullptr, *curr = nullptr;
    int carry = 0;
    
    while(a_node || b_node) {
    	int sum = carry;
		if(a_node)
			sum += a_node->val;
		if(b_node) 
			sum += b_node->val;
			
    	if(sum >= 10) {
    		carry = 1;
    		sum = sum%10;
    	}
    	else
    		carry = 0;
    		
    	if(!new_node) {
    		new_node = new ListNode(sum);
    		curr = new_node;
    	}
    	else {
    		new_node->next = new ListNode(sum);
    		new_node = new_node->next;
    	}
    	if(a_node)
    		a_node = a_node->next;
    	if(b_node)
    		b_node = b_node->next;
    }
	if(carry)
		new_node->next = new ListNode(1);
    return curr;
    
}

void printList(ListNode* head) {
	while(head) {
		cout<<head->val<<"->";
		head = head->next;
	}
	cout<<"nullptr";
}

void createList_A(ListNode *(*head_A)) {
	(*head_A) = new ListNode(0);
	(*head_A)->next = new ListNode(9);
	(*head_A)->next->next = new ListNode(9);
	(*head_A)->next->next->next = new ListNode(9);
}

void createList_B(ListNode *(*head_B)) {
	(*head_B) = new ListNode(0);
	(*head_B)->next = new ListNode(1);
	(*head_B)->next->next = new ListNode(9);
	(*head_B)->next->next->next = new ListNode(9);
	(*head_B)->next->next->next->next = new ListNode(9);

}


int main() {
	ListNode *head_A = nullptr, *head_B = nullptr;
	
	createList_A(&head_A);
	createList_B(&head_B);
	
	ListNode *sum = addTwoNumbers(head_A, head_B);
	if(sum)
		printList(sum);
	return 0;
}
