#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>

cv::Mat3b process_image(cv::Mat3b & in,
                        unsigned int cell_size = 10,
                        unsigned int cell_thickness = 1,
                        float circle_ratio = 1./6,
                        cv::Scalar grid_color = cv::Scalar::all(100)) {
  unsigned int cols = in.cols, rows = in.rows;
  printf("process_image(%ix%i)\n", cols, rows);
  cv::Mat3b out;
  out.create(cell_size * rows, cell_size * cols);
  out.setTo(cv::Vec3b(255, 255, 255));
  // draw grid
  for (int row = 0; row < rows; ++row)
    cv::line(out, cv::Point(0, row * cell_size),
             cv::Point(cols * cell_size, row * cell_size),
             grid_color, cell_thickness);
  for (int col = 0; col < cols; ++col)
    cv::line(out, cv::Point(col * cell_size, 0),
             cv::Point(col * cell_size, rows * cell_size),
             grid_color, cell_thickness);

  unsigned int circle_radius = circle_ratio * cell_size;
  for (int row = 0; row < rows; ++row) {
    int y = row * cell_size + cell_size/2;
    for (int col = 0; col < cols; ++col) {
      int x = col * cell_size + cell_size/2;
      cv::Vec3b color = in.at<cv::Vec3b>(row, col);
      cv::circle(out, cv::Point(x, y), circle_radius,
                 cv::Scalar(color[0], color[1], color[2]), -1);
    } // end for col
  } // end for row

  // rotate 90° if needed
  if (out.cols > out.rows) {
    cv::transpose(out, out);
    cv::flip(out, out, 0);
  }
  return out;
} // end process_image()

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Synopsis: %s FILENAME\n", argv[0]);
    return -1;
  }
  std::string filename(argv[1]);
  cv::Mat3b in = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
  cv::Mat3b out = process_image(in);
  cv::imshow("out", out);
  std::string filename_out = "/tmp/fill2paint.png";
  cv::imwrite(filename_out, out);
  printf("Saved file '%s'\n", filename_out.c_str());
  cv::imshow("fill2paint", out);
  cv::waitKey(0);
  return 0;
}
