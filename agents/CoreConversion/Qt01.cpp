#ifdef HAVE_QT
#include <QVector>
#include <algorithm>
using namespace std;




void Qt01() {

    // 1.
    {
        QVector<int> v = {5, 3, 1, 4, 2};




        // Default ascending order
        sort(v.begin(), v.end());




        v.push_back(6);
        v.push_back(7);
        v.push_back(8);

        v.insert(v.begin() + 1, 5);

        bool b = v.isEmpty();
        (void)b;

        v.reserve(100);

        v[0] = 1;
        v.back() = 10;
        

        v.resize(100, 5);
        v.squeeze(); // shrink capacity

        v.erase(v.begin() + 2);
        v.erase(v.begin() + 1, v.begin() + 3); // count of 3-1=2
        v.pop_back(); // void

        int count = v.size();

        v.clear();
    }
    
}

#endif
