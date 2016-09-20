#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "VPDetector_RCSCfMIoMW_Tools_test"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vp_detectors/RCSCfMIoMW/Tools.hpp>

using namespace vanishing_point;

cv::Point2f lineIntersection(cv::Point3f line_initial,
                                                    cv::Point3f line_final) {

  cv::Mat1f mat_lines = cv::Mat1f::ones(2, 3);
  mat_lines.row(0) = cv::Mat1f(line_initial).t();
  mat_lines.row(1) = cv::Mat1f(line_final).t();
  // std::cout << "Mat with lines" << mat_lines << std::endl;

  cv::Mat1f temp_point;
  cv::SVD::solveZ(mat_lines, temp_point);
  // std::cout << "Point raw" << temp_point.t() << std::endl;

  cv::Point2f intersection_point(nanf("There is no intersection"),
                                 nanf("There is no intersection"));
  float epslon = 1e-8;
  if (fabs(temp_point[2][0]) > epslon) {
    intersection_point.x = temp_point[0][0] / temp_point[2][0];
    intersection_point.y = temp_point[1][0] / temp_point[2][0];
  }

  return intersection_point;
}

cv::Point2f checkVPTriangle(cv::Point2f vp1,
                            cv::Point2f vp2,
                            cv::Point2f center){
  double alpha1 = -1/((vp1.y - center.y)/(vp1.x - center.x));
  double alpha2 = -1/((vp2.y - center.y)/(vp2.x - center.x));

  double const1 = vp2.y - vp2.x*alpha1;
  double const2 = vp1.y - vp1.x*alpha2;

  cv::Point3f line1(alpha1, -1, const1);
  cv::Point3f line2(alpha2, -1, const2);

  return lineIntersection(line1, line2);
}

BOOST_AUTO_TEST_CASE(centerPoint_testCase){

  std::vector<cv::Vec4f> line_segments = {cv::Vec4f(0,0,10,10),
    cv::Vec4f(-2,-2,10,10), cv::Vec4f(5,5,10,10), cv::Vec4f(5,5,-1,-1)};

  std::vector<cv::Point2f> gt_centers = {cv::Point2f(5,5), cv::Point2f(4,4),
    cv::Point2f(7.5,7.5), cv::Point2f(2,2)};

  for (uint i = 0; i < line_segments.size(); i++) {
    cv::Point2f center = lineSegmentCenterPoint(line_segments[i]);
    BOOST_CHECK_CLOSE(center.x, gt_centers[i].x, 0.01);
    BOOST_CHECK_CLOSE(center.y, gt_centers[i].y, 0.01);
    // std::cout << i << " Center "<< center << std::endl;
  }

}

BOOST_AUTO_TEST_CASE(distancePoint2Line_testCase){

  std::vector<cv::Point3f> lines = {cv::Point3f(2.0, -3, -4.0),
      cv::Point3f(6.0, -5.0, 10.0), cv::Point3f(3.0, 4.0, 0),
      cv::Point3f(3.0, -4.0, -25.0)};

  std::vector<cv::Point2f> points = {cv::Point2f(5,6), cv::Point2f(-3,7),
    cv::Point2f(2,-1), cv::Point2f(0,0)};
  std::vector<double> gt_distance = {3.328, 5.506, 0.400, 5.0};

  for (uint i = 0; i < lines.size(); i++) {
    double distance = distancePoint2Line(lines[i],points[i]);
    BOOST_CHECK_CLOSE(distance, gt_distance[i], 0.01);
    // Debug
    //std::cout << i << " distance "<< distance << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(errorLineSegmentPoint2VP_testCase){

  std::vector<double> gt_error = {10, 5, 6, 100, 3, 9};

  cv::Point2f center(250, 250);
  std::vector<cv::Vec4f> line_segments = {
    cv::Vec4f(center.x+10,center.y, center.x-10,center.y),
    cv::Vec4f(center.x+5,center.y+5, center.x-5,center.y-5),
    cv::Vec4f(center.x+6,center.y+100, center.x-6,center.y-100),
    cv::Vec4f(center.x,center.y+100, center.x,center.y-100),
    cv::Vec4f(center.x+6,center.y+3, center.x-6,center.y-3),
    cv::Vec4f(center.x+20,center.y+9, center.x-20,center.y-9)};

  std::vector<cv::Point3f> homogeneos_vp = {
    cv::Point3f(250*0.5, 2*0.5, 0.4999), cv::Point3f(250*0.5, 2*0.5, 0.49999),
    cv::Point3f(250*0.5, 2*0.5, 0.499999), cv::Point3f(2*0.3, 250*0.3, 0.299),
    cv::Point3f(2*0.3, 250*0.3, 0.299), cv::Point3f(2*0.3, 250*0.3, 0.299)};
  for (uint i = 0; i < line_segments.size(); i++) {
    double error = errorLineSegmentPoint2VP(line_segments[i], homogeneos_vp[i]);
    BOOST_CHECK_CLOSE(error, gt_error[i], 0.01);
    // std::cout<<" I "<<i<< " error "<< error <<std::endl;
  }
}


BOOST_AUTO_TEST_CASE(estimationVPby4LinesCase1_testCase){

  cv::Mat3b image = cv::Mat3b::zeros(500,500);
  cv::Point2f center_point(image.cols/2, image.rows/2);

  std::vector< std::vector<cv::Vec4f> > set4_segments(2);
  set4_segments[0]= {
              cv::Vec4f(0.6, 0.1, 0.7, 0.1), cv::Vec4f(0.55, 0.2, 0.55, 0.3),
              cv::Vec4f(0.7, 0.4, 0.8, 0.5), cv::Vec4f(0.8, 0.7, 0.7, 0.8)};

  set4_segments[1]= {
              cv::Vec4f(0.3, 0.0, 0.4, 0.0), cv::Vec4f(0.3, 0.2, 0.4, 0.1),
              cv::Vec4f(0.1, 0.7, 0.1, 0.8), cv::Vec4f(0.2, 0.9, 0.3, 0.9)};

  for (uint k = 0; k < set4_segments.size(); k++) {
    std::vector<cv::Vec4f> line_segments = set4_segments[k];

    for (uint i = 0; i < line_segments.size(); i++) {
      cv::Mat temp_mat = cv::Mat(line_segments).row(i);
      for (uint j = 0; j < 2; j++) {
        cv::Mat1f local_mat(temp_mat);
        local_mat[0][j*2] = local_mat[0][j*2]*image.cols - center_point.x;
        local_mat[0][j*2+1] = local_mat[0][j*2+1]*image.rows - center_point.y;
      }
    }

    std::vector<cv::Point2f> vps = estimationVPby4LinesCase1(line_segments);

    for (uint i = 0; i < line_segments.size(); i++) {
      cv::Point2f point1(line_segments[i][0],line_segments[i][1]);
      cv::Point2f point2(line_segments[i][2],line_segments[i][3]);
      cv::line( image, point1 + center_point, point2 + center_point,
                cv::Scalar(0, 255, 255), image.rows * 0.002);
    }

    cv::Point2f vp3 = checkVPTriangle(vps[0], vps[1], cv::Point2f(0,0));

    BOOST_CHECK_CLOSE(vp3.x, vps[2].x, 0.001);
    BOOST_CHECK_CLOSE(vp3.y, vps[2].y, 0.001);

    // visual debug
    // std::cout << "VPS FINAL " <<  cv::Mat(vps) << std::endl;
    // std::cout << "VP Check " << vp3 << std::endl;
    //
    // cv::circle( image, center_point, image.rows * 0.005,
    //             cv::Scalar(255,255,255), -1);
    // for (uint i = 0; i < vps.size(); i++) {
    //   cv::circle( image, vps[i] +center_point, image.rows * 0.005,
    //               cv::Scalar(0,255,0), -1);
    // }
    //
    // cv::imshow("out", image);
    // cv::waitKey();
    // image = cv::Mat3b::zeros(500,500);
  }
}
