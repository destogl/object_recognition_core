/*
 * twoDToThreeD.cpp
 *
 *  Created on: Jun 16, 2011
 *      Author: vrabaud
 */

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <ecto/ecto.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <pcl/point_types.h>
#include <pcl/point_cloud.h>

#include <iostream>

#include "opencv_candidate/PoseRT.h"

#include "csv.h"

typedef unsigned int ObjectId;

using ecto::tendrils;

namespace object_recognition
{
namespace tod
{

/** Ecto implementation of a module that takes
 *
 */
struct GuessCsvWriter
{
  static void declare_params(tendrils& p)
  {
    p.declare<std::string>("base_directory", "Base directory");
    p.declare<std::string>("config_file", "Configuration file");
    p.declare<std::string>("team_name", "The name of the team to consider");
    p.declare<int>("run_number", "The run number");
  }

  static void declare_io(const tendrils& params, tendrils& inputs, tendrils& outputs)
  {
    inputs.declare<std::vector<ObjectId> >("object_ids", "the id's of the found objects");
    inputs.declare<std::vector<opencv_candidate::Pose> >("poses", "The poses of the found objects");
  }

  void configure(const tendrils& params, const tendrils& inputs,const tendrils& outputs)
  {
    team_name_ = params.get<std::string>("team_name");
    run_number_ = params.get<int>("run_number");
  }

  /** Get the 2d keypoints and figure out their 3D position from the depth map
   * @param inputs
   * @param outputs
   * @return
   */
  int process(const tendrils& inputs,const tendrils& outputs)
  {
    // match to our objects
    const std::vector<ObjectId> &object_ids = inputs.get<std::vector<ObjectId> >("object_ids");
    const std::vector<opencv_candidate::Pose> &poses = inputs.get<std::vector<opencv_candidate::Pose> >("poses");

    RunInfo run_info;
    run_info.ts.set();
    run_info.runID = run_number_;
    run_info.name = team_name_;
    tod::CSVOutput csv_out = openCSV(run_info);
    int dID = 0; //detection id
    for (unsigned int i = 0; i < object_ids.size(); ++i)
    {
      const ObjectId & object_id = object_ids[i];
      const opencv_candidate::Pose & pose = poses[i];

      tod::PoseInfo poseInfo;
      cv::Mat R = pose.r<cv::Mat>();
      cv::Mat T = pose.t<cv::Mat>();
      for (int i = 0; i < 9; i++)
      {
        poseInfo.Rot[i] = R.at<double>(i % 3, i / 3);
      }

      poseInfo.Tx = T.at<double>(0);
      poseInfo.Ty = T.at<double>(1);
      poseInfo.Tz = T.at<double>(2);
      poseInfo.ts.set();
      //poseInfo.frame = point_cloud.header.seq;
      poseInfo.oID = object_id;
      poseInfo.dID = dID++; //training (only one detection per frame)
      writeCSV(csv_out, poseInfo);
    }

    return 0;
  }
private:
  int run_number_;
  std::string team_name_;
};
}
}

ECTO_CELL(tod, object_recognition::tod::GuessCsvWriter, "GuessCsvWriter",
          "Given guesses, writes them to a CSV.");
