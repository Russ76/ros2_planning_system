// Copyright 2019 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <memory>
#include <vector>

#include "lifecycle_msgs/msg/transition.hpp"
#include "lifecycle_msgs/msg/state.hpp"

#include "plansys2_executor/ActionExecutorClient.hpp"

namespace plansys2
{

ActionExecutorClient::ActionExecutorClient(
  const std::string & node_name,
  const std::chrono::nanoseconds & rate)
: CascadeLifecycleNode(node_name),
  rate_(rate),
  commited_(false)
{
  declare_parameter("action_name");
  declare_parameter("specialized_arguments");
}

using CallbackReturnT =
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
using std::placeholders::_1;

CallbackReturnT
ActionExecutorClient::on_configure(const rclcpp_lifecycle::State & state)
{
  if (!get_parameter("action_name", action_managed_)) {
    RCLCPP_ERROR(get_logger(), "action_name parameter not set");
  }
  get_parameter_or<std::vector<std::string>>(
    "specialized_arguments", specialized_arguments_, std::vector<std::string>({}));

  action_hub_pub_ = create_publisher<plansys2_msgs::msg::ActionExecution>(
    "/actions_hub", rclcpp::QoS(100).reliable());
  action_hub_sub_ = create_subscription<plansys2_msgs::msg::ActionExecution>(
    "/actions_hub", rclcpp::QoS(100).reliable(),
    std::bind(&ActionExecutorClient::action_hub_callback, this, _1));

  action_hub_pub_->on_activate();

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
ActionExecutorClient::on_activate(const rclcpp_lifecycle::State & state)
{
  timer_ = create_wall_timer(
    rate_, std::bind(&ActionExecutorClient::do_work, this));

  return CallbackReturnT::SUCCESS;
}

CallbackReturnT
ActionExecutorClient::on_deactivate(const rclcpp_lifecycle::State & state)
{
  timer_ = nullptr;

  return CallbackReturnT::SUCCESS;
}

void
ActionExecutorClient::action_hub_callback(const plansys2_msgs::msg::ActionExecution::SharedPtr msg)
{
  switch (msg->type) {
    case plansys2_msgs::msg::ActionExecution::REQUEST:
      if (get_current_state().id() == lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE &&
        !commited_ && should_execute(msg->action, msg->arguments))
      {
        commited_ = true;
        send_response(msg);
      }
      break;
    case plansys2_msgs::msg::ActionExecution::CONFIRM:
      if (get_current_state().id() == lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE &&
        commited_ && msg->node_id == get_name())
      {
        current_arguments_ = msg->arguments;
        trigger_transition(lifecycle_msgs::msg::Transition::TRANSITION_ACTIVATE);
        commited_ = false;
      }
      break;
    case plansys2_msgs::msg::ActionExecution::REJECT:
      if (msg->node_id == get_name()) {
        commited_ = false;
      }
      break;
    case plansys2_msgs::msg::ActionExecution::RESPONSE:
    case plansys2_msgs::msg::ActionExecution::FEEDBACK:
    case plansys2_msgs::msg::ActionExecution::FINISH:
      break;
    default:
      RCLCPP_ERROR(
        get_logger(), "Msg %d type not recognized in %s executor performer",
        msg->type, get_name());
      break;
  }
}

bool
ActionExecutorClient::should_execute(
  const std::string & action, const std::vector<std::string> & args)
{
  if (action != action_managed_) {
    return false;
  }

  if (!specialized_arguments_.empty()) {
    if (specialized_arguments_.size() != args.size()) {
      RCLCPP_WARN(
        get_logger(), "current and specialized arguments lenght doesn't match %zu %zu",
        args.size(), specialized_arguments_.size());
    }

    for (int i = 0; i < specialized_arguments_.size() && i < args.size(); i++) {
      if (specialized_arguments_[i] != "" && args[i] != "" &&
        specialized_arguments_[i] != args[i])
      {
        return false;
      }
    }
  }

  return true;
}

void
ActionExecutorClient::send_response(
  const plansys2_msgs::msg::ActionExecution::SharedPtr msg)
{
  plansys2_msgs::msg::ActionExecution msg_resp(*msg);
  msg_resp.type = plansys2_msgs::msg::ActionExecution::RESPONSE;
  msg_resp.node_id = get_name();

  action_hub_pub_->publish(msg_resp);
}

void
ActionExecutorClient::send_feedback(float completion, const std::string & status)
{
  plansys2_msgs::msg::ActionExecution msg_resp;
  msg_resp.type = plansys2_msgs::msg::ActionExecution::FEEDBACK;
  msg_resp.node_id = get_name();
  msg_resp.action = action_managed_;
  msg_resp.arguments = current_arguments_;
  msg_resp.completion = completion;
  msg_resp.status = status;

  action_hub_pub_->publish(msg_resp);
}

void
ActionExecutorClient::finish(bool success, float completion, const std::string & status)
{
  trigger_transition(lifecycle_msgs::msg::Transition::TRANSITION_DEACTIVATE);

  plansys2_msgs::msg::ActionExecution msg_resp;
  msg_resp.type = plansys2_msgs::msg::ActionExecution::FINISH;
  msg_resp.node_id = get_name();
  msg_resp.action = action_managed_;
  msg_resp.arguments = current_arguments_;
  msg_resp.completion = completion;
  msg_resp.status = status;
  msg_resp.success = success;

  action_hub_pub_->publish(msg_resp);
}

}  // namespace plansys2
