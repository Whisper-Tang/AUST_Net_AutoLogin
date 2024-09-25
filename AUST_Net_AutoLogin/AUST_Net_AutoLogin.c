#define _CRT_SECURE_NO_WARNINGS //��scanf�������ò�����
//#pragma warning(disable:4996)
#pragma warning(disable:6031)//���þ�����ʾ:" C6031 ����ֵ�����ԣ�"scanf" "#
#define _CRT_SECURE_NO_WARNINGS //��scanf�������ò�����
//#pragma warning(disable:4996)
#pragma warning(disable:6031)//���þ�����ʾ:" C6031 ����ֵ�����ԣ�"scanf" "#


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define CONFIG_FILE "config.txt"  // �����ļ�������
#define MAX_ATTEMPTS 3             // ����Ե�¼����

// �������ýṹ�壬���ڴ洢�û���Ϣ������
typedef struct {
    char username[50];            // �û���
    char password[50];            // ����
    char line[10];                // ��¼��·
    int showSuccessMessage;       // ��¼�ɹ�ʱ�Ƿ���ʾ��ʾ
    int autoLogin;                // �Ƿ񿪻����������Զ���¼
} Config;

// ����һ���ṹ�壬���ڴ洢��������Ӧ����
struct MemoryStruct {
    char* memory;                 // �洢��Ӧ���ݵ�ָ��
    size_t size;                  // ��Ӧ���ݵĴ�С
};

//// �ص����������ڴ�����������ص�����
//static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, struct MemoryStruct* userp) {
//    size_t realsize = size * nmemb;  // ����ʵ�����ݴ�С
//    userp->memory = realloc(userp->memory, userp->size + realsize + 1); // ��չ�ڴ�
//    if (userp->memory == NULL) {      // ����ڴ�����Ƿ�ɹ�
//        printf("�����ڴ�\n");         // ���������Ϣ
//        return 0;                     // ���� 0 ��ʾ�ڴ����ʧ��
//    }
//    memcpy(&(userp->memory[userp->size]), contents, realsize); // �����յ������ݸ��Ƶ��ڴ�
//    userp->size += realsize;          // �������ݴ�С
//    userp->memory[userp->size] = '\0'; // ����ַ���������
//    return realsize;                  // ����ʵ�ʴ�������ݴ�С
//}
// ʹ���м�ָ�룺�� realloc ֮��ʹ��һ���м�ָ�� new_memory ���������Ƿ�ɹ���
// �ص����������ڴ�����������ص�����
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, struct MemoryStruct* userp) {
    size_t realsize = size * nmemb;  // ����ʵ�����ݴ�С
    char* new_memory = realloc(userp->memory, userp->size + realsize + 1); // ��չ�ڴ�
    if (new_memory == NULL) {         // ����ڴ�����Ƿ�ɹ�
        printf("�����ڴ�\n");         // ���������Ϣ
        return 0;                     // ���� 0 ��ʾ�ڴ����ʧ��
    }
    userp->memory = new_memory;       // ��ȫ�ظ���ָ��
    memcpy(&(userp->memory[userp->size]), contents, realsize); // �����յ������ݸ��Ƶ��ڴ�
    userp->size += realsize;          // �������ݴ�С
    userp->memory[userp->size] = '\0'; // ����ַ���������
    return realsize;                  // ����ʵ�ʴ�������ݴ�С
}

// ���������ļ�
void loadConfig(Config* config) {
    FILE* file = fopen(CONFIG_FILE, "r");      // �������ļ�
    if (file) {                                 // ����ļ�����
        fscanf(file, "%s %s %s %d", config->username,
            config->password, config->line,
            &config->autoLogin);                // ��ȡ����
        fclose(file);                           // �ر��ļ�
        config->showSuccessMessage = 1;        // Ĭ����ʾ�ɹ���ʾ
    }
    else {
        // ��������ļ������ڣ�����Ĭ��ֵ
        strcpy(config->username, "");           // Ĭ���û���Ϊ��
        strcpy(config->password, "");           // Ĭ������Ϊ��
        strcpy(config->line, "unicom");         // Ĭ�ϵ�¼��·Ϊ��ͨ
        config->autoLogin = 0;                  // Ĭ�ϲ�����������
    }
}

// ���������ļ�
void saveConfig(Config* config) {
    FILE* file = fopen(CONFIG_FILE, "w");      // �������ļ�����д��
    fprintf(file, "%s\n%s\n%s\n%d\n\n", config->username,
        config->password, config->line,
        config->autoLogin);                     // ��������

    // д�������ļ�˵��
    fprintf(file, "//�����ļ�˵����\n");
    fprintf(file, "//��1�У�У԰���˻���ѧ�ţ�\n");
    fprintf(file, "//��2�У���¼����\n");
    fprintf(file, "//��3�У���¼��·����¼��·��дӢ��Сд����ͨ��unicom���ƶ���cmcc�����ţ�aust����ְ����jzg��\n");
    fprintf(file, "//��4�У��Ƿ񿪻��Զ���¼��1 = �ǣ�0 = ��\n");
    fprintf(file, "//�����ÿ������������½�ɹ��������κ���ʾ\n");
    fprintf(file, "//���ʵ����д���ã��Ǿ�ɾ�������ļ�ֱ�����е�¼���������Լ�����(�s�F����)�s��ߩ���\n");
    fclose(file);                               // �ر��ļ�
}

// ��¼У԰���ĺ���
int loginToCampusNetwork(Config* config) {
    CURL* curl;                                 // ���� CURL ���
    CURLcode res;                               // CURL ����ֵ
    char url[256];                              // �洢���� URL ���ַ���

    // ��ϵ�¼����� URL��ʹ���û�������¼��·������
    snprintf(url, sizeof(url), "http://10.255.0.19/drcom/login?callback=dr1003&DDDDD=%s@%s&upass=%s&0MKKey=123456&R1=0&R3=0&R6=0&para=00&v6ip=&v=2172",
        config->username, config->line, config->password);

    curl_global_init(CURL_GLOBAL_DEFAULT);      // ��ʼ�� CURL ��
    curl = curl_easy_init();                     // ��ʼ�� CURL ���
    if (curl) {
        struct MemoryStruct chunk;               // ���� MemoryStruct �Դ洢��Ӧ
        chunk.memory = malloc(1);                // ��ʼ�����ڴ�
        chunk.size = 0;                          // ��ʼ����СΪ 0

        curl_easy_setopt(curl, CURLOPT_URL, url);             // ��������� URL
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // ����д�ص�����
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk); // ����д�����ݵĽṹ��

        res = curl_easy_perform(curl);              // ִ������
        if (res != CURLE_OK) {                       // ��������Ƿ�ɹ�
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res)); // ���������Ϣ
            free(chunk.memory);                       // �ͷ��ڴ�
            curl_easy_cleanup(curl);                 // ���� CURL ���
            return 0;                                // ��¼ʧ��
        }

        //// �����Ӧ������ȷ���Ƿ��¼�ɹ�
        //if (strstr(chunk.memory, "\"result\":1") != NULL) { // ��� JSON ��Ӧ�е� result �ֶ�
        //    free(chunk.memory);                        // �ͷ��ڴ�
        //    curl_easy_cleanup(curl);                  // ���� CURL ���
        //    return 1;                                 // ��¼�ɹ�
        //}
        //else {
        //    printf("��Ӧ����: %s\n", chunk.memory);   // ���ʵ����Ӧ�����Ա����
        //    free(chunk.memory);                        // �ͷ��ڴ�
        //    curl_easy_cleanup(curl);                  // ���� CURL ���
        //    return 0;                                 // ��¼ʧ��
        //}
        // ���ӿ�ָ���飺�ڵ��� strstr ֮ǰ��� chunk.memory �Ƿ�Ϊ NULL��ȷ�����õİ�ȫ�ԡ�
        // �����Ӧ������ȷ���Ƿ��¼�ɹ�
        if (chunk.memory != NULL && strstr(chunk.memory, "\"result\":1") != NULL) { // ��� JSON ��Ӧ�е� result �ֶ�
            free(chunk.memory);                        // �ͷ��ڴ�
            curl_easy_cleanup(curl);                  // ���� CURL ���
            return 1;                                 // ��¼�ɹ�
        }
        else {
            if (chunk.memory != NULL) {
                printf("��Ӧ����: %s\n", chunk.memory);   // ���ʵ����Ӧ�����Ա����
                free(chunk.memory);                        // �ͷ��ڴ�
            }
            curl_easy_cleanup(curl);                  // ���� CURL ���
            return 0;                                 // ��¼ʧ��
        }
    }
    curl_global_cleanup();                          // ����ȫ�� CURL ����
    return 0;                                       // ��¼ʧ��
}

// �����û���������
void handleUserInputConfig(Config* config) {
    printf("������У԰���˻���ѧ�ţ�: ");
    scanf("%s", config->username);                // ��ȡ�û���

    printf("�������¼����: ");
    scanf("%s", config->password);                // ��ȡ����

    printf("��ѡ���¼��·:\n");
    printf("1. ����ͨ\t");
    printf("2. �Ʋ���\t");
    printf("3. �粻��\t");
    printf("4. ��ְ��\n");
    int choice;
    scanf("%d", &choice);                         // ��ȡ�û�ѡ��

    // �����û�ѡ�����õ�¼��·
    switch (choice) {
    case 1:
        strcpy(config->line, "unicom");      // ��ͨ
        break;
    case 2:
        strcpy(config->line, "cmcc");        // �ƶ�
        break;
    case 3:
        strcpy(config->line, "aust");        // ����
        break;
    case 4:
        strcpy(config->line, "jzg");         // ��ְ��
        break;
    default:
        printf("��Чѡ��Ĭ��ѡ����ͨ\"\n");
        strcpy(config->line, "unicom");      // Ĭ����·Ϊ��ͨ
        break;
    }

    // �Ƿ�����ΪPC�����Զ�����
    printf("�Ƿ�����ΪPC�����Զ�������(1=�ǣ�0=��): ");
    scanf("%d", &config->autoLogin);              // ��ȡ�Ƿ��Զ���¼

    saveConfig(config);                          // ���������ļ�
}

int main() {
    Config config;                             // �������ö���
    loadConfig(&config);                       // �������ļ�������Ϣ

    // ��������ļ������ڣ�����ʾ�û�������Ϣ
    if (strlen(config.username) == 0 || strlen(config.password) == 0) {
        handleUserInputConfig(&config);        // �����û���������
    }

    int attempts = 0;                          // ��¼���Լ���
    while (attempts < MAX_ATTEMPTS) {          // ���Ե�¼����ೢ�� MAX_ATTEMPTS ��     
            printf("���ڳ��Ե�½...\n");
        if (loginToCampusNetwork(&config)) {   // ��¼У԰��
            if (config.showSuccessMessage) {
                printf("��¼�ɹ���\n");         // ����ɹ���Ϣ
            }
            break;                              // �ɹ����˳�ѭ��
        }
        else {
            printf("��¼ʧ�ܣ�\n");             // ���ʧ����Ϣ
            attempts++;                          // ���ӳ��Լ���
        }

        // ������Դ�����������ʾ�޸��˻���Ϣ
        if (attempts >= MAX_ATTEMPTS) {
            printf("�ѳ��Ե�¼ %d ��ʧ��!!\n�Ƿ���Ҫ�޸��˻���Ϣ��(1=�ǣ�0=��): ", MAX_ATTEMPTS);
            int modify;
            scanf("%d", &modify);                // ��ȡ�û�ѡ��
            if (modify == 1) {
                handleUserInputConfig(&config);  // �����û���������
                attempts = 0;                     // ���ó��Լ���
            }
            else {
                printf("���������\n");           // �˳�����
                exit(0);
            }
        }
    }

    // ��������ǰ��������
    if (config.autoLogin == 0)
    {
        printf("�����������������˳�...\n");
        getchar();                                  // �ȴ��û�����
    }
    
    return 0;                                   // ���� 0����ʾ����ɹ�����
}

