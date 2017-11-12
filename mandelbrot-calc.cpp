#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <string>

using namespace std;

int main(int argc, char* args[]){
    cerr<<"in calc"<<endl<<flush;
    string s;
    getline(cin, s);
    cout<<s<<endl;
    cerr<<"From Calc: "<<s<<endl<<flush;

//
//    auto fd = atoi(args[1]);
//    char buf[30];
//    //read(fd, buf, 5);
////    cout<<buf<<endl;
//
//    dup2(fd, fileno(stdin));
////
////    string s;
////    getline(cin, s);
////    cout<<s<<endl;
//
//    if(cin.bad()){
//        perror("cin bad in calc");
//        exit(-10);
//    }
//
//    while(!cin.eof()){
//        string s;
//        getline(cin, s);
//        cout<<s<<endl;
//    }
////
////    char c;
////    while(!cin.eof()){
////        cin>>c;
////        cout<<c;
////    }

//    cout<<fd<<endl;
}

