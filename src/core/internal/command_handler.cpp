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

#include "core/internal/command_handler.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint16_t

#include <string>  // for string

extern "C" {
#include "sds.h"
}

#include <common/utils.h>
#include <common/value.h>  // for ErrorValue, etc

namespace fastonosql {
namespace core {
namespace internal {

CommandHandler::CommandHandler(ICommandTranslator* translator) : translator_(translator) {}

common::Error CommandHandler::Execute(const command_buffer_t& command, FastoObject* out) {
  command_buffer_t stabled_command = StableCommand(command);
  if (stabled_command.empty()) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  int argc;
  sds* argv = sdssplitargslong(stabled_command.data(), &argc);
  if (!argv) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  commands_args_t argvv;
  for (int i = 0; i < argc; ++i) {
    std::string str(argv[i], sdslen(argv[i]));
    argvv.push_back(str);
  }
  common::Error err = Execute(argvv, out);
  sdsfreesplitres(argv, argc);
  return err;
}

common::Error CommandHandler::Execute(commands_args_t argv, FastoObject* out) {
  const command_t* cmd = nullptr;
  size_t off = 0;
  common::Error err = translator_->TestCommandLineArgs(argv, &cmd, &off);
  if (err && err->IsError()) {
    return err;
  }

  commands_args_t stabled;
  for (size_t i = off; i < argv.size(); ++i) {
    stabled.push_back(argv[i]);
  }
  return cmd->func_(this, stabled, out);
}

}  // namespace internal
}  // namespace core
}  // namespace fastonosql
