/*
 Copyright (c) 2013, Ford Motor Company
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following
 disclaimer in the documentation and/or other materials provided with the
 distribution.

 Neither the name of the Ford Motor Company nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "can_cooperation/can_module.h"
#include "can_cooperation/mobile_command_factory.h"
#include "can_cooperation/can_module_event.h"
#include "can_cooperation/event_engine/event_dispatcher.h"
#include "can_cooperation/can_module_constants.h"
#include "application_manager/application.h"
#include "can_cooperation/can_tcp_connection.h"
#include "can_cooperation/message_helper.h"
#include "can_cooperation/policy_helper.h"
#include "utils/logger.h"

namespace can_cooperation {

using namespace json_keys;

using functional_modules::ProcessResult;
using functional_modules::GenericModule;
using functional_modules::PluginInfo;
using functional_modules::MobileFunctionID;
using event_engine::EventDispatcher;
namespace hmi_api = functional_modules::hmi_api;

EXPORT_FUNCTION_IMPL(CANModule);
CREATE_LOGGERPTR_GLOBAL(logger_, "CanModule")

CANModule::CANModule()
  : GenericModule(kCANModuleID),
    can_connection_(new CANTCPConnection),
    from_can_("FromCan To Mobile", this),
    from_mobile_("FromMobile To Can", this),
    is_scan_started_(false) {
  can_connection_->set_observer(this);
  plugin_info_.name = "ReverseSDLPlugin";
  plugin_info_.version = 1;
  SubscribeOnFunctions();
}

void CANModule::SubscribeOnFunctions() {
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::TUNE_RADIO);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::TUNE_UP);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::TUNE_DOWN);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::GRANT_ACCESS);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::CANCEL_ACCESS);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::START_SCAN);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::STOP_SCAN);
  plugin_info_.mobile_function_list.push_back(
    MobileFunctionID::GET_SEAT_CONTROL);
  plugin_info_.mobile_function_list.push_back(
    MobileFunctionID::CLIMATE_CONTROL_ON);
  plugin_info_.mobile_function_list.push_back(
    MobileFunctionID::ON_CONTROL_CHANGED);
  plugin_info_.mobile_function_list.push_back(
    MobileFunctionID::ON_RADIO_DETAILS);
  plugin_info_.mobile_function_list.push_back(
    MobileFunctionID::ON_PRESETS_CHANGED);

  plugin_info_.hmi_function_list.push_back(hmi_api::grant_access);
  plugin_info_.hmi_function_list.push_back(hmi_api::cancel_access);
  plugin_info_.hmi_function_list.push_back(hmi_api::on_control_changed);
  plugin_info_.hmi_function_list.push_back(hmi_api::on_reverse_apps_allowing);
  plugin_info_.hmi_function_list.push_back(hmi_api::on_primary_device);
}

CANModule::~CANModule() {
  RemoveAppExtensions();
}

functional_modules::PluginInfo CANModule::GetPluginInfo() const {
  return plugin_info_;
}

ProcessResult CANModule::ProcessMessage(application_manager::MessagePtr msg) {
  DCHECK(msg);
  if (!msg) {
    LOG4CXX_ERROR(logger_, "Null pointer message received.");
    return ProcessResult::FAILED;
  }

  msg->set_function_name(MessageHelper::GetMobileAPIName(
      static_cast<functional_modules::MobileFunctionID>(msg->function_id())));

  commands::Command* command = MobileCommandFactory::CreateCommand(msg);
  if (command) {
    request_controller_.AddRequest(msg->correlation_id(), command);
    command->Run();
  } else {
    return ProcessResult::CANNOT_PROCESS;
  }

  return ProcessResult::PROCESSED;
}

void CANModule::SendMessageToCan(const MessageFromMobile& msg) {
  LOG4CXX_INFO(logger_, "Message to Can: " << msg);
  from_mobile_.PostMessage(msg);
}

ProcessResult CANModule::ProcessHMIMessage(
  application_manager::MessagePtr msg) {
  LOG4CXX_INFO(logger_, "HMI message: " << msg->json_message());
  return HandleMessage(msg);
}

void CANModule::OnCANMessageReceived(const CANMessage& message) {
  from_can_.PostMessage(MessageFromCAN(message));
}

void CANModule::OnCANConnectionError(ConnectionState state,
                                     const std::string& info) {
  if (ConnectionState::INVALID == state) {
    this->NotifyObservers(
      functional_modules::ModuleObserver::CAN_CONNECTION_FAILURE);
  }
  // TODO(PV): remove pending requests to CAN with error response.
}

void CANModule::Handle(const MessageFromMobile message) {
  if (ConnectionState::OPENED != can_connection_->SendMessage(message)) {
    LOG4CXX_ERROR(logger_, "Failed to send message to CAN");
  }
}

void CANModule::Handle(const MessageFromCAN can_msg) {
  application_manager::MessagePtr msg(
    new application_manager::Message(
      protocol_handler::MessagePriority::kDefault));

  Json::FastWriter writer;
  msg->set_json_message(writer.write(can_msg));

  LOG4CXX_INFO(logger_, "Can message: " << can_msg);

  if (HandleMessage(msg) != ProcessResult::PROCESSED) {
    LOG4CXX_ERROR(logger_, "Failed process CAN message!");
  }
}

functional_modules::ProcessResult CANModule::HandleMessage(
  application_manager::MessagePtr msg) {

  LOG4CXX_INFO(logger_, "CANModule::HandleMessage");

  Json::Value value;
  Json::Reader reader;
  reader.parse(msg->json_message(), value);

  std::string function_name;

  // Request or notification
  if (value.isMember(kMethod)) {
    function_name = value[kMethod].asCString();

    if (value.isMember(kId)) {
      msg->set_message_type(application_manager::MessageType::kRequest);
    } else {
      msg->set_message_type(application_manager::MessageType::kNotification);
    }
    // Response
  } else if (value.isMember(kResult) && value[kResult].isMember(kMethod)) {
    function_name = value[kResult][kMethod].asCString();
    msg->set_message_type(application_manager::MessageType::kResponse);
    // Error response
  } else if (value.isMember(kError) && value[kError].isMember(kData)
             && value[kError][kData].isMember(kMethod)) {
    function_name = value[kError][kData][kMethod].asCString();
    msg->set_message_type(application_manager::MessageType::kErrorResponse);
  } else {
    DCHECK(false);
    return ProcessResult::FAILED;
  }

  if (value.isMember(kId)) {
    msg->set_correlation_id(value[kId].asInt());
  } else if (application_manager::MessageType::kNotification != msg->type()) {
    DCHECK(false);
    return ProcessResult::FAILED;
  }

  msg->set_protocol_version(application_manager::ProtocolVersion::kV3);

  switch (msg->type()) {
    case application_manager::MessageType::kResponse:
    case application_manager::MessageType::kErrorResponse: {
      CanModuleEvent event(msg, function_name);
      EventDispatcher<application_manager::MessagePtr, std::string>::instance()
      ->raise_event(event);
      break;
    }
    case application_manager::MessageType::kNotification: {
      if (functional_modules::can_api::on_preset_changed == function_name) {
        msg->set_function_id(MobileFunctionID::ON_PRESETS_CHANGED);
      } else if (functional_modules::hmi_api::on_control_changed
                 == function_name) {
        msg->set_function_id(MobileFunctionID::ON_CONTROL_CHANGED);
      } else if (functional_modules::can_api::on_radio_details
                 == function_name) {
        msg->set_function_id(MobileFunctionID::ON_RADIO_DETAILS);
      } else if (functional_modules::hmi_api::on_reverse_apps_allowing
                 == function_name) {
        if (value.isMember(json_keys::kParams)) {
          if (value[json_keys::kParams].isMember(message_params::kAllowed)) {
            PolicyHelper::OnRSDLFunctionalityAllowing(
              value[json_keys::kParams][message_params::kAllowed].asBool());
          } else {
            LOG4CXX_ERROR(logger_, "Invalid OnReverseAppsAllowing notification");
          }
        }
        break;
      } else if (functional_modules::hmi_api::on_primary_device
                 == function_name) {
        if (value.isMember(json_keys::kParams)) {
          if (value[json_keys::kParams].isMember(message_params::kDevice)) {
            // TODO(KKolodiy): take out "input" from message
            // and give it into SetPrimaryDevice
            PolicyHelper::SetPrimaryDevice(
              value[json_keys::kParams][message_params::kDevice]
              [json_keys::kId].asInt(), "GUI");
          } else {
            LOG4CXX_ERROR(logger_, "Invalid OnPrimaryDevice notification");
          }
        }
        break;
      }

      msg->set_function_name(MessageHelper::GetMobileAPIName(
          static_cast<functional_modules::MobileFunctionID>(msg->function_id())));

      NotifyMobiles(msg);
      break;
    }
    case application_manager::MessageType::kRequest:
    default: {
      return ProcessResult::FAILED;;
    }
  }

  return ProcessResult::PROCESSED;
}

void CANModule::NotifyMobiles(application_manager::MessagePtr msg) {
  typedef std::vector<application_manager::ApplicationSharedPtr> AppList;
   AppList applications =
          service()->GetApplications(CANModule::instance()->GetModuleID());
  for (AppList::iterator i = applications.begin();
      i != applications.end(); ++i) {
    application_manager::MessagePtr message(
        new application_manager::Message(*msg));
    message->set_connection_key((*i)->app_id());

    commands::Command* command = MobileCommandFactory::CreateCommand(message);
    if (command) {
      command->Run();
    }
  }
}

void CANModule::SendResponseToMobile(application_manager::MessagePtr msg) {
  service()->SendMessageToMobile(msg);
  request_controller_.DeleteRequest(msg->correlation_id());
}

void CANModule::SendTimeoutResponseToMobile(
  application_manager::MessagePtr msg) {
  service()->SendMessageToMobile(msg);
}

bool CANModule::IsScanStarted() const {
  return is_scan_started_;
}

void CANModule::SetScanStarted(bool is_scan_started) {
  is_scan_started_ = is_scan_started;
}

void CANModule::RemoveAppExtensions() {
  std::vector<application_manager::ApplicationSharedPtr> applications =
    service()->GetApplications(this->GetModuleID());

  std::vector<application_manager::ApplicationSharedPtr>::iterator it =
    applications.begin();

  for (; it != applications.end(); ++it) {
    application_manager::ApplicationSharedPtr app = *it;
    if (app) {
      service()->ResetAccess(app->app_id());
      app->RemoveExtension(CANModule::instance()->GetModuleID());
    }
  }
}

void CANModule::RemoveAppExtension(uint32_t app_id) {
  application_manager::ApplicationSharedPtr app = service()->GetApplication(
        app_id);

  if (app) {
    service()->ResetAccess(app->app_id());
    app->RemoveExtension(kCANModuleID);
  }
}

}  //  namespace can_cooperation
