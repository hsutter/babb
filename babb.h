
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2019 Herb Sutter. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////


#ifndef BABB_BABB_H
#define BABB_BABB_H

#include <memory>
#include <random>
#include <cassert>

namespace babb {

struct shared {
    inline static int fail_once_per  = 100000;	// avg #allocations between failures
    inline static int max_run_length = 5;	    // max #consecutive failures

    static auto invariant() { return fail_once_per > 0 && max_run_length >= 1; }

    //----------------------------------------------------------------------------
    //
    //	init: Initialize the failure injection frequency and distribution
    //
    //	Optionally put one call to this in your main().
    //  If you don't, the default is as if init(100000, 5).
    //
    //  fail_once_per:  avg #allocations between failures
    //  max_run_length: max #consecutive failures (once we have triggered a new one)
    //
    //----------------------------------------------------------------------------

    void init(int fail_once_per, int max_run_length) {
        babb::shared::fail_once_per = fail_once_per;
        babb::shared::max_run_length = max_run_length;
        assert(invariant());
    }
};


//----------------------------------------------------------------------------
//
//	inject_random_failure()
//
//  Put a call to this function inside each of your custom allocation functions
//  that could throw bad_alloc, including in any replacement operator new and
//  in any custom allocator's ::allocate function you are using that can fail.
//
//----------------------------------------------------------------------------

void inject_random_failure() {
    thread_local std::random_device rd;
    thread_local std::mt19937 mt(rd());
    thread_local std::uniform_real_distribution<double> dist(0., 100.);
    thread_local int run_in_progress = 0;

    auto trigger_a_new_run =
        [&]{ return dist(mt) < 1./shared::fail_once_per/shared::max_run_length; };

    auto invariant = 
        [&]{ return 0 <= run_in_progress && run_in_progress <= shared::max_run_length; };
    assert(invariant() && shared::invariant());

    if (run_in_progress == 0 && trigger_a_new_run()) {
        run_in_progress = 1 + (dist(mt)/100)*(shared::max_run_length-1);
        assert(invariant() && run_in_progress > 0);
    }

    if (run_in_progress > 0) {
        --run_in_progress;
        throw std::bad_alloc();
    }
}


}

#endif
