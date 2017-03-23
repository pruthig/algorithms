//create the BST from a sorted array

#include<iostream>


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

//tree printer
void printPostOrder(struct treeStruct *ptr){
if(ptr == NULL)
    return;

printPostOrder(ptr->left);
printPostOrder(ptr->right);
cout<<ptr->element<<",";
}



struct treeStruct* createTree(int *a, int l, int r){
int mid = (l+r)/2;
if(l == r){
struct treeStruct* temp = newNode(a[l]);
return temp;
}
struct treeStruct* nNode = newNode(a[mid]);
nNode->left = createTree(a, l, mid-1);
nNode->right = createTree(a, mid+1, r);
return nNode;
}

int main(){
int arr[] = {2, 3, 4};
int length = sizeof(arr)/sizeof(arr[0]);

struct treeStruct* nodeNew = createTree(arr, 0, length-1);
printPostOrder(nodeNew);
cout<<endl;
return 0;
}



