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

void deleteDuplicate(Node_ptr prev, Node_ptr cur) {
  if(!prev || !cur)
    return;
    
  if(prev->data == cur->data) {
    Node_ptr tmp = cur->next;
    delete cur;
    prev->next = tmp;
    deleteDuplicate(prev, tmp);
  }
  else
    deleteDuplicate(cur, cur->next);
}
      
    
// when talking about rotation by blocks think about tail recursion
// Driver code 
int main() 
{ 
	Node_ptr one, two, three, four, five, six, seven, eight, nine; 
  one = new Node(12); 
	two = new Node(12); 
	three = new Node(10); 
	four = new Node(11); 
	five = new Node(11); 
	six = new Node(11); 
	seven = new Node(11); 
  eight = new Node(3); 
  nine = new Node(3); 

	one->next = two;
  two->next=three;
  three->next=four;
  four->next=five;
  five->next=six;
  six->next=seven;
  seven->next=eight;
  eight->next = nine;
  cout<<"Before modification: "<<endl;
  printList(one); 
  deleteDuplicate(one, two);
  cout<<"After modification: "<<endl;
  printList(one); 

	return 0; 
} 
