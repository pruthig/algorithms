//Find the matrix with largest number of 1's in term of dimensions


#include<iostream>


using namespace std;


void findIsland(int i, int j);
int count = 0;
bool marked[][4] = {
						{false, false, false, false},
						{false, false, false, false},
						{false, false, false, false},
						{false, false, false, false}
					};


int mx[][4]= 
		{
			{1, 0, 0, 1},
			{0, 0, 1, 1},
			{1, 0, 1, 0},
			{1, 0, 0, 1}
		};

int main()
{
		for(int i = 0; i<=3; ++i){
			for(int j =0; j<=3; ++j){
				if(marked[i][j] == false && mx[i][j] == 1){
					cout<<"\ni and j identified as:"<<i<<j;
					++count;
					marked[i][j] = true;
					findIsland(i,j);
				}
			}
		}
	cout<<"\nNumber of islands :"<<count<<endl;
	return 0;
}
			
void findIsland(int i, int j)
{

	for(int k=-1; k<=1; ++k){
		for(int l=-1; l<=1; ++l){
			if((k==0 && l==0) || (k+i)<0 || (k+i)>3 || (l+j)<0 || (l+j)>3 || marked[k+i][l+j] == true || mx[k+i][l+j] == 0)
				continue;
			else{
				marked[k+i][l+j] = true;
				findIsland(k+i, l+j);
			}
		}
	}

}
				
			
