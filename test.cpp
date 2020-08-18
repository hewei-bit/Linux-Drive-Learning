#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>



using namespace std;
int main()
{
    vector<pair<int,int>> ans;
    int n;
    cin >> n;
    string tmp;
    int value;
    for(int i = 0;i < n/4;i++)
    {
        tmp = to_string(i*4);
        value = stoi(tmp);
        reverse(tmp.begin(),tmp.end());
        if(i == stoi(tmp))
        {
            ans.push_back(make_pair(i,value));
        }

    }
    cout << ans.size() << endl;
    for(auto p : ans)
        cout << p.first<< " " << p.second << endl;

    return 0;

}