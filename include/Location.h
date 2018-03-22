#pragma once

#include "IdComponent.h"

namespace HDF5 {

/// HDF5 location.
class Location : public IdComponent {
 public:
  /// Check if a link (can be a group) exists under the current location.
  bool exists(const char *path) const {
    return H5Lexists(id, path, H5P_DEFAULT);
  }

  bool exists(const std::string &path) const { return exists(path.c_str()); }

  /// Check if a given path corresponds to a group.
  /// If the path doesn't exist, return false.
  bool is_group(const char *path) const {
    if (!exists(path)) return false;
    // Try to open it as a generic object and check its type.
    hid_t obj_id = H5Oopen(id, path, H5P_DEFAULT);
    if (obj_id < 0) throw Exception("Location::is_group");
    bool g = get_type(obj_id) == H5I_GROUP;
    H5Oclose(obj_id);
    return g;
  }

  bool is_group(const std::string &path) const {
    return is_group(path.c_str());
  }

 protected:
  /// Construct from existing location id.
  Location(hid_t loc_id) : IdComponent(loc_id) {}
};

}  // namespace HDF5
