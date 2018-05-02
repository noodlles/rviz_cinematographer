/** @file
 *
 * Simple rqt plugin to edit trajectories.
 *
 * @author Jan Razlaw
 */

#ifndef RQT_POSE_INTERPOLATOR_TRAJECTORY_EDITOR_H
#define RQT_POSE_INTERPOLATOR_TRAJECTORY_EDITOR_H

#include <fstream>

#include <ros/ros.h>
#include <ros/package.h>

#include <tf/tf.h>
#include <tf/transform_datatypes.h>

#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PoseArray.h>
#include <view_controller_msgs/CameraPlacement.h>
#include <nav_msgs/Path.h>

#include <interactive_markers/interactive_marker_server.h>
#include <interactive_markers/menu_handler.h>

#include <rqt_gui_cpp/plugin.h>

#include <QWidget>
#include <QFileDialog>

#include <rqt_pose_interpolator/utils.h>
#include "ui_trajectory_editor.h"

#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>


namespace pose_interpolator {

/**
 * @brief Manipulates the rviz camera.
 */
class TrajectoryEditor : public rqt_gui_cpp::Plugin
{

Q_OBJECT
public:
  struct InteractiveMarkerWithTime
  {
    InteractiveMarkerWithTime(visualization_msgs::InteractiveMarker&& input_marker, const double time)
    : marker(input_marker)
      , transition_time(time)
    {
    }

    visualization_msgs::InteractiveMarker marker;
    double transition_time;
  };

  typedef InteractiveMarkerWithTime TimedMarker;
  typedef std::list<TimedMarker> MarkerList;


  /** @brief Constructor. */
  TrajectoryEditor();

  /**
   * @brief Sets up subscribers and publishers and connects GUI to functions.
   *
   * @param context     the plugin context.
   */
  virtual void initPlugin(qt_gui_cpp::PluginContext& context);

  /**
   * @brief Shuts down the subscribers and publishers.
   */
  virtual void shutdownPlugin();

  /**
   * @brief Saves settings. TODO.
   *
   * @param plugin_settings     plugin-specific settings
   * @param instance_settings   instance-specific settings
   */
  virtual void saveSettings(qt_gui_cpp::Settings& plugin_settings,
                            qt_gui_cpp::Settings& instance_settings) const;

  /**
   * @brief Restores settings. TODO.
   *
   * @param plugin_settings     plugin-specific settings
   * @param instance_settings   instance-specific settings
   */
  virtual void restoreSettings(const qt_gui_cpp::Settings& plugin_settings,
                               const qt_gui_cpp::Settings& instance_settings);

  /**
   * @brief Saves the current camera pose in #cam_pose_.
   *
   * @param[in] cam_pose    pointer to current camera pose.
   */
  void camPoseCallback(const geometry_msgs::Pose::ConstPtr& cam_pose);

  /**
  * @brief Publishes the transition step by step.
  *
  * @param[in] event    timer information.
  */
  void transitionStepsPublisherCallback(const ros::TimerEvent &event);

Q_SIGNALS:
  void updateRequested();

public slots:
  /** @brief Moves rviz camera to currently selected pose.*/
  void moveCamToCurrent();
  /** @brief Moves rviz camera to pose before currently selected one.*/
  void moveCamToPrev();
  /** @brief Moves rviz camera to pose after currently selected one.*/
  void moveCamToNext();
  /** @brief Update marker with values from GUI.*/
  void updateCurrentMarker();
  /** @brief Sets currently selected pose to the current pose of the rviz camera.*/
  void setCurrentPoseToCam();
  /** @brief Sets the frame_id of the markers.*/
  void setMarkerFrames();
  /** @brief Loads a series of markers from a file.*/
  void loadTrajectoryFromFile();
  /** @brief Saves poses and transition times of interactive markers to a file.*/
  void saveTrajectoryToFile();

private:
  /**
   * @brief Creates a CameraPlacement hull.
   * @return CameraPlacement.
   */
  view_controller_msgs::CameraPlacement makeCameraPlacement();

  /**
   * @brief Creates an InteractiveMarker hull.
   *
   * @param[in] x   x position of marker.
   * @param[in] y   y position of marker.
   * @param[in] z   z position of marker.
   * @return InteractiveMarker.
   */
  visualization_msgs::InteractiveMarker makeMarker(double x=0.0,
                                                   double y=0.0,
                                                   double z=0.0);

  /**
   * @brief Moves rviz camera to marker pose by publishing a CameraDisplacement message.
   *
   * @param[in] marker   provides the pose and transition time.
   */
  void moveCamToMarker(const TimedMarker& marker);

  /**
   * @brief Sets members to pose of currently moved interactive marker.
   * @param[in] feedback    feedback the interaction with the interactive marker generates.
   */
  void processFeedback(const visualization_msgs::InteractiveMarkerFeedbackConstPtr& feedback);

  /**
   * @brief Rotates a vector by a quaternion.
   *
   * @param[in] vector  input vector.
   * @param[in] quat    input rotation.
   * @return the rotated vector.
   */
  tf::Vector3 rotateVector(const tf::Vector3& vector,
                           const geometry_msgs::Quaternion& quat);

  /**
   * @brief Reconstructs trajectory from current markers.
   */
  void updateTrajectory();

  /**
   * @brief Safes poses and times of markers to yaml file.
   *
   * @param[in] file_path   path to the file.
   */
  void safeTrajectoryToFile(const std::string& file_path);

  /**
   * @brief Publishes the trajectory that is defined by the markers.
   *
   * @param[in] feedback    feedback from selected marker.
   */
  void publishTrajectory(const visualization_msgs::InteractiveMarkerFeedbackConstPtr& feedback);

  /**
   * @brief Adds a marker between the selected marker and the one before.
   *
   * @param[in] feedback    feedback from selected marker.
   */
  void addMarkerBefore(const visualization_msgs::InteractiveMarkerFeedbackConstPtr& feedback);

  /**
   * @brief Adds a marker between the selected marker and the one after.
   *
   * @param[in] feedback    feedback from selected marker.
   */
  void addMarkerBehind(const visualization_msgs::InteractiveMarkerFeedbackConstPtr& feedback);

  /**
   * @brief Removes the selected marker.
   *
   * @param[in] feedback    feedback from selected marker.
   */
  void removeMarker(const visualization_msgs::InteractiveMarkerFeedbackConstPtr& feedback);

  /**
   * @brief Saves markers in server.
   *
   * @param[in] markers     markers.
   */
  void fillServer(MarkerList& markers);

  /**
   * @brief Load marker poses from a file.
   *
   * @param[in] nh          node handle.
   * @param[in] param_name  root name of the parameters.
   */
  void loadParams(const ros::NodeHandle& nh,
                  const std::string& param_name);

  /**
   * @brief Gets marker with specified name.
   *
   * @param[in] marker_name name of marker.
   * @return marker with marker_name.
   */
  InteractiveMarkerWithTime& getMarkerByName(const std::string& marker_name);

  /**
   * @brief Sets the input as the #current_marker_.
   *
   * Additionally updates the GUI elements and sets the color of the input marker from to green.
   *
   * @param[in] marker  input marker.
   */
  void setCurrentTo(TimedMarker& marker);

  /**
   * @brief Sets the value of the spin_box to the value without triggering a signal.
   *
   * @param[in,out] spin_box    the updated spin box.
   * @param[in]     value       the value the spin box is set to.
   */
  void setValueQuietly(QDoubleSpinBox* spin_box, double value);


  /** @brief Ui object - connection to GUI. */
  Ui::trajectory_editor ui_;
  /** @brief Widget. */
  QWidget* widget_;

  /** @brief Publishes the camera placement. */
  ros::Publisher camera_placement_pub_;
  /** @brief Publishes the trajectory that is defined by the markers. */
  ros::Publisher view_poses_array_pub_;
  /** @brief Publishes the transition steps between two markers. */
  ros::Publisher transition_steps_pub_;
  /** @brief Subscribes to the camera pose. */
  ros::Subscriber camera_pose_sub_;
  /** @brief Publishes the trajectory steps in a specified rate. */
  ros::Timer trajectory_publish_timer_;
   /** @brief Publishing rate. */
  double timer_rate_;

  /** @brief Connects markers to callbacks. */
  interactive_markers::MenuHandler menu_handler_;
  /** @brief Stores markers - needed for #menu_handler. */
  std::shared_ptr<interactive_markers::InteractiveMarkerServer> server_;

  /** @brief Current camera pose. */
  geometry_msgs::Pose cam_pose_;

  /** @brief Currently selected marker. */
  TimedMarker current_marker_;

  /** @brief Currently maintained list of TimedMarkers. */
  MarkerList markers_;

  /** @brief Flag that initiates the publishing of the transition steps. */
  bool publish_transition_steps_;
  /** @brief Duration of the current transition. */
  ros::Duration current_transition_duration_;
  /** @brief Time the current transition started. */
  ros::Time transition_start_time_;
  /** @brief Pose defining the start of the current transition. */
  geometry_msgs::Pose start_pose_;
  /** @brief Marker defining the end position. */
  geometry_msgs::Pose end_pose_;
};

} // namespace

#endif //RQT_POSE_INTERPOLATOR_TRAJECTORY_EDITOR_H
