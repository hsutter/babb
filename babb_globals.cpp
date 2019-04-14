
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


#include "babb.h"

//----------------------------------------------------------------------------
//
//  If you don't already replace global operator new, then include this file
//  in your project. It will put calls to inject_random_failure into
//  all of the standard operator new functions for you.
//
//----------------------------------------------------------------------------

void* operator new  ( std::size_t count )
{
    inject_random_failure();
    return malloc(count);
	// TODO: is there a way to call the non-replaced one from here instead of using malloc?
}

void* operator new[]( std::size_t count )
{
    inject_random_failure();
    return malloc(count);
}

// TODO: C++17 aligned versions
//
// void* operator new  ( std::size_t count, std::align_val_t al)
// {
//     inject_random_failure();
//     return malloc(count);
// }
//
// void* operator new[]( std::size_t count, std::align_val_t al)
// {
//     inject_random_failure();
//     return malloc(count);
// }


void operator delete(void * ptr) throw()
{
	free(ptr);
}

void operator delete[](void *p) throw()
{
    free(ptr);
}

void operator delete(void * ptr, std::size_t sz) throw()
{
	free(ptr);
}

void operator delete[](void *p, std::size_t sz) throw()
{
    free(ptr);
}

// TODO: C++17 aligned versions
//
// void operator delete  ( void* ptr, std::align_val_t al )
// {
//     free(ptr);
// }
//
// void operator delete[]( void* ptr, std::align_val_t al )
// {
//     free(ptr);
// }
//
// void operator delete  ( void* ptr, std::size_t sz, std::align_val_t al )
// {
//     free(ptr);
// }
//
// void operator delete[]( void* ptr, std::size_t sz, std::align_val_t al )
// {
//     free(ptr);
// }
