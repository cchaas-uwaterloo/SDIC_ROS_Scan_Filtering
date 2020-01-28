#pragma once

#include "beam_calibration/CameraModel.h"
#include "beam_calibration/PinholeCamera.h"
#include "beam_calibration/TfTree.h"
#include <darknet_ros_msgs/BoundingBox.h>
#include <geometry_msgs/Twist.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/common/transforms.h>
#include <cv_bridge/cv_bridge.h>
#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/PointCloud2.h>
#include <std_srvs/Trigger.h>
#include <darknet_ros_msgs/BoundingBoxes.h>
#include <darknet_ros_msgs/BoundingBox.h>
#include <darknet_ros_msgs/ObjectCount.h>
#include "beam_utils/math.hpp"
#include <std_msgs/Int16.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/common/common_headers.h>
#include <pcl/features/normal_3d.h>
#include <pcl/io/pcd_io.h>
#include <pcl/console/parse.h>
#include <opencv2/datasets/gr_skig.hpp>
#include <beam_calibration/LadybugCamera.h>

typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PCLPointCloud2 PointCloud2;

enum camera_t {
  lblink,
  lbcam0,
  lbcam1,
  lbcam2,
  lbcam3,
  lbcam4,
  lbcam5,
  lbtest,
  lbLast
};

namespace point_filter {

class PointFilter {
public:
  explicit PointFilter(ros::NodeHandle& node_handle);

  ~PointFilter() = default;

private:
  /**
   * Function for reading ROS parameters on initialization.
   * @return Returns true if successful.
   */
  bool ReadParameters();

  bool LoadIntrinsics();

  bool LoadExtrinsics();

  /*
  * @brief callback function for image topic subscription
  */
  void ImageCallback(const sensor_msgs::Image& image_msg);

  /*
  * @brief callback function for Lidar topic subscription
  */
  void LidarCallback(const sensor_msgs::PointCloud2& lidar_msg);

  /*
  * @brief callback funtion for Bounding Box topic subscription
  */
  void BoundingBoxCallback(const darknet_ros_msgs::BoundingBoxesConstPtr& bounding_box_msg);

  /*
  * @brief callback funcion for found_object count topic subscription
  * @descr filtering and publishing to filtered topic done here
  */
  void ObjectCountCallback(const darknet_ros_msgs::ObjectCountConstPtr& msg);

  /*
  * @brief callback funtion for Camera Number topic subscription
  */
  void CameraNumberCallback(const std_msgs::Int16 msg);

  ros::NodeHandle& nh_;      // ROS node handle.
  ros::NodeHandle nh_priv_;  // Private ROS node handle.
  ros::Publisher publisher_, publisher_test, publisher_ack_; // ROS publisher for filtered cloud
  beam_calibration::TfTree extrinsics_; // Robot extrinsics (pysical calibrations between sensors)
  std::shared_ptr<beam_calibration::CameraModel>
      intrinsics_[6]; // camera intrinsics (internal camera calibrations)
  ros::Subscriber image_subscriber_, lidar_subscriber_,
      bounding_box_subscriber_, object_count_subscriber_, camera_number_subscriber_;
  std::string bounding_box_topic_, image_topic_, lidar_topic_, publish_topic_, object_count_topic_, publish_topic_test_, camera_number_topic_, publish_ack_topic_;

  PointCloud::Ptr scan_; //raw point cloud comming from LiDAR topic

  PointCloud::Ptr  filteredScanLidarInv_;

  //PointCloud::Ptr displayBBPCL_; //point cloud showing the radiating bounding boxes

  //PointCloud::Ptr filteredScan_; //filtered point cloud with vehicles and people removed
  sensor_msgs::PointCloud2 scanOutput_; //filtered output converted to ROS message format

  sensor_msgs::PointCloud2 scanOutputInv_;

  std_msgs::Int16 ack_m;

  cv::Mat imageCam_[6];
  cv::Mat lastImage_;
  uint8_t imageSem_;

  darknet_ros_msgs::BoundingBoxes boundingBoxListCam_[6];

  uint8_t objectFoundCountCam_[6];
  float filterIntensity_;  //factor affecting how much gets filtered, a higher value corresponds to more being filtered from the point scan


  std::string lbLinkName_, lbCam0Name_, lbCam1Name_, lbCam2Name_, lbCam3Name_,
          lbCam4Name_, lbCam5Name_, lidarBaseName_;

  uint8_t reading_camera_;

  /*test transform*/
  Eigen::Affine3d transform_TEST;

  /*
  * @brief Transforms the raw point cloud in LiDAR reference frame to camera's reference frame
  * @params Takes raw point cloud data received via ROS Topic and enumerated camera to transform to
  * @return void
  */
  void transformToCameraFrame(const PointCloud::Ptr input_scan, PointCloud::Ptr out_scan, camera_t camera);

  /*
  * @brief Transforms the point cloud in the camera's reference frame back to the LiDAR reference frame
  * @params Takes point cloud data in camera frame and enumerated camera to transform from
  * @return void
  */
  void transformToLidarFrame(const PointCloud::Ptr input_scan, PointCloud::Ptr out_scan, camera_t camera);

  /*
  * @brief Filters out point cloud points identified as belonging to vehicles or other filterObjects
  * @params Takes point cloud data in camera's reference frame
  * @return filtered point cloud
  */
  PointCloud::Ptr filterObjects(PointCloud::Ptr input_scan_, camera_t camera);

  /*
  * @brief Helper function to compare two images
  */
  bool imageIsEqual(const cv::Mat mat1, const cv::Mat mat2);

  /*
  * @brief Removes points more than one standard deviation from the mean in each bounding box from the filtered
  */
  void restoreOutliers(PointCloud::Ptr& cloud, uint8_t objectCount, const std::vector<std::vector<pcl::PointXYZ>>& bbPoints, float factor);

  /****************************************************************DEBUG FUNCTIONS***********************************************/

  /*
  * @Test
  * @brief Tests projection of points to camera frame(s) and outputs coordinates
  * @params Takes point cloud in camera's reference frame
  * @Return void
  */
  void testProjection(const PointCloud::Ptr cameraFrameScan_, camera_t camera);

  /*
  * @brief Test function to visualize point clouds
  * @note Not multi-threaded so will halt program execution until closed
  */
  void visualizer(const pcl::PointCloud<pcl::PointXYZ>::Ptr& cloud, std::string displayName);

  /*
  * @brief saves the camera image overlayed with the projected points and bounding boxes
  */
  void displayProjectionImage(camera_t camera, const cv::Mat image, std::vector<beam::Vec2> projectedPoints);



};

} // namespace beam_follower
