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

void testchar()
{
    char a = static_cast<char>(0xFF);
    int temp;
    for (int i=0;i<8;i++)
    {
        temp = (a >> i) & 1;
        cout << temp;
    }
    cout<<endl;
}

int main()
{
    testchar();
}
