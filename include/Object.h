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

  /// Check if attribute exists.
  bool has_attribute(const std::string &name) const {
    return H5Aexists(id, name.c_str());
  }

  /// Get object name.
  std::string name() const { return _get_name(&H5Iget_name); }

  /// Get name of file to which this object belongs.
  std::string filename() const { return _get_name(&H5Fget_name); }

  /// Open parent object.
  Group parent() const;

  /// Get object information.
  H5O_info_t get_obj_info() const {
    H5O_info_t info;
    H5Oget_info(id, &info);
    return info;
  }

 private:
  /// Calls a `H5?get_name` function (where `?` can be I or F, for object name
  /// or file name).
  std::string _get_name(ssize_t (*func)(hid_t, char *, size_t)) const {
    ssize_t size = func(this->id, nullptr, 0);
    if (size < 0)
      throw Exception("Object::_get_name", "First call to H5?get_name");
    std::vector<char> name(size + 1);
    if (func(this->id, name.data(), name.size()) < 0)
      throw Exception("Object::_get_name", "Second call to H5?get_name");
    return std::string(name.data());
  }
};

}  // namespace HDF5
