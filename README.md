# babb: `bad_alloc` Behaving Badly

## Overview

This library provides a simple way to inject random `bad_alloc` failures into your application, so that you and the C++ standards committee can learn more about how well real-world code actually deals with `bad_alloc` today.

## How to use this library to test your application

1. Insert a call to `babb::inject_random_failure()` inside each of your custom allocation functions that are allowed to fail by throwing `bad_alloc`, including in any such replacement `operator new` and in any custom allocator's `::allocate` function you are using that can fail.

2. If your project doesn't already replace global `operator new`, then add `babb_globals.cpp` to your project, which contains replacements for the throwing global `new` functions that add  the above injection calls.

3. Optionally, insert a single call to `babb::init(pct, run)` in your `main` function to control the frequency and clustering of failure injection. We suggest trying various values for these options.

    - `pct`: the average percentage of allocation attempts that will fail, from `0.0` to `100.0` (default: `0.01`)

    - `run`: the average run length of consecutive failures to simulate a cluster of failures in low-memory conditions, where the failures after the first in each cluster will be likely to get injected into the unwinding/handling code paths (default: `5`)


## Motivation

Many C++ projects are throught to be correct today for out-of-memory (OOM) conditions if a `bad_alloc` exception is thrown.

In practice, what we frequently discover is that if we audit such code we discover the code is actually broken (would not work), and that if we actively inject `bad_alloc` failures the program crashes. This is not for lack of trying to write the code well, but rather because `bad_alloc` is unlike all other exceptions in two ways:

1. It is pervasive: It can be encountered/emitted by the majority of functions, and so it generally cannot be tested using normal unit testing methods. Instead, OOM-testing requires techniques like fault injection (hence this library).

2. It requires careful handling: It cannot be assumed to be recoverable by running ordinary code, including to perform stack unwinding and call-site error handling, because ordinary code frequently tries to allocate memory. Instead, OOM-hardened code must be written carefully to observe restrictions and special idioms.

We would appreciate you running this against your project and reporting the results to [TODO: email] or by opening an Issue on this repo.

Thank you very much for your help in getting data to inform C++ standardization!
