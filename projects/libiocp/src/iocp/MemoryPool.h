#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_

#include <windows.h>

namespace iocp {
    namespace mp {

        template <class _T> struct Allocator
        {
            // typedefs
            typedef _T                  value_type;
            typedef value_type          *pointer;
            typedef value_type          &reference;
            typedef const value_type    *const_pointer;
            typedef const value_type    &const_reference;
            typedef size_t              size_type;
            typedef ptrdiff_t           difference_type;

            // rebind
            template <class _Other> struct rebind { typedef Allocator<_Other> other; };

            pointer address(reference val) const
            {
                return &val;
            }

            const_pointer address(const_reference val) const
            {
                return &val;
            }

            // construct default allocator (do nothing)
            Allocator() throw()
            {
            }

            // construct by copying (do nothing)
            Allocator(const Allocator<_T> &) throw()
            {
            }

            // construct from a related allocator (do nothing)
            template<class _Other>
            Allocator(const Allocator<_Other> &) throw()
            {
            }

            // assign from a related allocator (do nothing)
            template<class _Other>
            Allocator<_T> &operator=(const Allocator<_Other> &)
            {
                return *this;
            }

            // deallocate object at _Ptr, ignore size
            void deallocate(pointer ptr, size_type)
            {
                //delete ptr;
                //free(ptr);
                ::HeapFree(::GetProcessHeap(), 0, ptr);
            }

            // allocate array of cnt elements
            pointer allocate(size_type cnt)
            {
                //return _Allocate(cnt, (pointer)0);
                //return calloc(cnt, sizeof(_T));
                return (pointer)::HeapAlloc(::GetProcessHeap(), 0, cnt * sizeof(_T));
            }

            // allocate array of cnt elements, ignore hint
            pointer allocate(size_type cnt, const void *)
            {
                return allocate(cnt);
            }

            // default construct object at ptr
            void construct(_T *ptr)
            {
                if (ptr == nullptr) throw(std::bad_alloc());
                new ((void *)ptr) _T();
            }

            // construct object at ptr with value val
            void construct(_T *ptr, const _T &val)
            {
                if (ptr == nullptr) throw(std::bad_alloc());
                new ((void *)ptr) _T(val);
            }

            // construct _Obj(_Types...) at ptr
            template <class _Obj, class... _Types>
            void construct(_Obj *ptr, _Types &&...args)
            {
                if (ptr == nullptr) throw(std::bad_alloc());
                new ((void *)ptr) _Obj(std::forward<_Types>(args)...);
            }

            // destroy object at ptr
            template<class _U>
            void destroy(_U *ptr)
            {
                ptr->~_U();
            }

            // estimate maximum array size
            size_t max_size() const throw()
            {
                return ((size_t)(-1) / sizeof(_T));
            }
        };

        // test for allocator equality
        template <class _T, class _Other>
        inline bool operator==(const Allocator<_T> &, const Allocator<_Other> &) throw()
        {
            return true;
        }

        // test for allocator inequality
        template <class _T, class _Other>
        inline bool operator!=(const Allocator<_T> &left, const Allocator<_Other> &right) throw()
        {
            return !(left == right);
        }
    }
};

#endif
