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

#include "proxy/db/redis/driver.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t

#include <memory>  // for __shared_ptr, shared_ptr
#include <vector>  // for vector

#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/file_system.h>     // for copy_file
#include <common/intrusive_ptr.h>   // for intrusive_ptr
#include <common/qt/utils_qt.h>     // for Event<>::value_type
#include <common/sprintf.h>         // for MemSPrintf
#include <common/value.h>           // for Value, ErrorValue, etc

#include "core/connection_types.h"
#include "core/database/idatabase_info.h"  // for IDataBaseInfoSPtr, etc
#include "core/db_key.h"                   // for NDbKValue, NValue, ttl_t, etc
#include "core/server_property_info.h"     // for MakeServerProperty, etc
#include "proxy/command/command.h"         // for CreateCommand, etc
#include "proxy/command/command_logger.h"
#include "proxy/driver/root_locker.h"  // for RootLocker
#include "proxy/events/events_info.h"

#include "core/internal/cdb_connection.h"
#include "core/internal/db_connection.h"

#include "core/db/redis/config.h"                // for Config
#include "core/db/redis/database_info.h"         // for DataBaseInfo
#include "core/db/redis/db_connection.h"         // for DBConnection, INFO_REQUEST, etc
#include "core/db/redis/server_info.h"           // for ServerInfo, etc
#include "proxy/db/redis/command.h"              // for Command
#include "proxy/db/redis/connection_settings.h"  // for ConnectionSettings

#include "core/global.h"  // for FastoObjectCommandIPtr, etc

#define REDIS_SHUTDOWN_COMMAND "SHUTDOWN"
#define REDIS_BACKUP_COMMAND "SAVE"
#define REDIS_SET_PASSWORD_COMMAND_1ARGS_S "CONFIG SET requirepass %s"
#define REDIS_SET_MAX_CONNECTIONS_COMMAND_1ARGS_I "CONFIG SET maxclients %d"
#define REDIS_GET_DATABASES_COMMAND "CONFIG GET databases"
#define REDIS_GET_PROPERTY_SERVER_COMMAND "CONFIG GET *"
#define REDIS_PUBSUB_CHANNELS_COMMAND_1ARGS_S "PUBSUB CHANNELS %s"
#define REDIS_PUBSUB_NUMSUB_COMMAND_1ARGS_S "PUBSUB NUMSUB %s"

#define REDIS_SET_DEFAULT_DATABASE_COMMAND_1ARGS_S "SELECT %s"
#define REDIS_FLUSHDB_COMMAND "FLUSHDB"

#define BACKUP_DEFAULT_PATH "/var/lib/redis/dump.rdb"
#define EXPORT_DEFAULT_PATH "/var/lib/redis/dump.rdb"

namespace {

common::Value::Type convertFromStringRType(const std::string& type) {
  if (type.empty()) {
    return common::Value::TYPE_NULL;
  }

  if (type == "string") {
    return common::Value::TYPE_STRING;
  } else if (type == "list") {
    return common::Value::TYPE_ARRAY;
  } else if (type == "set") {
    return common::Value::TYPE_SET;
  } else if (type == "hash") {
    return common::Value::TYPE_HASH;
  } else if (type == "zset") {
    return common::Value::TYPE_ZSET;
  } else {
    return common::Value::TYPE_NULL;
  }
}

}  // namespace

namespace fastonosql {
namespace proxy {
namespace redis {

Driver::Driver(IConnectionSettingsBaseSPtr settings)
    : IDriverRemote(settings), impl_(new core::redis::DBConnection(this)) {
  COMPILE_ASSERT(core::redis::DBConnection::connection_t == core::REDIS,
                 "DBConnection must be the same type as Driver!");
  CHECK(Type() == core::REDIS);
}

Driver::~Driver() {
  delete impl_;
}

common::net::HostAndPort Driver::Host() const {
  core::redis::Config conf = impl_->config();
  return conf.host;
}

std::string Driver::NsSeparator() const {
  return impl_->NsSeparator();
}

std::string Driver::Delimiter() const {
  return impl_->Delimiter();
}

bool Driver::IsInterrupted() const {
  return impl_->IsInterrupted();
}

void Driver::SetInterrupted(bool interrupted) {
  return impl_->SetInterrupted(interrupted);
}

core::translator_t Driver::Translator() const {
  return impl_->Translator();
}

bool Driver::IsConnected() const {
  return impl_->IsConnected();
}

bool Driver::IsAuthenticated() const {
  return impl_->IsAuthenticated();
}

void Driver::InitImpl() {}

void Driver::ClearImpl() {}

core::FastoObjectCommandIPtr Driver::CreateCommand(core::FastoObject* parent,
                                                   const core::command_buffer_t& input,
                                                   core::CmdLoggingType ct) {
  return proxy::CreateCommand<Command>(parent, input, ct);
}

core::FastoObjectCommandIPtr Driver::CreateCommandFast(const core::command_buffer_t& input, core::CmdLoggingType ct) {
  return proxy::CreateCommandFast<Command>(input, ct);
}

common::Error Driver::SyncConnect() {
  ConnectionSettings* set = dynamic_cast<ConnectionSettings*>(settings_.get());  // +
  CHECK(set);
  core::redis::RConfig rconf(set->Info(), set->SSHInfo());
  return impl_->Connect(rconf);
}

common::Error Driver::SyncDisconnect() {
  return impl_->Disconnect();
}

common::Error Driver::ExecuteImpl(const core::command_buffer_t& command, core::FastoObject* out) {
  return impl_->Execute(command, out);
}

common::Error Driver::CurrentServerInfo(core::IServerInfo** info) {
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(MAKE_BUFFER(INFO_REQUEST), core::C_INNER);
  common::Error err = Execute(cmd.get());
  if (err && err->IsError()) {
    return err;
  }

  std::string content = common::ConvertToString(cmd.get());
  *info = core::redis::MakeRedisServerInfo(content);

  if (!*info) {
    return common::make_error_value("Invalid " INFO_REQUEST " command output", common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error Driver::CurrentDataBaseInfo(core::IDataBaseInfo** info) {
  if (!info) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  return impl_->Select(impl_->CurrentDBName(), info);
}

void Driver::HandleShutdownEvent(events::ShutDownRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ShutDownResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(MAKE_BUFFER(REDIS_SHUTDOWN_COMMAND), core::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->IsError()) {
    res.setErrorInfo(er);
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ShutDownResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleBackupEvent(events::BackupRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::BackupResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(MAKE_BUFFER(REDIS_BACKUP_COMMAND), core::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->IsError()) {
    res.setErrorInfo(er);
  } else {
    common::Error err = common::file_system::copy_file(MAKE_BUFFER(BACKUP_DEFAULT_PATH), res.path);
    if (err && err->IsError()) {
      res.setErrorInfo(err);
    }
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::BackupResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleExportEvent(events::ExportRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ExportResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  common::Error err = common::file_system::copy_file(res.path, EXPORT_DEFAULT_PATH);
  if (err && err->IsError()) {
    res.setErrorInfo(err);
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ExportResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleChangePasswordEvent(events::ChangePasswordRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ChangePasswordResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  std::string patternResult = common::MemSPrintf(REDIS_SET_PASSWORD_COMMAND_1ARGS_S, res.new_password);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(patternResult, core::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->IsError()) {
    res.setErrorInfo(er);
  }

  NotifyProgress(sender, 75);
  Reply(sender, new events::ChangePasswordResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ChangeMaxConnectionResponceEvent::value_type res(ev->value());
  NotifyProgress(sender, 25);
  std::string patternResult = common::MemSPrintf(REDIS_SET_MAX_CONNECTIONS_COMMAND_1ARGS_I, res.max_connection);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(patternResult, core::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->IsError()) {
    res.setErrorInfo(er);
  }

  NotifyProgress(sender, 75);
  Reply(sender, new events::ChangeMaxConnectionResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_GET_DATABASES_COMMAND, core::C_INNER);
  NotifyProgress(sender, 50);

  core::IDataBaseInfo* info = nullptr;
  common::Error err = CurrentDataBaseInfo(&info);
  if (err && err->IsError()) {
    res.setErrorInfo(err);
    NotifyProgress(sender, 75);
    Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  err = Execute(cmd.get());
  if (err && err->IsError()) {
    res.setErrorInfo(err);
    NotifyProgress(sender, 75);
    Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    NotifyProgress(sender, 100);
    return;
  }

  core::FastoObject::childs_t rchildrens = cmd->Childrens();
  CHECK_EQ(rchildrens.size(), 1);
  core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(rchildrens[0].get());  // +
  CHECK(array);
  common::ArrayValue* ar = array->Array();
  CHECK(ar);

  core::IDataBaseInfoSPtr curdb(info);
  std::string scountDb;
  if (ar->GetString(1, &scountDb)) {
    size_t countDb;
    if (common::ConvertFromString(scountDb, &countDb) && countDb > 0) {
      for (size_t i = 0; i < countDb; ++i) {
        core::IDataBaseInfoSPtr dbInf(new core::redis::DataBaseInfo(common::ConvertToString(i), false, 0));
        if (dbInf->Name() == curdb->Name()) {
          res.databases.push_back(curdb);
        } else {
          res.databases.push_back(dbInf);
        }
      }
    }
  } else {
    res.databases.push_back(curdb);
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  const std::string pattern_result = core::internal::GetKeysPattern(res.cursor_in, res.pattern, res.count_keys);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(pattern_result, core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error err = Execute(cmd);
  if (err && err->IsError()) {
    res.setErrorInfo(err);
  } else {
    core::FastoObject::childs_t rchildrens = cmd->Childrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(rchildrens[0].get());  // +
      if (!array) {
        goto done;
      }

      common::ArrayValue* arm = array->Array();
      if (!arm->GetSize()) {
        goto done;
      }

      std::string cursor;
      bool isok = arm->GetString(0, &cursor);
      if (!isok) {
        goto done;
      }

      uint64_t lcursor;
      if (common::ConvertFromString(cursor, &lcursor)) {
        res.cursor_out = lcursor;
      }

      rchildrens = array->Childrens();
      if (!rchildrens.size()) {
        goto done;
      }

      core::FastoObject* obj = rchildrens[0].get();
      core::FastoObjectArray* arr = dynamic_cast<core::FastoObjectArray*>(obj);  // +
      if (!arr) {
        goto done;
      }

      common::ArrayValue* ar = arr->Array();
      if (ar->IsEmpty()) {
        goto done;
      }

      std::vector<core::FastoObjectCommandIPtr> cmds;
      cmds.reserve(ar->GetSize() * 2);
      for (size_t i = 0; i < ar->GetSize(); ++i) {
        core::string_key_t key;
        bool isok = ar->GetString(i, &key);
        if (isok) {
          core::NKey k(core::key_t::MakeKeyString(key));
          core::NDbKValue dbv(k, core::NValue());
          core::command_buffer_writer_t wr_type;
          wr_type << "TYPE " << key;
          cmds.push_back(CreateCommandFast(wr_type.GetBuffer(), core::C_INNER));

          core::command_buffer_writer_t wr_ttl;
          wr_ttl << "TTL " << key;
          cmds.push_back(CreateCommandFast(wr_ttl.GetBuffer(), core::C_INNER));
          res.keys.push_back(dbv);
        }
      }

      err = impl_->ExecuteAsPipeline(cmds, &LOG_COMMAND);
      if (err && err->IsError()) {
        goto done;
      }

      for (size_t i = 0; i < res.keys.size(); ++i) {
        core::FastoObjectIPtr cmdType = cmds[i * 2];
        core::FastoObject::childs_t tchildrens = cmdType->Childrens();
        if (tchildrens.size()) {
          DCHECK_EQ(tchildrens.size(), 1);
          if (tchildrens.size() == 1) {
            std::string typeRedis = tchildrens[0]->ToString();
            common::Value::Type ctype = convertFromStringRType(typeRedis);
            common::ValueSPtr empty_val(common::Value::CreateEmptyValueFromType(ctype));
            res.keys[i].SetValue(empty_val);
          }
        }

        core::FastoObjectIPtr cmdType2 = cmds[i * 2 + 1];
        tchildrens = cmdType2->Childrens();
        if (tchildrens.size()) {
          DCHECK_EQ(tchildrens.size(), 1);
          if (tchildrens.size() == 1) {
            auto vttl = tchildrens[0]->Value();
            core::ttl_t ttl = 0;
            if (vttl->GetAsLongLongInteger(&ttl)) {
              core::NKey key = res.keys[i].GetKey();
              key.SetTTL(ttl);
              res.keys[i].SetKey(key);
            }
          }
        }
      }

      err = impl_->DBkcount(&res.db_keys_count);
      DCHECK(!err);
    }
  }
done:
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ServerPropertyInfoResponceEvent::value_type res(ev->value());
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(REDIS_GET_PROPERTY_SERVER_COMMAND, core::C_INNER);
  NotifyProgress(sender, 50);
  common::Error er = Execute(cmd);
  if (er && er->IsError()) {
    res.setErrorInfo(er);
  } else {
    core::FastoObject::childs_t ch = cmd->Childrens();
    if (ch.size()) {
      CHECK_EQ(ch.size(), 1);
      core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(ch[0].get());  // +
      if (array) {
        res.info = core::MakeServerProperty(array);
      }
    }
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ServerPropertyInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::ChangeServerPropertyInfoResponceEvent::value_type res(ev->value());

  NotifyProgress(sender, 50);
  std::string changeRequest = "CONFIG SET " + res.new_item.first + " " + res.new_item.second;
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(changeRequest, core::C_INNER);
  common::Error er = Execute(cmd);
  if (er && er->IsError()) {
    res.setErrorInfo(er);
  } else {
    res.is_change = true;
  }
  NotifyProgress(sender, 75);
  Reply(sender, new events::ChangeServerPropertyInfoResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

void Driver::HandleLoadServerChannelsRequestEvent(events::LoadServerChannelsRequestEvent* ev) {
  QObject* sender = ev->sender();
  NotifyProgress(sender, 0);
  events::LoadServerChannelsResponceEvent::value_type res(ev->value());

  NotifyProgress(sender, 50);
  const std::string loadChannelsRequest = common::MemSPrintf(REDIS_PUBSUB_CHANNELS_COMMAND_1ARGS_S, res.pattern);
  core::FastoObjectCommandIPtr cmd = CreateCommandFast(loadChannelsRequest, core::C_INNER);
  common::Error err = Execute(cmd);
  if (err && err->IsError()) {
    res.setErrorInfo(err);
    goto done;
  } else {
    core::FastoObject::childs_t rchildrens = cmd->Childrens();
    if (rchildrens.size()) {
      CHECK_EQ(rchildrens.size(), 1);
      core::FastoObjectArray* array = dynamic_cast<core::FastoObjectArray*>(rchildrens[0].get());  // +
      if (!array) {
        goto done;
      }

      common::ArrayValue* arm = array->Array();
      if (!arm->GetSize()) {
        goto done;
      }

      std::vector<core::FastoObjectCommandIPtr> cmds;
      cmds.reserve(arm->GetSize());
      for (size_t i = 0; i < arm->GetSize(); ++i) {
        std::string channel;
        bool isok = arm->GetString(i, &channel);
        if (isok) {
          core::NDbPSChannel c(channel, 0);
          cmds.push_back(
              CreateCommandFast(common::MemSPrintf(REDIS_PUBSUB_NUMSUB_COMMAND_1ARGS_S, channel), core::C_INNER));
          res.channels.push_back(c);
        }
      }

      err = impl_->ExecuteAsPipeline(cmds, &LOG_COMMAND);
      if (err && err->IsError()) {
        res.setErrorInfo(err);
        goto done;
      }

      for (size_t i = 0; i < res.channels.size(); ++i) {
        core::FastoObjectIPtr subCount = cmds[i];
        core::FastoObject::childs_t tchildrens = subCount->Childrens();
        if (tchildrens.size()) {
          DCHECK_EQ(tchildrens.size(), 1);
          if (tchildrens.size() == 1) {
            core::FastoObjectArray* array_sub = dynamic_cast<core::FastoObjectArray*>(tchildrens[0].get());  // +
            if (array_sub) {
              common::ArrayValue* array_sub_inner = array_sub->Array();
              common::Value* fund_sub = nullptr;
              if (array_sub_inner->Get(1, &fund_sub)) {
                std::string lsub_str;
                uint32_t lsub;
                if (fund_sub->GetAsString(&lsub_str) && common::ConvertFromString(lsub_str, &lsub)) {
                  res.channels[i].SetNumberOfSubscribers(lsub);
                }
              }
            }
          }
        }
      }
    }
  }

done:
  NotifyProgress(sender, 75);
  Reply(sender, new events::LoadServerChannelsResponceEvent(this, res));
  NotifyProgress(sender, 100);
}

core::IServerInfoSPtr Driver::MakeServerInfoFromString(const std::string& val) {
  core::IServerInfoSPtr res(core::redis::MakeRedisServerInfo(val));
  return res;
}

}  // namespace redis
}  // namespace proxy
}  // namespace fastonosql
