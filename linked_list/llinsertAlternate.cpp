//This program merges 2 linkes list at alternate positions

#include<iostream>
#include<cstdlib>


using namespace  std;

void insertElement(int s);
void printElement(int s);
void mergeList();

struct linked
{
		int node;
		struct linked *next;
};

struct linked *head1 = NULL;
struct linked *head2 = NULL;

int main()
{
		int input;

		do
		{
				
				cout<<"1. Insert to 1st"<<endl;
				cout<<"2. Insert to 2nd"<<endl;
				cout<<"4. print 1:"<<endl;
				cout<<"5. print 2:"<<endl;
				cout<<"6. print Sum"<<endl;
				cout<<"-1. Exit"<<endl;
				cout<<"Enter the input\n";
				cin>>input;

				switch(input)
				{
						case 1:
							insertElement(1);
							break;
						case 2:
							insertElement(2);
							break;
						case 4:
							printElement(1);
							break;
						case 5:
							printElement(2);
							break;
						case 6:		
							mergeList();
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


void mergeList()
{
		struct linked* nxt1 = NULL;
		struct linked* nxt2 = NULL;
		
		struct linked* dummy1 = NULL;
		struct linked* dummy2 = NULL;

		dummy1 = head1;
		dummy2 = head2;
		nxt1 = dummy1->next;
		nxt2 = dummy2->next;
		while(nxt1 && nxt2)
		{
			dummy1->next = dummy2;
			dummy2->next = nxt1;
			dummy1 = nxt1;
			if(dummy1)
				nxt1 = dummy1->next;
			dummy2 = nxt2;
			if(dummy2)
				nxt2 = dummy2->next;
		}

	    dummy1->next = dummy2;
}

void printElement(int c)
{
	struct linked* dummy = NULL;
	if(c == 2)
		dummy= head2;
	else
		dummy = head1;

	while(dummy != NULL)
	{
		cout<<dummy->node<<"-> ";
		dummy = dummy->next;
	}
	cout<<"NULL"<<endl;
} 


