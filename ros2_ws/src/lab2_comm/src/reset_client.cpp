#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "lab2_interfaces/srv/reset_filter.hpp"

using namespace std::chrono_literals;

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<rclcpp::Node>("reset_client");

  auto client =
    node->create_client<lab2_interfaces::srv::ResetFilter>(
      "reset_filter"
    );

  if (!client->wait_for_service(5s))
  {
    RCLCPP_ERROR(
      node->get_logger(),
      "Service unavailable (5s timeout)"
    );

    rclcpp::shutdown();
    return 1;
  }

  auto request = std::make_shared<
    lab2_interfaces::srv::ResetFilter::Request
  >();

  auto future = client->async_send_request(request);

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);

  std::thread worker(
    [&executor]()
    {
      executor.spin();
    }
  );

  auto status = future.wait_for(5s);

  if (status == std::future_status::ready)
  {
    RCLCPP_INFO(
      node->get_logger(),
      "Reset reply: %s",
      future.get()->message.c_str()
    );
  }
  else
  {
    RCLCPP_ERROR(
      node->get_logger(),
      "Reset request timed out or failed"
    );
  }

  executor.cancel();
  worker.join();

  rclcpp::shutdown();

  return status == std::future_status::ready ? 0 : 1;
}
