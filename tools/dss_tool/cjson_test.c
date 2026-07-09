#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

int main() {
    // 创建一个简单的JSON对象
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", "test");
    cJSON_AddNumberToObject(root, "value", 42);
    
    // 将JSON对象转换为字符串
    char *json_str = cJSON_Print(root);
    printf("Created JSON: %s\n", json_str);
    
    // 释放内存
    cJSON_free(json_str);
    cJSON_Delete(root);
    
    // 解析JSON字符串
    const char *test_json = "{\"name\":\"test\",\"value\":42}";
    cJSON *parsed = cJSON_Parse(test_json);
    if (parsed == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error parsing JSON: %s\n", error_ptr);
        }
        return 1;
    }
    
    // 获取解析后的值
    cJSON *name = cJSON_GetObjectItem(parsed, "name");
    cJSON *value = cJSON_GetObjectItem(parsed, "value");
    
    if (name && value) {
        printf("Parsed JSON - name: %s, value: %d\n", name->valuestring, value->valueint);
    }
    
    // 释放内存
    cJSON_Delete(parsed);
    
    return 0;
}
