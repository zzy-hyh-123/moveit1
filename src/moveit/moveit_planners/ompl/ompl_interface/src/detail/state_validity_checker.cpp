
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include <moveit/ompl_interface/detail/state_validity_checker.h>
#include <moveit/ompl_interface/model_based_planning_context.h>
#include <moveit/profiler/profiler.h>
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


void ompl_interface::StateValidityChecker::logCollisionContacts(moveit::core::RobotState& robot_state,
                                                               const char* reason) const
{
  collision_detection::CollisionRequest req;
  req.group_name = group_name_;
  req.contacts = true;
  req.max_contacts = 20;
  req.max_contacts_per_pair = 1;

  collision_detection::CollisionResult res;
  planning_context_->getPlanningScene()->checkCollision(req, res, robot_state);

  if (!res.collision)
  {
    ROS_WARN_NAMED(LOGNAME, "%s (group '%s'): the last sampled state is collision-free", reason,
                   group_name_.c_str());
    return;
  }

  std::vector<const collision_detection::Contact*> contacts;
  for (const auto& entry : res.contacts)
    for (const auto& contact : entry.second)
      contacts.push_back(&contact);
  std::sort(contacts.begin(), contacts.end(),
            [](const collision_detection::Contact* a, const collision_detection::Contact* b)
            { return a->depth > b->depth; });

  std::string text;
  char buffer[256];
  for (std::size_t i = 0; i < contacts.size() && i < 3; ++i)
  {
    const collision_detection::Contact& c = *contacts[i];
    std::snprintf(buffer, sizeof(buffer), "%s%s <-> %s depth=%.4f pos=(%.3f, %.3f, %.3f)",
                  i == 0 ? "" : "; ", c.body_name_1.c_str(), c.body_name_2.c_str(), c.depth, c.pos.x(),
                  c.pos.y(), c.pos.z());
    text += buffer;
  }
  if (text.empty())
    text = "no contact details reported";

  ROS_WARN_NAMED(LOGNAME, "%s (group '%s'): %s", reason, group_name_.c_str(), text.c_str());
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

  // 只计算机械臂运动连杆与环境障碍物的最小距离（排除固定底座连杆，忽略自碰撞）
  return computeRobotWorldDistance(*robot_state);
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

  // 使用带膨胀的碰撞环境，计算距离时已考虑 default_robot_padding
  planning_context_->getPlanningScene()->getCollisionEnv()->distanceRobot(req, res, state);
  return res.minimum_distance.distance;
}
