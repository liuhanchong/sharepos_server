#include "obj.h"
#include <string.h>

task obj::task;

obj::obj()
{
    
}

obj::~obj()
{
    
}

char *obj::jsonstringform(cJSON *jsons, char *name)
{
    char *value = cJSON_Print(cJSON_GetObjectItem(jsons, name));
    if (value == NULL)
    {
        return NULL;
    }
    
    long len = strlen(value);
    
    //判断字符串合法性
    if (value[0] != '\"' || value[len - 1] != '\"')
    {
        return value;
    }
    
    int index = 0;
    while (index < len - 1)
    {
        value[index] = value[index + 1];
        index++;
    }
    
    value[index - 1] = '\0';
    
    return value;
}
