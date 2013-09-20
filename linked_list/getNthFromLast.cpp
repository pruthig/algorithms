#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement();
void getNthFromLastElement();
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
cout<<"=====================\n"<<"1. Insert"<<endl<<"2. Access Nth index"<<endl<<"3. print"<<endl<<"-1. Exit"<<"\n===================="<<endl;
cout<<"Enter the input\n";
cin>>input;

switch(input){
case 1:
insertElement();
break;
case 2:
getNthFromLastElement();
break;
case 3:
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

void getNthFromLastElement(){
int element;
int count = 0;
struct linked *dummy = head->next;
cout<<"Enter the index you want to access\n";
cin>>element;

if(element<=0){
cout<<"Invalid index entered\n";
return;
}

struct linked *counter = head;
int actualCount = 0;
while(counter->next != NULL){
	actualCount++;
	counter = counter->next;
}

while(dummy != NULL){
count++;
if(count == actualCount - elementi + 1){
cout<<"Element found is : "<<dummy->node<<endl;
return;
}
dummy = dummy->next;
}
cout<<"Index overflow"<<endl;
}


void printElement(){

struct linked *dummy = head->next;

while(dummy != NULL){
cout<<dummy->node<<"-> ";
dummy = dummy->next;
}
cout<<"NULL"<<endl;
} 










