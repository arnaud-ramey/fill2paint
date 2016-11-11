#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <map>

enum Mode {
  IMAGE2CIRCLES = 0,
  IMAGE2NUMBERS = 1,
};

template<typename _Tp>
struct compare_colors : public std::binary_function<_Tp, _Tp, bool> {
  bool operator()(const _Tp& x, const _Tp& y) const {
    return x[0] < y[0]
        || (x[0] == y[0] && x[1] < y[1])
        || (x[0] == y[0] && x[1] == y[1] && x[2] < y[2]);
  }
};

cv::Mat3b process_image(cv::Mat3b & in,
                        Mode mode = IMAGE2CIRCLES,
                        unsigned int cell_size = 10,
                        unsigned int cell_thickness = 1,
                        float circle_ratio = 1./6,
                        cv::Scalar grid_color = cv::Scalar::all(100)) {
  unsigned int cols = in.cols, rows = in.rows;
  printf("process_image(%ix%i)\n", cols, rows);
  std::map<cv::Vec3b, std::string, compare_colors<cv::Vec3b> > color2label;
  if (mode == IMAGE2NUMBERS) {
    for (unsigned int row = 0; row < rows; ++row) {
      for (unsigned int col = 0; col < cols; ++col) {
        cv::Vec3b color = in.at<cv::Vec3b>(row, col);
        if (color2label.find(color) == color2label.end()) {
          std::ostringstream label;
          label << color2label.size() + 1;
          color2label[color] = label.str();
        }
      } // end for col
    } // end for row
  } // end if if (mode == IMAGE2NUMBERS)

  cv::Mat3b out;
  out.create(cell_size * rows, cell_size * cols);
  out.setTo(cv::Vec3b(255, 255, 255));
  // draw grid
  for (unsigned int row = 0; row < rows; ++row)
    cv::line(out, cv::Point(0, row * cell_size),
             cv::Point(cols * cell_size, row * cell_size),
             grid_color, cell_thickness);
  for (unsigned int col = 0; col < cols; ++col)
    cv::line(out, cv::Point(col * cell_size, 0),
             cv::Point(col * cell_size, rows * cell_size),
             grid_color, cell_thickness);

  unsigned int circle_radius = circle_ratio * cell_size;
  for (unsigned int row = 0; row < rows; ++row) {
    int down = (row+1) * cell_size, ycenter = down - cell_size/2;
    for (unsigned int col = 0; col < cols; ++col) {
      int left = col * cell_size, xcenter = left + cell_size/2;
      cv::Vec3b color = in.at<cv::Vec3b>(row, col);
      if (mode == IMAGE2CIRCLES)
        cv::circle(out, cv::Point(xcenter, ycenter), circle_radius,
                   cv::Scalar(color[0], color[1], color[2]), -1);
      else if (mode == IMAGE2NUMBERS) {
        std::string label = color2label[color];
        cv::putText(out, label, cv::Point(left+2, down-2), CV_FONT_HERSHEY_PLAIN,
                    1, grid_color);
      }
    } // end for col
  } // end for row


  // add caption
  if (mode == IMAGE2NUMBERS) {
    unsigned int ncolors = color2label.size();
    printf("%i colors\n", ncolors);
    unsigned int caption_cell_width = 50;
    cv::Mat3b caption;
    caption.create(caption_cell_width, ncolors * caption_cell_width);
    std::map<cv::Vec3b, std::string>::const_iterator color_it = color2label.begin();
    for (unsigned int i = 0; i < ncolors; ++i) {
      cv::Rect roi(i * caption_cell_width, 0, caption_cell_width, caption_cell_width);
      caption(roi).setTo(color_it->first);
      cv::Point text_pos(i * caption_cell_width + caption_cell_width / 3,
                         2 * caption_cell_width / 3);
      cv::putText(caption, color_it->second, text_pos, CV_FONT_HERSHEY_PLAIN, 2,
                  CV_RGB(255,255,255), 2);
      cv::putText(caption, color_it->second, text_pos, CV_FONT_HERSHEY_PLAIN, 2,
                  CV_RGB(0,0,0), 1);
      ++color_it;
    }
    cv::Mat3b pasted(out.rows + caption.rows, cv::max(out.cols, caption.cols),
                     cv::Vec3b(255,255,255));
    cv::Mat3b pasted_roi = pasted(cv::Rect(0, 0, caption.cols, caption.rows));
    caption.copyTo(pasted_roi);
    out.copyTo(pasted(cv::Rect(0, caption.rows, out.cols, out.rows)));
    pasted.copyTo(out);
  } // end if (mode == IMAGE2NUMBERS)

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

  cv::Mat3b out_circles = process_image(in, IMAGE2CIRCLES, 12);
  std::string filename_out_circles = "/tmp/fill2paint_circles.png";
  cv::imwrite(filename_out_circles, out_circles);
  printf("Saved file '%s'\n", filename_out_circles.c_str());
  cv::imshow("fill2paint_circles", out_circles);

  cv::Mat3b out_numbers = process_image(in, IMAGE2NUMBERS, 15);
  std::string filename_out_numbers = "/tmp/fill2paint_numbers.png";
  cv::imwrite(filename_out_numbers, out_numbers);
  printf("Saved file '%s'\n", filename_out_numbers.c_str());
  cv::imshow("fill2paint_numbers", out_numbers);

  cv::waitKey(0);
  return 0;
}
