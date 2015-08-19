#include<stdio.h>
#include<string.h>


int main(){

	char s1[3] = "io";
	char s2[14] = "choooseyooooo";
	int length , k = 0;

	length = strlen(s2);

	printf("Length of s2 is : %d\n", length);  
	//to be removed from s2

	for(int i = 0 ; i < strlen(s1); ++i) {
		k = 0;
		for(int j = 0; j < length; ){    
			if(s1[i] == s2[j]){
				++j;
				continue;
			}
			else
				s2[k++] = s2[j++];
		}
		length = k;
	}
	s2[k] = '\0';
	printf("Printing s2 where chars are removed : %s\n", s2);

	return 0;
}

