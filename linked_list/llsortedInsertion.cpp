//two approached for linked list
//operations...
//1. Single traversing
//2. maintain 2 pointers to get what you want..

#include<iostream>
#include<cstdlib>


using namespace std;

void insertElement();
void printElement();
bool checkForSorted();


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
cout<<"=====================\n"<<"1. Insert"<<endl<<"2. print"<<endl<<"-1. Exit"<<"\n===================="<<endl;
cout<<"Enter the input\n";
cin>>input;

switch(input){
case 1:
insertElement();
break;
case 2:
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
if(checkForSorted() == false)
{
	cout<<"List not sorted\n";
	return;
}
struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
newNode->next = NULL;
newNode->node = element;

if(dummy->next == NULL){
dummy->next = newNode;
return;
}

while(dummy != NULL){
if(dummy->next->node > element){
newNode->next = dummy->next;
dummy->next = newNode;
return;
}

dummy = dummy->next;

}
}//End of function

bool checkForSorted(){
struct linked *dummy = head->next;
struct linked *prev = dummy;

while(dummy != NULL){
if(dummy->node < prev->node){
return false;
}
prev = dummy;
dummy = dummy->next;
}
return true;
}


void printElement(){

struct linked *dummy = head->next;

while(dummy != NULL){
cout<<dummy->node<<"-> ";
dummy = dummy->next;
}
cout<<"NULL"<<endl;
} 










