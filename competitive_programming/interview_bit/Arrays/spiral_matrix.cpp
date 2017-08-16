vector<int> Solution::spiralOrder(const vector<vector<int> > &A) {
	vector<int> result;
	int t = 0, b = A.size()-1, l = 0, r = A.at(0).size()-1;
	int dir = 0;   //0->left to right, 1->top to bottom, 2->right to left, 3->bottom to top
	
	while(l <= r && t <= b) {
	    // print l -> r
	    if(dir == 0) {
	        for(int i = l; i  <= r; ++i) {
	            result.push_back(A.at(t).at(i));
	        }
	        t++;
	        
	    }
	    // print t->b
	    else if(dir == 1) {
	        for(int i = t; i  <= b; ++i) {
	            result.push_back(A.at(i).at(r));
	        }
	        r--;
	        
	    }
	    // print r->l
	    else if(dir == 2) {
	        for(int i = r; i  >= l; --i) {
	            result.push_back(A.at(b).at(i));
	        }
	        b--;
	        
	    }
	    // print b->t
	    else {
	        for(int i = b; i  >= t; --i) {
	            result.push_back(A.at(i).at(l));
	        }
	        l++;
	        
	    }
	    dir = (dir+1)%4;
	}
	// DO STUFF HERE AND POPULATE result
	return result;
}
