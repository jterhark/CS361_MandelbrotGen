#include <iostream>
#include <unistd.h>
#include <iomanip>

using namespace std;

int main(int argc, char* args[]){
    cout<<"in calc"<<endl<<flush;


    auto fd = atoi(args[1]);
    char buf[30];
    //read(fd, buf, 5);
//    cout<<buf<<endl;

    dup2(fd, fileno(stdin));

    char c;
    while(!cin.eof()){
        cin>>c;
        cout<<c;
    }

    cout<<fd<<endl;
}

