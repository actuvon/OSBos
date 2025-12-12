/* OSBos.h

license: MIT - 2025 - actuvon

Description: A class to quickly set up a trivial polling-based, non-deterministic
             parallel processing framework. dirt. simple. with millis().
 
             This library glues together a bunch of asynchronous threads like OSB plywood 
             is made from gluing together a bunch of wood chips.

Initial Draft: 2020

Useage: Check the README and examples on https://github.com/actuvon/OSBos

*/

#ifndef OSBos_h
#define OSBos_h

#include<Arduino.h> 
#include<functional>  // standard c++ library

using AsyncFunc = std::function<int8_t(void)>;      // A thread should return 0 while it's running, or 1 when it has finished successfully. A thread can return negative numbers to indicate error codes. Any non-zero return will cause a thread to be terminated.
using CallbackFunc = std::function<void(int8_t)>;
using AlarmFunc = std::function<void(void)>;

typedef struct Thread {
public:
  bool Active;                      // The kernal will ignore any threads with this set to false
  AsyncFunc RootMethod;             // The main method for the thread
  unsigned int ReadyPeriod_ms;      // The time that the kernel will wait between calls to this method

  
  bool IsTerminalTask;               // True if the task is designed to do its job then terminate. False for tasks that keep going forever.
  bool EnableCancellationTimer;      // For terminal tasks: If true, the task will cancel itself if it runs longer than its cancellation timer.
  unsigned long CancellationTime_ms; // For terminal tasks: If the task runs longer than this, cancel it. Requires enabling the cancellation timer.

  bool DeadlineAlarmOn;             // Turn this on to notify if a task misses a deadline
  unsigned int DeadlinePeriod_ms;   // The maximum amount of time allowed between calls to this method, before triggering an alarm
  AlarmFunc DeadlineAlarm;          // Executed any time a deadline is missed

  bool RuntimeMonitoringOn;         // Inefficient, but you can use this to check and see how long tasks run for in microseconds, and use the runtime alarm
  bool RuntimeAlarmOn;              // Turn this on to notify if a task ran too long. Requires turning on runtime monitoring.
  unsigned long RuntimeAlarm_us;    // If the thread runs longer than this, it will trigger an alarm
  AlarmFunc RuntimeAlarm;           // Executed any time a method runs longer than expected


  Thread() :
    Active(false),
    RootMethod(nullptr),
    ReadyPeriod_ms(0),

    IsTerminalTask(false),
    EnableCancellationTimer(false),
    CancellationTime_ms(0),

    DeadlineAlarmOn(false),
    DeadlinePeriod_ms(0),
    DeadlineAlarm(nullptr),

    RuntimeMonitoringOn(false),
    RuntimeAlarmOn(false),
    RuntimeAlarm_us(0),
    RuntimeAlarm(nullptr),

    _completionCallback(nullptr),
    _startTime_ms(0),
    _lastExecuted_ms(0),
    _lastRuntime_us(0)
  {}

  Thread(AsyncFunc rootMethod, unsigned int readyPeriod_ms) : Thread()
  {
    this->RootMethod = rootMethod;
    this->ReadyPeriod_ms = readyPeriod_ms;
    this->Active = true;
  }

private:
  friend class OSBos; // Apparently classes can have friends?!?! This brings me joy. Lets OSBos access the completion callback.

  CallbackFunc _completionCallback; //  For terminal tasks: executed once a terminal task finishes
  unsigned long _startTime_ms;        // For terminal tasks: helps us see how long they have been running.

  unsigned long _lastExecuted_ms;   // The millis time that the thread was last run
  unsigned long _lastRuntime_us;    // The duration that the thread hogged the CPU when last called
};

// A helper function to easily generate new terminal tasks
Thread NewTerminalTask(AsyncFunc rootMethod, unsigned int readyPeriod_ms);

class OSBos {
public:
  OSBos(uint8_t numThreads);
  ~OSBos();

  void RunKernel(); // Monitor and run active threads

  void AddThread(Thread& thread);

  /* TerminalAsyncTasks are designed to have a non-blocking RootMethod() that gets called
     repeatedly until it returns a non-zero value to indicate that it's done.
     Terminal tasks get set up like normal threads but should have Active = false. 
     To start a terminal task, we call this function. We pass in a callback function (usually
     set up as a lambda) to be called when the task eventually returns a non-zero value. */
  void StartTerminalAsyncTask(Thread& task, CallbackFunc callbackFunc);

    // You aren't supposed to copy or move this class - the dynamic memory doesn't handle that.
    OSBos(const OSBos&) = delete;
    OSBos& operator=(const OSBos&) = delete;
    OSBos(OSBos&&) = delete;
    OSBos& operator=(OSBos&&) = delete;

private:
  Thread** _threads; // A pointer to an array of threads

  uint8_t _nextThreadIx; // Helps me make sure I don't overwrite anything.

  unsigned char _numberOfThreads; // The maximum number of threads that can be managed with this OSBos instance
};

#endif