//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2009 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MSGPACK_TYPE_PAIR_HPP
#define MSGPACK_TYPE_PAIR_HPP

#include "msgpack/versioning.hpp"
#include "msgpack/object_fwd.hpp"
#include <utility>

namespace msgpack {

MSGPACK_API_VERSION_NAMESPACE(v1) {

template <typename T1, typename T2>
inline msgpack::object const& operator>> (msgpack::object const& o, std::pair<T1, T2>& v)
{
    if(o.type != msgpack::type::ARRAY) { throw msgpack::type_error(); }
    if(o.via.array.size != 2) { throw msgpack::type_error(); }
    o.via.array.ptr[0].convert(v.first);
    o.via.array.ptr[1].convert(v.second);
    return o;
}

template <typename Stream, typename T1, typename T2>
inline msgpack::packer<Stream>& operator<< (msgpack::packer<Stream>& o, const std::pair<T1, T2>& v)
{
    o.pack_array(2);
    o.pack(v.first);
    o.pack(v.second);
    return o;
}

template <typename T1, typename T2>
inline void operator<< (msgpack::object::with_zone& o, const std::pair<T1, T2>& v)
{
    o.type = msgpack::type::ARRAY;
    msgpack::object* p = static_cast<msgpack::object*>(o.zone.allocate_align(sizeof(msgpack::object)*2));
    o.via.array.ptr = p;
    o.via.array.size = 2;
    p[0] = msgpack::object(v.first, o.zone);
    p[1] = msgpack::object(v.second, o.zone);
}

}  // MSGPACK_API_VERSION_NAMESPACE(v1)

}  // namespace msgpack

#endif // MSGPACK_TYPE_PAIR_HPP
