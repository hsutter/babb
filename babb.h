
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

namespace babb {

static double pct = 0.01;	// the average percentage of allocations to  fail
static int    run = 5;		// average run length of consecutive failures


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

	auto trigger_a_new_run = [&]{ return dist(mt) < (pct/run); };

	assert(run_in_progress >= 0);

	if (run_in_progress == 0 && trigger_a_new_run()) {
		run_in_progress = int(run * dist(mt) / 50.);
		assert(0 <= run_in_progress && run_in_progress <= 2*run);
	}

    if (run_in_progress > 0) {
		--run_in_progress;
		throw std::bad_alloc();
	}
}


//----------------------------------------------------------------------------
//
//	init(pct, run)
//
//	Optionally put a call to this in your main(). If you don't, the default
//  is as if init(0.01, 5).
//
//  pct: average % of allocations that will fail (default: 0.01)
//  run: average # of consecutive failures in a cluster (default: 5)
//
//----------------------------------------------------------------------------

void init(double pct, int run) {
	babb::pct = pct;
	babb::run = run;
}

}

#endif
