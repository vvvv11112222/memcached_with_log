#include "logmc.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// 全局变量定义（静态初始化互斥锁，避免重复初始化）
FILE *logmc_file = NULL;                     // 日志文件指针（初始化为NULL）
pthread_mutex_t logmc_mutex = PTHREAD_MUTEX_INITIALIZER;  // 静态初始化互斥锁
logmc_level_t logmc_current_level = LOGMC_LEVEL_INFO;      // 默认日志级别为INFO

/**
 * 辅助函数：将日志级别转换为字符串（提升日志可读性）
 * @param level 日志级别枚举值
 * @return 级别对应的字符串（如"INFO"、"ERROR"）
 */
static const char* logmc_level_to_str(logmc_level_t level) {
    switch (level) {
        case LOGMC_LEVEL_NONE:  return "NONE";
        case LOGMC_LEVEL_ERROR: return "ERROR";
        case LOGMC_LEVEL_INFO:  return "INFO";
        case LOGMC_LEVEL_DEBUG: return "DEBUG";
        default:                return "UNKNOWN";
    }
}

/**
 * 初始化日志模块
 * @param log_path 日志文件路径（NULL则输出到stdout）
 * @param level 日志级别（LOGMC_LEVEL_xxx）
 * @return 0成功，-1失败（失败时已清理部分资源）
 */
int logmc_init(const char *log_path, logmc_level_t level) {
    // 1. 验证日志级别有效性
    if (level < LOGMC_LEVEL_NONE || level > LOGMC_LEVEL_DEBUG) {
        fprintf(stderr, "logmc_init: 无效的日志级别 %d（范围：0-%d）\n", 
                level, LOGMC_LEVEL_DEBUG);
        return -1;
    }

    // 2. 打开日志文件（优先使用用户指定路径，否则输出到控制台）
    if (log_path != NULL) {
        logmc_file = fopen(log_path, "a");  // 追加模式（保留历史日志）
        if (logmc_file == NULL) {
            fprintf(stderr, "logmc_init: 打开日志文件失败 '%s'：%s\n", 
                    log_path, strerror(errno));
            return -1;
        }
    } else {
        logmc_file = stdout;  // 默认为标准输出（方便调试）
    }

    // 3. 设置日志级别（必须在文件打开后，否则日志输出可能失败）
    logmc_current_level = level;

    // 4. 输出初始化成功日志（首次日志输出，验证基本功能）
    LOGMC_INFO("日志模块初始化完成 [路径: %s, 级别: %s(%d)]",
               (log_path ? log_path : "stdout"),
               logmc_level_to_str(level), level);

    return 0;
}

/**
 * 清理日志模块资源（优化后）
 */
void logmc_cleanup(void) {
    // 移除加锁操作：清理阶段无并发，且锁可能已被销毁
    if (logmc_file != NULL && logmc_file != stdout) {
        LOGMC_INFO("日志模块开始清理资源");  // 最后一条日志输出
        fclose(logmc_file);
        logmc_file = NULL;
    }

    // 销毁互斥锁（仅在未销毁时执行，避免重复销毁）
    // 注意：pthread_mutex_destroy 对已销毁的锁操作是未定义行为，需确保只调用一次
    static int is_destroyed = 0;
    if (!is_destroyed) {
        pthread_mutex_destroy(&logmc_mutex);
        is_destroyed = 1;
    }

    logmc_current_level = LOGMC_LEVEL_NONE;
}
/**
 * 动态调整日志级别（线程安全）
 * @param level 新日志级别（LOGMC_LEVEL_xxx）
 */
void logmc_set_level(logmc_level_t level) {
    // 验证级别有效性
    if (level < LOGMC_LEVEL_NONE || level > LOGMC_LEVEL_DEBUG) {
        // 即使级别无效，也要加锁输出错误日志（避免并发问题）
        LOGMC_ERROR("尝试设置无效日志级别 %d（范围：0-%d）", 
                   level, LOGMC_LEVEL_DEBUG);
        return;
    }

    // 加锁保护全局变量修改
    pthread_mutex_lock(&logmc_mutex);
    logmc_level_t old_level = logmc_current_level;
    logmc_current_level = level;
    pthread_mutex_unlock(&logmc_mutex);

    // 输出级别变更日志（包含新旧级别对比）
    LOGMC_INFO("日志级别已更新 [旧: %s(%d) → 新: %s(%d)]",
               logmc_level_to_str(old_level), old_level,
               logmc_level_to_str(level), level);
}