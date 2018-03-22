#pragma once

#include "Location.h"
#include "DataSpace.h"

#include <vector>

namespace HDF5 {

class Attribute;
class DataType;
class Group;

/// HDF5 object: DataSet, DataType or Group.
/// Attributes can be attached to the object.
class Object : public Location {
 protected:
  /// Construct from existing location id.
  Object(hid_t obj_id) : Location(obj_id) {}

 public:
  /// Create attribute. By default, a simple (scalar) dataspace is used.
  Attribute create_attribute(const char *name, const DataType &type,
                             const DataSpace &space = DataSpace());

  Attribute create_attribute(const std::string &name, const DataType &type,
                             const DataSpace &space = DataSpace());

  /// Open existing attribute.
  Attribute open_attribute(const char *name) const;

  Attribute open_attribute(const std::string &name) const;

  /// Create and write attribute (higher-level function).
  template <typename T>
  Attribute write_attribute(const T &val, const std::string &name,
                            const DataSpace &space);

  /// Create and write attribute (higher-level function).
  /// The dataspace is inferred from the input data.
  template <typename T>
  Attribute write_attribute(const T &val, const std::string &name);

  /// Open and read attribute (higher-level function).
  template <typename T>
  T read_attribute(const std::string &name) const;

  /// Get object name.
  std::string name() const {
    ssize_t size = H5Iget_name(this->id, nullptr, 0);
    if (size < 0)
      throw Exception("Object::name", "First call to H5Iget_name");
    std::vector<char> name(size + 1);
    if (H5Iget_name(this->id, name.data(), name.size()) < 0)
      throw Exception("Object::name", "Second call to H5Iget_name");
    return std::string(name.data());
  }

  /// Open parent object.
  Group parent() const;

  /// Get object information.
  H5O_info_t get_obj_info() const {
    H5O_info_t info;
    H5Oget_info(id, &info);
    return info;
  }
};

}  // namespace HDF5
