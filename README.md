# babb: `bad_alloc` Behaving Badly

## Overview

This library provides a simple way to inject random `bad_alloc` failures into your application, so that you and the C++ standards committee can learn more about how well real-world code actually deals with `bad_alloc` today. It works on C++11 and higher.

**Please try this in your project(s) and report the results via [this SurveyMonkey survey](https://www.surveymonkey.com/r/PJSZ2F9).** Thank you very much for your help in getting data to inform C++ standardization!


## Motivation

Many C++ projects are thought to be correct today for out-of-memory (OOM) conditions if a `bad_alloc` exception is thrown. However, `bad_alloc` is unlike all other exceptions in two ways:

1. It is pervasive: It can be encountered/emitted by the majority of functions, and so it generally cannot be tested using normal unit testing methods. Instead, OOM-testing requires techniques like fault injection (hence this library).

2. (Hypothesis) It requires careful handling: It cannot be assumed to be recoverable by running ordinary code, including to perform stack unwinding and call-site error handling using ordinary exception handling, because ordinary code frequently tries to allocate memory. Instead, OOM-hardened code must be written carefully to observe restrictions and special idioms.

This project exists to help validate or invalidate this hypothesis, and we appreciate your help in gathering current data.


## How to use this library to test your application

### To install failure injection

For each of your custom allocation functions, including in any of your replacement `operator new` function and in any custom allocator's `::allocate` function:

- If the function signals allocation failure by throwing `bad_alloc`, insert a call to `babb::this_thread.inject_random_failure()`.

- If the function signals allocation failure by throwing some other exception type `CustomOOMType`, insert a call to `babb::this_thread.inject_random_failure<CustomOOMType>()`.

- If the function signals allocation failure by returning null, insert: `if (babb::this_thread.should_inject_random_failure()) return nullptr;`


### Convenience helper file for global installation

For convenience, if your project **does not** already replace global `operator new`: You can add `new_replacements.cpp` to your project, which contains replacements for the user-replaceable global `new` functions that add the above injection calls the standard global operators, including those that throw `bad_alloc` and those that return `nullptr`.


### Options

We suggest trying various values for these options:

   - Globally, you can call `babb::shared.set_failure_profile(fail_once_per, max_run_length)` to control the frequency and clustering of failure injection. This is also used as the default for each new thread's failure frequency and clustering whenever the new thread's `thread_local` storage is created.

      - `fail_once_per`: the average #allocation attempts before one fails (default: 100,000)

      - `max_run_length`: the maximum # consecutive failures in a cluster (default: 5)
   
   - In each thread, you can call `babb::this_thread.set_failure_profile(fail_once_per, max_run_length)` to change these frequencies, or call `babb::this_thread.pause(true)` to pause, or `(false)` to resume, all failure injection on this thread. Pausing can be useful to work around calls to OOM-unsafe functions in third-party libraries (though if those are failing that's data too).

