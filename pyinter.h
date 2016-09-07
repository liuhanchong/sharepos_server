#ifndef INFACE_H
#define INFACE_H

#include "util.h"
#include <python2.7/Python.h>

typedef PyObject pyobject;

typedef struct sysc
{
    unsigned char ip[20];
    int port;
    
    unsigned char host[20];
    unsigned char user[20];
    unsigned char paw[20];
    unsigned char db[20];
    int dbport;
} sysc;

#ifdef __cplusplus
extern "C"
{
#endif

cbool getsyscon(const char *filename, struct sysc *sysc);
 
#ifdef __cplusplus
}
#endif
 
#endif