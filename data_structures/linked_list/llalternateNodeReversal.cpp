//two approached for linked list
//reveral...
//1. Iteration and 
//2. recursion

#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement();
void alternateReverse();
void printList();
struct linked* reverseList(struct linked* startNode);


struct linked{
int node;
struct linked *next;
};

struct linked *head = NULL;
struct linked* new_head;
struct linked* new_cur;
struct linked* old_cur;
	
int main()
{
int input;

do{
cout<<"=====================\n"<<"1. Insert"<<endl<<"2. reverse"<<endl<<"3. print"<<endl<<"-1. Exit"<<"\n===================="<<endl;
cout<<"Enter the input\n";
cin>>input;

switch(input){
case 1:
insertElement();
break;
case 2:
alternateReverse();
break;
case 3:
printList();
break;
case -1:
exit(0);
break;
default:
cout<<"Invalid input\n";
break;
}

}while(1);

return 0;
}


void insertElement(){
int element;
struct linked* dummy = head;
cout<<"Enter the element you want to insert\n";
cin>>element;

struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
newNode->next = NULL;
newNode->node = element;

if(head == NULL){
head = newNode;
return;
}

while(dummy->next != NULL)
	dummy = dummy->next;


dummy->next = newNode;
}

void printList(){

struct linked *dummy = head;

while(dummy != NULL){
cout<<dummy->node<<"-> ";
dummy = dummy->next;
}
cout<<"NULL"<<endl;
} 


struct linked* reverseList(struct linked* startNode){
  struct linked* new_root = 0;
  while (startNode) {
    struct linked* next = startNode->next;
    startNode->next = new_root;
    new_root = startNode;
    startNode = next;
  }
  return new_root;
}




void alternateReverse(){
	//creating alternate nodes for the program

	struct linked* new_head;	
	old_cur = head;
	new_head = old_cur->next;
	new_cur = new_head;

	while(new_cur != NULL && old_cur != NULL && new_cur->next != NULL && old_cur->next != NULL){
		old_cur->next =  new_cur->next;
		old_cur = old_cur->next;
		new_cur->next = old_cur->next;
		new_cur = new_cur->next;
	}
	//reverse the list call the function with passing new_head pointer;
	if(new_cur != NULL)
		new_cur->next = NULL;

	new_head = reverseList(new_head);
	//now append that list to the old list
	old_cur->next = new_head;
	//alternate list reversed now, let us print the same
	printList();
}


