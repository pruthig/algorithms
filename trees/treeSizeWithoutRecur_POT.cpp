#include<iostream>
#include<stack>


using namespace std;

typedef struct treeStruct{
int element;
struct treeStruct *left;
struct treeStruct *right;
}treeStruct;


struct treeStruct* newNode(int data)
{
struct treeStruct *newElement = new(struct treeStruct);
newElement->left = NULL;
newElement->right = NULL;
newElement->element = data;
return newElement;
}

struct treeStruct* treeCreator(){
//create the main pointer
struct treeStruct *mainPtr;
mainPtr = newNode(10);
mainPtr->left = newNode(6);
mainPtr->right = newNode(12);

mainPtr->left->left = newNode(2);
mainPtr->left->right = newNode(9);

mainPtr->right->left = newNode(11);
mainPtr->right->right = newNode(18);

mainPtr->right->right->left = newNode(16);
mainPtr->right->right->right = newNode(20);

return mainPtr;
}

int sizeWithoutRecur(struct treeStruct *ptr)
{
	stack<treeStruct *> s;
	int size = 0;

	if(ptr == NULL)
		return size;

	while(1)
	{
		if(ptr == NULL)
		{
			if(s.size() == 0)
				break;
			else
			{
				ptr = s.top();
				s.pop();	
			}
		}
	        size++;	
		if(ptr->right != NULL)
		{
			s.push(ptr->right);
		}
		ptr = ptr->left;

	}
	return size;
}

int main()
{
	struct treeStruct *rootPtr = treeCreator();
	int count = sizeWithoutRecur(rootPtr);
	cout<<"Number of  nodes in tree : "<<count<<endl;
	return 0;
}



