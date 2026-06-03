
#pragma once

#include <ros/ros.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/message_filter.h>
#include <message_filters/subscriber.h>
#include <sensor_msgs/PointCloud2.h>
#include <moveit/occupancy_map_monitor/occupancy_map_updater.h>
#include <moveit/point_containment_filter/shape_mask.h>

#include <memory>

#include <unordered_set>
#include <octomap/OcTreeStamped.h>
namespace occupancy_map_monitor
{
class PointCloudOctomapUpdater : public OccupancyMapUpdater
{
public:
  PointCloudOctomapUpdater();
  ~PointCloudOctomapUpdater() override;

  bool setParams(XmlRpc::XmlRpcValue& params) override;

  bool initialize() override;
  void start() override;
  void stop() override;
  ShapeHandle excludeShape(const shapes::ShapeConstPtr& shape) override;
  void forgetShape(ShapeHandle handle) override;

protected:
  virtual void updateMask(const sensor_msgs::PointCloud2& cloud, const Eigen::Vector3d& sensor_origin,
                          std::vector<int>& mask);

private:
  bool getShapeTransform(ShapeHandle h, Eigen::Isometry3d& transform) const;
  void cloudMsgCallback(const sensor_msgs::PointCloud2::ConstPtr& cloud_msg);
  void stopHelper();

  ros::NodeHandle root_nh_;
  ros::NodeHandle private_nh_;

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

  ros::Time last_update_time_;

  /* params */
  std::string point_cloud_topic_;
  double scale_;
  double padding_;
  double max_range_;
  unsigned int point_subsample_;
  double max_update_rate_;
  std::string filtered_cloud_topic_;
  std::string ns_;
  ros::Publisher filtered_cloud_publisher_;

  message_filters::Subscriber<sensor_msgs::PointCloud2>* point_cloud_subscriber_;
  tf2_ros::MessageFilter<sensor_msgs::PointCloud2>* point_cloud_filter_;

  /* used to store all cells in the map which a given ray passes through during raycasting.
     we cache this here because it dynamically pre-allocates a lot of memory in its contsructor */
  octomap::KeyRay key_ray_;

  std::unique_ptr<point_containment_filter::ShapeMask> shape_mask_;
  std::vector<int> mask_;
};
}  // namespace occupancy_map_monitor
