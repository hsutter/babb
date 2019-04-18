
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
using namespace babb;

int main () {
	const int N = 1000;
	int total = 0;

	this_thread.set_failure_profile(10, 10);

	for (int i = 0; i < N; ++i) {
		try { (void) new int; cout << '.'; }
		catch (const bad_alloc &) { cout << '!'; ++total; }
		catch (...) { assert(!"other exception was thrown"); }
	}

	cout << "\nSummary: " << total << " failures in "
         << N << " requests (avg. 1 per " << N / total << ")\n";
}
