//linked list palindrome check with recursion
//...
//..
//.
#include<iostream>
#include<cstdlib>
#include<stack>


using namespace  std;

void insertElement();
void checkPalindrome();
void printElement();
bool checkWithLoop(struct linked* h, int p, int lvl);

struct linked{
int node;
struct linked *next;
};

struct linked *head = NULL;
static struct linked* second = NULL;
int main()
{
int input;

do{
cout<<"=====================\n"<<"1. Insert"<<endl<<"2. Check Palindrome"<<endl<<"3. print"<<endl<<"-1. Exit"<<"\n===================="<<endl;
cout<<"Enter the input\n";
cin>>input;

switch(input){
case 1:
insertElement();
break;
case 2:
checkPalindrome();
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
cout<<"Enter the element you want to insert\n";
cin>>element;

struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
newNode->next = NULL;
newNode->node = element;

if(head == NULL){
	head = newNode;
	return;
}

	
struct linked* dummy = head;
while(dummy->next != NULL)
	dummy = dummy->next;


//insert now
dummy->next = newNode;
}



void checkPalindrome(){
struct linked *dummy = head;

int count = 0;
for(;dummy != NULL; count++, dummy=dummy->next);
cout<<"Number of element in the list "<<count<<endl;

dummy = head;

//dummy points to middle after this operation
for(int i=0; i <= count/2 - 1 ;i++, dummy = dummy->next);

second = dummy->next;

dummy = head;

if (checkWithLoop(dummy, count, 0))
	cout<<"List is Palindrome\n";
else
	cout<<"list is not palindrome\n";

}

void printElement(){

struct linked *dummy = head;

while(dummy != NULL){
cout<<dummy->node<<"-> ";
dummy = dummy->next;
}
cout<<"NULL"<<endl;
} 


bool checkWithLoop(struct linked* h, int p, int lvl){
if(lvl == p/2)
	return true;
else
	checkWithLoop(h->next, p, lvl+1);



//wont go down now
if(second->node != h->node)
	return false;

second = second->next;  //compare it with current node;
	return true;
}












