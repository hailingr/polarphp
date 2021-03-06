//===--- UUID.h - UUID generation -------------------------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This is an interface over the standard OSF uuid library that gives UUIDs
// sane m_value semantics and operators.
//
//===----------------------------------------------------------------------===//

// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarphp software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/02/13.

#include "polarphp/basic/Uuid.h"


// WIN32 doesn't natively support <uuid/uuid.h>. Instead, we use Win32 APIs.
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <objbase.h>
#include <string>
#include <algorithm>
#else
#include <uuid/uuid.h>
#endif

namespace polar {
namespace basic {

UUID::UUID(FromRandom_t)
{
#if defined(_WIN32)
  ::UUID uuid;
  ::CoCreateGuid(&uuid);

  memcpy(m_value, &uuid, Size);
#else
  uuid_generate_random(m_value);
#endif
}

UUID::UUID(FromTime_t)
{
#if defined(_WIN32)
  ::UUID uuid;
  ::CoCreateGuid(&uuid);

  memcpy(m_value, &uuid, Size);
#else
  uuid_generate_time(m_value);
#endif
}

UUID::UUID()
{
#if defined(_WIN32)
  ::UUID uuid = *((::UUID *)&m_value);
  UuidCreateNil(&uuid);
  memcpy(m_value, &uuid, Size);
#else
  uuid_clear(m_value);
#endif
}

std::optional<UUID> UUID::fromString(const char *s)
{
#if defined(_WIN32)
  RPC_CSTR t = const_cast<RPC_CSTR>(reinterpret_cast<const unsigned char*>(s));

  ::UUID uuid;
  RPC_STATUS status = UuidFromStringA(t, &uuid);
  if (status == RPC_S_INVALID_STRING_UUID) {
    return None;
  }

  UUID result = UUID();
  memcpy(result.m_value, &uuid, Size);
  return result;
#else
  UUID result;
  if (uuid_parse(s, result.m_value))
    return std::nullopt;
  return result;
#endif
}

void UUID::toString(SmallVectorImpl<char> &out) const
{
  out.resize(UUID::StringBufferSize);
#if defined(_WIN32)
  ::UUID uuid;
  memcpy(&uuid, m_value, Size);

  RPC_CSTR str;
  UuidToStringA(&uuid, &str);

  char* signedStr = reinterpret_cast<char*>(str);
  memcpy(out.data(), signedStr, StringBufferSize);
  std::transform(std::begin(out), std::end(out), std::begin(out), toupper);
#else
  uuid_unparse_upper(m_value, out.getData());
#endif
  // Pop off the null terminator.
  assert(out.back() == '\0' && "did not null-terminate?!");
  out.pop_back();
}

int UUID::compare(UUID y) const
{
#if defined(_WIN32)
  RPC_STATUS s;
  ::UUID uuid1;
  memcpy(&uuid1, m_value, Size);

  ::UUID uuid2;
  memcpy(&uuid2, y.m_value, Size);

  return UuidCompare(&uuid1, &uuid2, &s);
#else
  return uuid_compare(m_value, y.m_value);
#endif
}

RawOutStream &operator<<(RawOutStream &os, UUID uuid)
{
  SmallString<UUID::StringBufferSize> buf;
  uuid.toString(buf);
  os << buf;
  return os;
}

} // basic
} // polar
