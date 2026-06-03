

#ifndef MOVEIT_POINTCLOUD_OCTOMAP_UPDATER_POINTCLOUD_OCTOMAP_UPDATER_
#define MOVEIT_POINTCLOUD_OCTOMAP_UPDATER_POINTCLOUD_OCTOMAP_UPDATER_

#include <ros/ros.h>
#include <std_srvs/Trigger.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/message_filter.h>
#include <message_filters/subscriber.h>
#include <sensor_msgs/PointCloud2.h>
#include <moveit/occupancy_map_monitor/occupancy_map_updater.h>
#include <moveit/point_containment_filter/shape_mask.h>

#include <unordered_set>

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

private:
  void stopHelper();
  void cloudMsgCallback(const sensor_msgs::PointCloud2::ConstPtr& cloud_msg);
  bool getShapeTransform(ShapeHandle h, Eigen::Isometry3d& transform) const;
  void updateMask(const sensor_msgs::PointCloud2& cloud, const Eigen::Vector3d& sensor_origin,
                  std::vector<int>& mask);
  
  // 全局地图更新服务回调
  bool updateGlobalMapCallback(std_srvs::Trigger::Request& req, std_srvs::Trigger::Response& res);

  ros::NodeHandle private_nh_;
  ros::NodeHandle root_nh_;
  double scale_;
  double padding_;
  double max_range_;
  unsigned int point_subsample_;
  double max_update_rate_;
  std::string point_cloud_topic_;
  std::string filtered_cloud_topic_;
  std::string ns_;
  message_filters::Subscriber<sensor_msgs::PointCloud2>* point_cloud_subscriber_;
  tf2_ros::MessageFilter<sensor_msgs::PointCloud2>* point_cloud_filter_;
  ros::Publisher filtered_cloud_publisher_;

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  std::unique_ptr<point_containment_filter::ShapeMask> shape_mask_;
  std::vector<int> mask_;
  ShapeTransformCache transform_cache_;
  octomap::KeyRay key_ray_;
  ros::Time last_update_time_;

  // 全局地图模式专用
  ros::ServiceServer update_global_map_srv_;
  bool global_update_pending_;
  sensor_msgs::PointCloud2::ConstPtr latest_global_cloud_;
  std::unordered_set<octomap::OcTreeKey, octomap::OcTreeKey::KeyHash> global_locked_cells_;

  // 动态地图模式专用
  octomap::KeySet last_frame_dynamic_cells_;
};
}  // namespace occupancy_map_monitor

#endif