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

#include "core/db/forestdb/database_info.h"

#include "core/connection_types.h"  // for connectionTypes::FORESTDB

namespace fastonosql {
namespace core {
namespace forestdb {

DataBaseInfo::DataBaseInfo(const std::string& name, bool isDefault, size_t size, const keys_container_t& keys)
    : IDataBaseInfo(name, isDefault, FORESTDB, size, keys) {}

DataBaseInfo* DataBaseInfo::Clone() const {
  return new DataBaseInfo(*this);
}

}  // namespace forestdb
}  // namespace core
}  // namespace fastonosql
