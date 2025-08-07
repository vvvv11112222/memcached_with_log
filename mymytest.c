#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libmemcached/memcached.h>

// 配置参数
#define MEMCACHED_HOST "localhost"
#define MEMCACHED_PORT 11211
#define TOTAL_COUNT 1000  // 要写入的键值对总数

int main() {
    // 1. 创建memcached实例
    memcached_st *memc = memcached_create(NULL);
    if (memc == NULL) {
        fprintf(stderr, "错误：无法创建memcached实例\n");
        return EXIT_FAILURE;
    }

    // 2. 添加服务器地址
    memcached_server_st *servers = NULL;
    servers = memcached_server_list_append(servers, MEMCACHED_HOST, MEMCACHED_PORT, NULL);
    if (servers == NULL) {
        fprintf(stderr, "错误：无法添加服务器地址\n");
        memcached_free(memc);
        return EXIT_FAILURE;
    }

    // 3. 连接到服务器
    memcached_return_t rc = memcached_server_push(memc, servers);
    memcached_server_list_free(servers);  // 释放服务器列表
    if (rc != MEMCACHED_SUCCESS) {
        fprintf(stderr, "错误：连接服务器失败 - %s\n", memcached_strerror(memc, rc));
        memcached_free(memc);
        return EXIT_FAILURE;
    }

    // 4. 写入1000个键值对
    int success = 0;
    char key[20];    // 存储键名
    char value[100]; // 存储值内容

    for (int i = 0; i < TOTAL_COUNT; i++) {
        // 生成键值对
        snprintf(key, sizeof(key), "test_key_%04d", i);
        snprintf(value, sizeof(value), "这是第%d个测试值", i);

        // 写入数据（永不过期）
        rc = memcached_set(
            memc,
            key, strlen(key),    // 键
            value, strlen(value),// 值
            0,                   // 过期时间（0=永不过期）
            0                    // 标志位
        );

        if (rc == MEMCACHED_SUCCESS) {
            success++;
        } else {
            fprintf(stderr, "警告：写入键 %s 失败 - %s\n", 
                    key, memcached_strerror(memc, rc));
        }

        // 每100个输出一次进度
        if ((i + 1) % 100 == 0) {
            printf("已完成 %d/%d 个键值对写入\n", i + 1, TOTAL_COUNT);
        }
    }

    // 5. 输出统计结果
    printf("\n操作完成：成功写入 %d 个，失败 %d 个\n", 
           success, TOTAL_COUNT - success);

    // 6. 清理资源
    memcached_free(memc);
    return EXIT_SUCCESS;
}
