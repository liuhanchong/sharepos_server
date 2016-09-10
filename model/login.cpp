#include "login.h"
#include "util.h"
#include "cjson.h"
#include "evbuffer.h"
#include "res.h"
#include "log.h"

login::login()
{
    
}

login::~login()
{
    
}

void login::setnick(char *nick)
{
    this->nick = nick;
}

void login::setpaw(char *paw)
{
    this->paw = paw;
}

void login::setphone(char *phone)
{
    this->phone = phone;
}

int login::save()
{
    dbnode *db = res::getinstance()->getdbinst();
    if (db == NULL)
    {
        return 6;
    }
    
    //判断是否注册
    int count = 0;
    string sql = "select count(1) as count from user t where t.tel = '" + phone + "'";
    if (db->dbi->querysql((char *)sql.data()) == 1)
    {
        if (db->dbi->getrecordresult() == 1)
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
            res::getinstance()->reldbinst(db);
            ploginfo(LERROR, "user::login->getrecordresult failed");
            return 2;
        }
    }
    else
    {
        res::getinstance()->reldbinst(db);
        ploginfo(LERROR, "user::login->querysql failed");
        return 2;
    }
    
    if (count > 0)
    {
        return 4;
    }
    
    //加入用户
    sql = "insert into user(tel, nick) values('" + phone + "', '" + nick + "')";
    if (db->dbi->modifysql((char *)sql.data()) == 0)
    {
        res::getinstance()->reldbinst(db);
        ploginfo(LERROR, "user::login->modifysql failed");
        return 2;
    }
    
    
    res::getinstance()->reldbinst(db);
    
    return 1;
}

int login::handle(struct evbuffer *resbuf, const char *url, const char *rdata, int len)
{
    cJSON *jsons = cJSON_Parse(rdata);
    
    setnick(jsonstringform(jsons, (char *)"nickName"));
    setphone(jsonstringform(jsons, (char *)"telPhone"));
    
    //生成json格式数据返回
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "state", save());
    char *wdata = cJSON_Print(root);
    evbuffer_add(resbuf, wdata, strlen(wdata));
    cJSON_Delete(jsons);
    
    return 1;
}





