#define _CRT_SECURE_NO_WARNINGS //让scanf函数可用不报错
//#pragma warning(disable:4996)
#pragma warning(disable:6031)//禁用警告提示:" C6031 返回值被忽略："scanf" "#
#define _CRT_SECURE_NO_WARNINGS //让scanf函数可用不报错
//#pragma warning(disable:4996)
#pragma warning(disable:6031)//禁用警告提示:" C6031 返回值被忽略："scanf" "#


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define CONFIG_FILE "config.txt"  // 配置文件的名称
#define MAX_ATTEMPTS 3             // 最大尝试登录次数

// 定义配置结构体，用于存储用户信息和设置
typedef struct {
    char username[50];            // 用户名
    char password[50];            // 密码
    char line[10];                // 登录线路
    int showSuccessMessage;       // 登录成功时是否显示提示
    int autoLogin;                // 是否开机自启动并自动登录
} Config;

// 定义一个结构体，用于存储服务器响应数据
struct MemoryStruct {
    char* memory;                 // 存储响应数据的指针
    size_t size;                  // 响应数据的大小
};

//// 回调函数，用于处理服务器返回的数据
//static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, struct MemoryStruct* userp) {
//    size_t realsize = size * nmemb;  // 计算实际数据大小
//    userp->memory = realloc(userp->memory, userp->size + realsize + 1); // 扩展内存
//    if (userp->memory == NULL) {      // 检查内存分配是否成功
//        printf("不够内存\n");         // 输出错误信息
//        return 0;                     // 返回 0 表示内存分配失败
//    }
//    memcpy(&(userp->memory[userp->size]), contents, realsize); // 将接收到的数据复制到内存
//    userp->size += realsize;          // 更新数据大小
//    userp->memory[userp->size] = '\0'; // 添加字符串结束符
//    return realsize;                  // 返回实际处理的数据大小
//}
// 使用中间指针：在 realloc 之后使用一个中间指针 new_memory 来检查分配是否成功。
// 回调函数，用于处理服务器返回的数据
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, struct MemoryStruct* userp) {
    size_t realsize = size * nmemb;  // 计算实际数据大小
    char* new_memory = realloc(userp->memory, userp->size + realsize + 1); // 扩展内存
    if (new_memory == NULL) {         // 检查内存分配是否成功
        printf("不够内存\n");         // 输出错误信息
        return 0;                     // 返回 0 表示内存分配失败
    }
    userp->memory = new_memory;       // 安全地更新指针
    memcpy(&(userp->memory[userp->size]), contents, realsize); // 将接收到的数据复制到内存
    userp->size += realsize;          // 更新数据大小
    userp->memory[userp->size] = '\0'; // 添加字符串结束符
    return realsize;                  // 返回实际处理的数据大小
}

// 加载配置文件
void loadConfig(Config* config) {
    FILE* file = fopen(CONFIG_FILE, "r");      // 打开配置文件
    if (file) {                                 // 如果文件存在
        fscanf(file, "%s %s %s %d", config->username,
            config->password, config->line,
            &config->autoLogin);                // 读取配置
        fclose(file);                           // 关闭文件
        config->showSuccessMessage = 1;        // 默认显示成功提示
    }
    else {
        // 如果配置文件不存在，设置默认值
        strcpy(config->username, "");           // 默认用户名为空
        strcpy(config->password, "");           // 默认密码为空
        strcpy(config->line, "unicom");         // 默认登录线路为联通
        config->autoLogin = 0;                  // 默认不开机自启动
    }
}

// 保存配置文件
void saveConfig(Config* config) {
    FILE* file = fopen(CONFIG_FILE, "w");      // 打开配置文件用于写入
    fprintf(file, "%s\n%s\n%s\n%d\n\n", config->username,
        config->password, config->line,
        config->autoLogin);                     // 保存配置

    // 写入配置文件说明
    fprintf(file, "//配置文件说明：\n");
    fprintf(file, "//第1行：校园网账户（学号）\n");
    fprintf(file, "//第2行：登录密码\n");
    fprintf(file, "//第3行：登录线路（登录线路填写英文小写，联通：unicom、移动：cmcc、电信：aust、教职工：jzg）\n");
    fprintf(file, "//第4行：是否开机自动登录（1 = 是，0 = 否）\n");
    fprintf(file, "//若设置开机自启，则登陆成功不会有任何提示\n");
    fprintf(file, "//如果实在是写不好，那就删了配置文件直接运行登录程序，它会自己生成(╯‵□′)╯︵┻━┻\n");
    fclose(file);                               // 关闭文件
}

// 登录校园网的函数
int loginToCampusNetwork(Config* config) {
    CURL* curl;                                 // 定义 CURL 句柄
    CURLcode res;                               // CURL 返回值
    char url[256];                              // 存储请求 URL 的字符串

    // 组合登录请求的 URL，使用用户名、登录线路和密码
    snprintf(url, sizeof(url), "http://10.255.0.19/drcom/login?callback=dr1003&DDDDD=%s@%s&upass=%s&0MKKey=123456&R1=0&R3=0&R6=0&para=00&v6ip=&v=2172",
        config->username, config->line, config->password);

    curl_global_init(CURL_GLOBAL_DEFAULT);      // 初始化 CURL 库
    curl = curl_easy_init();                     // 初始化 CURL 句柄
    if (curl) {
        struct MemoryStruct chunk;               // 创建 MemoryStruct 以存储响应
        chunk.memory = malloc(1);                // 初始分配内存
        chunk.size = 0;                          // 初始化大小为 0

        curl_easy_setopt(curl, CURLOPT_URL, url);             // 设置请求的 URL
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // 设置写回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk); // 设置写入数据的结构体

        res = curl_easy_perform(curl);              // 执行请求
        if (res != CURLE_OK) {                       // 检查请求是否成功
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res)); // 输出错误信息
            free(chunk.memory);                       // 释放内存
            curl_easy_cleanup(curl);                 // 清理 CURL 句柄
            return 0;                                // 登录失败
        }

        //// 检查响应内容以确定是否登录成功
        //if (strstr(chunk.memory, "\"result\":1") != NULL) { // 检查 JSON 响应中的 result 字段
        //    free(chunk.memory);                        // 释放内存
        //    curl_easy_cleanup(curl);                  // 清理 CURL 句柄
        //    return 1;                                 // 登录成功
        //}
        //else {
        //    printf("响应内容: %s\n", chunk.memory);   // 输出实际响应内容以便调试
        //    free(chunk.memory);                        // 释放内存
        //    curl_easy_cleanup(curl);                  // 清理 CURL 句柄
        //    return 0;                                 // 登录失败
        //}
        // 增加空指针检查：在调用 strstr 之前检查 chunk.memory 是否为 NULL，确保调用的安全性。
        // 检查响应内容以确定是否登录成功
        if (chunk.memory != NULL && strstr(chunk.memory, "\"result\":1") != NULL) { // 检查 JSON 响应中的 result 字段
            free(chunk.memory);                        // 释放内存
            curl_easy_cleanup(curl);                  // 清理 CURL 句柄
            return 1;                                 // 登录成功
        }
        else {
            if (chunk.memory != NULL) {
                printf("响应内容: %s\n", chunk.memory);   // 输出实际响应内容以便调试
                free(chunk.memory);                        // 释放内存
            }
            curl_easy_cleanup(curl);                  // 清理 CURL 句柄
            return 0;                                 // 登录失败
        }
    }
    curl_global_cleanup();                          // 清理全局 CURL 环境
    return 0;                                       // 登录失败
}

// 处理用户输入配置
void handleUserInputConfig(Config* config) {
    printf("请输入校园网账户（学号）: ");
    scanf("%s", config->username);                // 读取用户名

    printf("请输入登录密码: ");
    scanf("%s", config->password);                // 读取密码

    printf("请选择登录线路:\n");
    printf("1. 联不通\t");
    printf("2. 移不动\t");
    printf("3. 电不信\t");
    printf("4. 教职工\n");
    int choice;
    scanf("%d", &choice);                         // 读取用户选择

    // 根据用户选择设置登录线路
    switch (choice) {
    case 1:
        strcpy(config->line, "unicom");      // 联通
        break;
    case 2:
        strcpy(config->line, "cmcc");        // 移动
        break;
    case 3:
        strcpy(config->line, "aust");        // 电信
        break;
    case 4:
        strcpy(config->line, "jzg");         // 教职工
        break;
    default:
        printf("无效选择，默认选择联通\"\n");
        strcpy(config->line, "unicom");      // 默认线路为联通
        break;
    }

    // 是否设置为PC开机自动启动
    printf("是否设置为PC开机自动启动？(1=是，0=否): ");
    scanf("%d", &config->autoLogin);              // 读取是否自动登录

    saveConfig(config);                          // 保存配置文件
}

int main() {
    Config config;                             // 创建配置对象
    loadConfig(&config);                       // 从配置文件加载信息

    // 如果配置文件不存在，则提示用户输入信息
    if (strlen(config.username) == 0 || strlen(config.password) == 0) {
        handleUserInputConfig(&config);        // 处理用户输入配置
    }

    int attempts = 0;                          // 登录尝试计数
    while (attempts < MAX_ATTEMPTS) {          // 尝试登录，最多尝试 MAX_ATTEMPTS 次     
            printf("正在尝试登陆...\n");
        if (loginToCampusNetwork(&config)) {   // 登录校园网
            if (config.showSuccessMessage) {
                printf("登录成功！\n");         // 输出成功信息
            }
            break;                              // 成功则退出循环
        }
        else {
            printf("登录失败！\n");             // 输出失败信息
            attempts++;                          // 增加尝试计数
        }

        // 如果尝试次数已满，提示修改账户信息
        if (attempts >= MAX_ATTEMPTS) {
            printf("已尝试登录 %d 次失败!!\n是否需要修改账户信息？(1=是，0=否): ", MAX_ATTEMPTS);
            int modify;
            scanf("%d", &modify);                // 读取用户选择
            if (modify == 1) {
                handleUserInputConfig(&config);  // 处理用户输入配置
                attempts = 0;                     // 重置尝试计数
            }
            else {
                printf("程序结束。\n");           // 退出程序
                exit(0);
            }
        }
    }

    // 结束程序前的清理工作
    if (config.autoLogin == 0)
    {
        printf("程序结束，按任意键退出...\n");
        getchar();                                  // 等待用户输入
    }
    
    return 0;                                   // 返回 0，表示程序成功结束
}

