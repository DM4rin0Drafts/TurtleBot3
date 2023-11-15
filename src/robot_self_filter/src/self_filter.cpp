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

#include "robot_self_filter/self_filter.h"

    SelfFilter::SelfFilter (ros::NodeHandle& nh_,ros::NodeHandle& pnh_)
    {
      pnh_.param<std::string> ("sensor_frame", sensor_frame_, std::string ());
      pnh_.param<double> ("subsample_value", subsample_param_, 0.0);
      self_filter_ = new filters::SelfFilter<pcl::PointCloud<pcl::PointXYZI> > (pnh_);

      sub_ = new message_filters::Subscriber<sensor_msgs::PointCloud2> (nh_, "cloud_in", 10);	
      mn_ = new tf::MessageFilter<sensor_msgs::PointCloud2> (*sub_, tf_, "", 30);

      //mn_ = new tf::MessageNotifier<sensor_msgs::PointCloud2>(tf_, boost::bind(&SelfFilter::cloudCallback, this, _1), "cloud_in", "", 1);
      pointCloudPublisher_ = nh_.advertise<sensor_msgs::PointCloud2>("cloud_out", 10);
      std::vector<std::string> frames;
      self_filter_->getSelfMask()->getLinkNames(frames);
      if (frames.empty())
      {
        ROS_INFO ("No valid frames have been passed into the self filter. Using a callback that will just forward scans on.");
        no_filter_sub_ = nh_.subscribe<sensor_msgs::PointCloud2> ("cloud_in", 1, boost::bind(&SelfFilter::noFilterCallback, this, _1));
      }
      else
      {
        ROS_INFO ("Valid frames were passed in. We'll filter them.");
        mn_->setTargetFrames (frames);
        mn_->registerCallback (boost::bind (&SelfFilter::cloudCallback, this, _1));
      }
    }
      
    SelfFilter::~SelfFilter (void)
    {
      delete self_filter_;
      delete mn_;
      delete sub_;
    }
      
    void 
      SelfFilter::noFilterCallback (const sensor_msgs::PointCloud2ConstPtr &cloud)
    {
      pointCloudPublisher_.publish (cloud);
      ROS_DEBUG ("Self filter publishing unfiltered frame");
    }
      
    void SelfFilter::cloudCallback (const sensor_msgs::PointCloud2ConstPtr &cloud2)
    {
      ROS_DEBUG ("Got pointcloud that is %f seconds old", (ros::Time::now() - cloud2->header.stamp).toSec ());
      //std::vector<int> mask;
      ros::WallTime tm = ros::WallTime::now ();

      pcl::PointCloud<pcl::PointXYZI>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZI>), cloud_filtered(new pcl::PointCloud<pcl::PointXYZI>);
      pcl::fromROSMsg (*cloud2, *cloud);

      if (subsample_param_ != 0.0)
      {
        pcl::PointCloud<pcl::PointXYZI> cloud_downsampled;
        // Set up the downsampling filter
        grid_.setLeafSize (subsample_param_, subsample_param_, subsample_param_);     // 1cm leaf size
        grid_.setInputCloud (cloud);
        grid_.filter (cloud_downsampled);

        self_filter_->updateWithSensorFrame (cloud_downsampled, *cloud_filtered, sensor_frame_);
      } 
      else 
      {
        self_filter_->updateWithSensorFrame (*cloud, *cloud_filtered, sensor_frame_);
      }      

      //double sec = (ros::WallTime::now() - tm).toSec ();

      ROS_DEBUG ("Self filter: reduced %d points to %d points in %f seconds", (int)cloud->points.size(), (int)cloud_filtered->points.size (), (ros::WallTime::now() - tm).toSec ());

      sensor_msgs::PointCloud2Ptr cloud2_out = boost::make_shared<sensor_msgs::PointCloud2>();
      pcl::toROSMsg(*cloud_filtered, *cloud2_out);
      pointCloudPublisher_.publish(cloud2_out);
    }

