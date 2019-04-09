#include <fastcdr/FastCdr.h>
#include <Eigen/Dense>
#include <iostream>

template<typename _Scalar, int _Rows, int _Cols>
eprosima::fastcdr::FastCdr& operator<<(eprosima::fastcdr::FastCdr& ser, const Eigen::Matrix<_Scalar, _Rows, _Cols>& mat)
{
  const auto r = mat.rows();
  const auto c = mat.cols();
  ser << static_cast<uint32_t>(r) << static_cast<uint32_t>(c) << static_cast<uint32_t>(sizeof(_Scalar));
  ser.serializeArray(mat.data(), r * c);
  return ser;
}

template<typename _Scalar, int _Rows, int _Cols>
eprosima::fastcdr::FastCdr& operator>>(eprosima::fastcdr::FastCdr& ser, Eigen::Matrix<_Scalar, _Rows, _Cols>& mat)
{
  uint32_t r{};
  uint32_t c{};
  uint32_t s{};

  ser >> r >> c >> s;
  if((r != mat.rows()) || (c != mat.cols()) || (s != sizeof(_Scalar)))
  {
    std::ostringstream str;
    str << "Cannot deserialise " << r << "x" << c << "x[" << s << " bytes] into " << mat.rows() << "x" << mat.cols() << "x[" << sizeof(_Scalar) << " bytes]";
    throw std::runtime_error(str.str());
  }
  ser.deserializeArray(mat.data(), mat.rows() * mat.cols());
  return ser;
}

int main(int argc, const char* argv[])
{
  try {

  (void)argc;
  (void)argv;

  Eigen::MatrixXd d1(2,2); //(what about the scalar type definition?)
  d1 << 1.3,2.4,3.6,4.8;

  // serialise the first object
  eprosima::fastcdr::FastBuffer bfr;
  eprosima::fastcdr::FastCdr ser(bfr);
  ser << d1;

  Eigen::Matrix2d d2;
  // take the serial buffer and deserialise into second object
  eprosima::fastcdr::FastBuffer serialisedData(bfr.getBuffer(), bfr.getBufferSize());
  eprosima::fastcdr::FastCdr des(serialisedData);
  des >> d2;

  std::cout << d2 << std::endl;
  } catch (const std::exception& ex) {
    std::cout << ex.what() << "\n";
  }

  return 0;
}
