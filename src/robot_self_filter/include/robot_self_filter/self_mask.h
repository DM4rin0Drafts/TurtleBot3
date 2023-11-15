/*
 * Copyright (c) 2008, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ROBOT_SELF_FILTER_SELF_MASK_
#define ROBOT_SELF_FILTER_SELF_MASK_

#include <sensor_msgs/PointCloud.h>
#include <geometric_shapes/bodies.h>
#include <tf/transform_listener.h>
#include <boost/bind.hpp>
#include <string>
#include <vector>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>


#include <urdf/model.h>
#include <resource_retriever/retriever.h>
#include <geometric_shapes/shape_operations.h>
#include <geometric_shapes/mesh_operations.h>
#include <geometric_shapes/body_operations.h>
#include <ros/console.h>
#include <algorithm>
#include <sstream>
#include <climits>

#include <tf_conversions/tf_eigen.h>

#include <pcl_conversions/pcl_conversions.h>

#if defined(IS_ASSIMP3)
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#else
#include <assimp/aiScene.h>
#include <assimp/assimp.hpp>
#include <assimp/aiPostProcess.h>
#endif

namespace robot_self_filter
{   
    static inline Eigen::Isometry3d urdfPose2EigenTransform(const urdf::Pose &pose)
    {
        return Eigen::Translation3d(pose.position.x, pose.position.y, pose.position.z) * Eigen::Quaterniond(pose.rotation.x, pose.rotation.y, pose.rotation.z, pose.rotation.w);
    }
    
    static shapes::Shape* constructShape(const urdf::Geometry *geom)
    {
        ROS_ASSERT(geom);
        
        shapes::Shape *result = NULL;
        switch (geom->type)
        {
            case urdf::Geometry::SPHERE:
            {
                result = new shapes::Sphere(dynamic_cast<const urdf::Sphere*>(geom)->radius);
                break;
            }
            case urdf::Geometry::BOX:
            {
                urdf::Vector3 dim = dynamic_cast<const urdf::Box*>(geom)->dim;
                result = new shapes::Box(dim.x, dim.y, dim.z);
                break;
            }
            case urdf::Geometry::CYLINDER:
            {
                result = new shapes::Cylinder(dynamic_cast<const urdf::Cylinder*>(geom)->radius,
                                              dynamic_cast<const urdf::Cylinder*>(geom)->length);
                break;
            }
            case urdf::Geometry::MESH:
            {
                const urdf::Mesh *mesh = dynamic_cast<const urdf::Mesh*>(geom);
                if (!mesh->filename.empty())
                {
                    Eigen::Vector3d scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
                    result = shapes::createMeshFromResource(mesh->filename, scale);
                } else
                    ROS_WARN("Empty mesh filename");
                break;
            }
            default:
            {
                ROS_ERROR("Unknown geometry type: %d", (int)geom->type);
                break;
            }
        }
        return (result);
    }
    
    /** \brief The possible values of a mask computed for a point */
    enum
    {
        INSIDE = 0,
        OUTSIDE = 1,
        SHADOW = 2,
        INVALID =3
    };
    
    struct LinkInfo
    {
        std::string name;
        double padding;
        double scale;
    };
    
    /** \brief Computing a mask for a pointcloud that states which points are inside the robot */
    
    template <typename T>
    class SelfMask
    {	
    protected:
        
        struct SeeLink
        {
            SeeLink(void)
            {
                body = unscaledBody = NULL;
            }
            
            std::string   name;
            bodies::Body *body;
            bodies::Body *unscaledBody;
            Eigen::Isometry3d constTransf;
            double        volume;
        };
        
        struct SortBodies
        {
            bool operator()(const SeeLink &b1, const SeeLink &b2)
            {
                return (b1.volume > b2.volume);
            }
        };
        
    public:
        
        /** \brief Construct the filter */
        SelfMask(tf::TransformListener &tf, const std::vector<LinkInfo> &links, ros::NodeHandle nh)
        : tf_(tf)
        , nh_(nh)
        {
            configure (links);
        }
        
        /** \brief Destructor to clean up */
        ~SelfMask(void)
        {
            freeMemory ();
        }
        
        /** \brief Compute the containment mask (INSIDE or OUTSIDE) for a given pointcloud. If a mask element is INSIDE, the point
         *            is inside the robot. The point is outside if the mask element is OUTSIDE.
         */
        void maskContainment (const T& data_in, std::vector<int> &mask)
        {
            mask.resize(data_in.points.size());
            if (bodies_.empty())
                std::fill(mask.begin(), mask.end(), (int)OUTSIDE);
            else
            {
                assumeFrame(data_in.header.frame_id, pcl_conversions::fromPCL(data_in.header).stamp);
                maskAuxContainment(data_in, mask);
            }
        }
        
        /** \brief Compute the intersection mask for a given
         *            pointcloud. If a mask element can have one of the values
         *            INSIDE, OUTSIDE or SHADOW. If the value is SHADOW, the
         *            point is on a ray behind the robot and should not have
         *            been seen. If the mask element is INSIDE, the point is
         *            inside the robot. The sensor frame is specified to obtain
         *            the origin of the sensor. A callback can be registered for
         *            the first intersection point on each body.
         */
        void maskIntersection (const T& data_in, const std::string &sensor_frame, const double min_sensor_dist,
                               std::vector<int> &mask, const boost::function<void(const Eigen::Vector3d&)> &callback = NULL)
        {
            mask.resize(data_in.points.size());
            if (bodies_.empty()) {
                std::fill(mask.begin(), mask.end(), (int)OUTSIDE);
            }
            else
            {
                assumeFrame(data_in.header.frame_id, pcl_conversions::fromPCL(data_in.header).stamp, sensor_frame, min_sensor_dist);
                if (sensor_frame.empty())
                    maskAuxContainment(data_in, mask);
                else
                    maskAuxIntersection(data_in, mask, callback);
            }
        };
        
        /** \brief Compute the intersection mask for a given pointcloud. If a mask
         *            element can have one of the values INSIDE, OUTSIDE or SHADOW. If the value is SHADOW,
         *            the point is on a ray behind the robot and should not have
         *            been seen. If the mask element is INSIDE, the point is inside
         *            the robot. The origin of the sensor is specified as well.
         */
        void maskIntersection (const T& data_in, const Eigen::Vector3d &sensor_pos, const double min_sensor_dist,
                               std::vector<int> &mask, const boost::function<void(const Eigen::Vector3d&)> &callback = NULL)
        {
            mask.resize(data_in.points.size());
            if (bodies_.empty())
                std::fill(mask.begin(), mask.end(), (int)OUTSIDE);
            else
            {
                assumeFrame(data_in.header.frame_id, pcl_conversions::fromPCL(data_in.header).stamp, sensor_pos, min_sensor_dist);
                maskAuxIntersection(data_in, mask, callback);
            }
        };
        
        
        /** \brief Assume subsequent calls to getMaskX() will be in the frame passed to this function.
         *   The frame in which the sensor is located is optional */
        void assumeFrame (const std::string &frame_id, const ros::Time &stamp)
        {
            const unsigned int bs = bodies_.size();
            
            // place the links in the assumed frame 
            for (unsigned int i = 0 ; i < bs ; ++i)
            {
                std::string err;
                if(!tf_.waitForTransform(frame_id, bodies_[i].name, stamp, ros::Duration(.1), ros::Duration(.01), &err)) {
                    ROS_ERROR("WaitForTransform timed out from %s to %s after 100ms.  Error string: %s", bodies_[i].name.c_str(), frame_id.c_str(), err.c_str());      
                } 
                
                // find the transform between the link's frame and the pointcloud frame
                tf::StampedTransform transf;
                Eigen::Isometry3d transf_eigen;
                try
                {
                    tf_.lookupTransform(frame_id, bodies_[i].name, stamp, transf);
                    tf::transformTFToEigen(transf, transf_eigen);
                }
                catch(tf::TransformException& ex)
                {
                    transf.setIdentity();
                    ROS_ERROR("Unable to lookup transform from %s to %s. Exception: %s", bodies_[i].name.c_str(), frame_id.c_str(), ex.what());	
                }
                
                // set it for each body; we also include the offset specified in URDF
                bodies_[i].body->setPose(transf_eigen * bodies_[i].constTransf);
                bodies_[i].unscaledBody->setPose(transf_eigen * bodies_[i].constTransf);
            }
            
            computeBoundingSpheres();
        };
        
        /** \brief Assume subsequent calls to getMaskX() will be in the frame passed to this function.
         *   The frame in which the sensor is located is optional */
        void assumeFrame (const std::string &frame_id, const ros::Time &stamp, const std::string &sensor_frame, const double min_sensor_dist)
        {
            assumeFrame(frame_id,stamp);
            
            std::string err;
            if(!tf_.waitForTransform(frame_id, sensor_frame, stamp, ros::Duration(.1), ros::Duration(.01), &err)) {
                ROS_ERROR("WaitForTransform timed out from %s to %s after 100ms.  Error string: %s", sensor_frame.c_str(), frame_id.c_str(), err.c_str());
                sensor_pos_.setZero();
            } 
            
            //transform should be there
            // compute the origin of the sensor in the frame of the cloud
            try
            {
                tf::StampedTransform transf;
                tf_.lookupTransform(frame_id, sensor_frame, stamp, transf);
                tf::vectorTFToEigen(transf.getOrigin(), sensor_pos_);
            }
            catch(tf::TransformException& ex)
            {
                sensor_pos_.setZero();
                ROS_ERROR("Unable to lookup transform from %s to %s.  Exception: %s", sensor_frame.c_str(), frame_id.c_str(), ex.what());
            }
            
            min_sensor_dist_ = min_sensor_dist;
        };
        
        /** \brief Assume subsequent calls to getMaskX() will be in the frame passed to this function.
         *  Also specify which possition to assume for the sensor (frame is not needed) */
        void assumeFrame (const std::string &frame_id, const ros::Time &stamp, const Eigen::Vector3d &sensor_pos, const double min_sensor_dist)
        {
            assumeFrame(frame_id,stamp);
            sensor_pos_ = sensor_pos;
            min_sensor_dist_ = min_sensor_dist;
        };
        
        /** \brief Get the containment mask (INSIDE or OUTSIDE) value for an individual point. No
         *            setup is performed, assumeFrame() should be called before use */
        int getMaskContainment (double x, double y, double z) const
        {
            return getMaskContainment(Eigen::Vector3d(x, y, z));
        };
        
        /** \brief Get the containment mask (INSIDE or OUTSIDE) value for an individual point. No
         *            setup is performed, assumeFrame() should be called before use */
        int getMaskContainment (const Eigen::Vector3d &pt) const
        {
            const unsigned int bs = bodies_.size();
            int out = OUTSIDE;
            for (unsigned int j = 0 ; out == OUTSIDE && j < bs ; ++j)
                if (bodies_[j].body->containsPoint(pt))
                    out = INSIDE;
                return out;
        }
        
        /** \brief Get the intersection mask (INSIDE, OUTSIDE or
         *            SHADOW) value for an individual point. No setup is
         *            performed, assumeFrame() should be called before use */
        int getMaskIntersection (double x, double y, double z, const boost::function<void(const Eigen::Vector3d&)> &callback = NULL) const
        {
            return getMaskContainment(Eigen::Vector3d(x, y, z));
        };
        /** \brief Get the intersection mask (INSIDE, OUTSIDE or
         *            SHADOW) value for an individual point. No setup is
         *            performed, assumeFrame() should be called before use */
        int getMaskIntersection (const Eigen::Vector3d &pt, const boost::function<void(const Eigen::Vector3d&)> &callback = NULL) const
        {  
            const unsigned int bs = bodies_.size();
            
            // we first check is the point is in the unscaled body. 
            // if it is, the point is definitely inside
            int out = OUTSIDE;
            for (unsigned int j = 0 ; out == OUTSIDE && j < bs ; ++j)
                if (bodies_[j].unscaledBody->containsPoint(pt))
                    out = INSIDE;
                
                if (out == OUTSIDE)
                {
                    // we check it the point is a shadow point 
                    Eigen::Vector3d dir(sensor_pos_ - pt);
                    tfScalar  lng = dir.norm();
                    if (lng < min_sensor_dist_)
                        out = INSIDE;
                    else
                    {
                        dir /= lng;
                        
                        EigenSTL::vector_Vector3d intersections;
                        for (unsigned int j = 0 ; out == OUTSIDE && j < bs ; ++j)
                            if (bodies_[j].body->intersectsRay(pt, dir, &intersections, 1))
                            {
                                if (dir.dot(sensor_pos_ - intersections[0]) >= 0.0)
                                {
                                    if (callback)
                                        callback(intersections[0]);
                                    out = SHADOW;
                                }
                            }
                            
                            // if it is not a shadow point, we check if it is inside the scaled body
                            for (unsigned int j = 0 ; out == OUTSIDE && j < bs ; ++j)
                                if (bodies_[j].body->containsPoint(pt))
                                    out = INSIDE;
                    }
                }
                return (out);
        };
        
        /** \brief Get the set of link names that have been instantiated for self filtering */
        void getLinkNames (std::vector<std::string> &frames) const
        {
            for (unsigned int i = 0 ; i < bodies_.size() ; ++i)
                frames.push_back(bodies_[i].name);
        };
        
    private:
        /** \brief Free memory. */
        void freeMemory (void){
            for (unsigned int i = 0 ; i < bodies_.size() ; ++i)
            {
                if (bodies_[i].body)
                    delete bodies_[i].body;
                if (bodies_[i].unscaledBody)
                    delete bodies_[i].unscaledBody;
            }
            bodies_.clear ();
        }
        
        
        
        /** \brief Configure the filter. */
        bool configure (const std::vector<LinkInfo> &links)
        {
            // in case configure was called before, we free the memory
            freeMemory();
            sensor_pos_.setZero();
            
            std::string content;
            boost::shared_ptr<urdf::Model> urdfModel;
            
            //Workaround because remapping doesn't appear to work with nodelets
            //std::string robot_description_param_name;
            //nh_.param("robot_description_param_name", robot_description_param_name, std::string("robot_description"));
            //ROS_INFO("Using robot_description param name: %s", (nh_.getNamespace() + robot_description_param_name).c_str());

            std::string param_key;

            if (nh_.getParam("/robot_description", content) || (nh_.searchParam("robot_description", param_key) && nh_.getParam(param_key, content)))
            {
                urdfModel = boost::shared_ptr<urdf::Model>(new urdf::Model());
                if (!urdfModel->initString(content))
                {
                    ROS_ERROR("Unable to parse URDF description!");
                    return false;
                }
            }
            else
            {
                ROS_ERROR("Robot model not found! Did you remap 'robot_description'?");
                return false;
            }
            
            std::stringstream missing;
            
            // from the geometric model, find the shape of each link of interest
            // and create a body from it, one that knows about poses and can 
            // check for point inclusion
            for (unsigned int i = 0 ; i < links.size() ; ++i)
            {
                const urdf::Link *link = urdfModel->getLink(links[i].name).get();
                if (!link)
                {
                    missing << " " << links[i].name;
                    continue;
                }
                //ROS_INFO("Name '%s'", links[i].name.c_str());
                //ROS_INFO("Size: '%d'", link->collision_array.size());
                
                
                if (!(link->collision && link->collision->geometry))
                {
                    ROS_WARN("No collision geometry specified for link '%s'", links[i].name.c_str());
                    continue;
                }
                
                for (size_t j = 0; j < link->collision_array.size(); ++j){
                    
                    shapes::Shape *shape = constructShape(link->collision_array[j]->geometry.get());
                    
                    if (!shape)
                    {
                        ROS_ERROR("Unable to construct collision shape for link '%s'", links[i].name.c_str());
                        continue;
                    }
                    
                    SeeLink sl;
                    sl.body = bodies::createBodyFromShape(shape);
                    
                    if (sl.body)
                    {
                        sl.name = links[i].name;
                        
                        // collision models may have an offset, in addition to what TF gives
                        // so we keep it around
                        sl.constTransf = urdfPose2EigenTransform(link->collision_array[j]->origin);
                        
                        sl.body->setScale(links[i].scale);
                        sl.body->setPadding(links[i].padding);
                        ROS_INFO_STREAM("Self see link name " <<  links[i].name << " padding " << links[i].padding);
                        sl.volume = sl.body->computeVolume();
                        sl.unscaledBody = bodies::createBodyFromShape(shape);
                        bodies_.push_back(sl);
                    }
                    else
                        ROS_WARN("Unable to create point inclusion body for link '%s'", links[i].name.c_str());
                    
                    delete shape;
                }
            }
            
            if (missing.str().size() > 0)
                ROS_WARN("Some links were included for self mask but they do not exist in the model:%s", missing.str().c_str());
            
            if (bodies_.empty())
                ROS_WARN("No robot links will be checked for self mask");
            
            // put larger volume bodies first -- higher chances of containing a point
            std::sort(bodies_.begin(), bodies_.end(), SortBodies());
            
            bspheres_.resize(bodies_.size());
            bspheresRadius2_.resize(bodies_.size());
            
            for (unsigned int i = 0 ; i < bodies_.size() ; ++i)
                ROS_DEBUG("Self mask includes link %s with volume %f", bodies_[i].name.c_str(), bodies_[i].volume);
            
            //ROS_INFO("Self filter using %f padding and %f scaling", padd, scale);
            
            return true; 
        };
        
        /** \brief Compute bounding spheres for the checked robot links. */
        void computeBoundingSpheres (void)
        {
            const unsigned int bs = bodies_.size();
            for (unsigned int i = 0 ; i < bs ; ++i)
            {
                bodies_[i].body->computeBoundingSphere(bspheres_[i]);
                bspheresRadius2_[i] = bspheres_[i].radius * bspheres_[i].radius;
            }
        }
        
        /** \brief Perform the actual mask computation. */
        void maskAuxContainment (const T& data_in, std::vector<int> &mask)
        {
            const unsigned int bs = bodies_.size();
            const unsigned int np = data_in.points.size();
            
            // compute a sphere that bounds the entire robot
            bodies::BoundingSphere bound;
            bodies::mergeBoundingSpheres(bspheres_, bound);	  
            tfScalar radiusSquared = bound.radius * bound.radius;
            
            // we now decide which points we keep
            //#pragma omp parallel for schedule(dynamic) 
            for (int i = 0 ; i < (int)np ; ++i)
            {
                Eigen::Vector3d pt = Eigen::Vector3d(data_in.points[i].x, data_in.points[i].y, data_in.points[i].z);
                if (pt.hasNaN()) {
                  mask[i] = INVALID;
                  continue;
                }

                int out = OUTSIDE;
                if ((bound.center - pt).squaredNorm() < radiusSquared)
                    for (unsigned int j = 0 ; out == OUTSIDE && j < bs ; ++j)
                        if (bodies_[j].body->containsPoint(pt))
                            out = INSIDE;
                        
                        mask[i] = out;
            }
        };
        
        /** \brief Perform the actual mask computation. */
        void maskAuxIntersection (const T& data_in, std::vector<int> &mask, const boost::function<void(const Eigen::Vector3d&)> &callback)
        {
            const unsigned int bs = bodies_.size();
            const unsigned int np = data_in.points.size();
            
            // compute a sphere that bounds the entire robot
            bodies::BoundingSphere bound;
            bodies::mergeBoundingSpheres(bspheres_, bound);	  
            tfScalar radiusSquared = bound.radius * bound.radius;
            
            //std::cout << "Testing " << np << " points\n";
            
            // we now decide which points we keep
            //#pragma omp parallel for schedule(dynamic) 
            for (int i = 0 ; i < (int)np ; ++i)
            {
                bool print = false;
                //if(i%100 == 0) print = true;
                Eigen::Vector3d pt = Eigen::Vector3d(data_in.points[i].x, data_in.points[i].y, data_in.points[i].z);
                int out = OUTSIDE;
                if (pt.hasNaN()) {
                  mask[i] = INVALID;
                  continue;
                }
                
                // we first check is the point is in the unscaled body. 
                // if it is, the point is definitely inside
                if ((bound.center - pt).squaredNorm() < radiusSquared)
                    for (unsigned int j = 0 ; out == OUTSIDE && j < bs ; ++j)
                        if (bodies_[j].unscaledBody->containsPoint(pt)) 
                        {
                            if(print)
                                std::cout << "Point " << i << " in unscaled body part " << bodies_[j].name << std::endl;
                            out = INSIDE;
                        }
                        
                        // if the point is not inside the unscaled body,
                        if (out == OUTSIDE)
                        {
                            // we check it the point is a shadow point 
                            Eigen::Vector3d dir(sensor_pos_ - pt);
                            tfScalar  lng = dir.norm();
                            if (lng < min_sensor_dist_) 
                            {
                                out = INSIDE;
                                //std::cout << "Point " << i << " less than min sensor distance away\n";
                            }
                            else
                            {		
                                dir /= lng;
                                EigenSTL::vector_Vector3d intersections;
                                for (unsigned int j = 0 ; out == OUTSIDE && j < bs ; ++j) 
                                {
                                    if (bodies_[j].body->intersectsRay(pt, dir, &intersections, 1))
                                    {
                                        if (dir.dot(sensor_pos_ - intersections[0]) >= 0.0)
                                        {
                                            if (callback)
                                                callback(intersections[0]);
                                            out = SHADOW;
                                            if(print) std::cout << "Point " << i << " shadowed by body part " << bodies_[j].name << std::endl;
                                        }
                                    }
                                }
                                // if it is not a shadow point, we check if it is inside the scaled body
                                if (out == OUTSIDE && (bound.center - pt).squaredNorm() < radiusSquared)
                                    for (unsigned int j = 0 ; out == OUTSIDE && j < bs ; ++j)
                                        if (bodies_[j].body->containsPoint(pt)) 
                                        {
                                            if(print) 
                                                std::cout << "Point " << i << " in scaled body part " << bodies_[j].name << std::endl;
                                            out = INSIDE;
                                        }
                            }
                        }
                        mask[i] = out;
            }
        };
        
        tf::TransformListener               &tf_;
        ros::NodeHandle                     nh_;
        
        Eigen::Vector3d                     sensor_pos_;
        double                              min_sensor_dist_;
        
        std::vector<SeeLink>                bodies_;
        std::vector<double>                 bspheresRadius2_;
        std::vector<bodies::BoundingSphere> bspheres_;
    };
}

#endif
