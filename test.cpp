
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


//----------------------------------------------------------------------------
//  Smoke test
//----------------------------------------------------------------------------

#include <iostream>
using namespace std;

#include "babb.h"

int main() {
	constexpr int N = 1000;
	static_assert(N > 800, "test assumes at least 800 allocation attempts");

	int total = 0;

	babb::this_thread.set_failure_profile(10, 10);

	cout << "===== Testing bad_alloc:\n";
    int i = 0;
	while (++i < 200) {
		try { (void) new int; cout << '.'; }
		catch (const bad_alloc &) { cout << '!'; ++total; }
		catch (...) { assert(!"other exception was thrown"); }
	}
	{ // there should be a gap of no failures from 200 to 700
    babb::state_guard save(babb::this_thread);
    babb::this_thread.pause(true);
	while (++i < 700) {
		try { (void) new int; cout << '.'; }
		catch (...) { assert(!"no exception should be thrown, injection is paused"); }
	}
    }
	while (++i < N) {
		try { (void) new int; cout << '.'; }
		catch (const bad_alloc &) { cout << '!'; ++total; }
		catch (...) { assert(!"other exception was thrown"); }
	}

	cout << "\nSummary: " << total << " failures in "
		<< N << " requests (avg. 1 per " << N / total << ")\n\n";

	cout << "===== Testing nothrow/nullptr:\n";
    total = 0;
    i = 0;
	while (++i < 200) {
		if (new (nothrow) int) { cout << '.'; }
        else { cout << '!'; ++total; };
	}
	{ // there should be a gap of no failures from 200 to 700
    babb::state_guard save(babb::this_thread);
    babb::this_thread.pause(true);
	while (++i < 700) {
		if (new (nothrow) int) { cout << '.'; }
        else { assert(!"no exception should be thrown, injection is paused"); };
	}
    }
	while (++i < N) {
		if (new (nothrow) int) { cout << '.'; }
        else { cout << '!'; ++total; };
	}

	cout << "\nSummary: " << total << " failures in "
		<< N << " requests (avg. 1 per " << N / total << ")\n";
}
