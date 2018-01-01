// We will find and print all paths leading to a given sum
#include<iostream>
#include<vector>

using namespace std;

namespace {
	vector< vector<int> > result;
};

typedef struct treeStruct
{
	int val;
	struct treeStruct *left;
	struct treeStruct *right;
}treeStruct;



struct treeStruct* newNode(int data)
{
	struct treeStruct *newElement = new(struct treeStruct);
	newElement->left = NULL;
	newElement->right = NULL;
	newElement->val = data;
	return newElement;
}

struct treeStruct* treeCreator()
{
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
/*
                                        10
                                      /    \
                                    6      12
                                   / \     / \
                                  2   9   11  18
                                              / \
                                            16   20
*/

void printList(vector<vector<int>> res) {
	if(res.empty()) {
		cout<<"List is empty\n";
		return;
	}
		
	for(auto a: res) {
		for(auto b : a)
			cout<<" "<<b;
		cout<<endl;
	}
}

void findListWithGivenSum(struct treeStruct *ptr, int count, int sum, vector<int>& tmp_vec)
{
	if(ptr == NULL)
		return;
	if(ptr->left == NULL && ptr->right == NULL) {
		if(count+ptr->val == sum) {
			tmp_vec.push_back(ptr->val);
			result.push_back(tmp_vec);
			tmp_vec.pop_back();		
			return;
		}
	}
	count += ptr->val;
	tmp_vec.push_back(ptr->val);
	
	findListWithGivenSum(ptr->left, count, sum, tmp_vec);
	findListWithGivenSum(ptr->right, count, sum, tmp_vec);
	
	count -= ptr->val;
	tmp_vec.pop_back();
}

vector<vector<int>> findListWithGivenSumUtil(treeStruct *rootPtr) {
	int sum = 0;
	cout<<"Enter the sum to find\n";
	cin>>sum;
	
	vector<int> tmp_vec;
	tmp_vec.clear();
	
	findListWithGivenSum(rootPtr, 0, sum, tmp_vec);
	return result;
}

int main()
{
	
	struct treeStruct *rootPtr = treeCreator();
	vector<vector<int>> res = findListWithGivenSumUtil(rootPtr);
	printList(res);
	return 0;
}



