/*
 * Copyright (c) 2018, Ford Motor Company
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

#ifndef SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_HELP_PROMPT_MANAGER_IMPL_H_
#define SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_HELP_PROMPT_MANAGER_IMPL_H_

#include "application_manager/help_prompt_manager.h"
#include "utils/timer.h"

namespace application_manager {

class Application;
class ApplicationManager;

/**
 * @brief The HelpPromptManagerImpl class implemented interface
 * HelpPromptManager
 * is a mechanism for handling "help" requests
 * by the user when the application does not provide any or such information.
 */
class HelpPromptManagerImpl : public HelpPromptManager {
 public:
  /**
   * @brief Continer for buffering VR help commands
  */
  typedef std::map<uint32_t, smart_objects::SmartObjectSPtr> VRCommandsMap;
  /**
   * @brief Class constructor
   * @param app owner for this class
   */
  HelpPromptManagerImpl(Application& app, ApplicationManager& app_manager);
  /**
   * @brief Class destructor
   */
  ~HelpPromptManagerImpl();
  /**
   * @brief The type sends info to HMI
   */
  enum class SendingType { kNoneSend, kSendHelpPrompt, kSendVRHelp, kSendBoth };

  /**
   * @brief Adds command to constructed values, and send SetGlobalProperties if
   * required
   */
  void OnVrCommandAdded(const uint32_t cmd_id,
                        const smart_objects::SmartObject& command) OVERRIDE;
  /**
   * @brief Removes command from constructed values, and send
   * SetGlobalProperties if required
   */
  void OnVrCommandDeleted(const uint32_t cmd_id) OVERRIDE;
  /**
     * @brief Stop constructing vrHelp and/or helpPrompt if they are present in
     * message
     * @param msg containing GlobalProperties
     * @param is_response determines is the request or response for the for
     * the SetGlobalPropertiesRequest
     */
  void OnSetGlobalPropertiesReceived(const smart_objects::SmartObject& msg,
                                     const bool is_response) OVERRIDE;
  /**
    * @brief Starts mechanism for handling "help" requests
    * @param is_restore determines the need to restore commands
    * after resuming
    */
  void OnAppActivated(const bool is_restore) OVERRIDE;
  /**
    * @brief Stop mechanism for handling "help" requests
    */
  void OnAppUnregistered() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(HelpPromptManagerImpl);
  /**
   * @brief Send TTS request to HMI
   */
  void SendTTSRequest();
  /**
   * @brief Send UI request to HMI
   */
  void SendUIRequest();
  /**
   * @brief Send TTS and UI requests to HMI
   */
  void SendBothRequests();
  /**
   * @brief Send TTS or UI or both Requests
   */
  void SendRequests();
  /**
   * @brief Construct the helpPrompt parameter
   */
  void CreatePromptMsg(smart_objects::SmartObject& out_msg_params);
  /**
   * @brief Construct the vrHelp parameter
   */
  void CreateVRMsg(smart_objects::SmartObject& out_msg_params);
  /**
   * @brief Getting request type to send HMI
   * @param msg containing request or response for the
   * SetGlobalPropertiesRequest
   * @param is_response determines is the request or response for the for
   * the SetGlobalPropertiesRequest
   * @return SendingType
   */
  SendingType GetSendingType(const smart_objects::SmartObject& msg,
                             const bool is_response);

  Application& app_;
  ApplicationManager& app_manager_;
  VRCommandsMap vr_commands_;
  SendingType sending_type_;
  std::size_t count_requests_commands_;
  bool is_tts_send_;
  bool is_ui_send_;
};

}  //  namespace application_manager

#endif  // SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_HELP_PROMPT_MANAGER_IMPL_H_