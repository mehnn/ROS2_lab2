
cmake_minimum_required(VERSION 3.8)
project(lab2_comm)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(lab2_interfaces REQUIRED)

foreach(target
  data_filter
  reset_client
  process_array_server
)
  add_executable(${target} src/${target}.cpp)

  ament_target_dependencies(${target}
    rclcpp
    rclcpp_action
    std_msgs
    lab2_interfaces
  )

  install(
    TARGETS ${target}
    DESTINATION lib/${PROJECT_NAME}
  )
endforeach()

ament_package()
