#include "user.h"

user::user()
{
    
}

user::~user()
{
    
}

void user::setnick(char *nick)
{
    this->nick = nick;
}

void user::setpaw(char *paw)
{
    this->paw = paw;
}

void user::setphone(char *phone)
{
    this->phone = phone;
}

string user::getusername()
{
    return this->nick;
}

string user::getpaw()
{
    return this->paw;
}

string user::getphone()
{
    return this->phone;
}

int user::login()
{
    dbnode *db = obj::task.getdbinst();
    
    //服务器繁忙
    if (db == NULL)
    {
        return 6;
    }
    
    //判断是否注册
    int count = 0;
    string sql = "select count(1) as count from user t where t.tel = '" + getphone() + "'";
    if (db->dbi->querysql((char *)sql.data()) == SUCCESS)
    {
        if (db->dbi->getrecordresult() == SUCCESS)
        {
            while (db->dbi->iseof())
            {
                count = db->dbi->getint((char *)"count");
                break;
            }
            db->dbi->releaserecordresult();
        }
        else
        {
            obj::task.reldbinst(db);
            ploginfo(LERROR, "user::login->getrecordresult failed");
            return 2;
        }
    }
    else
    {
        obj::task.reldbinst(db);
        ploginfo(LERROR, "user::login->querysql failed");
        return 2;
    }
    
    if (count > 0)
    {
        return 4;
    }
    
    //加入用户
    sql = "insert into user(tel, nick) values('" + getphone() + "', '" + getusername() + "')";
    if (db->dbi->modifysql((char *)sql.data()) == FAILED)
    {
        obj::task.reldbinst(db);
        ploginfo(LERROR, "user::login->modifysql failed");
        return 2;
    }
    
    
    obj::task.reldbinst(db);
    
    return 1;
}


