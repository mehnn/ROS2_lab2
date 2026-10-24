
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "lab2_interfaces/action/process_array.hpp"

using namespace std::chrono_literals;

using ProcessArray = lab2_interfaces::action::ProcessArray;
using GoalHandle =
  rclcpp_action::ServerGoalHandle<ProcessArray>;

class ProcessArrayServer : public rclcpp::Node
{
public:
  ProcessArrayServer() : Node("process_array_server")
  {
    using namespace std::placeholders;

    server_ = rclcpp_action::create_server<ProcessArray>(
      this,
      "process_array",
      std::bind(&ProcessArrayServer::goal, this, _1, _2),
      std::bind(&ProcessArrayServer::cancel, this, _1),
      std::bind(&ProcessArrayServer::accepted, this, _1)
    );

    RCLCPP_INFO(
      get_logger(),
      "ProcessArray action server ready"
    );
  }

private:
  rclcpp_action::GoalResponse goal(
    const rclcpp_action::GoalUUID &,
    std::shared_ptr<const ProcessArray::Goal> request)
  {
    RCLCPP_INFO(
      get_logger(),
      "Received %zu numbers",
      request->numbers.size()
    );

    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse cancel(
    const std::shared_ptr<GoalHandle>)
  {
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void accepted(const std::shared_ptr<GoalHandle> handle)
  {
    std::thread(
      [this, handle]()
      {
        execute(handle);
      }
    ).detach();
  }

  void execute(const std::shared_ptr<GoalHandle> handle)
  {
    auto result = std::make_shared<ProcessArray::Result>();
    auto feedback = std::make_shared<ProcessArray::Feedback>();

    for (const auto value : handle->get_goal()->numbers)
    {
      if (handle->is_canceling())
      {
        handle->canceled(result);

        RCLCPP_INFO(get_logger(), "Goal canceled");
        return;
      }

      result->sum += value;
      feedback->processed_count++;

      handle->publish_feedback(feedback);

      std::this_thread::sleep_for(500ms);
    }

    if (rclcpp::ok())
    {
      handle->succeed(result);

      RCLCPP_INFO(
        get_logger(),
        "Goal succeeded, sum=%ld",
        static_cast<long>(result->sum)
      );
    }
  }

  rclcpp_action::Server<ProcessArray>::SharedPtr server_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::spin(
    std::make_shared<ProcessArrayServer>()
  );

  rclcpp::shutdown();

  return 0;
}
