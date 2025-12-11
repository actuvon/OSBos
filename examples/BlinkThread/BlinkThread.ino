#include <OSBos.h>

OSBos kernel(1); // Set up a kernel capable of handling just 1 thread

// Write a function for a thread to call
int8_t th_Blink(){
  static bool currentState = false; // static variables are your friend here. static means that this variable will be false the first time this function is called, but the next time this function is called, it will remember its old value.

	currentState = !currentState;

	digitalWrite(LED_BUILTIN, currentState);

	return 0; // Tell OSBos that all is well, and we don't want to stop running
}

Thread BlinkThread(th_Blink, 1000); // Set up a BlinkThread to run th_Blink() every 1000 milliseconds

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  kernel.AddThread(BlinkThread);
}

void loop(){
  kernel.RunKernel();
}