
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2019 Herb Sutter and Marshall Clow. All rights reserved.
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
#include <iostream>

namespace babb {

//----------------------------------------------------------------------------
//  State values to control failure frequency and status
//  We'll keep a global state, and a per-thread state
//----------------------------------------------------------------------------

class state {
protected:
    int once_per  = 100000;    	// avg #allocations between failures
    int run_length = 5;	        // max #consecutive failures
    bool paused = false;        // is failure injection currently paused

    // non-auto explicit return type is for portability to pre-C++14 compilers
    bool invariant() 
        { return once_per > 0 && run_length > 0; }

public:
    //----------------------------------------------------------------------------
    //
    //	set_failure_profile: Change current failure injection profile
    //
    //	Call:
    //      shared.set_failure_profile to set the current global defaults
    //      this_thread.set_failure_profile to set this thread's current values
    //  Each thread initially defaults to the shared values.
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
    //	This can be useful to work around individual calls to OOM-unsafe functions
    //  in third-party libraries (though if those are failing that's data too).
    //
    //  paused:  true to pause, false to resume
    //
    //----------------------------------------------------------------------------

    void pause(bool paused) {
        this->paused = paused;
    }
};

//----------------------------------------------------------------------------
//
//	Helper RAII type to save/restore current settings
//
//	This can be useful to work around entire third-party libraries that are
//  OOM-unsafe (though if those are failing that's data too).
//
//----------------------------------------------------------------------------
class state_guard {
    state& original;
    state saved;
public:
    state_guard(state& s) : original(s), saved(s) { }
    ~state_guard() { original = saved; }
};


//----------------------------------------------------------------------------
//  Global state (used for thread defaults)
//----------------------------------------------------------------------------

state shared;


//----------------------------------------------------------------------------
//  Per-thread state
//----------------------------------------------------------------------------

class this_thread_ : public state {
    std::random_device rd;
    std::mt19937_64 mt;
    std::uniform_real_distribution<double> dist;
    int run_in_progress = 0;

public:
    this_thread_() : state(shared), mt(rd()), dist(0., 1.) { }


    //----------------------------------------------------------------------------
    //
    //	should_inject_random_failure()
    //
    //  Returns true if it's time to inject a failure in this thread.
    //
    //----------------------------------------------------------------------------

    bool should_inject_random_failure() {
        assert(invariant());

        if (paused) return false;

        auto trigger_a_new_run =
            [&]{ return dist(mt) < 1./once_per/(run_length/2.); };

        if (run_in_progress == 0 && trigger_a_new_run()) {
            run_in_progress = 1 + int(dist(mt)*(run_length-1));
            assert(invariant() && run_in_progress > 0);
        }

        if (run_in_progress > 0) { 
            --run_in_progress; 
            return true;
        }
        else
            return false;        
    }


    //----------------------------------------------------------------------------
    //
    //	inject_random_failure()
    //
    //  Put a call to this function inside each of your custom allocation functions
    //  that could throw an allocation exception, including any custom non-nothrow
    //  operator new and any custom allocator's allocate function that can fail by
    //  throwing.
    //
    //----------------------------------------------------------------------------

    template<class E = std::bad_alloc>
    void inject_random_failure() {
        if (should_inject_random_failure())
            throw E();
        // NOTE: We don't have to take care here to ensure that this doesn't allocate
        // normal memory, because the implementation is already required to be robust
        // so that "throw bad_alloc()" works in low-memory situations. Typically that
        // means bad_alloc objects must live in dedicated private "emergency reserve"
        // memory; otherwise, if "new int" fails an ordinary "new bad_alloc" to throw
        // the exception will also immediately fail. So it's up to implementations to
        // make this line work, and if they don't then that's useful data too.
    }
};
thread_local this_thread_ this_thread;

}

#endif
