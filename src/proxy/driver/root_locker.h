/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>  // for string

#include <common/types.h>  // for time64_t

#include "core/global.h"  // for FastoObjectIPtr, etc

class QObject;
namespace fastonosql {
namespace proxy {
class IDriver;
}
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {

class RootLocker : core::FastoObject::IFastoObjectObserver {
 public:
  RootLocker(IDriver* parent, QObject* receiver, const core::command_buffer_t& text, bool silence);
  virtual ~RootLocker();

  core::FastoObjectIPtr Root() const;

 protected:
  // notification of execute events
  virtual void ChildrenAdded(core::FastoObjectIPtr child) override;
  virtual void Updated(core::FastoObject* item, core::FastoObject::value_t val) override;

 private:
  core::FastoObjectIPtr root_;
  IDriver* parent_;
  QObject* receiver_;
  const common::time64_t tstart_;
  const bool silence_;
};

}  // namespace proxy
}  // namespace fastonosql
