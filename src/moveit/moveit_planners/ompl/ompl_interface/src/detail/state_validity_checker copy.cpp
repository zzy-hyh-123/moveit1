

#include <moveit/ompl_interface/detail/state_validity_checker.h>
#include <moveit/ompl_interface/model_based_planning_context.h>
#include <moveit/profiler/profiler.h>
#include <moveit/collision_detection/collision_matrix.h>
#include <moveit/robot_model/joint_model.h>
#include <ros/ros.h>

namespace ompl_interface
{
constexpr char LOGNAME[] = "state_validity_checker";
}  // namespace ompl_interface

ompl_interface::StateValidityChecker::StateValidityChecker(const ModelBasedPlanningContext* pc)
  : ompl::base::StateValidityChecker(pc->getOMPLSimpleSetup()->getSpaceInformation())
  , planning_context_(pc)
  , group_name_(pc->getGroupName())
  , tss_(pc->getCompleteInitialRobotState())
  , verbose_(false)
  , inter_arm_safety_distance_(0.03)
{
  specs_.clearanceComputationType = ompl::base::StateValidityCheckerSpecs::APPROXIMATE;
  specs_.hasValidDirectionComputation = false;

  collision_request_with_distance_.distance = true;
  collision_request_with_cost_.cost = true;

  collision_request_simple_.group_name = planning_context_->getGroupName();
  collision_request_with_distance_.group_name = planning_context_->getGroupName();
  collision_request_with_cost_.group_name = planning_context_->getGroupName();

  collision_request_simple_verbose_ = collision_request_simple_;
  collision_request_simple_verbose_.verbose = true;

  collision_request_with_distance_verbose_ = collision_request_with_distance_;
  collision_request_with_distance_verbose_.verbose = true;

  // 初始化运动连杆集合（排除固定底座）
  initMovingLinks();
}

void ompl_interface::StateValidityChecker::setVerbose(bool flag)
{
  verbose_ = flag;
}

bool ompl_interface::StateValidityChecker::isValid(const ompl::base::State* state, bool verbose) const
{
  // Use cached validity if it is available
  if (state->as<ModelBasedStateSpace::StateType>()->isValidityKnown())
    return state->as<ModelBasedStateSpace::StateType>()->isMarkedValid();

  // ROS_INFO_THROTTLE_NAMED(1.0, "ompl_interface", "===== Entered isValid function! Checking state validity =====");

  if (!si_->satisfiesBounds(state))
  {
    if (verbose)
      ROS_INFO_NAMED(LOGNAME, "State outside bounds");
    const_cast<ob::State*>(state)->as<ModelBasedStateSpace::StateType>()->markInvalid();
    return false;
  }

  moveit::core::RobotState* robot_state = tss_.getStateStorage();
  planning_context_->getOMPLStateSpace()->copyToRobotState(*robot_state, state);

  // check path constraints
  const kinematic_constraints::KinematicConstraintSetPtr& kset = planning_context_->getPathConstraints();
  if (kset && !kset->decide(*robot_state, verbose).satisfied)
  {
    const_cast<ob::State*>(state)->as<ModelBasedStateSpace::StateType>()->markInvalid();
    return false;
  }

  // check feasibility
  if (!planning_context_->getPlanningScene()->isStateFeasible(*robot_state, verbose))
  {
    const_cast<ob::State*>(state)->as<ModelBasedStateSpace::StateType>()->markInvalid();
    return false;
  }

  // check collision avoidance
  collision_detection::CollisionResult res;
  planning_context_->getPlanningScene()->checkCollision(
      verbose ? collision_request_simple_verbose_ : collision_request_simple_, res, *robot_state);
  if (!res.collision)
  {
    const_cast<ob::State*>(state)->as<ModelBasedStateSpace::StateType>()->markValid();
  }
  else
  {
    const_cast<ob::State*>(state)->as<ModelBasedStateSpace::StateType>()->markInvalid();
  }
  return !res.collision;
}

bool ompl_interface::StateValidityChecker::isValid(const ompl::base::State* state, double& dist, bool verbose) const
{
  // Use cached validity and distance if they are available
  if (state->as<ModelBasedStateSpace::StateType>()->isValidityKnown() &&
      state->as<ModelBasedStateSpace::StateType>()->isGoalDistanceKnown())
  {
    dist = state->as<ModelBasedStateSpace::StateType>()->distance;
    return state->as<ModelBasedStateSpace::StateType>()->isMarkedValid();
  }

  if (!si_->satisfiesBounds(state))
  {
    if (verbose)
      ROS_INFO_NAMED(LOGNAME, "State outside bounds");
    const_cast<ob::State*>(state)->as<ModelBasedStateSpace::StateType>()->markInvalid(0.0);
    return false;
  }

  moveit::core::RobotState* robot_state = tss_.getStateStorage();
  planning_context_->getOMPLStateSpace()->copyToRobotState(*robot_state, state);

  // check path constraints
  const kinematic_constraints::KinematicConstraintSetPtr& kset = planning_context_->getPathConstraints();
  if (kset)
  {
    kinematic_constraints::ConstraintEvaluationResult cer = kset->decide(*robot_state, verbose);
    if (!cer.satisfied)
    {
      dist = cer.distance;
      const_cast<ob::State*>(state)->as<ModelBasedStateSpace::StateType>()->markInvalid(dist);
      return false;
    }
  }

  // check feasibility
  if (!planning_context_->getPlanningScene()->isStateFeasible(*robot_state, verbose))
  {
    dist = 0.0;
    return false;
  }

  // check collision avoidance
  collision_detection::CollisionResult res;
  planning_context_->getPlanningScene()->checkCollision(
      verbose ? collision_request_with_distance_verbose_ : collision_request_with_distance_, res, *robot_state);
  dist = res.distance;
  return !res.collision;

  // 0.01m安全距离判断
  // const double MIN_SAFE_DIST = 0.01;
  // if (dist > 0.0 && dist < MIN_SAFE_DIST) {
  //   ROS_INFO_THROTTLE_NAMED(1.0,"ompl_interface", "Distance to obstacle: %.3fm < 0.01m (unsafe)", dist);
  //   if (verbose) ROS_WARN_NAMED(LOGNAME, "Distance to obstacle: %.3fm < 0.03m (unsafe)", dist);
  //   // 标记状态无效，并缓存这个距离值
  //   const_cast<ob::State*>(state)->as<ModelBasedStateSpace::StateType>()->markInvalid(dist);
  //   return false;
  // }else{
  //   return !res.collision;
  // }
}


double ompl_interface::StateValidityChecker::cost(const ompl::base::State* state) const
{
  double cost = 0.0;

  moveit::core::RobotState* robot_state = tss_.getStateStorage();
  planning_context_->getOMPLStateSpace()->copyToRobotState(*robot_state, state);

  // Calculates cost from a summation of distance to obstacles times the size of the obstacle
  collision_detection::CollisionResult res;
  planning_context_->getPlanningScene()->checkCollision(collision_request_with_cost_, res, *robot_state);

  for (const collision_detection::CostSource& cost_source : res.cost_sources)
    cost += cost_source.cost * cost_source.getVolume();

  return cost;
}

double ompl_interface::StateValidityChecker::clearance(const ompl::base::State* state) const
{
  moveit::core::RobotState* robot_state = tss_.getStateStorage();
  planning_context_->getOMPLStateSpace()->copyToRobotState(*robot_state, state);

  collision_detection::CollisionResult res;
  planning_context_->getPlanningScene()->checkCollision(collision_request_with_distance_, res, *robot_state);
  return res.collision ? 0.0 : (res.distance < 0.0 ? std::numeric_limits<double>::infinity() : res.distance);
}

double ompl_interface::StateValidityChecker::distanceEnvironment(const ompl::base::State* state) const
{
  moveit::core::RobotState* robot_state = tss_.getStateStorage();
  planning_context_->getOMPLStateSpace()->copyToRobotState(*robot_state, state);

  // 1. 臂运动连杆 vs 环境障碍物
  double env_dist = computeRobotWorldDistance(*robot_state);

  // 2. 臂运动连杆 vs 另一臂运动连杆（仅双臂场景）
  double result = env_dist;
  if (!other_arm_group_name_.empty())
  {
    double raw_inter_arm_dist = computeInterArmDistance(*robot_state);
    // 应用双臂安全距离偏移（类似 default_robot_padding 的效果）
    double effective_inter_arm_dist = raw_inter_arm_dist - inter_arm_safety_distance_;
    result = std::min(env_dist, effective_inter_arm_dist);
    ROS_DEBUG_NAMED(LOGNAME, "distanceEnvironment: env=%.4f, inter_arm_raw=%.4f, inter_arm_effective=%.4f, "
                    "safety_margin=%.4f, result=%.4f",
                    env_dist, raw_inter_arm_dist, effective_inter_arm_dist,
                    inter_arm_safety_distance_, result);
  }

  return result;
}

void ompl_interface::StateValidityChecker::initMovingLinks()
{
  const moveit::core::JointModelGroup* group = planning_context_->getJointModelGroup();
  if (!group)
  {
    ROS_WARN_NAMED(LOGNAME, "No joint model group found, distanceEnvironment may not work correctly");
    return;
  }

  // 构建本组运动连杆集合：排除沿运动链全是 FIXED 关节的连杆
  const std::vector<const moveit::core::LinkModel*>& all_links = group->getUpdatedLinkModels();
  for (const moveit::core::LinkModel* link : all_links)
  {
    if (!isLinkFixedToWorld(link))
    {
      moving_link_set_.insert(link);
    }
  }
  ROS_DEBUG_NAMED(LOGNAME, "Moving links for group '%s': %zu / %zu total links",
                  group->getName().c_str(), moving_link_set_.size(), all_links.size());

  // 读取双臂配置参数
  ros::NodeHandle nh;
  nh.param("/robot_description_planning/dual_arm_other_group", other_arm_group_name_, std::string(""));
  nh.param("/robot_description_planning/dual_arm_safety_distance", inter_arm_safety_distance_, 0.03);

  // 如果配置了另一臂，构建其运动连杆集合
  if (!other_arm_group_name_.empty())
  {
    const moveit::core::JointModelGroup* other_group =
        planning_context_->getRobotModel()->getJointModelGroup(other_arm_group_name_);
    if (other_group)
    {
      const std::vector<const moveit::core::LinkModel*>& other_links = other_group->getUpdatedLinkModels();
      for (const moveit::core::LinkModel* link : other_links)
      {
        if (!isLinkFixedToWorld(link))
        {
          other_moving_links_.insert(link);
        }
      }
      ROS_INFO_NAMED(LOGNAME, "Dual-arm mode: group '%s' (%zu moving links) vs '%s' (%zu moving links), safety=%.3fm",
                     group->getName().c_str(), moving_link_set_.size(),
                     other_arm_group_name_.c_str(), other_moving_links_.size(),
                     inter_arm_safety_distance_);
    }
    else
    {
      ROS_WARN_NAMED(LOGNAME, "Other arm group '%s' not found in robot model", other_arm_group_name_.c_str());
      other_arm_group_name_.clear();
    }
  }
}

bool ompl_interface::StateValidityChecker::isLinkFixedToWorld(const moveit::core::LinkModel* link) const
{
  // 沿运动链向上遍历：如果所有祖先关节都是 FIXED，则该连杆固定于世界
  const moveit::core::LinkModel* curr = link;
  while (curr)
  {
    const moveit::core::JointModel* parent_joint = curr->getParentJointModel();
    if (parent_joint && parent_joint->getType() != moveit::core::JointModel::FIXED)
      return false;  // 发现可动关节
    if (!parent_joint)
      break;  // 到达根关节（无父关节）
    curr = parent_joint->getParentLinkModel();
  }
  return true;  // 全路径都是 FIXED 关节
}

double ompl_interface::StateValidityChecker::computeRobotWorldDistance(
    const moveit::core::RobotState& state) const
{
  collision_detection::DistanceRequest req;
  collision_detection::DistanceResult res;

  req.group_name = group_name_;
  req.enable_nearest_points = true;
  req.active_components_only = &moving_link_set_;  // 只检查运动连杆
  req.type = collision_detection::DistanceRequestType::GLOBAL;

  planning_context_->getPlanningScene()->getCollisionEnv()->distanceRobot(req, res, state);
  return res.minimum_distance.distance;
}

double ompl_interface::StateValidityChecker::computeInterArmDistance(
    const moveit::core::RobotState& state) const
{
  if (other_moving_links_.empty())
    return std::numeric_limits<double>::max();

  // 构造自定义 ACM：默认所有对都 ALWAYS（跳过），只对跨臂对设为 NEVER（需计算）
  const std::vector<std::string>& all_link_names =
      planning_context_->getRobotModel()->getLinkModelNames();
  collision_detection::AllowedCollisionMatrix temp_acm(all_link_names, true);  // true = all ALWAYS

  // 将跨臂连杆对设为 NEVER（必须检测距离）
  for (const moveit::core::LinkModel* link1 : moving_link_set_)
  {
    for (const moveit::core::LinkModel* link2 : other_moving_links_)
    {
      temp_acm.setEntry(link1->getName(), link2->getName(), false);  // false = NEVER
    }
  }

  collision_detection::DistanceRequest req;
  collision_detection::DistanceResult res;

  req.group_name = group_name_;
  req.acm = &temp_acm;
  req.type = collision_detection::DistanceRequestType::GLOBAL;

  planning_context_->getPlanningScene()->getCollisionEnv()->distanceSelf(req, res, state);
  return res.minimum_distance.distance;
}
