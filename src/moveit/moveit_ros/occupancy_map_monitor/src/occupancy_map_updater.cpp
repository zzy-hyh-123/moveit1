
#include <moveit/occupancy_map_monitor/occupancy_map_monitor.h>
#include <moveit/occupancy_map_monitor/occupancy_map_updater.h>

namespace occupancy_map_monitor
{
static const std::string LOGNAME = "occupancy_map_monitor";

OccupancyMapUpdater::OccupancyMapUpdater(const std::string& type) : type_(type)
{
}

OccupancyMapUpdater::~OccupancyMapUpdater() = default;

void OccupancyMapUpdater::setMonitor(OccupancyMapMonitor* monitor)
{
  monitor_ = monitor;
  tree_ = monitor->getOcTreePtr();
}

void OccupancyMapUpdater::readXmlParam(XmlRpc::XmlRpcValue& params, const std::string& param_name, double* value)
{
  if (params.hasMember(param_name))
  {
    if (params[param_name].getType() == XmlRpc::XmlRpcValue::TypeInt)
      *value = (int)params[param_name];
    else
      *value = (double)params[param_name];
  }
}

void OccupancyMapUpdater::readXmlParam(XmlRpc::XmlRpcValue& params, const std::string& param_name, unsigned int* value)
{
  if (params.hasMember(param_name))
    *value = (int)params[param_name];
}

bool OccupancyMapUpdater::updateTransformCache(const std::string& target_frame, const ros::Time& target_time)
{
  transform_cache_.clear();
  if (transform_provider_callback_)
  {
    bool success = transform_provider_callback_(target_frame, target_time, transform_cache_);
    if (!success)
      ROS_ERROR_THROTTLE_NAMED(
          1, LOGNAME,
          "Transform cache was not updated. Self-filtering may fail. If transforms were not available yet, consider "
          "setting robot_description_planning/shape_transform_cache_lookup_wait_time to wait longer for transforms");
    return success;
  }
  else
  {
    ROS_WARN_THROTTLE_NAMED(1, LOGNAME, "No callback provided for updating the transform cache for octomap updaters");
    return false;
  }
}
}  // namespace occupancy_map_monitor
