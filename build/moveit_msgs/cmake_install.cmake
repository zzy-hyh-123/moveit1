# Install script for directory: /home/zhiyuan/moveit/src/moveit_msgs

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/zhiyuan/moveit/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/action" TYPE FILE FILES
    "/home/zhiyuan/moveit/src/moveit_msgs/action/ExecuteTrajectory.action"
    "/home/zhiyuan/moveit/src/moveit_msgs/action/MoveGroup.action"
    "/home/zhiyuan/moveit/src/moveit_msgs/action/MoveGroupSequence.action"
    "/home/zhiyuan/moveit/src/moveit_msgs/action/Pickup.action"
    "/home/zhiyuan/moveit/src/moveit_msgs/action/Place.action"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/msg" TYPE FILE FILES
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/ExecuteTrajectoryAction.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/ExecuteTrajectoryActionGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/ExecuteTrajectoryActionResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/ExecuteTrajectoryActionFeedback.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/ExecuteTrajectoryGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/ExecuteTrajectoryResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/ExecuteTrajectoryFeedback.msg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/msg" TYPE FILE FILES
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupAction.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupActionGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupActionResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupActionFeedback.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupFeedback.msg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/msg" TYPE FILE FILES
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupSequenceAction.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupSequenceActionGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupSequenceActionResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupSequenceActionFeedback.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupSequenceGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupSequenceResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/MoveGroupSequenceFeedback.msg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/msg" TYPE FILE FILES
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PickupAction.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PickupActionGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PickupActionResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PickupActionFeedback.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PickupGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PickupResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PickupFeedback.msg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/msg" TYPE FILE FILES
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PlaceAction.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PlaceActionGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PlaceActionResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PlaceActionFeedback.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PlaceGoal.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PlaceResult.msg"
    "/home/zhiyuan/moveit/devel/share/moveit_msgs/msg/PlaceFeedback.msg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/msg" TYPE FILE FILES
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/AllowedCollisionEntry.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/AllowedCollisionMatrix.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/AttachedCollisionObject.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/BoundingVolume.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/CartesianPoint.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/CartesianTrajectory.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/CartesianTrajectoryPoint.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/CollisionObject.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/ConstraintEvalResult.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/Constraints.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/CostSource.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/ContactInformation.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/DisplayTrajectory.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/DisplayRobotState.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/GenericTrajectory.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/Grasp.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/GripperTranslation.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/JointConstraint.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/JointLimits.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/LinkPadding.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/LinkScale.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/MotionPlanRequest.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/MotionPlanResponse.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/MotionPlanDetailedResponse.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/MotionSequenceItem.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/MotionSequenceRequest.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/MotionSequenceResponse.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/MoveItErrorCodes.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/TrajectoryConstraints.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/ObjectColor.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/OrientationConstraint.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/OrientedBoundingBox.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PlaceLocation.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PlannerInterfaceDescription.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PlannerParams.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PlanningScene.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PlanningSceneComponents.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PlanningSceneWorld.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PlanningOptions.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PositionConstraint.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/RobotState.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/RobotTrajectory.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/VisibilityConstraint.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/WorkspaceParameters.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/KinematicSolverInfo.msg"
    "/home/zhiyuan/moveit/src/moveit_msgs/msg/PositionIKRequest.msg"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/srv" TYPE FILE FILES
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetMotionPlan.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/ExecuteKnownTrajectory.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetStateValidity.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetCartesianPath.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetPlanningScene.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GraspPlanning.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/ApplyPlanningScene.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/QueryPlannerInterfaces.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetMotionSequence.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetPositionFK.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetPositionIK.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetPlannerParams.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/SetPlannerParams.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/UpdatePointcloudOctomap.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/SaveMap.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/LoadMap.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/SaveRobotStateToWarehouse.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/ListRobotStatesInWarehouse.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/GetRobotStateFromWarehouse.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/CheckIfRobotStateExistsInWarehouse.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/RenameRobotStateInWarehouse.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/DeleteRobotStateFromWarehouse.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/ChangeControlDimensions.srv"
    "/home/zhiyuan/moveit/src/moveit_msgs/srv/ChangeDriftDimensions.srv"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/cmake" TYPE FILE FILES "/home/zhiyuan/moveit/build/moveit_msgs/catkin_generated/installspace/moveit_msgs-msg-paths.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/home/zhiyuan/moveit/devel/include/moveit_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/roseus/ros" TYPE DIRECTORY FILES "/home/zhiyuan/moveit/devel/share/roseus/ros/moveit_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/common-lisp/ros" TYPE DIRECTORY FILES "/home/zhiyuan/moveit/devel/share/common-lisp/ros/moveit_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/gennodejs/ros" TYPE DIRECTORY FILES "/home/zhiyuan/moveit/devel/share/gennodejs/ros/moveit_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  execute_process(COMMAND "/usr/bin/python3" -m compileall "/home/zhiyuan/moveit/devel/lib/python3/dist-packages/moveit_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages" TYPE DIRECTORY FILES "/home/zhiyuan/moveit/devel/lib/python3/dist-packages/moveit_msgs")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/zhiyuan/moveit/build/moveit_msgs/catkin_generated/installspace/moveit_msgs.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/cmake" TYPE FILE FILES "/home/zhiyuan/moveit/build/moveit_msgs/catkin_generated/installspace/moveit_msgs-msg-extras.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs/cmake" TYPE FILE FILES
    "/home/zhiyuan/moveit/build/moveit_msgs/catkin_generated/installspace/moveit_msgsConfig.cmake"
    "/home/zhiyuan/moveit/build/moveit_msgs/catkin_generated/installspace/moveit_msgsConfig-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/moveit_msgs" TYPE FILE FILES "/home/zhiyuan/moveit/src/moveit_msgs/package.xml")
endif()

