

#include <moveit/ompl_interface/detail/state_validity_checker.h>
#include <moveit/ompl_interface/model_based_planning_context.h>
#include <moveit/profiler/profiler.h>
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
