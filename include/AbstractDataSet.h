#pragma once

#include "DataType.h"
#include "DataSpace.h"

namespace HDF5 {

/// Pure virtual class containing functionality shared by DataSet and Attribute.
class AbstractDataSet {
 public:
  /// Get associated datatype.
  virtual DataType get_datatype() const = 0;

  /// Get associated dataspace.
  virtual DataSpace get_dataspace() const = 0;
};

}  // namespace HDF5
