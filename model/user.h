#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

#include "obj.h"

class user
{
public:
    user();
    ~user();
    
    void setnick(char *nick);
    void setpaw(char *paw);
    void setphone(char *phone);
    
    string getusername();
    string getpaw();
    string getphone();
    
    int login();
    
private:
    string nick;
    string paw;
    string phone;
    
};

#endif 
