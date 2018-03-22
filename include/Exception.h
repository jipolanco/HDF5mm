#pragma once

// Exception handling for HDF5 errors.

#include <hdf5.h>

#include <iostream>
#include <stdexcept>

namespace HDF5 {

/// HDF5 exception.
class Exception : public std::runtime_error {
 public:
  /// Create an exception with the name of the function.
  Exception(const std::string &func_name, const std::string &what = "")
      : std::runtime_error("Error in function '" + func_name + "'" +
                           (what.empty() ? "." : ":\n" + what)) {}
};

}  // namespace HDF5
