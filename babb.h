
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

//----------------------------------------------------------------------------
//  Global state
//----------------------------------------------------------------------------

class {
    int once_per  = 100000; 	// avg #allocations between failures
    int run_length = 5;	        // max #consecutive failures

    auto invariant() 
        { return once_per > 0 && run_length >= 1; }

public:
    auto fail_once_per()  { return once_per;   }
    auto max_run_length() { return run_length; }

    //----------------------------------------------------------------------------
    //
    //	set_failure_profile: Set a default failure injection profile
    //
    //	Optionally put one call to this in your main().
    //  If you don't, the default is as if init(100000, 5).
    //
    //  fail_once_per:  avg #allocations between failures
    //  max_run_length: max #consecutive failures (once we have triggered a new one)
    //
    //----------------------------------------------------------------------------

    void set_failure_profile(int fail_once_per, int max_run_length) {
        once_per = fail_once_per;
        run_length = max_run_length;
        assert(invariant());
    }
} shared;


//----------------------------------------------------------------------------
//  Per-thread state
//----------------------------------------------------------------------------

thread_local class this_thread_ {   
    std::random_device rd;
    std::mt19937_64 mt;
    std::uniform_real_distribution<double> dist;
    int run_in_progress = 0;

    int once_per  = shared.fail_once_per(); 	// avg #allocations between failures
    int run_length = shared.max_run_length();   // max #consecutive failures
    bool paused = false;

    auto invariant() { return once_per > 0 && run_length >= 1; }

public:
    //----------------------------------------------------------------------------
    //
    //	set_failure_profile: Change this thread's failure injection profile
    //
    //	The default is to use the default frequency.
    //
    //  fail_once_per:  avg #allocations between failures
    //  max_run_length: max #consecutive failures (once we have triggered a new one)
    //
    //----------------------------------------------------------------------------

    void set_failure_profile(int fail_once_per, int max_run_length) {
        once_per = fail_once_per;
        run_length = max_run_length;
        assert(invariant());
    }

    //----------------------------------------------------------------------------
    //
    //	pause: Pause or unpause fault injection on this thread.
    //
    //	This can be useful to work around calls to OOM-unsafe functions in
    //  third-party libraries (though if those are failing that's data too).
    //
    //  paused:  true to pause, false to resume
    //
    //----------------------------------------------------------------------------

    void pause(bool paused) {
        this->paused = paused;
    }


    this_thread_() : mt(rd()), dist(0., 1.) { assert(invariant()); }

    auto fail_now() { 
        assert(invariant());

        if (paused) return false;

        auto trigger_a_new_run =
            [&]{ return dist(mt) < 1./once_per/(run_length/2.); };

        if (run_in_progress == 0 && trigger_a_new_run()) {
            run_in_progress = 1 + (dist(mt)/100.)*(run_length-1);
            assert(invariant() && run_in_progress > 0);
        }

        if (run_in_progress > 0) { --run_in_progress; return true; } 
        return false;
    }
} this_thread;


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
    if (this_thread.fail_now()) {
        throw std::bad_alloc();
        // NOTE: We don't have to take care here to ensure that this doesn't allocate
        // normal memory, because the implementation is already required to be robust
        // so that "throw bad_alloc()" works in low-memory situations. Typically that
        // means bad_alloc objects must live in dedicated private "emergency reserve"
        // memory; otherwise, if "new int" fails an ordinary "new bad_alloc" to throw
        // the exception will also immediately fail. So it's up to implementations to
        // make this line work, and if they don't then that's useful data too.
    }
}


}

#endif
