# OSBos
A class to quickly set up a trivial parallel processing framework. dirt. simple. with millis().
This library glues together a bunch of asynchronous threads like OSB plywood is made from gluing together a bunch of wood chips.

This library is an effective way of letting your Arduino rub its belly and tap its head at the same time. It doesn't use any rigid hardware timing or any of that strict deterministic stuff. It's a more relaxed way of just letting the pieces of your program do their thing without fussing about too much.

You start by making yourself a 'kernel' ...
```cpp
#include <OSBos.h>

OSBos kernel(1); // Set up a kernel capable of handling just 1 thread
```

Then set up the functions you want to run in parallel ...
```cpp
int8_t th_Blink(){
	static bool currentState = false; // static variables are your friend here. static means that this variable will be false the first time this function is called, but the next time this function is called, it will remember its old value.

	currentState = !currentState;

	digitalWrite(LED_BUILTIN, currentState);

	return 0; // Tell OSBos that all is well, and we don't want to stop running
}
```

Then build yourself some 'threads' ...
```cpp
Thread BlinkThread(th_Blink, 1000); // Set up a BlinkThread to run th_Blink() every 1000 milliseconds
```

Then add the threads to the kernel in your setup() ...
```cpp
kernel.AddThread(BlinkThread);
```

Then in your loop(), just tell OSBos to do its thing ...
```cpp
kernel.RunKernel();
```

If you are new to multithreading and asynchronous programming, there will probably be a little bit of a learning curve, but just spend some time playing with the examples, and maybe do a little reading on "cooperative multithreading".

OSBos was originally designed to juggle threads that all run forever in the main loop, but now it also supports "Terminal Async Tasks", which are designed to run for a little while and then trigger a callback function set by the user.