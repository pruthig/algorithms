#include<iostream>

using namespace std;

// Definition for singly-linked list.
struct ListNode {
  int value;
  ListNode *next;
  ListNode(int x) : value(x), next(NULL) {}
};
 
ListNode* getIntersectionNode(ListNode* A, ListNode* B) {
    // Do not write main() function.
    // Do not read input, instead use the arguments to the function.
    // Do not print the output, instead return values as specified
    // Still have a doubt. Checkout www.interviewbit.com/pages/sample_codes/ for more details
    int size_A = 0, size_B = 0, diff = 0;
    ListNode *tmp = A;
    int count = 0;
    ListNode *tmp_A = A, *tmp_B = B;
    
    // Calculate size of A
    while(tmp) {
        ++count;
        tmp = tmp->next;
    }
        
    size_A = count;
    
    // Calculate size of B
    count = 0;
    tmp = B;
    while(tmp) {
        ++count;
        tmp = tmp->next;
    }
    size_B = count;
    
    //if(size_A == 0 || size_B == 0)
        //return nullptr;
    
    diff = abs(size_A - size_B);
    //cout<<"Size_A and B: "<<size_A<<" "<<size_B<<endl;
    //cout<<"Diff is: "<<endl;
    size_B = count;
    if(size_A > size_B) {
        while(diff && tmp_A) {
            tmp_A = tmp_A->next;
            --diff;
        }
    }
    else if(size_B > size_A) {
        while(diff && tmp_B) {
            tmp_B = tmp_B->next;
            --diff;
        }
    }
    else {
        /* do nothing */
    }
    // Now traverse till common is encountered
    while(tmp_A && tmp_B) {
        if(tmp_A == tmp_B)
            return tmp_A;
        tmp_A = tmp_A->next;
        tmp_B = tmp_B->next;
    }
    return nullptr;
}

void printList(ListNode* head) {
	while(head) {
		cout<<head->value<<"->";
		head = head->next;
	}
	cout<<"nullptr";
}

void createList_A(ListNode *(*head_A), ListNode *head_C) {
	(*head_A) = new ListNode(1);
	(*head_A)->next = new ListNode(5);
	(*head_A)->next->next = new ListNode(6);
	(*head_A)->next->next->next = new ListNode(7);
	(*head_A)->next->next->next->next = new ListNode(9);
	(*head_A)->next->next->next->next->next = head_C; 
}

void createList_B(ListNode *(*head_B), ListNode *head_C) {
	(*head_B) = new ListNode(-1);
	(*head_B)->next = new ListNode(0);
	(*head_B)->next->next = new ListNode(1);
	(*head_B)->next->next->next = new ListNode(2);
	(*head_B)->next->next->next->next = new ListNode(9);
	(*head_B)->next->next->next->next->next = head_C; 

}

void createList_Intersection(ListNode *(*head_C)) {
	(*head_C) = new ListNode(6);
	(*head_C)->next = new ListNode(7);
	(*head_C)->next->next = new ListNode(9);	
}

int main() {
	ListNode (*head_A) = nullptr, (*head_B) = nullptr, (*head_C) = nullptr;
	createList_Intersection(&head_C);
	createList_A(&head_A, head_C);
	createList_B(&head_B, head_C);
	
	ListNode *intersect = getIntersectionNode(head_A, head_B);
	if(intersect)
		printList(intersect);
	return 0;
}
