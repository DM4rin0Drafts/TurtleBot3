/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/** \author Ioan Sucan */

#ifndef ROBOT_SELF_FILTER_SELF_FILTER_
#define ROBOT_SELF_FILTER_SELF_FILTER_

#include <ros/ros.h>
#include <sstream>
#include "robot_self_filter/self_see_filter.h"
#include <tf/message_filter.h>
#include <message_filters/subscriber.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/filters/voxel_grid.h>

class SelfFilter
{
  public:
    SelfFilter (ros::NodeHandle& nh_,ros::NodeHandle& pnh_);    
    ~SelfFilter (void);
      
  private:
    void noFilterCallback (const sensor_msgs::PointCloud2ConstPtr &cloud);
      
    void cloudCallback (const sensor_msgs::PointCloud2ConstPtr &cloud2);

    tf::TransformListener                                 tf_;
    //tf::MessageNotifier<sensor_msgs::PointCloud>           *mn_;
    //ros::NodeHandle                                       nh_, root_handle_;

    tf::MessageFilter<sensor_msgs::PointCloud2>           *mn_;
    message_filters::Subscriber<sensor_msgs::PointCloud2> *sub_;

    filters::SelfFilter<pcl::PointCloud<pcl::PointXYZI> > *self_filter_;
    std::string sensor_frame_;
    double subsample_param_;

    ros::Publisher                                        pointCloudPublisher_;
    ros::Subscriber                                       no_filter_sub_;

    pcl::VoxelGrid<pcl::PointXYZI>                         grid_;
};

#endif
