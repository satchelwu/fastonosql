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

#pragma once

#include <memory>  // for shared_ptr
#include <string>  // for string
#include <vector>  // for vector

#include "core/connection_settings/connection_settings.h"  // for IConnectionSettingsBaseSPtr, etc
#include "core/connection_types.h"                         // for connectionTypes

namespace fastonosql {
namespace core {

struct SentinelSettings {
  typedef std::vector<IConnectionSettingsBaseSPtr> sentinel_nodes_t;
  SentinelSettings();

  IConnectionSettingsBaseSPtr sentinel;
  sentinel_nodes_t sentinel_nodes;
};

std::string sentinelSettingsToString(const SentinelSettings& sent);
bool sentinelSettingsfromString(const std::string& text, SentinelSettings* sent);

class ISentinelSettingsBase : public IConnectionSettings {
 public:
  typedef SentinelSettings sentinel_connection_t;
  typedef std::vector<sentinel_connection_t> sentinel_connections_t;

  sentinel_connections_t Sentinels() const;
  void AddSentinel(sentinel_connection_t sent);

  static ISentinelSettingsBase* CreateFromType(connectionTypes type,
                                               const connection_path_t& conName);
  static ISentinelSettingsBase* FromString(const std::string& val);

  virtual std::string ToString() const override;
  virtual ISentinelSettingsBase* Clone() const = 0;

 protected:
  ISentinelSettingsBase(const connection_path_t& connectionName, connectionTypes type);

 private:
  sentinel_connections_t sentinel_nodes_;
};
typedef common::shared_ptr<ISentinelSettingsBase> ISentinelSettingsBaseSPtr;

}  // namespace core
}  // namespace fastonosql
