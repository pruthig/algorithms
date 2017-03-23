//two ways two find is list is palindrome
//1. use stack or
//2  recursion..
//below is the implementation

#include<iostream>
#include<cstdlib>
#include<stack>


using namespace  std;

void insertElement();
void checkPalindrome();
void printElement();


struct linked{
char node;
struct linked *next;
};

stack<char> mainStack;
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

void checkPalindrome(){
struct linked *dummy = head->next;
int count = 0;
for(;dummy != NULL; count++, dummy=dummy->next);
cout<<"Number of element in the list "<<count<<endl;
//Push some elements on the stack..after resetting the dummy.
dummy = head->next;

for(int i=0; i <= count/2 -1 ;i++, dummy = dummy->next)
mainStack.push(dummy->node);

struct linked *middle = dummy;

//if the count is even...pop and compare
if(count%2 == 0){
for(int i=count/2; i<=count-1; i++, middle = middle->next){

if(mainStack.top() == middle->node){
mainStack.pop();
continue;
}
else{
cout<<"Word is not palindrome"<<endl;
return;
}  

}//End for loop

cout<<"Word is palindrome"<<endl;
return;
}//end if condition for even count

else{
//reset the dummy
middle = dummy;
middle = middle->next;

for(int i=count/2 + 1; i<=count-1; i++, middle = middle->next){

if(mainStack.top() == middle->node){
mainStack.pop();
continue;
}
else{
cout<<"Word is not palindrome"<<endl;
return;
}

}//End for loop
cout<<"Word is palindrome"<<endl;
return;
}//End else condition....


}

void printElement(){

struct linked *dummy = head->next;

while(dummy != NULL){
cout<<dummy->node<<"-> ";
dummy = dummy->next;
}
cout<<"NULL"<<endl;
} 









