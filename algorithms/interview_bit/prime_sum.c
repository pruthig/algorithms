bool IsPrime(int A) {
    // Do not write main() function.
    // Do not read input, instead use the arguments to the function.
    // Do not prlong the output, instead return values as specified
    // Still have a doubt. Checkout www.longerviewbit.com/pages/sample_codes/ for more 
    
    if(A == 1 || A == 0)
        return false;
    if(A ==2 || A== 3)
        return true;
    for(int i = 2; i <= sqrt(A); ++i)
        if(A%i == 0)
            return false;
        
    return true;
}

vector<int> sieve(int A) {
    // Initially whole array is considered to be of prime numbers after that for each 
    // the multiple of that is rendered non-prime but setting its value to false
    bool *arr = new bool[A+1];

    // 0 and 1 are not prime
    arr[0] = false;
    arr[1] = false;
    vector<int> vlong;
    
    // Initially, let us consider all of them primes
    for(int i = 2; i <= A; ++i)
        arr[i]  = true; 
    
    for(int a = 2; a <= sqrt(A); ++a) {
        if(arr[a] == false)
            continue;
        if(IsPrime(a)) {
            for(int i = 2; a*i <= A; ++i) {
                    arr[a*i] = false;
            }
        }
    }
    
    for(int i = 0; i <= A; ++i)
        if(arr[i] != false) {
            vlong.push_back(i);
        }
        
    delete[] arr;
    
    return vlong;
   
}

vector<int> Solution::primesum(int A) {
    // Get the prime set
    unordered_set<int> n_set;
    vector<int> primeSet = sieve(A);

    vector<int> res;
    int i = 0; int j = primeSet.size()-1;
    while(i < j) {
        int cur_sum = primeSet.at(i) + primeSet.at(j);
        if(cur_sum < A) {
            ++i;
        }
        else if(cur_sum > A) {            
            if(A == primeSet.at(i)*2) {
                res.push_back(primeSet.at(i));
                res.push_back(primeSet.at(i));
                return res;
            }
            --j;
        }
        else {
            res.push_back(primeSet.at(i));
            res.push_back(primeSet.at(j));
            return res;
        }
        
        
    }
          
   return res; 
    
}