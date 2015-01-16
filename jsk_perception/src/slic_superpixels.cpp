// -*- mode: c++ -*-
/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014, JSK Lab
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
 *     disclaimer in the documentation and/o2r other materials provided
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

#include "jsk_perception/slic_superpixels.h"
#include "slic.h"

namespace jsk_perception
{
      
  void SLICSuperPixels::onInit()
  {
    nh_ = ros::NodeHandle(getNodeHandle(), "image");
    pnh_ = getPrivateNodeHandle();
    it_.reset(new image_transport::ImageTransport(nh_));
    pub_debug_ = pnh_.advertise<sensor_msgs::Image>("debug", 1);
    image_sub_ = it_->subscribe("", 1, &SLICSuperPixels::imageCallback, this);
  }
  
  void SLICSuperPixels::imageCallback(const sensor_msgs::Image::ConstPtr& image)
  {
    boost::mutex::scoped_lock lock(mutex_);
    cv_bridge::CvImagePtr cv_ptr;
    cv_ptr = cv_bridge::toCvCopy(image, sensor_msgs::image_encodings::BGR8);
    cv::Mat bgr_image = cv_ptr->image;

    IplImage bgr_image_ipl;
    bgr_image_ipl = bgr_image;
    IplImage* lab_image = cvCloneImage(&bgr_image_ipl);
    IplImage* out_image = cvCloneImage(&bgr_image_ipl);
    // slic
    cvCvtColor(&bgr_image_ipl, lab_image, CV_BGR2Lab);
    int w = image->width, h = image->height;
    int nr_superpixels = 200;
    int nc = 4;
    double step = sqrt((w * h) / (double) nr_superpixels);
    Slic slic;
    slic.generate_superpixels(lab_image, step, nc);
    slic.create_connectivity(lab_image);
    slic.display_contours(out_image, CV_RGB(255,0,0));
    cv::Mat debug_image = cv::cvarrToMat(out_image);
    pub_debug_.publish(cv_bridge::CvImage(
                         image->header,
                         sensor_msgs::image_encodings::BGR8,
                         debug_image).toImageMsg());
  }

}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS (jsk_perception::SLICSuperPixels, nodelet::Nodelet);
