#ifndef PYINTER_H
#define PYINTER_H

#include <python2.7/Python.h>

#ifdef __cplusplus
extern "C"
{
#endif
    
typedef PyObject pyobject;
    
struct sysc
{
    /*tcp*/
    unsigned char ip[20];
    int port;
    
    /*db*/
    unsigned char host[20];
    unsigned char user[20];
    unsigned char paw[20];
    unsigned char db[20];
    int dbport;
};

int getsyscon(const char *filename, struct sysc *sysc);
 
#ifdef __cplusplus
}
#endif
 
#endif