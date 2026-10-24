
#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "lab2_interfaces/srv/reset_filter.hpp"

class DataFilter : public rclcpp::Node
{
public:
  DataFilter() : Node("data_filter")
  {
    rclcpp::QoS qos(rclcpp::KeepLast(1));
    qos.transient_local().reliable();

    rclcpp::PublisherOptions options;

    options.qos_overriding_options =
      rclcpp::QosOverridingOptions({
        rclcpp::QosPolicyKind::Depth,
        rclcpp::QosPolicyKind::Reliability,
        rclcpp::QosPolicyKind::Durability
      });

    options.event_callbacks.matched_callback =
      [this](rclcpp::MatchedInfo & info)
      {
        RCLCPP_INFO(
          get_logger(),
          "Output subscribers: %d",
          info.current_count
        );

        if (info.current_count > 0 && !input_)
        {
          input_ = create_subscription<std_msgs::msg::String>(
            "input_topic",
            10,
            [this](const std_msgs::msg::String::SharedPtr msg)
            {
              std_msgs::msg::String out;
              out.data = msg->data;

              std::transform(
                out.data.begin(),
                out.data.end(),
                out.data.begin(),
                [](unsigned char c)
                {
                  return static_cast<char>(std::toupper(c));
                }
              );

              output_->publish(out);

              RCLCPP_INFO(
                get_logger(),
                "Published: %s",
                out.data.c_str()
              );
            }
          );

          RCLCPP_INFO(
            get_logger(),
            "Input subscription started"
          );
        }
        else if (info.current_count == 0 && input_)
        {
          input_.reset();

          RCLCPP_INFO(
            get_logger(),
            "Input subscription stopped"
          );
        }
      };

    output_ = create_publisher<std_msgs::msg::String>(
      "uppercase_topic",
      qos,
      options
    );

    reset_ = create_service<lab2_interfaces::srv::ResetFilter>(
      "reset_filter",
      [this](
        const std::shared_ptr<
          lab2_interfaces::srv::ResetFilter::Request>,
        std::shared_ptr<
          lab2_interfaces::srv::ResetFilter::Response> response)
      {
        response->success = true;
        response->message = "Filter reset (stateless example)";

        RCLCPP_INFO(
          get_logger(),
          "ResetFilter called"
        );
      }
    );

    reset_->configure_introspection(
      get_clock(),
      rclcpp::SystemDefaultsQoS(),
      RCL_SERVICE_INTROSPECTION_CONTENTS
    );

    RCLCPP_INFO(get_logger(), "DataFilter ready");
  }

private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr output_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr input_;
  rclcpp::Service<
    lab2_interfaces::srv::ResetFilter
  >::SharedPtr reset_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DataFilter>());
  rclcpp::shutdown();
  return 0;
}
