/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include "proxy/connection_settings/iconnection_settings_remote.h"

#include <common/macros.h>  // for CHECK

namespace fastonosql {
namespace proxy {

IConnectionSettingsRemote::IConnectionSettingsRemote(const connection_path_t& connectionPath,
                                                     core::connectionTypes type)
    : IConnectionSettingsBase(connectionPath, type) {
  CHECK(IsRemoteType(type));
}

std::string IConnectionSettingsRemote::FullAddress() const {
  common::net::HostAndPort host = Host();
  return common::ConvertToString(host);
}

}  // namespace proxy
}  // namespace fastonosql