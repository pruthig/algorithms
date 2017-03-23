#include<iostream>
#include<cstdlib>
#include<string>
#include<sstream>
#include<fstream>



using namespace std;

void merge(int s, int e, int *p);
void msort(int s, int e, int *p);
unsigned int g_c = 0;



int main(){
int buf[100000];
string str = "";
stringstream ss;
int i = 0;

ifstream ifs("/home/pruthi/Desktop/IntegerArray.txt");

while(getline(ifs, str)){
std::istringstream iss(str);
iss >> buf[i];
++i;
}

/*int buf[250] ={16, 216, 179, 202, 2, 86, 107, 76, 8, 203, 114, 41, 207, 125, 44, 84, 138, 32, 245, 77, 168, 112, 35, 62, 66, 143, 26, 246, 175, 214, 237, 181, 171, 40, 121, 19, 109, 240, 58, 150, 101, 82, 230, 31, 80, 235, 153, 56, 12, 123, 146, 188, 89, 59, 53, 217, 195, 21, 110, 74, 113, 47, 210, 17, 206, 165, 229, 54, 226, 144, 100, 205, 60, 45, 184, 211, 154, 198, 104, 180, 213, 24, 173, 185, 38, 103, 105, 48, 142, 223, 145, 148, 78, 93, 108, 81, 130, 218, 194, 57, 1, 201, 135, 95, 127, 233, 43, 88, 9, 155, 141, 187, 157, 51, 22, 115, 182, 225, 69, 122, 176, 163, 177, 50, 28, 248, 34, 249, 49, 75, 97, 193, 7, 68, 149, 189, 242, 151, 170, 111, 160, 71, 167, 243, 178, 236, 46, 70, 120, 14, 172, 39, 30, 199, 162, 36, 140, 6, 42, 96, 67, 116, 119, 219, 159, 200, 222, 72, 204, 197, 174, 37, 152, 27, 164, 129, 99, 239, 221, 156, 5, 137, 102, 18, 241, 227, 166, 3, 238, 106, 98, 191, 196, 224, 13, 134, 79, 11, 33, 23, 131, 15, 10, 29, 244, 147, 20, 186, 73, 0, 128, 232, 132, 192, 139, 126, 247, 61, 208, 94, 183, 136, 190, 52, 55, 169, 133, 158, 25, 87, 4, 65, 64, 90, 91, 161, 92, 63, 209, 118, 83, 234, 231, 220, 117, 124, 228, 212, 215, 85};
*/
merge(0, 99999, buf);
cout<<"After Merging\n";

cout<<"Value of inversions "<<g_c<<endl;
return 0;
}


void merge(int s, int e, int *p){
if(s>e || s == e)
	return;
int mid = (s+e)/2;
merge(s, mid, p);
merge(mid+1, e, p);
msort(s, e, p);
}

//3, 5, 4, 6, 8, 9, 1
//0                 6
//         m
//         3
//m-s+1 and e-m = 4, 3

void msort(int s, int e, int *p){

int m = (s+e)/2;
int l1 = (m-s+1);
int l2 = (e-m);
unsigned int l_c = 0;

cout<<"s and e are :"<<s<<", "<<e<<endl;
int *arr1 = (int*)malloc( sizeof(int) * l1) ;
int *arr2 = (int*)malloc( sizeof(int) * l2) ;

int *temp = (int*)malloc(sizeof(int) * (l1+l2)) ;

for(int i = s; i<=m; ++i)
	*(arr1+i-s) = *(p+i);

for(int i=m+1; i<=e; i++)
	*(arr2+i-m-1) = *(p+i);

int i = 0, j = 0, k = 0;

for(; k<(l1+l2); ++k){
	if(i == l1){
		*(temp+k) = *(arr2+j);
		++j;
		continue;
	}
	if(j == l2){
		*(temp+k) = *(arr1+i);
		++i;
		l_c = l_c + j;
		continue;
	}
	
	if( *(arr1+i) < *(arr2+j) || *(arr1+i) == *(arr2+j) ){
		*(temp+k) = *(arr1+i);
		l_c = l_c + j;
		++i;
	
	}
	else{
			
		*(temp+k) = *(arr2+j);
		++j;
	}

	//g_c = g_c + l_c;
		
		
}
for(int i = s; i<=e; ++i)
	*(p+i) = *(temp+i-s);



g_c = g_c+l_c;

}




