
#include <cmath>
#include <moveit/pointcloud_octomap_updater/pointcloud_octomap_updater.h>
#include <moveit/occupancy_map_monitor/occupancy_map_monitor.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2/LinearMath/Vector3.h>
#include <tf2/LinearMath/Transform.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <XmlRpcException.h>

#include <memory>

namespace occupancy_map_monitor
{
static const std::string LOGNAME = "occupancy_map_monitor";
std::deque<octomap::KeySet> PointCloudOctomapUpdater::point_cloud_window_;
const int WINDOW_SIZE = 3; // 滑动窗口大小：3帧（0.3秒，10Hz雷达）


std::unordered_set<octomap::OcTreeKey, octomap::OcTreeKey::KeyHash> PointCloudOctomapUpdater::global_locked_cells_;
octomap::KeySet PointCloudOctomapUpdater::last_frame_local_cells_;
std::mutex PointCloudOctomapUpdater::map_mutex_;

PointCloudOctomapUpdater::PointCloudOctomapUpdater()
  : OccupancyMapUpdater("PointCloudUpdater")
  , private_nh_("~")
  , scale_(1.0)
  , padding_(0.0)
  , max_range_(std::numeric_limits<double>::infinity())
  , point_subsample_(1)
  , max_update_rate_(0)
  , point_cloud_subscriber_(nullptr)
  , point_cloud_filter_(nullptr)
{
}

PointCloudOctomapUpdater::~PointCloudOctomapUpdater()
{
  stopHelper();
}

bool PointCloudOctomapUpdater::setParams(XmlRpc::XmlRpcValue& params)
{
  try
  {
    if (!params.hasMember("point_cloud_topic"))
      return false;
    point_cloud_topic_ = static_cast<const std::string&>(params["point_cloud_topic"]);

    readXmlParam(params, "max_range", &max_range_);
    readXmlParam(params, "padding_offset", &padding_);
    readXmlParam(params, "padding_scale", &scale_);
    readXmlParam(params, "point_subsample", &point_subsample_);
    if (params.hasMember("max_update_rate"))
      readXmlParam(params, "max_update_rate", &max_update_rate_);
    if (params.hasMember("filtered_cloud_topic"))
      filtered_cloud_topic_ = static_cast<const std::string&>(params["filtered_cloud_topic"]);
    if (params.hasMember("ns"))
      ns_ = static_cast<const std::string&>(params["ns"]);
  }
  catch (XmlRpc::XmlRpcException& ex)
  {
    ROS_ERROR_STREAM_NAMED(LOGNAME, "XmlRpc Exception: " << ex.getMessage());
    return false;
  }

  return true;
}

bool PointCloudOctomapUpdater::initialize()
{
  tf_buffer_ = std::make_shared<tf2_ros::Buffer>();
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, root_nh_);
  shape_mask_ = std::make_unique<point_containment_filter::ShapeMask>();
  shape_mask_->setTransformCallback(
      [this](ShapeHandle shape, Eigen::Isometry3d& tf) { return getShapeTransform(shape, tf); });

  std::string prefix = "";
  if (!ns_.empty())
    prefix = ns_ + "/";
  if (!filtered_cloud_topic_.empty())
    filtered_cloud_publisher_ =
        private_nh_.advertise<sensor_msgs::PointCloud2>(prefix + filtered_cloud_topic_, 10, false);
  return true;
}

void PointCloudOctomapUpdater::start()
{
  if (point_cloud_subscriber_)
    return;
  /* subscribe to point cloud topic using tf filter*/
  point_cloud_subscriber_ = new message_filters::Subscriber<sensor_msgs::PointCloud2>(root_nh_, point_cloud_topic_, 5);
  if (tf_listener_ && tf_buffer_ && !monitor_->getMapFrame().empty())
  {
    point_cloud_filter_ = new tf2_ros::MessageFilter<sensor_msgs::PointCloud2>(*point_cloud_subscriber_, *tf_buffer_,
                                                                               monitor_->getMapFrame(), 5, root_nh_);
    point_cloud_filter_->registerCallback(
        [this](const sensor_msgs::PointCloud2::ConstPtr& cloud) { cloudMsgCallback(cloud); });
    ROS_INFO_NAMED(LOGNAME, "Listening to '%s' using message filter with target frame '%s'", point_cloud_topic_.c_str(),
                   point_cloud_filter_->getTargetFramesString().c_str());
  }
  else
  {
    point_cloud_subscriber_->registerCallback(
        [this](const sensor_msgs::PointCloud2::ConstPtr& cloud) { cloudMsgCallback(cloud); });
    ROS_INFO_NAMED(LOGNAME, "Listening to '%s'", point_cloud_topic_.c_str());
  }
}

void PointCloudOctomapUpdater::stopHelper()
{
  delete point_cloud_filter_;
  delete point_cloud_subscriber_;
}

void PointCloudOctomapUpdater::stop()
{
  stopHelper();
  point_cloud_filter_ = nullptr;
  point_cloud_subscriber_ = nullptr;
}

ShapeHandle PointCloudOctomapUpdater::excludeShape(const shapes::ShapeConstPtr& shape)
{
  ShapeHandle h = 0;
  if (shape_mask_)
    h = shape_mask_->addShape(shape, scale_, padding_);
  else
    ROS_ERROR_NAMED(LOGNAME, "Shape filter not yet initialized!");
  return h;
}

void PointCloudOctomapUpdater::forgetShape(ShapeHandle handle)
{
  if (shape_mask_)
    shape_mask_->removeShape(handle);
}

bool PointCloudOctomapUpdater::getShapeTransform(ShapeHandle h, Eigen::Isometry3d& transform) const
{
  ShapeTransformCache::const_iterator it = transform_cache_.find(h);
  if (it != transform_cache_.end())
  {
    transform = it->second;
  }
  return it != transform_cache_.end();
}

void PointCloudOctomapUpdater::updateMask(const sensor_msgs::PointCloud2& /*cloud*/,
                                          const Eigen::Vector3d& /*sensor_origin*/, std::vector<int>& /*mask*/)
{
}

void PointCloudOctomapUpdater::cloudMsgCallback(const sensor_msgs::PointCloud2::ConstPtr& cloud_msg)
{
  ROS_DEBUG_NAMED(LOGNAME, "Received a new point cloud message");
  ros::WallTime start = ros::WallTime::now();

  if (max_update_rate_ > 0)
  {
    if (ros::Time::now() - last_update_time_ <= ros::Duration(1.0 / max_update_rate_))
      return;
    last_update_time_ = ros::Time::now();
  }

  if (monitor_->getMapFrame().empty())
    monitor_->setMapFrame(cloud_msg->header.frame_id);

  /* get transform for cloud into map frame */
  tf2::Stamped<tf2::Transform> map_h_sensor;
  if (monitor_->getMapFrame() == cloud_msg->header.frame_id)
    map_h_sensor.setIdentity();
  else
  {
    if (tf_buffer_)
    {
      try
      {
        tf2::fromMsg(tf_buffer_->lookupTransform(monitor_->getMapFrame(), cloud_msg->header.frame_id,
                                                 cloud_msg->header.stamp),
                     map_h_sensor);
      }
      catch (tf2::TransformException& ex)
      {
        ROS_ERROR_STREAM_NAMED(LOGNAME, "Transform error of sensor data: " << ex.what() << "; quitting callback");
        return;
      }
    }
    else
      return;
  }

  /* compute sensor origin in map frame */
  const tf2::Vector3& sensor_origin_tf = map_h_sensor.getOrigin();
  octomap::point3d sensor_origin(sensor_origin_tf.getX(), sensor_origin_tf.getY(), sensor_origin_tf.getZ());
  Eigen::Vector3d sensor_origin_eigen(sensor_origin_tf.getX(), sensor_origin_tf.getY(), sensor_origin_tf.getZ());

  if (!updateTransformCache(cloud_msg->header.frame_id, cloud_msg->header.stamp))
    return;

  /* mask out points on the robot */
  shape_mask_->maskContainment(*cloud_msg, sensor_origin_eigen, 0.0, max_range_, mask_);
  updateMask(*cloud_msg, sensor_origin_eigen, mask_);

  octomap::KeySet current_occupied_cells, model_cells;
  std::unique_ptr<sensor_msgs::PointCloud2> filtered_cloud;

  std::unique_ptr<sensor_msgs::PointCloud2Iterator<float>> iter_filtered_x;
  std::unique_ptr<sensor_msgs::PointCloud2Iterator<float>> iter_filtered_y;
  std::unique_ptr<sensor_msgs::PointCloud2Iterator<float>> iter_filtered_z;

  if (!filtered_cloud_topic_.empty())
  {
    filtered_cloud = std::make_unique<sensor_msgs::PointCloud2>();
    filtered_cloud->header = cloud_msg->header;
    sensor_msgs::PointCloud2Modifier pcd_modifier(*filtered_cloud);
    pcd_modifier.setPointCloud2FieldsByString(1, "xyz");
    pcd_modifier.resize(cloud_msg->width * cloud_msg->height);

    iter_filtered_x = std::make_unique<sensor_msgs::PointCloud2Iterator<float>>(*filtered_cloud, "x");
    iter_filtered_y = std::make_unique<sensor_msgs::PointCloud2Iterator<float>>(*filtered_cloud, "y");
    iter_filtered_z = std::make_unique<sensor_msgs::PointCloud2Iterator<float>>(*filtered_cloud, "z");
  }
  size_t filtered_cloud_size = 0;

  tree_->lockRead();

  try
  {
    for (unsigned int row = 0; row < cloud_msg->height; row += point_subsample_)
    {
      unsigned int row_c = row * cloud_msg->width;
      sensor_msgs::PointCloud2ConstIterator<float> pt_iter(*cloud_msg, "x");
      pt_iter += row_c;

      for (unsigned int col = 0; col < cloud_msg->width; col += point_subsample_, pt_iter += point_subsample_)
      {
        if (!std::isnan(pt_iter[0]) && !std::isnan(pt_iter[1]) && !std::isnan(pt_iter[2]))
        {
          if (mask_[row_c + col] == point_containment_filter::ShapeMask::INSIDE)
          {
            tf2::Vector3 point_tf = map_h_sensor * tf2::Vector3(pt_iter[0], pt_iter[1], pt_iter[2]);
            model_cells.insert(tree_->coordToKey(point_tf.getX(), point_tf.getY(), point_tf.getZ()));
          }
          else if (mask_[row_c + col] == point_containment_filter::ShapeMask::CLIP)
          {
            continue;
          }
          else
          {
            tf2::Vector3 point_tf = map_h_sensor * tf2::Vector3(pt_iter[0], pt_iter[1], pt_iter[2]);
            current_occupied_cells.insert(tree_->coordToKey(point_tf.getX(), point_tf.getY(), point_tf.getZ()));
            
            if (filtered_cloud)
            {
              **iter_filtered_x = pt_iter[0];
              **iter_filtered_y = pt_iter[1];
              **iter_filtered_z = pt_iter[2];
              ++filtered_cloud_size;
              ++*iter_filtered_x;
              ++*iter_filtered_y;
              ++*iter_filtered_z;
            }
          }
        }
      }
    }
  }
  catch (...)
  {
    tree_->unlockRead();
    return;
  }

  tree_->unlockRead();

  /* cells that overlap with the model are not occupied */
  for (const octomap::OcTreeKey& model_cell : model_cells)
    current_occupied_cells.erase(model_cell);

  // ==============================================
  // 核心：滑动窗口点云累积
  // ==============================================
  // 把当前帧点云加入窗口
  point_cloud_window_.push_back(current_occupied_cells);
  
  // 如果窗口超过大小，移除最旧的一帧
  if (point_cloud_window_.size() > WINDOW_SIZE)
  {
    point_cloud_window_.pop_front();
  }

  // 合并窗口内所有帧的点云，得到完整的360度点云
  octomap::KeySet all_occupied_cells;
  for (const auto& frame : point_cloud_window_)
  {
    all_occupied_cells.insert(frame.begin(), frame.end());
  }

  tree_->lockWrite();
  try
  {
    std::lock_guard<std::mutex> lock(map_mutex_);

    if (ns_ == "global")
    {
      // ==============================================
      // 全局插件逻辑：完全保留你的原逻辑
      // ==============================================
      ROS_INFO_NAMED(LOGNAME, "Global map update triggered - resetting map...");

      // 第一步：清除所有旧的全局体素
      for (const octomap::OcTreeKey& key : global_locked_cells_)
      {
        tree_->setNodeValue(key, tree_->getClampingThresMinLog());
      }
      global_locked_cells_.clear();

      // 第二步：用当前点云生成全新的全局地图
      for (const octomap::OcTreeKey& occupied_cell : all_occupied_cells)
      {
        tree_->setNodeValue(occupied_cell, tree_->getClampingThresMaxLog());
        global_locked_cells_.insert(occupied_cell);
      }

      // 第三步：机械臂自身区域设为安全
      const float lg = tree_->getClampingThresMinLog() - tree_->getClampingThresMaxLog();
      for (const octomap::OcTreeKey& model_cell : model_cells)
      {
        tree_->setNodeValue(model_cell, lg);
      }

      ROS_INFO_NAMED(LOGNAME, "Global map reset complete. Locked %zu static obstacle voxels.", global_locked_cells_.size());
    }
    else
    {
      // ==============================================
      // 局部插件逻辑：每帧用完整窗口点云全量更新
      // ==============================================
      // 第一步：收集所有需要清除的局部体素
      std::unordered_set<octomap::OcTreeKey, octomap::OcTreeKey::KeyHash> cells_to_clear;

      // 只清除激光雷达扫描范围内的体素（性能优化）
      octomap::point3d bbx_min(
        sensor_origin.x() - max_range_, 
        sensor_origin.y() - max_range_, 
        0.0
      );
      octomap::point3d bbx_max(
        sensor_origin.x() + max_range_, 
        sensor_origin.y() + max_range_, 
        2.0
      );

      for (octomap::OcTree::leaf_bbx_iterator it = tree_->begin_leafs_bbx(bbx_min, bbx_max), end = tree_->end_leafs_bbx(); it != end; ++it)
      {
        octomap::OcTreeKey key = it.getKey();
        if (global_locked_cells_.count(key) == 0)
        {
          cells_to_clear.insert(key);
        }
      }

      // 第二步：一次性清除所有局部体素
      for (const octomap::OcTreeKey& key : cells_to_clear)
      {
        tree_->setNodeValue(key, tree_->getClampingThresMinLog());
      }

      // 第三步：添加窗口内所有帧的合并点云（完整360度）
      for (const octomap::OcTreeKey& occupied_cell : all_occupied_cells)
      {
        if (global_locked_cells_.count(occupied_cell) > 0)
          continue;
        
        tree_->setNodeValue(occupied_cell, tree_->getClampingThresMaxLog());
      }

      // 第四步：机械臂自身区域设为安全
      const float lg = tree_->getClampingThresMinLog() - tree_->getClampingThresMaxLog();
      for (const octomap::OcTreeKey& model_cell : model_cells)
      {
        if (global_locked_cells_.count(model_cell) > 0)
          continue;
        
        tree_->setNodeValue(model_cell, lg);
      }
    }
  }
  catch (...)
  {
    ROS_ERROR_NAMED(LOGNAME, "Internal error while updating octree");
  }
  tree_->unlockWrite();

  ROS_DEBUG_NAMED(LOGNAME, "Processed point cloud in %lf ms, window size: %zu, total cells: %zu", 
                  (ros::WallTime::now() - start).toSec() * 1000.0, 
                  point_cloud_window_.size(), 
                  all_occupied_cells.size());
  tree_->triggerUpdateCallback();
  
  if (filtered_cloud)
  {
    sensor_msgs::PointCloud2Modifier pcd_modifier(*filtered_cloud);
    pcd_modifier.resize(filtered_cloud_size);
    filtered_cloud_publisher_.publish(*filtered_cloud);
  }
}
}  // namespace occupancy_map_monitor