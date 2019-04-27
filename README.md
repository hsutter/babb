# babb: `bad_alloc` Behaving Badly

## Overview

This library provides a simple way to inject random memory allocation failures into your application, so that you and the C++ standards committee can learn more about how well real-world code actually deals with allocation failure today. This library requires C++11 or higher.

**Please try this in your project(s) and report the results via [this SurveyMonkey survey](https://www.surveymonkey.com/r/PJSZ2F9).** Thank you very much for your help in getting data to inform C++ standardization!


## Motivation

Many C++ projects are thought to be correct today for memory allocation failure, particularly if it is reported by throwing an exception. However, allocation failure is unlike all other exceptions in two ways:

1. It is pervasive: It can be encountered/emitted by the majority of functions, and so it generally cannot be tested using normal unit testing methods. Instead, allocation failure testing requires techniques like fault injection (hence this library).

2. (Hypothesis) It requires careful handling: It cannot be assumed to be recoverable by running ordinary code, including to perform stack unwinding and call-site error handling using ordinary exception handling, because ordinary code frequently tries to allocate memory. Instead, allocation failure-hardened code must be written carefully to observe restrictions and special idioms.

This project exists to help validate or invalidate this hypothesis, and we appreciate your help in gathering current data.


## How to use this library to test your application

### To install failure injection

For each of your custom allocation functions, including in any of your replacement `operator new` function and in any custom allocator's `::allocate` function:

- If the function signals allocation failure by throwing `bad_alloc`, insert a call to `babb::this_thread.inject_random_failure()`.

- If the function signals allocation failure by throwing some other exception type `CustomType`, insert a call to `babb::this_thread.inject_random_failure<CustomType>()`.

- If the function signals allocation failure by returning null, insert: `if (babb::this_thread.should_inject_random_failure()) return nullptr;`


### Convenience helper file (if you don't already replace global `operator new`)

For convenience, if your project **does not** already replace global `operator new`, you can add `new_replacements.cpp` to your project. It contains replacements for the user-replaceable global `new` functions that add the above injection calls the standard global operators, including those that throw `bad_alloc` and those that return `nullptr`.


### Options

We suggest trying various values for these options:

   - Globally, you can call `babb::shared.set_failure_profile(fail_once_per, max_run_length)` to control the frequency and clustering of failure injection. This is also used as the default for each new thread.

      - `fail_once_per`: the average #allocation attempts before one fails (default: 100,000)

      - `max_run_length`: the maximum # consecutive failures in a cluster (default: 5)
   
   - In each thread, you can call `babb::this_thread.set_failure_profile(fail_once_per, max_run_length)` to change these frequencies, or call `babb::this_thread.pause(true)` to pause, or `(false)` to resume, all failure injection on this thread. Pausing can be useful to work around calls to allocation failure-unsafe functions in third-party libraries (though if those are failing that's data too).

   - For either `babb::shared` or `babb::this_thread`, you can use the RAII helper `babb::state_guard` to push/pop changes to the state. For example, you can create a local object using `babb::state_guard save(babb::this_thread);` and then make other changes, including pausing and nested state guards, and when the guard object is destroyed it will restore the original state as it was when the guard was created.
   This can be useful to suppress failure injection within a particular module (e.g., third-party or shared library) by wrapping all the library's entry points in a scope guard and then pausing failure injection. Because the scope guards can nest, this will be correct even if the module's entry point functions happen to invoke each other directly and so create nested guards.