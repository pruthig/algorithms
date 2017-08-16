// Input : X and Y co-ordinates of the points in order. 
// Each point is represented by (X[i], Y[i])
int Solution::coverPoints(vector<int> &X, vector<int> &Y) {
    // distance between two points will be max of 9y1-y0, x1-x0)
    if(X.size() == 1 || Y.size() == 1 || X.size() == 0 || Y.size() == 0)
        return 0;
    int sum = 0;
    for(int i = 0; i < X.size()-1; ++i) {
        sum += max( abs(Y.at(i+1) - Y.at(i)) , abs(X.at(i+1) - X.at(i)) );
    }
    return sum;
}
