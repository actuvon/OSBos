#include "Arduino.h"
#include "OSBos.h"

OSBos::OSBos(uint8_t numThreads){
  _numberOfThreads = numThreads;
  _nextThreadIx = 0;

  _threads = new Thread*[_numberOfThreads];

  for(int i=0; i<_numberOfThreads; i++) _threads[i] = new Thread();
}

OSBos::~OSBos(){
  for(int i=0; i<_numberOfThreads; i++) delete _threads[i];
  delete _threads;
}

void OSBos::AddThread(Thread& thread){
  if(_nextThreadIx < _numberOfThreads){
    _threads[_nextThreadIx++] = &thread;
  } else Serial.println(F("\nOSBos: ERROR: Tried to add a thread, but OSBos is already full! Increase the maximum number of threads allowed in the OSBos constructor."));
}

void OSBos::StartTerminalAsyncTask(Thread& task, CallbackFunc callbackFunc){
  bool taskFound = false;

  for(int i=0; i<_numberOfThreads; i++){
    if(_threads[i] == &task){
      taskFound = true;
      break;
    }
  }

  if(taskFound){
    if(task.Active){
      Serial.println(F("\nOSBos: ERROR: Tried to start a task that is already running. This is not allowed."));
      return;
    }

    task.Active = true;
    task._startTime_ms = millis();

    task._completionCallback = callbackFunc;
  } else Serial.println(F("\nOSBos: ERROR: Tried to start a terminal async task that was not already registered as a thread. Tasks need to be set up as normal threads with Active=false before they can be started."));
}

void OSBos::RunKernel(){
  unsigned long now = millis();
  Thread* t;

  for(int i=0; i<_numberOfThreads; i++){
    t = _threads[i]; 

    if(t->Active){
      if((unsigned long)(now - t->_lastExecuted_ms) >= t->ReadyPeriod_ms){ // The static cast here is apparently needed to tell some compilers to handle the rollover correctly.

        if(t->DeadlineAlarmOn && (unsigned long)(now - t->_lastExecuted_ms) > t->DeadlinePeriod_ms){
          if(t->DeadlineAlarm == nullptr) Serial.print(F("\nOSBos: ERROR: A task triggered a deadline alarm, but no deadline alarm function was registered."));
          else t->DeadlineAlarm();
        }
        t->_lastExecuted_ms = now;

        unsigned long preRun_us = 0;
        if(t->RuntimeMonitoringOn) preRun_us = micros();

        if(t->RootMethod == nullptr) Serial.println(F("\nOSBos: Thread not assigned."));
        else{
          int8_t returnCode = t->RootMethod();

          if(t->IsTerminalTask){
            if(t->EnableCancellationTimer && (unsigned long)(now - t->_startTime_ms) > t->CancellationTime_ms) returnCode = -1; // A return code of -1 means that the task was cancelled by OSBos.

            if(returnCode != 0){ // A non-zero return code means to stop the task.
              t->Active = false;

              if(t->_completionCallback != nullptr) t->_completionCallback(returnCode);
            }
          }
        }

        if(t->RuntimeMonitoringOn) t->_lastRuntime_us = micros() - preRun_us;

        if(t->RuntimeAlarmOn && t->_lastRuntime_us > t->RuntimeAlarm_us){
          if(t->RuntimeAlarm == nullptr){
            Serial.print(F("\nOSBos: ERROR: A task triggered a runtime alarm, but no runteime alarm function was registered."));
          } 
          else t->RuntimeAlarm();
        }
      }
    }
  }
}