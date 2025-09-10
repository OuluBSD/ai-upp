#ifdef HAVE_POCO
#include <vector>
#include <algorithm>





void Poco01() {
    using namespace std;

    // 1.
    {
        vector<int> v = {5, 3, 1, 4, 2};



        // Default ascending order
        sort(v.begin(), v.end());




        v.push_back(6);
        v.push_back(7);
        v.push_back(8);

        v.insert(v.begin() + 1, 5);

        bool b = v.empty(); (void)b;

        v.reserve(100);

        v[0] = 1;
        v.back() = 10;
        

        v.resize(100, 5);
        
        v.shrink_to_fit();

        v.erase(v.begin() + 2);
        v.erase(v.begin() + 1, v.begin() + 3); // count of 3-1=2
        v.pop_back(); // void

        int count = (int)v.size();

        v.clear();
    }
    
}

#endif
