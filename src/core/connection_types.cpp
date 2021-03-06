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

#include "core/connection_types.h"

#include <stddef.h>  // for size_t
#include <string>    // for string, operator==

#include <common/convert2string.h>
#include <common/macros.h>  // for NOTREACHED, SIZEOFMASS

namespace {
const char* connnectionType[] = {"Redis",   "Memcached", "SSDB",      "LevelDB", "RocksDB",
                                 "UnQLite", "LMDB",      "UpscaleDB", "ForestDB"};
const std::string connnectionMode[] = {"Interactive mode"};
const std::string serverTypes[] = {"Master", "Slave"};
const std::string serverState[] = {"Up", "Down"};
const std::string serverModes[] = {"Standalone", "Sentinel", "Cluster"};
}  // namespace

namespace fastonosql {
namespace core {

bool IsRemoteType(connectionTypes type) {
  return type == REDIS || type == MEMCACHED || type == SSDB;
}

bool IsSupportTTLKeys(connectionTypes type) {
  return type == REDIS || type == MEMCACHED || type == SSDB;
}

bool IsLocalType(connectionTypes type) {
  return type == ROCKSDB || type == LEVELDB || type == LMDB || type == UPSCALEDB || type == UNQLITE || type == FORESTDB;
}

bool IsCanSSHConnection(connectionTypes type) {
  return type == REDIS;
}

const char* CommandLineHelpText(connectionTypes type) {
  if (type == REDIS) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-h &lt;hostname&gt;</b>      Server "
           "hostname (default: "
           "127.0.0.1).<br/>"
           "<b>-p &lt;port&gt;</b>          Server port "
           "(default: 6379).<br/>"
           "<b>-s &lt;socket&gt;</b>        Server socket "
           "(overrides hostname "
           "and port).<br/>"
           "<b>-a &lt;password&gt;</b>      Password to "
           "use when connecting to "
           "the server.<br/>"
           "<b>-i &lt;interval&gt; seconds per command.<br/>"
           "                   It is possible to specify "
           "sub-second times like "
           "<b>-i</b> 0.1.<br/>"
           "<b>-n &lt;db&gt;</b>            Database "
           "number.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>"
           "<b>-c</b>                 Enable cluster mode "
           "(follow -ASK and "
           "-MOVED "
           "redirections).<br/>"
           "<b>--latency</b>          Enter a special mode "
           "continuously "
           "sampling latency.<br/>"
           "<b>--latency-history</b>  Like "
           "<b>--latency</b> but tracking "
           "latency changes over "
           "time.<br/>"
           "                   Default time interval is 15 "
           "sec. Change it "
           "using <b>-i</b>.<br/>"
           "<b>--slave</b>            Simulate a slave "
           "showing commands "
           "received from the "
           "master.<br/>"
           "<b>--rdb &lt;filename&gt;</b>   Transfer an "
           "RDB dump from remote "
           "server to local "
           "file.<br/>"
           /*"<b>--pipe</b>             Transfer raw Redis
           protocol from stdin
           to server.<br/>"
           "<b>--pipe-timeout &lt;n&gt;</b> In <b>--pipe
           mode</b>, abort with
           error if after sending
           all data.<br/>"
           "                   no reply is received within
           &lt;n&gt;
           seconds.<br/>"
           "                   Default timeout: %d. Use 0 to
           wait
           forever.<br/>"*/
           "<b>--bigkeys</b>          Sample Redis keys "
           "looking for big "
           "keys.<br/>"
           "<b>--scan</b>             List all keys using "
           "the SCAN "
           "command.<br/>"
           "<b>--pattern &lt;pat&gt;</b>    Useful with "
           "<b>--scan</b> to "
           "specify a SCAN "
           "pattern.<br/>"
           "<b>--intrinsic-latency &lt;sec&gt;</b> Run a "
           "test to measure "
           "intrinsic system "
           "latency.<br/>"
           "                   The test will run for the "
           "specified amount of "
           "seconds.<br/>";
  }
  if (type == MEMCACHED) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-h &lt;hostname&gt;</b>      Server "
           "hostname (default: "
           "127.0.0.1).<br/>"
           "<b>-p &lt;port&gt;</b>          Server port "
           "(default: 11211).<br/>"
           "<b>-u &lt;username&gt;</b>      Username to "
           "use when connecting to "
           "the server.<br/>"
           "<b>-a &lt;password&gt;</b>      Password to "
           "use when connecting to "
           "the server.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>";
  }
  if (type == SSDB) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-u &lt;username&gt;</b>      Username to "
           "use when connecting to "
           "the server.<br/>"
           "<b>-a &lt;password&gt;</b>      Password to "
           "use when connecting to "
           "the server.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>";
  }
  if (type == LEVELDB) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-f &lt;db&gt;</b>            Directory path to "
           "database.<br/>"
           "<b>-c </b>            Create database if "
           "missing.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>";
  }
  if (type == ROCKSDB) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-f &lt;db&gt;</b>            Directory path to "
           "database.<br/>"
           "<b>-c </b>            Create database if "
           "missing.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>";
  }
  if (type == UNQLITE) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-f &lt;db&gt;</b>            File path to "
           "database.<br/>"
           "<b>-c </b>            Create database if "
           "missing.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>";
  }
  if (type == LMDB) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-f &lt;db&gt;</b>            Directory path to "
           "database.<br/>"
           "<b>-c </b>            Create database if "
           "missing.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>";
  }
  if (type == UPSCALEDB) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-f &lt;db&gt;</b>            Directory path to "
           "database.<br/>"
           "<b>-c </b>            Create database if "
           "missing.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>";
  }
  if (type == FORESTDB) {
    return "<b>Usage: [OPTIONS] [cmd [arg [arg "
           "...]]]</b><br/>"
           "<b>-f &lt;db&gt;</b>            Directory path to "
           "database.<br/>"
           "<b>-c </b>            Create database if "
           "missing.<br/>"
           "<b>-d &lt;delimiter&gt;</b>     Multi-bulk "
           "delimiter in for raw "
           "formatting (default: "
           "\\n).<br/>"
           "<b>-ns &lt;separator&gt;</b>    Namespace "
           "separator.<br/>";
  }

  NOTREACHED();
  return nullptr;
}

const char* ConnectionTypeToString(connectionTypes t) {
  return connnectionType[t];
}

}  // namespace core
}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::core::connectionTypes t) {
  return fastonosql::core::ConnectionTypeToString(t);
}

bool ConvertFromString(const std::string& from, fastonosql::core::connectionTypes* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < SIZEOFMASS(connnectionType); ++i) {
    if (from == connnectionType[i]) {
      *out = static_cast<fastonosql::core::connectionTypes>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

bool ConvertFromString(const std::string& from, fastonosql::core::serverTypes* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < SIZEOFMASS(serverTypes); ++i) {
    if (from == serverTypes[i]) {
      *out = static_cast<fastonosql::core::serverTypes>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

bool ConvertFromString(const std::string& from, fastonosql::core::serverState* out) {
  for (size_t i = 0; i < SIZEOFMASS(serverState); ++i) {
    if (from == serverState[i]) {
      *out = static_cast<fastonosql::core::serverState>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

std::string ConvertToString(fastonosql::core::serverTypes st) {
  return serverTypes[st];
}

std::string ConvertToString(fastonosql::core::serverState st) {
  return serverState[st];
}

std::string ConvertToString(fastonosql::core::serverMode md) {
  return serverModes[md];
}

std::string ConvertToString(fastonosql::core::ConnectionMode t) {
  return connnectionMode[t];
}

}  // namespace common
