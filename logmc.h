#ifndef LOGMC_H
#define LOGMC_H

#include <stdio.h>
#include <time.h>
#include <pthread.h>

// 日志级别定义
typedef enum {
    LOGMC_LEVEL_NONE = 0,   // 不输出任何日志
    LOGMC_LEVEL_ERROR = 1,  // 错误日志
    LOGMC_LEVEL_INFO = 2,   // 信息日志
    LOGMC_LEVEL_DEBUG = 3   // 调试日志
} logmc_level_t;

// 日志模块全局变量（外部声明）
extern FILE *logmc_file;
extern pthread_mutex_t logmc_mutex;
extern logmc_level_t logmc_current_level;

// 日志模块初始化函数
// 参数: log_path - 日志文件路径，NULL则输出到stdout
//       level - 初始日志级别
// 返回值: 0成功，-1失败
int logmc_init(const char *log_path, logmc_level_t level);

// 日志模块清理函数
void logmc_cleanup(void);

// 设置日志级别
void logmc_set_level(logmc_level_t level);

// 日志输出宏定义
#define LOGMC_ERROR(fmt, ...) \
    do { \
        if (logmc_current_level >= LOGMC_LEVEL_ERROR) { \
            time_t now = time(NULL); \
            struct tm local_tm; \
            localtime_r(&now, &local_tm); \
            pthread_mutex_lock(&logmc_mutex); \
            fprintf(logmc_file, "[%04d-%02d-%02d %02d:%02d:%02d] [ERROR] " fmt "\n", \
                local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, \
                local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, ##__VA_ARGS__); \
            fflush(logmc_file); \
            pthread_mutex_unlock(&logmc_mutex); \
        } \
    } while (0)

#define LOGMC_INFO(fmt, ...) \
    do { \
        if (logmc_current_level >= LOGMC_LEVEL_INFO) { \
            time_t now = time(NULL); \
            struct tm local_tm; \
            localtime_r(&now, &local_tm); \
            pthread_mutex_lock(&logmc_mutex); \
            fprintf(logmc_file, "[%04d-%02d-%02d %02d:%02d:%02d] [INFO] " fmt "\n", \
                local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, \
                local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, ##__VA_ARGS__); \
            fflush(logmc_file); \
            pthread_mutex_unlock(&logmc_mutex); \
        } \
    } while (0)

#define LOGMC_DEBUG(fmt, ...) \
    do { \
        if (logmc_current_level >= LOGMC_LEVEL_DEBUG) { \
            time_t now = time(NULL); \
            struct tm local_tm; \
            localtime_r(&now, &local_tm); \
            pthread_mutex_lock(&logmc_mutex); \
            fprintf(logmc_file, "[%04d-%02d-%02d %02d:%02d:%02d] [DEBUG] " fmt "\n", \
                local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday, \
                local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, ##__VA_ARGS__); \
            fflush(logmc_file); \
            pthread_mutex_unlock(&logmc_mutex); \
        } \
    } while (0)

#endif  // LOGMC_H

