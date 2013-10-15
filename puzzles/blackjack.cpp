#include<iostream>

using namespace std;

int N;
int A[10];
int Answer;

int main(int argc, char** argv)
{
	int test_case;
	int A11[10];
	/*
	   freopen function below opens input.txt file in read only mode, and afterward,
	   the program will read from input.txt file instead of standard(keyboard) input.
	   To test your program, you may save input data in input.txt file,
	   and use freopen function to read from the file when using cin function.
	   You may remove the comment symbols(//) in the below statement and use it.
	   Use #include<cstdio> or #include<stdio.h> to use the function in your program.
	   But before submission, you must remove the freopen function or rewrite comment symbols(//).
	 */
	// freopen("input.txt", "r", stdin);


	/*
	   Your program should handle 10 test cases given.
	 */
	for(test_case = 1; test_case <= 10; ++test_case)
	{
		int i;

		/*
			 Read each test case from standard input.
		*/
		cin >> N;
		if(N > 10)
			return -1;

		for(i = 0; i < N; i++)
		{
			cin >> A[i];
		}

		//copy first.
		for(i = 0; i<N; i++)
		{
			A11[i] = A[i];
		}

		for(i = 0; i<N; i++)
		{
			if(A11[i] == 1){
				A11[i] = 11;
				break;
			}
		}


		int sum = 0;
		int max = 0;
		int prev = 0;

		int sum11 = 0;
		int max11 = 0;
		int prev11 = 0;
		
		for(int i = 0;i<N; i++)
		{
			sum = sum + A[i];
		
			if(sum > 21)
			{
				break;
			}

			if(sum <= 21 && sum > max)
			max = sum;

			
		}

		for(int i = 0;i<N; i++)
		{
			sum11 = sum11 + A11[i];
		
			if(sum11 > 21)
			{
				break;
			}

			if(sum11 <= 21 && sum11 > max11)
			max11 = sum11;

			
		}
                 
        if(max >= max11)     
			Answer = max;
		else 
			Answer = max11;


		// Print the answer to standard output(screen).
		cout << "#" << test_case;
		cout << " " << Answer << endl;
		
	}

	
	return 0;//Your program should return 0 on normal termination.
}

