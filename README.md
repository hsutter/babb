# babb: `bad_alloc` Behaving Badly

## Overview

This library provides a simple way to inject random `bad_alloc` failures into your application, so that you and the C++ standards committee can learn more about how well real-world code actually deals with `bad_alloc` today.

**We would appreciate you running this against your project and reporting the results via [this SurveyMonkey survey](https://www.surveymonkey.com/r/PJSZ2F9).** Thank you very much for your help in getting data to inform C++ standardization!


## Motivation

Many C++ projects are thought to be correct today for out-of-memory (OOM) conditions if a `bad_alloc` exception is thrown. However, `bad_alloc` is unlike all other exceptions in two ways:

1. It is pervasive: It can be encountered/emitted by the majority of functions, and so it generally cannot be tested using normal unit testing methods. Instead, OOM-testing requires techniques like fault injection (hence this library).

2. (Hypothesis) It requires careful handling: It cannot be assumed to be recoverable by running ordinary code, including to perform stack unwinding and call-site error handling using ordinary exception handling, because ordinary code frequently tries to allocate memory. Instead, OOM-hardened code must be written carefully to observe restrictions and special idioms.

This project exists to help validate or invalidate this hypothesis, and we appreciate your help in gathering current data.


## How to use this library to test your application

1. Insert a call to `babb::this_thread.inject_random_failure()` inside each of your custom allocation functions that are allowed to fail by throwing `bad_alloc`, including in any such replacement `operator new` and in any custom allocator's `::allocate` function you are using that can fail.

2. If your project doesn't already replace global `operator new`, then add `babb_globals.cpp` to your project, which contains replacements for the throwing global `new` functions that add  the above injection calls.

3. Optionally, insert a single call to `babb::shared.set_failure_profile(fail_once_per, max_run_length)` in your `main` function to control the frequency and clustering of failure injection. We suggest trying various values for these options.

    - `fail_once_per`: the average #allocation attempts before one fails (default: 100,000)

    - `max_run_length`: the maximum # consecutive failures in a cluster (default: 5)

Advanced per-thread options:

   - Each thread's failure frequency can be changed at any time by calling `babb::per_thread.set_failure_profile(fail_once_per, max_run_length)`.

   - To pause or resume failures on this thread, call `babb::per_thread.pause(true)` to pause, `(false)` to resume. This can be useful to work around calls to OOM-unsafe functions in third-party libraries (though if those are failing that's data too).

