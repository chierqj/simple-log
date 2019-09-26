/*
 * MIT License
 *
 * Copyright (c) 2019 YangWuuu
 * https://github.com/YangWuuu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LOG_H
#define LOG_H

#define USE_LOG
#ifdef USE_LOG

#include <cstdio>
#include <ctime>
#include <mutex>

enum {
    LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL
};

typedef void (*log_LockFn)(void *udata, int lock);

static struct {
    void *udata;
    log_LockFn lock;
    FILE *fp;
    int level;
    int quiet;
} L;

std::mutex global_log_mutex;

static const char *level_names[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

static void lock() {
    if (L.lock) {
        L.lock(L.udata, 1);
    }
}

static void unlock() {
    if (L.lock) {
        L.lock(L.udata, 0);
    }
}

void log_set_udata(void *udata) {
    L.udata = udata;
}

void log_set_lock(log_LockFn fn) {
    L.lock = fn;
}

void log_set_fp(FILE *fp) {
    L.fp = fp;
}

void log_set_level(int level) {
    L.level = level;
}

void log_set_quiet(int enable) {
    L.quiet = enable ? 1 : 0;
}

void log_log(int level, const char *file, int line, const char *fmt, ...) {
    if (level < L.level) {
        return;
    }

    using namespace std::chrono;
    time_point<high_resolution_clock> clock_now = high_resolution_clock::now();
    {
        std::unique_lock<std::mutex> lock_global(global_log_mutex);
        /* Acquire lock */
        lock();

        /* Get current time */
        time_t t = time(nullptr);
        struct tm *lt = localtime(&t);

        /* Log to stderr */
        if (!L.quiet) {
            char buf[16];
            buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#ifdef LOG_USE_COLOR
            fprintf(
                    stderr, "%s %s%-5s\x1b[0m \x1b[90m(ts: %.6lf) %s:%d:\x1b[0m ",
                    buf, level_colors[level], level_names[level],
                    duration_cast<nanoseconds>(clock_now.time_since_epoch()).count() / 1e9, file, line);
#else
            fprintf(stderr, "%s %-5s (ts: %.6lf) %s:%d: ", buf, level_names[level],
                    duration_cast<nanoseconds>(clock_now.time_since_epoch()).count() / 1e9, file, line);
#endif
            fprintf(stderr, "\n");
        }

        /* Log to file */
        if (L.fp) {
            char buf[32];
            buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
            fprintf(L.fp, "%s %-5s (ts: %.6lf) %s:%d: ", buf, level_names[level],
                    duration_cast<nanoseconds>(clock_now.time_since_epoch()).count() / 1000000000.0, file, line);
            fprintf(L.fp, "\n");
            fflush(L.fp);
        }

        /* Release lock */
        unlock();
    }
}

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#else //use log
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...)
#define log_fatal(...)

#define log_set_fp(...)
#define log_set_quiet(...)
#endif //use log

#endif
