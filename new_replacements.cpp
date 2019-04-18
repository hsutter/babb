
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

#include "babb.h"

//----------------------------------------------------------------------------
//
//  If you don't already replace global operator new, then include this file
//  in your project. It will put calls to this_thread.inject_random_failure()
//  into all of the standard operator new functions for you.
//
//----------------------------------------------------------------------------

#include <stdlib.h>
#include <new>

namespace op_new_detail {

	void *malloc(size_t size) { return ::malloc(size); }
	void  free   (void *p)    { ::free(p); }

	void *aligned_malloc(size_t size, size_t alignment)
	{
		void *p;
	#if defined(_WIN32)
		p = _aligned_malloc(size, alignment);
	#else
		if (::posix_memalign(&p, alignment, size) != 0)
			p = nullptr; // posix_memalign does not set p on failure
	#endif
		return p;
	}
	
	void throw_bad_alloc()
	{
		throw std::bad_alloc();
	}

}

void* operator new(std::size_t size)
{
    babb::this_thread.inject_random_failure();
    if (size == 0) size = 1;

    void* p;
    while ((p = op_new_detail::malloc(size)) == 0)
    {
     // If malloc fails and there is a new_handler, call it to try free up memory.
        std::new_handler nh = std::get_new_handler();
        if (!nh)
            op_new_detail::throw_bad_alloc();
        nh();
    }
    return p;
}

void* operator new(size_t size, const std::nothrow_t&) noexcept
{
    void* p = nullptr;
    try { p = ::operator new(size); }
    catch (...) {}
    return p;
}

void* operator new[](size_t size)
{
    return ::operator new(size);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept
{
    void* p = nullptr;
    try { p = ::operator new[](size); }
    catch (...) {}
    return p;
}

void operator delete(void* ptr) noexcept
{
    op_new_detail::free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    ::operator delete(ptr);
}

void operator delete(void* ptr, size_t) noexcept
{
    ::operator delete(ptr);
}

void operator delete[] (void* ptr) noexcept
{
    ::operator delete(ptr);
}

void operator delete[] (void* ptr, const std::nothrow_t&) noexcept
{
    ::operator delete[](ptr);
}

void operator delete[] (void* ptr, size_t) noexcept
{
    ::operator delete[](ptr);
}



#ifdef HAS_ALIGNED_ALLOCATIONS

void* operator new(std::size_t size, std::align_val_t alignment)
{
    babb::this_thread.inject_random_failure();
    if (size == 0) size = 1;
    if (static_cast<size_t>(alignment) < sizeof(void*))
      alignment = std::align_val_t(sizeof(void*));

    void* p;
    while ((p = op_new_detail::aligned_malloc(size, static_cast<size_t>(alignment))) == nullptr)
    {
     // If aligned_malloc fails and there is a new_handler, call it to try free up memory.
        std::new_handler nh = std::get_new_handler();
        if (!nh)
            op_new_detail::throw_bad_alloc();
        nh();
    }
    return p;
}

void* operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    void* p = nullptr;
    try { p = ::operator new(size, alignment); }
    catch (...) {}
    return p;
}

void* operator new[](size_t size, std::align_val_t alignment)
{
    return ::operator new(size, alignment);
}

void* operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    void* p = nullptr;
    try { p = ::operator new[](size, alignment); }
    catch (...) {}
    return p;
}

void operator delete(void* ptr, std::align_val_t) noexcept
{
    op_new_detail::free(ptr);
}

void operator delete(void* ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    ::operator delete(ptr, alignment);
}

void operator delete(void* ptr, size_t, std::align_val_t alignment) noexcept
{
    ::operator delete(ptr, alignment);
}

void operator delete[] (void* ptr, std::align_val_t alignment) noexcept
{
    ::operator delete(ptr, alignment);
}

void operator delete[] (void* ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    ::operator delete[](ptr, alignment);
}

void operator delete[] (void* ptr, size_t, std::align_val_t alignment) noexcept
{
    ::operator delete[](ptr, alignment);
}

#endif // !HAS_ALIGNED_ALLOCATIONS

