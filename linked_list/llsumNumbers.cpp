//Linked list to add sum of numbers represented by LL

#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement(int s);
void deleteElement();
void printElement(int s);
void createNode(int s);
void findSum();

struct linked
{
		int node;
		struct linked *next;
};

struct linked *head = NULL;
struct linked *head1 = NULL;
struct linked *head2 = NULL;
int main()
{
		int input;

		do
		{
				
				cout<<"=====================\n"<<"1. Insert to 1st"<<endl<<"2. Insert to 2nd"<<endl<<"3. delete"<<endl<<"4. print 1:"<<endl<<"5. print 2:"<<endl<<"6. print Sum"<<endl<<"-1. Exit"<<"\n===================="<<endl;
				cout<<"Enter the input\n";
				cin>>input;

				switch(input){
				case 1:
				insertElement(1);
				break;
				case 2:
				insertElement(2);
				break;
				case 3:
				deleteElement();
				break;
				case 4:
				printElement(1);
				break;
				case 5:
				printElement(2);
				break;
				case 6:		
				findSum();
				printElement(0);
				break;
		
				case -1:
				exit(0);
				default:
				cout<<"Invalid input\n";
				break;
				}

		}while(1);

		return 0;
}

void createNode(int sum){
		cout<<"Sum to process :"<<sum<<endl;
		struct linked *tmp = head;
		struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));

		newNode->node = sum;
		newNode->next = tmp;
		head = newNode;
}


void insertElement(int c)
{
		int element;
	
		if(c < 1 || c > 2)
			return;

		if(c == 1)
			struct linked* dummy = head1;
		else
			struct linked* dummy = head2;

		cout<<"Enter the element you want to insert\n";
		cin>>element;

		struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
		newNode->node = element;

		//head condition...
		struct linked *tmp;
		if( c == 1)
			tmp = head1;
		else
			tmp = head2;

		newNode->next = tmp;
		if(c == 1)
			head1 = newNode;
		else	
			head2 = newNode;
		cout<<"Hitting here\n";

		printElement(1);

}

void findSum()
{
		int  carry = 0;
		struct linked *d1 = head1;
		struct linked *d2 = head2;

		while(d1 || d2)
		{
				int s1 = d1?(d1->node):0;
				int s2 = d2?(d2->node):0;
				carry = ((s1+s2+carry)/10)?1:0;
				createNode( (s1+s2+carry)%10);

				if(d1)
					d1 = d1->next;
				if(d2)
					d2 = d2->next;
		}

		return;
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


void printElement(int c)
{
	struct linked* dummy = NULL;
	if( c == 1)
		dummy = head1;
	else if(c == 2)
		dummy = head2;
	else
		dummy = head;

	while(dummy != NULL)
	{
		cout<<dummy->node<<"-> ";
		dummy = dummy->next;
	}
	cout<<"NULL"<<endl;
} 


