#pragma once

#include "DataSet.h"

namespace HDF5 {

class File;

/// HDF5 group.
class Group : public Object {
 private:
  /// Disable constructing a Group from a File (which inherits from Group).
  ///
  /// This is disallowed because a File object should always be destructed
  /// through the File destructor (which calls `H5Fclose`). By casting a File
  /// into a Group, `H5Fclose` doesn't get called at destruction of the object,
  /// and therefore the file is not closed when it should.
  Group(const File &) = delete;

 protected:
  /// Close group.
  virtual void close() override {
    if (get_type(get_id()) != H5I_GROUP) return;
    if (H5Gclose(get_id()) < 0) throw Exception("Group::close");
  }

 public:
  /// Construct from existing group id.
  Group(hid_t group_id) : Object(group_id) {}

  virtual ~Group() {
    try {
      close();
    } catch (Exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  /// Create new group under the current object.
  Group create_group(const char *name) {
    hid_t id = H5Gcreate2(get_id(), name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (id < 0) throw Exception("Group::create_group");
    return id;
  }

  Group create_group(const std::string &name) {
    return create_group(name.c_str());
  }

  /// Recursively create groups in path if they don't exist.
  /// Returns handle to group `name`.
  Group create_groups(const std::string &name) {
    if (name.empty()) return *this;
    size_t n = name.rfind('/');  // find last occurence of '/'

    // If no '/' was found, create group `name` if it doesn't exist.
    if (n == name.npos)
      return exists(name) ? open_group(name) : create_group(name);

    // Recursively create groups before and after the `/`.
    return create_groups(name.substr(0, n)).create_groups(name.substr(n + 1));
  }

  /// Open existing group under the current object.
  Group open_group(const char *name) {
    hid_t id = H5Gopen2(get_id(), name, H5P_DEFAULT);
    if (id < 0) throw Exception("Group::open_group");
    return id;
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
    hid_t id = H5Dcreate2(get_id(), name, type.get_id(), space.get_id(),
                          H5P_DEFAULT, plist.get_id(), H5P_DEFAULT);
    if (id < 0) throw Exception("Group::create_dataset");
    return id;
  }

  DataSet create_dataset(
      const std::string &name, const DataType &type,
      const DataSpace &space = DataSpace(),
      const PropList::DSetCreat &plist = PropList::DSetCreat::DEFAULT()) {
    return create_dataset(name.c_str(), type, space, plist);
  }

  /// Open existing dataset.
  DataSet open_dataset(const char *name) const {
    hid_t id = H5Dopen2(get_id(), name, H5P_DEFAULT);
    if (id < 0) throw Exception("Group::open_dataset");
    return id;
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
  void create_soft_link(const char *target_path,
                          const char *link_name) {
    if (H5Lcreate_soft(target_path, get_id(), link_name,
                       H5P_LINK_CREATE_DEFAULT, H5P_LINK_ACCESS_DEFAULT) < 0)
      throw Exception("Group::create_soft_link");
  }

  void create_soft_link(const std::string &target_path,
                        const std::string &link_name) {
    create_soft_link(target_path.c_str(), link_name.c_str());
  }
};

}  // namespace HDF5
