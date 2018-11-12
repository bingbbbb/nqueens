#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

union longCharArr
{
    long long_;
    char charArr[8];
};
void testLongCharArr()
{
    longCharArr longCharArr_;
    longCharArr_.long_ = 0x353641424448494D;
    char* char_ = longCharArr_.charArr;
    for(int i=0;i<8;i++)
    {
        cout << char_[i] << endl;
    }
    cout << longCharArr_.long_ << endl;
}



int main()
{
    std::vector<int> vec{1, 2, 3};
    auto a = std::find(vec.begin(), vec.end(), 4);
    if(a != vec.end())
        cout << *a << endl;
    else
        cout << "not found" << endl;
}
