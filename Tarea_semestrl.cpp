#include<iostream>
#include<string>
#include<cmath>
#include<tuple>
#include<chrono>
#include<fstream>
#include<sstream>
#include<vector>
#include<list>
#include<random>
#include<map>
#include<algorithm>
#include<stack>

using namespace std;

struct User{

    long long ID;
    string username;
    int follower_count;
    int friends_count;
    vector<long long> friends;
    vector<long long> followers;
    tuple<float,float,float,float> political_index;
    float presence;
    

};