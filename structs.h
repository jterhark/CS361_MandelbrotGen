//
// Created by jake on 11/12/17.
//

#ifndef MANDELBROT_STRUCTS_H
#define MANDELBROT_STRUCTS_H

#endif //MANDELBROT_STRUCTS_H

#define FILENAMEMESSAGETYPE 123
#define DONEMESSAGETYPE 456

typedef struct{
    long int type{FILENAMEMESSAGETYPE};
    char filename[256];
}FilenameMessage;

typedef struct{
    long int type{DONEMESSAGETYPE};
}DoneMessage;

