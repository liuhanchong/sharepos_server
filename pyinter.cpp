#include "pyinter.h"
#include <exception> 
#include <string.h> 
using namespace std;

class parsepy
{
public:
    parsepy();
    ~parsepy();
    
public:
    int runpy(char *py);
    pyobject *import(char *model);
    pyobject *getattr(pyobject *moudle, char *attr);
    pyobject *newtuple(int size);
    void intuple(pyobject *tuple, int seq, pyobject *object);
    pyobject *call(pyobject *fun, pyobject *arg);
    void init();
    void finalize();
    pyobject *pysfs(char *str);
    pyobject *parseobj(pyobject *ret);
    char *parsestr(pyobject *ret);
    int parseint(pyobject *ret);
    void decref(pyobject *obj);
    void decrefex(pyobject **obja, int size);
    pyobject *exem(char *mo, char *attr, pyobject *arg);
    pyobject *setargtuple(pyobject **arg, int size);
    pyobject *getdict(pyobject *moudle);
    pyobject *getdictitem(pyobject *dict, char *item);
    pyobject *insclass(char *mo, char *cla, pyobject *arg, pyobject *kw);
    pyobject *callmethod(pyobject *cla, char *method);
    
private:
    
public:
    
private:
    
};

parsepy::parsepy()
{
    init();
}

parsepy::~parsepy()
{
    finalize();
}

int parsepy::runpy(char *py)
{
    return PyRun_SimpleString(py);
}

pyobject *parsepy::getattr(pyobject *moudle, char *attr)
{
    return PyObject_GetAttrString(moudle, attr);
}

pyobject *parsepy::newtuple(int size)
{
    return PyTuple_New(size);
}

void parsepy::intuple(pyobject *tuple, int seq, pyobject *object)
{
    PyTuple_SetItem(tuple, seq, object);
}

pyobject *parsepy::call(pyobject *fun, pyobject *arg)
{
    return PyObject_CallObject(fun, arg);
}

void parsepy::init()
{
    Py_Initialize();
}

void parsepy::finalize()
{
    Py_Finalize();
}

pyobject *parsepy::import(char *model)
{
    return PyImport_ImportModule(model);
}

pyobject *parsepy::pysfs(char *str)
{
    return PyString_FromString(str);
}

pyobject *parsepy::parseobj(pyobject *ret)
{
    pyobject *obj = NULL;
    if (PyArg_Parse(ret, "O", &obj) == 0)
    {
        return NULL;
    }
    
    return obj;
}

char *parsepy::parsestr(pyobject *ret)
{
    char *str = NULL;
    if (PyArg_Parse(ret, "s", &str) == 0)
    {
        return NULL;
    }
    
    return str;
}

int parsepy::parseint(pyobject *ret)
{
    int va = 0;
    PyArg_Parse(ret, "i", &va);
    return va;
}

void parsepy::decref(pyobject *obj)
{
    Py_DECREF(obj);
}

void parsepy::decrefex(pyobject **obja, int size)
{
    for (int i = 0; i < size; i++)
    {
        Py_DECREF(obja[i]);
    }
}

pyobject *parsepy::exem(char *mo, char *attr, pyobject *arg)
{
    pyobject *moudle = import(mo);
    if (!moudle)
    {
        return NULL;
    }
    
    pyobject *fun = getattr(moudle, attr);
    if (!fun)
    {
        decref(moudle);
        return NULL;
    }
    
    pyobject *ret = call(fun, arg);
    
    decref(moudle);
    decref(fun);
    
    return ret;
}

pyobject *parsepy::setargtuple(pyobject **arg, int size)
{
    pyobject *argtuple = newtuple(size);
    if (argtuple)
    {
        for (int i = 0; i < size; i++)
        {
            intuple(argtuple, i, arg[i]);
        }
    }
    return argtuple;
}

pyobject *parsepy::getdict(pyobject *moudle)
{
    return PyModule_GetDict(moudle);
}

pyobject *parsepy::getdictitem(pyobject *dict, char *item)
{
    return PyDict_GetItemString(dict, item);
}

pyobject *parsepy::insclass(char *mo, char *cla, pyobject *arg, pyobject *kw)
{
    pyobject *moudle = import(mo);
    if (!moudle)
    {
        return NULL;
    }
    
    pyobject *dict = getdict(moudle);
    if (!dict)
    {
        decref(moudle);
        return NULL;
    }
    
    pyobject *claobj = getdictitem(dict, cla);
    if (!claobj)
    {
        decref(dict);
        decref(moudle);
        return NULL;
    }
    
    return PyInstance_New(claobj, arg, kw);
}

pyobject *parsepy::callmethod(pyobject *cla, char *method)
{
    return PyObject_CallMethod(cla, method, NULL);
}

cbool getsyscon(const char *filename, struct sysc *sysc)
{	
	try
	{
		parsepy py;

		//import py moudle
		py.runpy((char *)"import sys");
        py.runpy((char *)"sys.path.append('.')");

		pyobject *sysclass = py.insclass((char *)"paconfig", (char *)"sys", NULL, NULL);
        if (!sysclass)
        {
            return FAILED;
        }

		//read config
		pyobject *obj[2] = {py.pysfs((char *)filename), sysclass};
		pyobject *argtuple = py.setargtuple(obj, 2);
		pyobject *ret = py.exem((char *)"paconfig", (char *)"getsyscon", argtuple);
        if (!ret)
        {
            return FAILED;
        }

		//获取类的成员
		pyobject *ip = py.callmethod(sysclass, (char *)"getip");
		char *cip = py.parsestr(ip);
		strncpy((char *)sysc->ip, cip, sizeof(sysc->ip));

		pyobject *port = py.callmethod(sysclass, (char *)"getport");
		sysc->port = py.parseint(port);
	}
	catch (exception &e)
	{
		return FAILED;
	}

	return SUCCESS;	
}