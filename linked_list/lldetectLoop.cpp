//two approached for linked list
//operations...
//1. Single traversing
//2. maintain 2 pointers to get what you want..

#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement(int n);


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

for(int i=1;i<=5;i++){
insertElement(i);
}

//Take two pointers ... one will move by 1 pos while another by 2...
struct linked *singleJump = head;
struct linked *doubleJump = head;

while(doubleJump->next != NULL){
singleJump = singleJump->next;
doubleJump = doubleJump->next->next;

if(singleJump == doubleJump){
cout<<endl<<"A loop was detected in linked list"<<endl;
return 0;
}
}//end while..

cout<<"there is no loop in linkedlist\n";
return 0;
}


void insertElement(int n){
int element;
struct linked* dummy = head;
while(dummy->next != NULL)
dummy = dummy->next;

struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));

if(n==5){
newNode->next = head;
}
else{
newNode->next = NULL;
}

newNode->node = element;

//insert now
dummy->next = newNode;
}








