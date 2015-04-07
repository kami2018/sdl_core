/*
 * Copyright (c) 2015, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <algorithm>

#include "policy/access_remote_impl.h"
#include "policy/cache_manager.h"
#include "types.h"
#include "utils/logger.h"

CREATE_LOGGERPTR_GLOBAL(logger_, "PolicyManagerImpl")

using policy_table::DeviceData;

namespace policy {

struct IsPrimary {
  bool operator ()(const DeviceData::MapType::value_type& item) {
    return *item.second.primary == true;
  }
};

struct SetPrimary {
 private:
  bool value_;
 public:
  explicit SetPrimary(bool value)
      : value_(value) {
  }
  void operator ()(DeviceData::MapType::value_type& item) {
    *item.second.primary = value_;
  }
};

struct Erase {
 private:
  const Subject& who_;
 public:
  explicit Erase(const Subject& who)
      : who_(who) {
  }
  void operator ()(AccessRemoteImpl::AccessControlList::value_type& row) {
    row.second.erase(who_);
  }
};

struct IsTypeAccess {
 private:
  const TypeAccess& type_;
 public:
  explicit IsTypeAccess(const TypeAccess& type)
      : type_(type) {
  }
  bool operator ()(const AccessRemoteImpl::AccessControlRow::value_type& item) {
    return item.second == type_;
  }
};

AccessRemoteImpl::AccessRemoteImpl(CacheManager& cache)
    : cache_(cache),
      primary_device_(),
      enabled_(true),
      acl_() {
}

void AccessRemoteImpl::Init() {
  DCHECK(cache_.pt_);
  enabled_ = cache_.pt_->policy_table.module_config.remote_control;

  DeviceData& devices = *cache_.pt_->policy_table.device_data;
  DeviceData::iterator d = std::find_if(devices.begin(), devices.end(),
                                        IsPrimary());
  if (d != devices.end()) {
    primary_device_ = d->first;
  }
}

bool AccessRemoteImpl::IsPrimaryDevice(const PTString& dev_id) const {
  return primary_device_ == dev_id;
}

bool AccessRemoteImpl::IsPassengerZone(const SeatLocation& seat,
                                       const SeatLocation& zone) const {
  return seat == zone;
}

TypeAccess AccessRemoteImpl::Check(const Subject& who,
                                   const Object& what) const {
  LOG4CXX_TRACE_ENTER(logger_);
  TypeAccess ret = TypeAccess::kDisallowed;
  AccessControlList::const_iterator i = acl_.find(what);
  if (i != acl_.end()) {
    const AccessControlRow& row = i->second;
    AccessControlRow::const_iterator j = row.find(who);
    if (j != row.end()) {
      // who has permissions
      ret = j->second;
    } else {
      // Look for somebody is who controls this object
      j = std::find_if(row.begin(), row.end(),
                       IsTypeAccess(TypeAccess::kAllowed));
      if (j != row.end()) {
        // Someone controls this object
        ret = TypeAccess::kDisallowed;
      } else {
        // Nobody controls this object
        ret = TypeAccess::kManual;
      }
    }
  } else {
    // Nobody controls this object
    ret = TypeAccess::kManual;
  }
  LOG4CXX_TRACE_EXIT(logger_);
  return ret;
}

void AccessRemoteImpl::Allow(const Subject& who, const Object& what) {
  acl_[what][who] = TypeAccess::kAllowed;
}

void AccessRemoteImpl::Deny(const Subject& who, const Object& what) {
  acl_[what][who] = TypeAccess::kDisallowed;
}

void AccessRemoteImpl::Reset(const Subject& who) {
  std::for_each(acl_.begin(), acl_.end(), Erase(who));
}

void AccessRemoteImpl::Reset(const Object& what) {
  acl_.erase(what);
}

void AccessRemoteImpl::SetPrimaryDevice(const PTString& dev_id) {
  primary_device_ = dev_id;
  DeviceData& devices = *cache_.pt_->policy_table.device_data;
  std::for_each(devices.begin(), devices.end(), SetPrimary(false));
  *devices[dev_id].primary = true;
  cache_.Backup();
}

void AccessRemoteImpl::Enable() {
  set_enabled(true);
}

void AccessRemoteImpl::Disable() {
  set_enabled(false);
}

void AccessRemoteImpl::set_enabled(bool value) {
  enabled_ = value;
  cache_.pt_->policy_table.module_config.remote_control = enabled_;
  cache_.Backup();
}

bool AccessRemoteImpl::IsEnabled() const {
  return enabled_;
}

}  // namespace policy