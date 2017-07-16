int IsPrime(int A) {
    // Do not write main() function.
    // Do not read input, instead use the arguments to the function.
    // Do not print the output, instead return values as specified
    // Still have a doubt. Checkout www.interviewbit.com/pages/sample_codes/ for more details
    if(A == 1 || A == 0)
        return 0;
    if(A ==2 || A== 3)
        return 1;
    for(int i = 2; i <= sqrt(A); ++i)
        if(A%i == 0)
            return 0;
        
    return 1;
}

vector<int> Solution::sieve(int A) {
	// Initially whole array is considered to be of prime numbers after that for each prime number
	// the multiple of that is rendered non-prime but setting its value to -1
    int *arr = new int[A+1];
    arr[0] = -1;
    arr[1] = -1;
    vector<int> vInt;
    // Initially, let us consider all of them primes
    for(int i = 2; i <= A; ++i)
        arr[i]  = i; 
    
    for(int a = 2; a <= A; ++a) {
        //cout<<"a is: "<<a<<endl;
        if(IsPrime(a)) {
            for(int i = 2; a*i <= A; ++i) {
                    //cout<<"a*i is: "<<a*i<<endl;
                    arr[a*i] = -1;
            }
        }
    }
    
    for(int i = 0; i <= A; ++i)
        if(arr[i] != -1)
            vInt.push_back(arr[i]);
        //cout<<" " <<arr[i];
        
    return vInt;
    
    
}

