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

#include <memory>  // for enable_shared_from_this
#include <string>  // for string
#include <vector>  // for vector

#include <common/value.h>  // for ValueSPtr

#include "core/connection_types.h"     // for core::connectionTypes
#include "core/db_key.h"               // for NKey (ptr only), etc
#include "core/icommand_translator.h"  // for translator_t
#include "proxy/proxy_fwd.h"           // for IDatabaseSPtr

#include "core/database/idatabase_info.h"  // for IDataBaseInfoSPtr
#include "core/server/iserver_info.h"      // for IServerInfoSPtr, etc
#include "proxy/events/events.h"           // for BackupResponceEvent, etc
#include "proxy/server/iserver_base.h"     // for IServerBase

#include "core/global.h"  // for FastoObject (ptr only), etc

namespace fastonosql {
namespace proxy {
class IDriver;
}
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {

class IServer : public IServerBase, public std::enable_shared_from_this<IServer> {
  Q_OBJECT
 public:
  typedef core::IDataBaseInfoSPtr database_t;
  typedef std::vector<database_t> databases_t;
  virtual ~IServer();

  // sync methods
  void StopCurrentEvent();
  bool IsConnected() const;
  bool IsCanRemote() const;
  bool IsSupportTTLKeys() const;

  core::translator_t Translator() const;

  core::connectionTypes Type() const;
  virtual std::string Name() const override;

  database_t CurrentDatabaseInfo() const;
  core::IServerInfoSPtr CurrentServerInfo() const;

  std::string Delimiter() const;
  std::string NsSeparator() const;
  IDatabaseSPtr CreateDatabaseByInfo(core::IDataBaseInfoSPtr inf);
  database_t FindDatabase(core::IDataBaseInfoSPtr inf) const;

 Q_SIGNALS:  // only direct connections
  void ConnectStarted(const events_info::ConnectInfoRequest& req);
  void ConnectFinished(const events_info::ConnectInfoResponce& res);

  void DisconnectStarted(const events_info::DisConnectInfoRequest& req);
  void DisconnectFinished(const events_info::DisConnectInfoResponce& res);

  void ShutdownStarted(const events_info::ShutDownInfoRequest& req);
  void ShutdownFinished(const events_info::ShutDownInfoResponce& res);

  void BackupStarted(const events_info::BackupInfoRequest& req);
  void BackupFinished(const events_info::BackupInfoResponce& res);

  void ExportStarted(const events_info::ExportInfoRequest& req);
  void ExportFinished(const events_info::ExportInfoResponce& res);

  void ChangePasswordStarted(const events_info::ChangePasswordRequest& req);
  void ChangePasswordFinished(const events_info::ChangePasswordResponce& res);

  void ChangeMaxConnectionStarted(const events_info::ChangeMaxConnectionRequest& req);
  void ChangeMaxConnectionFinished(const events_info::ChangeMaxConnectionResponce& res);

  void ExecuteStarted(const events_info::ExecuteInfoRequest& req);
  void ExecuteFinished(const events_info::ExecuteInfoResponce& res);

  void LoadDatabasesStarted(const events_info::LoadDatabasesInfoRequest& req);
  void LoadDatabasesFinished(const events_info::LoadDatabasesInfoResponce& res);

  void LoadServerInfoStarted(const events_info::ServerInfoRequest& req);
  void LoadServerInfoFinished(const events_info::ServerInfoResponce& res);

  void LoadServerHistoryInfoStarted(const events_info::ServerInfoHistoryRequest& req);
  void LoadServerHistoryInfoFinished(const events_info::ServerInfoHistoryResponce& res);

  void ClearServerHistoryStarted(const events_info::ClearServerHistoryRequest& req);
  void ClearServerHistoryFinished(const events_info::ClearServerHistoryResponce& req);

  void LoadServerPropertyStarted(const events_info::ServerPropertyInfoRequest& req);
  void LoadServerPropertyFinished(const events_info::ServerPropertyInfoResponce& res);

  void ChangeServerPropertyStarted(const events_info::ChangeServerPropertyInfoRequest& req);
  void ChangeServerPropertyFinished(const events_info::ChangeServerPropertyInfoResponce& res);

  void LoadServerChannelsStarted(const events_info::LoadServerChannelsRequest& req);
  void LoadServerChannelsFinished(const events_info::LoadServerChannelsResponce& res);

  void ProgressChanged(const events_info::ProgressInfoResponce& res);

  void ModeEntered(const events_info::EnterModeInfo& res);
  void ModeLeaved(const events_info::LeaveModeInfo& res);

  void RootCreated(const events_info::CommandRootCreatedInfo& res);
  void RootCompleated(const events_info::CommandRootCompleatedInfo& res);

  void LoadDataBaseContentStarted(const events_info::LoadDatabaseContentRequest& req);
  void LoadDatabaseContentFinished(const events_info::LoadDatabaseContentResponce& res);

  void LoadDiscoveryInfoStarted(const events_info::DiscoveryInfoRequest& res);
  void LoadDiscoveryInfoFinished(const events_info::DiscoveryInfoResponce& res);

 Q_SIGNALS:
  void ChildAdded(core::FastoObjectIPtr child);
  void ItemUpdated(core::FastoObject* item, common::ValueSPtr val);
  void ServerInfoSnapShoot(core::ServerInfoSnapShoot shot);

  void FlushedDB(core::IDataBaseInfoSPtr db);
  void CurrentDataBaseChanged(core::IDataBaseInfoSPtr db);
  void KeyRemoved(core::IDataBaseInfoSPtr db, core::NKey key);
  void KeyAdded(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void KeyLoaded(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void KeyRenamed(core::IDataBaseInfoSPtr db, core::NKey key, core::string_key_t new_name);
  void KeyTTLChanged(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl);
  void Disconnected();

 public:
  // async methods
  void Connect(const events_info::ConnectInfoRequest& req);              // signals: ConnectStarted, ConnectFinished
  void Disconnect(const events_info::DisConnectInfoRequest& req);        // signals: DisconnectStarted,
                                                                         // DisconnectFinished
  void LoadDatabases(const events_info::LoadDatabasesInfoRequest& req);  // signals: LoadDatabasesStarted,
                                                                         // LoadDatabasesFinished
  void LoadDatabaseContent(const events_info::LoadDatabaseContentRequest& req);  // signals: LoadDataBaseContentStarted,
                                                                                 // LoadDatabaseContentFinished
  void Execute(const events_info::ExecuteInfoRequest& req);                      // signals: ExecuteStarted

  void ShutDown(const events_info::ShutDownInfoRequest& req);                 // signals: ShutdownStarted,
                                                                              // ShutdownFinished
  void BackupToPath(const events_info::BackupInfoRequest& req);               // signals: BackupStarted, BackupFinished
  void ExportFromPath(const events_info::ExportInfoRequest& req);             // signals: ExportStarted, ExportFinished
  void ChangePassword(const events_info::ChangePasswordRequest& req);         // signals: ChangePasswordStarted,
                                                                              // ChangePasswordFinished
  void SetMaxConnection(const events_info::ChangeMaxConnectionRequest& req);  // signals: ChangeMaxConnectionStarted,
                                                                              // ChangeMaxConnectionFinished
  void LoadServerInfo(const events_info::ServerInfoRequest& req);             // signals:
  // LoadServerInfoStarted,
  // LoadServerInfoFinished
  void ServerProperty(const events_info::ServerPropertyInfoRequest& req);     // signals: LoadServerPropertyStarted,
                                                                              // LoadServerPropertyFinished
  void RequestHistoryInfo(const events_info::ServerInfoHistoryRequest& req);  // signals: LoadServerHistoryInfoStarted,
                                                                              // LoadServerHistoryInfoFinished
  void ClearHistory(const events_info::ClearServerHistoryRequest& req);       // signals: ClearServerHistoryStarted,
                                                                              // ClearServerHistoryFinished
  void ChangeProperty(
      const events_info::ChangeServerPropertyInfoRequest& req);  // signals: ChangeServerPropertyStarted,
                                                                 // ChangeServerPropertyFinished

  void LoadChannels(const events_info::LoadServerChannelsRequest& req);  // signals: LoadServerChannelsStarted,
                                                                         // LoadServerChannelsFinished

 protected:
  explicit IServer(IDriver* drv);  // take ownerships

  void StartCheckKeyExistTimer();
  void StopCheckKeyExistTimer();

  virtual void customEvent(QEvent* event) override;
  virtual void timerEvent(QTimerEvent* event) override;

  virtual IDatabaseSPtr CreateDatabase(core::IDataBaseInfoSPtr info) = 0;
  void Notify(QEvent* ev);

  // handle server events
  virtual void HandleConnectEvent(events::ConnectResponceEvent* ev);
  virtual void HandleDisconnectEvent(events::DisconnectResponceEvent* ev);
  virtual void HandleLoadServerInfoEvent(events::ServerInfoResponceEvent* ev);
  virtual void HandleLoadServerPropertyEvent(events::ServerPropertyInfoResponceEvent* ev);
  virtual void HandleServerPropertyChangeEvent(events::ChangeServerPropertyInfoResponceEvent* ev);
  virtual void HandleLoadServerChannelsEvent(events::LoadServerChannelsResponceEvent* ev);
  virtual void HandleShutdownEvent(events::ShutDownResponceEvent* ev);
  virtual void HandleBackupEvent(events::BackupResponceEvent* ev);
  virtual void HandleExportEvent(events::ExportResponceEvent* ev);
  virtual void HandleChangePasswordEvent(events::ChangePasswordResponceEvent* ev);
  virtual void HandleChangeMaxConnectionEvent(events::ChangeMaxConnectionResponceEvent* ev);
  virtual void HandleExecuteEvent(events::ExecuteResponceEvent* ev);

  // handle database events
  virtual void HandleLoadDatabaseInfosEvent(events::LoadDatabasesInfoResponceEvent* ev);
  virtual void HandleLoadDatabaseContentEvent(events::LoadDatabaseContentResponceEvent* ev);

  // handle command events
  virtual void HandleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev);

  IDriver* const drv_;
  databases_t databases_;

 private Q_SLOTS:
  void FlushDB();
  void CurrentDataBaseChange(core::IDataBaseInfoSPtr db);

  void KeyRemove(core::NKey key);
  void KeyAdd(core::NDbKValue key);
  void KeyLoad(core::NDbKValue key);
  void KeyRename(core::NKey key, core::string_key_t new_name);
  void KeyTTLChange(core::NKey key, core::ttl_t ttl);
  void KeyTTLLoad(core::NKey key, core::ttl_t ttl);

 private:
  void HandleCheckDBKeys(core::IDataBaseInfoSPtr db, core::ttl_t expired_time);

  void HandleEnterModeEvent(events::EnterModeEvent* ev);
  void HandleLeaveModeEvent(events::LeaveModeEvent* ev);

  // handle info events
  void HandleLoadServerInfoHistoryEvent(events::ServerInfoHistoryResponceEvent* ev);
  void HandleClearServerHistoryResponceEvent(events::ClearServerHistoryResponceEvent* ev);

  void ProcessDiscoveryInfo(const events_info::DiscoveryInfoRequest& req);

  core::IServerInfoSPtr server_info_;
  database_t current_database_info_;
  int timer_check_key_exists_id_;
};

}  // namespace proxy
}  // namespace fastonosql
