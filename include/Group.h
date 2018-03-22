#pragma once

#include "DataSet.h"

namespace HDF5 {

/// HDF5 group.
class Group : public Object {
 public:
  /// Construct from existing group id.
  Group(hid_t group_id) : Object(group_id) {}

  /// Close group.
  virtual void close() override {
    if (get_type(id) != H5I_GROUP) return;
    if (H5Gclose(id) < 0) throw Exception("Group::close");
  }

  virtual ~Group() {
    try {
      close();
    } catch (Exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  /// Create new group under the current object.
  Group create_group(const char *name) {
    return Group(
        H5Gcreate2(this->id, name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
  }

  Group create_group(const std::string &name) {
    return create_group(name.c_str());
  }

  /// Open existing group under the current object.
  Group open_group(const char *name) {
    return Group(H5Gopen2(this->id, name, H5P_DEFAULT));
  }

  Group open_group(const std::string &name) {
    return open_group(name.c_str());
  }

  /// Create new dataset under the current object.
  /// By default, a simple (scalar) dataspace is used.
  DataSet create_dataset(
      const char *name, const DataType &type,
      const DataSpace &space = DataSpace(),
      const PropList::DSetCreat &plist = PropList::DSetCreat::DEFAULT()) {
    return DataSet(H5Dcreate2(this->id, name, type.id, space.id, H5P_DEFAULT,
                              plist.id, H5P_DEFAULT));
  }

  DataSet create_dataset(
      const std::string &name, const DataType &type,
      const DataSpace &space = DataSpace(),
      const PropList::DSetCreat &plist = PropList::DSetCreat::DEFAULT()) {
    return create_dataset(name.c_str(), type, space, plist);
  }

  /// Open existing dataset.
  DataSet open_dataset(const char *name) const {
    return H5Dopen2(this->id, name, H5P_DEFAULT);
  }

  DataSet open_dataset(const std::string &name) const {
    return open_dataset(name.c_str());
  }

  /// Create and write dataset (higher-level function).
  template <typename T>
  DataSet write_dataset(
      const T &val, const std::string &name, const DataSpace &space) {
    static_assert(!std::is_pointer<T>::value,
                  "Pointer types not yet supported.");
    return create_dataset(name, PredType::from(val), space).write(val);
  }

  /// Create and write dataset (higher-level function).
  /// The file dataspace is inferred from the input data.
  template <typename T>
  DataSet write_dataset(const T &val, const std::string &name) {
    return write_dataset(val, name, DataSpace::from(val));
  }

  /// Open and read dataset (higher-level function).
  template <typename T>
  T read_dataset(const std::string &name) const {
    static_assert(!std::is_pointer<T>::value,
                  "Pointer types not yet supported.");
    T val;
    open_dataset(name).read(val);
    return val;
  }

  /// Create soft link from this location.
  herr_t create_soft_link(const char *target_path,
                          const char *link_name) {
    return H5Lcreate_soft(target_path, this->id, link_name,
                          H5P_LINK_CREATE_DEFAULT, H5P_LINK_ACCESS_DEFAULT);
  }

  herr_t create_soft_link(const std::string &target_path,
                          const std::string &link_name) {
    return create_soft_link(target_path.c_str(), link_name.c_str());
  }
};

}  // namespace HDF5
