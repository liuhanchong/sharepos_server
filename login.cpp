#include "login.h"

login::login()
{
    
}

login::~login()
{
    
}

struct evbuffer *login::handle(const char *url, cJSON *jsons)
{
    char *nick = jsonstringform(jsons, (char *)"nickName");
    char *tel = jsonstringform(jsons, (char *)"telPhone");
    
    ploginfo(LDEBUG, "url=%s nick=%s tel=%s", url, nick, tel);
    
    user.setnick(nick);
    user.setphone(tel);
    int ret = user.login();
    
    //生成json格式数据返回
    cJSON *root = cJSON_CreateObject();
    struct evbuffer *resbuf = evbuffer_new();
    cJSON_AddNumberToObject(root, "state", ret);
    char *data = cJSON_Print(root);
    evbuffer_add(resbuf, data, strlen(data));
    
    ploginfo(LDEBUG, "resdata=%s", resbuf->buffer);
    
    return resbuf;
}





