//two approached for linked list
//reveral...
//1. Iteration and 
//2. recursion

#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement();
void deleteElement();
void reverseList(struct linked*, struct linked*);
void printElement();


struct linked{
int node;
struct linked *next;
};

struct linked *head = (struct linked*)malloc(sizeof(struct linked));

int main()
{
int input;
head->next = NULL;
head->node = 0;

do{
cout<<"=====================\n"<<"1. Insert"<<endl<<"2. reverse"<<endl<<"3. delete "<<endl<<"4. print"<<endl<<"-1. Exit"<<"\n===================="<<endl;
cout<<"Enter the input\n";
cin>>input;

switch(input){
case 1:
insertElement();
break;
case 2:
reverseList(NULL, head);
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


void insertElement(){
int element;
struct linked* dummy = head;
cout<<"Enter the element you want to insert\n";
cin>>element;
while(dummy->next != NULL)
dummy = dummy->next;

struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
newNode->next = NULL;
newNode->node = element;

//insert now
dummy->next = newNode;
}

void deleteElement(){
int element;
struct linked *dummy = head;
cout<<"Enter the element you want to delete\n";
cin>>element;

struct linked *prev;
while(dummy != NULL){
if(dummy->node == element){
prev->next = dummy->next;
free(dummy);
cout<<"Element deleted successfully\n";
return;
}
prev = dummy;
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


void reverseList(struct linked *prev, struct linked *cur){
if(cur == NULL){
	head = prev;
	return;
}

reverseList(cur, cur->next);

cur->next = prev;
}







