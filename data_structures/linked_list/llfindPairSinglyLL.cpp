/* Program to find pair in sorted list which sums
 * up to a given number.
 */
#include <bits/stdc++.h> 
using namespace std; 

class Node { 
public: 
  Node(int x) {
    data=x;
    next=NULL;
  }
	int data; 
	Node* next; 
}; 

typedef Node * Node_ptr;

// This function prints contents of linked list 
// starting from the given node 
void printList(Node_ptr n) 
{ 
	while (n != NULL) { 
		cout << n->data << " "; 
		n = n->next; 
	} 
  cout<<endl;
} 

void find_pair(Node_ptr n_ptr, Node_ptr* fwd_ptr, int sum)
{
  if(!n_ptr || !fwd_ptr || !(*fwd_ptr)){
    return;
  }
  find_pair(n_ptr->next, fwd_ptr, sum);

  if((*fwd_ptr)->data < n_ptr->data) {
    if((*fwd_ptr)->data + n_ptr->data == sum) {
      cout<<"("<<(*fwd_ptr)->data<<", "<<n_ptr->data<<")"<<endl;
      (*fwd_ptr) = (*fwd_ptr)->next;
    }
    else if((*fwd_ptr)->data + n_ptr->data < sum) {
      while((*fwd_ptr)->data + n_ptr->data < sum)
        (*fwd_ptr) = (*fwd_ptr)->next;
      if((*fwd_ptr)->data + n_ptr->data == sum)
        cout<<"("<<(*fwd_ptr)->data<<", "<<n_ptr->data<<")"<<endl;
    }
  }
}

/* Driver code */
int main() 
{ 
	Node_ptr one, two, three, four, five, six, seven, eight; 

  one = new Node(3); 
	two = new Node(5); 
	three = new Node(7); 
	four = new Node(11); 
	five = new Node(15); 
	six = new Node(22); 
	seven = new Node(24); 
  eight = new Node(30); 

	one->next = two;
  two->next=three;
  three->next=four;
  four->next=five;
  five->next=six;
  six->next=seven;
  seven->next=eight;
  
  find_pair(one, &one, 29);

	return 0; 
}