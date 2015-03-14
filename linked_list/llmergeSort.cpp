//Merge Sort for Linked linked..

#include<iostream>
#include<cstdlib>


using namespace std;

typedef struct linked{
int node;
struct linked *next;
}linked;


void insertElement();
void printElement();
 struct linked* getlinkedPointer(int index);
struct linked* createlinked(int size);
void mergeSort(int l, int r);
void merge(int l, int mid, int r);
int size();

struct linked *head = NULL;

int main()
{
	int input;

	do{
		cout<<"=====================\n"<<"1. Insert"<<endl<<"2. Merge"<<endl<<"3. Print"<<endl<<"-1. Exit"<<"\n===================="<<endl;
		cout<<"Enter the input\n";
		cin>>input;

		switch(input)
		{
			case 1:
			insertElement();
			break;
			case 2:
			mergeSort(0, size()-1);
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

linked* getlinkedPointer(int index)
{
	
        struct linked* d = head;
        while(index != 0){
                d = d->next;
                index--;
        }
        return d;
}

void insertElement()
{
	int element;
	struct linked* dummy = head;
	cout<<"Enter the element you want to insert\n";
	cin>>element;

	struct linked *newNode = (struct linked*)malloc(sizeof(struct linked));
	newNode->next = NULL;
	newNode->node = element;
	if(head == NULL)
	{
		head = newNode;
		return;
	}
	if(dummy->next != NULL)
	while(dummy->next != NULL)
		dummy = (dummy)->next;

	dummy->next = newNode;

}

int size()
{
	int count = 0;
	struct linked *dummy = head;
	while(dummy != NULL){
		dummy = dummy->next;
		count++;
	}

	return count;
}

struct linked* createlinked(int size)
{
	if(size <= 0)
		return NULL;

	struct linked *h = (struct linked*)malloc(sizeof(struct linked*));
		
	struct linked *dummy = h;

	while(size != 1){
	dummy->next = (struct linked*)malloc(sizeof(struct linked*));
	dummy->next->node = 0;
	dummy->next->next = NULL;
	dummy = dummy->next;
	--size;
	}
	return h;
}

void mergeSort(int l, int r)
{
	if(l ==r)
		return;
	int mid = (l+r)/2;

	mergeSort(l, mid);
	mergeSort(mid+1, r);

	merge(l, mid, r);
}

void merge(int l, int mid, int r)
{
	struct linked *p1 = createlinked(mid-l+1);
	struct linked *p2 = createlinked(r-mid);
	
	struct linked* p_d = p1;
	for(int i = l; i <= mid; i++){

		p_d->node = getlinkedPointer(i)->node;
		p_d = p_d->next;
	}

	p_d = p2;

	for(int i = mid+1; i <= r; i++){
		p_d->node = getlinkedPointer(i)->node;
		p_d = p_d->next;
	}

	int i = l;
	while(p1 != NULL && p2 != NULL && i<=r)
	{
		if(p1->node < p2->node){
			getlinkedPointer(i)->node = p1->node;
			p1 = p1->next;
			
		}
		else{
			getlinkedPointer(i)->node = p2->node;
			p2 = p2->next;
		}
		i++;
	}

	if(p1 != NULL && p2 == NULL && i<=r){
		while(p1 != NULL){
			getlinkedPointer(i)->node = p1->node;
			p1 = p1->next;
			++i;
		}
	}

	if(p2 != NULL && p1 == NULL && i<=r){
		while(p2 != NULL){
			getlinkedPointer(i)->node = p2->node;
			p2 = p2->next;
			++i;
		}
	}
		
}

void printElement()
{

	struct linked *dummy = head;

	while(dummy != NULL){
	cout<<dummy->node<<"-> ";
	dummy = dummy->next;
	}
	cout<<"NULL"<<endl;
} 









