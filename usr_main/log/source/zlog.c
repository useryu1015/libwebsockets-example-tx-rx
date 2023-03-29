#include <stdbool.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "zlog.h"

#ifdef __linux__
#include <pthread.h>
pthread_mutex_t _log_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

zlog_config_t* ZLOG_CONFIG_PTR;

static const char* _module_names[] = { "virtual"/*(1)*/, "schedule"/*(2)*/, "gprs"/*(4)*/, "realtime"/*(8)*/,
        "hisdata"/*(16)*/, "modbus"/*(32)*/, "dlt645"/*(64)*/, "bacnet"/*(128)*/, "opcua"/*(256)*/,
        "gdw376"/*(512)*/, "iec104"/*(1024)*/, "crypto"/*(2048)*/, "httpd"/*(4096)*/, "nrsec3000" /*(8192)*/,
		"services"/*(16384)*/, "modem"/*(32768)*/, "config"/*(65536)*/, "commons"/*(131072)*/, "watchdog" /*(262144)*/};

static _Bool is_filter_accept(const char *file, const char *func, int line, int level) {
	int module_bits;

	if (NULL == ZLOG_CONFIG_PTR)
		return true;

	if (level < ZLOG_CONFIG_PTR->level + 2)
		return false;

	if (ZLOG_CONFIG_PTR->start_line && line < ZLOG_CONFIG_PTR->start_line) {
		return false;
	}

	if (ZLOG_CONFIG_PTR->end_line && line > ZLOG_CONFIG_PTR->end_line) {
		return false;
	}

	module_bits = ZLOG_CONFIG_PTR->module_bits;
//	module_bits= 16;
	if (module_bits) {
		unsigned int i;

		for (i = 0; i < sizeof(_module_names) / sizeof(_module_names[0]); ++i) {
			if (module_bits & (1 << i)) {
				if (strstr(file, _module_names[i])) {
					return true;
				}

			}
		}

		return false;
	}

	if ('\0' == ZLOG_CONFIG_PTR->file[0] && '\0' == ZLOG_CONFIG_PTR->func[0]) {
		return true;
	} else if ('\0' != ZLOG_CONFIG_PTR->file[0] && strstr(file, ZLOG_CONFIG_PTR->file)) {
		return true;
	} else if ('\0' != ZLOG_CONFIG_PTR->func[0] && strstr(func, ZLOG_CONFIG_PTR->func)) {
		return true;
	} else {
		return false;
	}
}

static void _log(FILE* fp, const char *file, const char *func, int line, int level) {
	time_t sys_time;
	struct tm sys_tm;

	time(&sys_time);
	localtime_r(&sys_time, &sys_tm);

	const char* level_desc[] = { "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL" };
	fprintf(fp, "%02d:%02d:%02d %ld %s %20s:%-4d %24s: ", sys_tm.tm_hour, sys_tm.tm_min, sys_tm.tm_sec,
	        syscall(SYS_gettid),level_desc[level], file, line, func);
}

static const char* get_simple_file(const char* file) {
	const char* str = strrchr(file, '/');
	return str ? (++str) : file;
}

void zlog(const char *file, const char *func, int line, int level, const char *format, ...) {
	va_list args;

	const char* sfile = get_simple_file(file);

	if (is_filter_accept(file, func, line, level)) {
		FILE* fp = level < ZLOG_LEVEL_WARN ? stdout : stderr;

#ifdef __linux__
		pthread_mutex_lock(&_log_mutex);
#endif
		do {
			_log(fp, sfile, func, line, level);

			va_start(args, format);
			vfprintf(fp, format, args);
			va_end(args);

			fprintf(fp, "\n");
			fflush(fp);
		} while (0);
#ifdef __linux__
		pthread_mutex_unlock(&_log_mutex);
#endif
	}
}

void hlog(const char *file, const char *func, int line, int level, const void *buf, size_t buflen) {
	int i;

	const char* sfile = get_simple_file(file);

	if (is_filter_accept(file, func, line, level)) {
		FILE* fp = level < ZLOG_LEVEL_WARN ? stdout : stderr;

#ifdef __linux__
		pthread_mutex_lock(&_log_mutex);
#endif
		do {
			_log(fp, sfile, func, line, level);

			fprintf(fp, "[%d] - ", buflen);

			for (i = 0; i < buflen; ++i) {
				fprintf(fp, "%02x ", ((unsigned char*) buf)[i]);
			}

			fprintf(fp, "\n");
			fflush(fp);
		} while (0);
#ifdef __linux__
		pthread_mutex_unlock(&_log_mutex);
#endif
	}
}

void hflog(const char *file, const char *func, int line, int level, const void *buf, size_t buflen, int fd) {
	int i;

	const char* sfile = get_simple_file(file);

	if (is_filter_accept(file, func, line, level)) {
		FILE* fp = level < ZLOG_LEVEL_WARN ? stdout : stderr;

#ifdef __linux__
		pthread_mutex_lock(&_log_mutex);
#endif
		do {
			_log(fp, sfile, func, line, level);

			fprintf(fp, "fd=%d, ", fd);
			fprintf(fp, "[%d] - ", buflen);

			for (i = 0; i < buflen; ++i) {
				fprintf(fp, "%02x ", ((unsigned char*) buf)[i]);
			}

			fprintf(fp, "\n");
			fflush(fp);
		} while (0);
#ifdef __linux__
		pthread_mutex_unlock(&_log_mutex);
#endif
	}
}

void hslog(const char *file, const char *func, int line, int level, const char f, const void *buf, size_t buflen) {
	int i;

	//const char* sfile = get_simple_file(file);

	if (is_filter_accept(file, func, line, level)) {
		FILE* fp = level < ZLOG_LEVEL_WARN ? stdout : stderr;

#ifdef __linux__
		pthread_mutex_lock(&_log_mutex);
#endif
	if (1 == buflen && 0xaa == ((unsigned char*) buf)[0])
		fprintf(fp, ".");
	else {
		fprintf(fp, "%c: ", f);
		for (i = 0; i < buflen; ++i) {
			fprintf(fp, "%02x ", ((unsigned char*) buf)[i]);
		}
		fprintf(fp, "\n");
		fflush(fp);
	}
#ifdef __linux__
		pthread_mutex_unlock(&_log_mutex);
#endif
	}
}



void hzlog(const char *file, const char *func, int line, int level, const char *str, const void *buf, size_t buflen) {
	int i;

	const char* sfile = get_simple_file(file);

	if (is_filter_accept(file, func, line, level)) {
		FILE* fp = level < ZLOG_LEVEL_WARN ? stdout : stderr;

#ifdef __linux__
		pthread_mutex_lock(&_log_mutex);
#endif
		do {
			_log(fp, sfile, func, line, level);

			fprintf(fp, "%s-", str);
			fprintf(fp, "dump[%d]:\n", buflen);

			for (i = 1; i <= buflen; ++i) {
				fprintf(fp, "%02x ", ((unsigned char*) buf)[i-1]);
                if(0==(i)%32){
			        fprintf(fp, "\n");
                }
			}

			fprintf(fp, "\n");
			fflush(fp);
		} while (0);
#ifdef __linux__
		pthread_mutex_unlock(&_log_mutex);
#endif
	}
}






