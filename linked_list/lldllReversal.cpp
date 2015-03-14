//this program will reverse the doubly linked list using recursion

#include<iostream>
#include<cstdlib>


using namespace  std;

struct linked;

void insertElement();
void deleteElement();
void reverseList(linked*, linked*);
void printElement();
void recurCaller();


struct linked{
int node;
struct linked *next;
struct linked *prev;
};

struct linked *head ;

int main()
{
int input;

do{
cout<<"=====================\n"<<"1. Insert"<<endl<<"2. Reverse"<<endl<<"3 Delete"<<endl<<"4. print"<<endl<<"-1. Exit"<<"\n===================="<<endl;
cout<<"Enter the input\n";
cin>>input;

switch(input){
case 1:
insertElement();
break;
case 2:
recurCaller();
break;
case 3:
deleteElement();
break;
case 4:
printElement();
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


void insertElement()
{
		int element;
		cout<<"Enter the element you want to insert\n";
		cin>>element;
		struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
		newNode->next = NULL;
		newNode->prev = NULL;
		newNode->node = element;
		//New node created above

		if(head == NULL)
		{
				head = newNode;
				newNode->prev = head;
				return;
		}
		else
		{
		struct linked* dummy = head;
		while(dummy->next != NULL)
			dummy = dummy->next;

		//insert now
		dummy->next = newNode;
		newNode->prev = dummy;
		}
}

void deleteElement()
{
		int element;
		struct linked *dummy = head;
		cout<<"Enter the element you want to delete\n";
		cin>>element;

		struct linked *oldCur;
		while(dummy != NULL)
		{
				if(dummy->node == element){
				oldCur->next = dummy->next;
				oldCur->next->prev = oldCur;
				free(dummy);
				cout<<"Element deleted successfully\n";
				return;
				}
				oldCur = dummy;
				dummy = dummy->next;
		}
		cout<<"Sorry Element not found"<<endl;
}


void printElement(){

struct linked *dummy = head;

while(dummy != NULL){
cout<<dummy->node<<"-> ";
dummy = dummy->next;
}
cout<<"NULL"<<endl;
} 


void recurCaller(){
reverseList(NULL, head);
cout<<"Linked list reversed\n";

}

void reverseList(linked* oldNode, linked* node)
{
		if(node == NULL){
		head = oldNode;
		oldNode->prev = head;
		return;
		}
		//Recursive call
		reverseList(node, node->next);
		node->next = oldNode;
		if(oldNode != NULL)
			oldNode->prev = node;
}






