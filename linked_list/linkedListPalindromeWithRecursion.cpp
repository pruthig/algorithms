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
char node;
struct linked *next;
};

struct linked *head = (struct linked*)malloc(sizeof(struct linked));

int main()
{
int input;
head->next = NULL;

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
char element;
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


static struct linked* second;

void checkPalindrome(){
struct linked *dummy = head->next;

int count = 0;
for(;dummy != NULL; count++, dummy=dummy->next);
cout<<"Number of element in the list "<<count<<endl;

dummy = head->next;

//dummy now points to middle
for(int i=0; i <= count/2 ;i++, dummy = dummy->next);

if(count%2 != 0)
	second = dummy->next;

	second = dummy;
dummy = head->next;

if (checkWithLoop(dummy, count, 0))
	cout<<"List is Palindrome\n";
else
	cout<<"list is not palindrome\n";

}

void printElement(){

struct linked *dummy = head->next;

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



if(second->node != h->node)
	return false;

second = second->next;  //compare it with current node;
	return true;
}












