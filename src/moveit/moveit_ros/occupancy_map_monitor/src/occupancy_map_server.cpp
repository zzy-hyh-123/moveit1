
#include <memory>
#include <ros/ros.h>
#include <tf2_ros/transform_listener.h>
#include <moveit/occupancy_map_monitor/occupancy_map_monitor.h>
#include <octomap_msgs/conversions.h>

static const std::string LOGNAME = "occupancy_map_server";

static void publishOctomap(ros::Publisher& octree_binary_pub, occupancy_map_monitor::OccupancyMapMonitor& server)
{
  octomap_msgs::Octomap map;

  map.header.frame_id = server.getMapFrame();
  map.header.stamp = ros::Time::now();

  server.getOcTreePtr()->lockRead();
  try
  {
    if (!octomap_msgs::binaryMapToMsgData(*server.getOcTreePtr(), map.data))
      ROS_ERROR_THROTTLE_NAMED(1, LOGNAME, "Could not generate OctoMap message");
  }
  catch (...)
  {
    ROS_ERROR_THROTTLE_NAMED(1, LOGNAME, "Exception thrown while generating OctoMap message");
  }
  server.getOcTreePtr()->unlockRead();

  octree_binary_pub.publish(map);
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "occupancy_map_server");
  ros::NodeHandle nh;
  ros::Publisher octree_binary_pub = nh.advertise<octomap_msgs::Octomap>("octomap_binary", 1);
  std::shared_ptr<tf2_ros::Buffer> buffer = std::make_shared<tf2_ros::Buffer>(ros::Duration(5.0));
  std::shared_ptr<tf2_ros::TransformListener> listener = std::make_shared<tf2_ros::TransformListener>(*buffer, nh);
  occupancy_map_monitor::OccupancyMapMonitor server(buffer);
  server.setUpdateCallback([&octree_binary_pub, &server] { return publishOctomap(octree_binary_pub, server); });
  server.startMonitor();

  ros::spin();
  return 0;
}
