#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <string>
#include <sys/shm.h>

using namespace std;

int main(int argc, char *args[]) {
    cerr << "in calc" << endl << flush;
    string s;

    int shmid = atoi(args[1]);
    int msg = atoi(args[2]);
    cerr << "From Calc: shmid: " << shmid << " msgid: " << msg << endl << flush;

    auto memory = (int *) shmat(shmid, NULL, 0);

    memory[0] = 47459;

    getline(cin, s);
    cout << s << endl;
    cerr << "From Calc: " << s << endl << flush;


    double xMin, xMax, yMin, yMax;
    int nRows, nCols, maxIters;

    cin >> xMin >> xMax >> yMin >> yMax >> nRows >> nCols >> maxIters;

    //cerr<<"xMin: " << xMin << " xMax" << xMax << " yMin: " <<yMin << "yMax: " <<yMax << " nRows: " << nRows << " nCols: "<<nCols<<" maxIters: "<<maxIters<<endl;

    printf("%f\n%f\n%f\n%f\n%d\n%d\n%d\n", xMin, xMax, yMin, yMax, nRows, nCols, maxIters);

    double deltaX = (xMax - xMin) / (nCols - 1);
    double deltaY = (yMax - yMin) / (nRows - 1);

    for (int r = 0; r < nRows; ++r) {
        double Cy = yMin + r * deltaY;
        for (int c = 0; c < nCols; ++c) {
            double Cx = xMin + c * deltaX,
                    Zx = 0.0,
                    Zy = 0.0;
            int n = 0;
            for (n = 0; n < maxIters; ++n) {
                if (Zx * Zx + Zy * Zy >= 4.0)
                    break;
                double Zx_next = Zx * Zx - Zy * Zy + Cx;
                double Zy_next = 2.0 * Zx * Zy + Cy;
                Zx=Zx_next;
                Zy=Zy_next;
            }
            if(n>=maxIters){
                *(memory + r*nCols + c) = -1;
            }else{
                *(memory + r*nCols + c) = n;
            }
        }

    }

    if (shmdt(memory) < 0) {
        perror("Cannot release shared memory in calc");
    }


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

