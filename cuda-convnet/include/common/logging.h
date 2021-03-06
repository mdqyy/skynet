#ifndef LEGION_LOGGING_H_
#define LEGION_LOGGING_H_

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include "common/strutil.h"

static inline uint64_t rdtsc() {
  uint32_t hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return (((uint64_t) hi) << 32) | ((uint64_t) lo);
}

static inline void breakpoint() {
  struct sigaction oldAct;
  struct sigaction newAct;
  newAct.sa_handler = SIG_IGN;
  sigaction(SIGTRAP, &newAct, &oldAct);
  raise(SIGTRAP);
  sigaction(SIGTRAP, &oldAct, NULL);
}

double Now();
std::string Hostname();
timeval timevalFromDouble(double t);
timespec timespecFromDouble(double t);

struct TimerBlock {
  double& t_;
  double start_;
  TimerBlock(double& t) : t_(t), start_(Now()) {}
  ~TimerBlock() {
    t_ += Now() - start_;
  }
};

void Sleep(double sleepTime);


enum LogLevel {
  kLogDebug = 0, kLogInfo = 1, kLogWarn = 2, kLogError = 3, kLogFatal = 4,
};

extern LogLevel currentLogLevel;
double get_processor_frequency();

#define EVERY_N(interval, operation)\
{ static int COUNT = 0;\
  if (COUNT++ % interval == 0) {\
    operation;\
  }\
}

#define START_PERIODIC(interval)\
{ static int64_t last = 0;\
  static int64_t cycles = (int64_t)(interval * get_processor_frequency());\
  static int COUNT = 0; \
  ++COUNT; \
  int64_t now = rdtsc(); \
  if (now - last > cycles) {\
    last = now;\
    COUNT = 0;

#define END_PERIODIC() } }

#define PERIODIC(interval, op)\
    START_PERIODIC(interval)\
    op;\
    END_PERIODIC()

void logAtLevel(LogLevel level, const char* file, int line, const char* fmt,
    ...);

#define Log_Debug(fmt, ...) logAtLevel(kLogDebug, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Log_Info(fmt, ...) logAtLevel(kLogInfo, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Log_Warn(fmt, ...) logAtLevel(kLogWarn, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Log_Error(fmt, ...) logAtLevel(kLogError, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Log_Fatal(fmt, ...) logAtLevel(kLogFatal, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define Log_Perror(fmt, ...)\
  Log_Warn("%s :: (System error: %s)", StringPrintf(fmt, ##__VA_ARGS__).c_str(), strerror(errno));

#define Log_PAssert(expr, fmt, ...)\
  if (!(expr)) { Log_Fatal("%s :: (System error: %s)", StringPrintf(fmt, ##__VA_ARGS__).c_str(), strerror(errno)); }

#define Log_Assert(expr, fmt, ...)\
  if(!(expr)) { Log_Fatal(fmt, ##__VA_ARGS__); }

#endif /* LEGION_LOGGING_H_ */
