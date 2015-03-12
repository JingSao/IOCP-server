#ifndef _MEMORY_POOL_H_

#include <stdlib.h>

namespace iocp {
    namespace mp {
        //template <typename T> using allocator0 = std::allocator<T>;
        template <typename _T> struct allocator
        {
            // typedefs...
            typedef _T                  value_type;
            typedef value_type          *pointer;
            typedef value_type          &reference;
            typedef value_type const    *const_pointer;
            typedef value_type const    &const_reference;
            typedef size_t              size_type;
            typedef ptrdiff_t           difference_type;

            // rebind...
            template <typename _Other> struct rebind
            {
                typedef allocator<_Other> other;
            };

            pointer address(reference val) const
            {
                return &val;
            }

            const_pointer address(const_reference val) const
            {
                return &val;
            }

            // construct default allocator (do nothing)
            allocator() throw()
            {
            }

            // construct by copying (do nothing)
            allocator(const allocator<_T> &) throw()
            {
            }

            // construct from a related allocator (do nothing)
            template<class _Other>
            allocator(const allocator<_Other> &) throw()
            {
            }

            // assign from a related allocator (do nothing)
            template<class _Other>
            allocator<_T> &operator=(const allocator<_Other> &)
            {
                return *this;
            }

            // deallocate object at _Ptr, ignore size
            void deallocate(pointer ptr, size_type)
            {
                //delete ptr;
                free(ptr);
            }

            // allocate array of cnt elements
            pointer allocate(size_type cnt)
            {
                //return _Allocate(cnt, (pointer)0);
                return (pointer)calloc(cnt, sizeof(_T));
            }

            // allocate array of cnt elements, ignore hint
            pointer allocate(size_type cnt, const void *)
            {
                return allocate(cnt);
            }

            // default construct object at ptr
            void construct(_T *ptr)
            {
                new ((void *)ptr) _T();
            }

            // construct object at ptr with value val
            void construct(_T *ptr, const _T &val)
            {
                new ((void *)ptr) _T(val);
            }

            // construct _Obj(_Types...) at ptr
            template <class _Obj, class... _Types>
            void construct(_Obj *ptr, _Types &&...args)
            {
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

        template <class _T, class _Other>
        inline bool operator==(const allocator<_T> &, const allocator<_Other>&) throw()
        {    // test for allocator equality
            return (true);
        }

        template <class _T, class _Other>
        inline bool operator!=(const allocator<_T> &left, const allocator<_Other> &_right) throw()
        {    // test for allocator inequality
            return !(left == right);
        }
    }
};

#endif
