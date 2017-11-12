#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* args[]){
    cout<<"in display"<<endl<<flush;
    string s;
    getline(cin, s);
    cerr<<"From display: " << s <<endl<<flush;
}