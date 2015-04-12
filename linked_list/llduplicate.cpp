//Program to remove duplicates from a linked list ...
// in this case when we encounter any duplicate character wer remove that character frm the list
#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement();
void deleteElement();
void printElement();
void removeDuplicate();

struct linked{
int node;
struct linked *next;
};

struct linked *head = NULL;

int main()
{
int input;

do{
cout<<"=====================\n"<<"1. Insert"<<endl<<"2. delete"<<endl<<"3. print"<<endl<<"4. Remove Duplicate"<<"-1. Exit"<<"\n===================="<<endl;
cout<<"Enter the input\n";
cin>>input;

switch(input){
case 1:
insertElement();
break;
case 2:
deleteElement();
break;
case 3:
printElement();
break;
case 4:
removeDuplicate();
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


//insert now
dummy->next = newNode;
}

void deleteElement()
{
	int element;
	struct linked *dummy = head;
	if(dummy == NULL)
	{
		return;
	}

	struct linked* tmp;
	cout<<"Enter the element you want to delete\n";
	cin>>element;

	//head case;
	if(dummy->node == element)
	{
		tmp = head;
		head = head->next;
		free(tmp);
		return;
	}

	while(dummy != NULL && dummy->next != NULL)
	{
		if(dummy->next->node == element)
		{
			tmp = dummy->next;
			dummy->next = tmp->next;
			free(tmp);
			cout<<"Element deleted successfully\n";
			return;

		}
	}
	cout<<"Sorry Element not found"<<endl;
}


void printElement()
{
	struct linked *dummy = head;

	while(dummy != NULL)
	{
		cout<<dummy->node<<"-> ";
		dummy = dummy->next;
	}
	cout<<"NULL"<<endl;
} 




void removeDuplicate()
{

	struct linked *dummy = head;
	int tmp;

	while(dummy != NULL)
	{
		tmp = dummy->node;
		while(dummy != NULL && dummy->next != NULL && dummy->next->node == tmp)
		{
			struct linked* t = dummy->next;
			dummy->next = t->next;
			free(t);
		}
		dummy = dummy->next;
	}

}








