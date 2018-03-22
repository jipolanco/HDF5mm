#pragma once

#include "Group.h"
#include "PropList.h"

namespace HDF5 {

class File : public Group {
 public:
  /// Open or create HDF5 file.
  /// By default, files are open in read-only mode.
  File(const char *name, unsigned flags = H5F_ACC_RDONLY,
       const PropList::FileAcc &fapl = PropList::FileAcc::DEFAULT())
      : Group(_open_or_create(name, flags, fapl)) {}

  File(const std::string &name, unsigned flags = H5F_ACC_RDONLY,
       const PropList::FileAcc &fapl = PropList::FileAcc::DEFAULT())
      : File(name.c_str(), flags, fapl) {}

  /// Open or create HDF5 file.
  ///
  /// Flags string can either be "r" (read-only), "r+" (read-write,
  /// preserving existing content) or "w" (read-write, destroying existing
  /// files).
  File(const char *name, const std::string &flags,
       const PropList::FileAcc &fapl = PropList::FileAcc::DEFAULT())
      : File(name, _str_to_flags(flags), fapl) {}

  File(const std::string &name, const std::string &flags,
       const PropList::FileAcc &fapl = PropList::FileAcc::DEFAULT())
      : File(name.c_str(), flags, fapl) {}

  /// Close file.
  virtual void close() override {
    if (H5Fclose(id) < 0) throw Exception("File::close");
  }

  ~File() {
    try {
      close();
    } catch (Exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  /// Determine whether a file exists and is a HDF5 file.
  static bool is_hdf5(const char *filename) {
    return H5Fis_hdf5(filename) > 0;
  }

  static bool is_hdf5(const std::string &filename) {
    return is_hdf5(filename.c_str());
  }

  /// Get number of open object identifiers in this file.
  ssize_t get_obj_count(unsigned int types = H5F_OBJ_ALL) const {
    return H5Fget_obj_count(id, types);
  }

 private:
  /// Open or create HDF5 file, according to the given flags.
  /// Returns id of file object.
  static hid_t _open_or_create(const char *name, unsigned flags,
                               const PropList::FileAcc &fapl) {
    if (flags & (H5F_ACC_TRUNC | H5F_ACC_EXCL | H5F_ACC_CREAT))
      return H5Fcreate(name, flags, H5P_DEFAULT, fapl.id);
    else
      return H5Fopen(name, flags, fapl.id);
  }

  /// Converts a string to the equivalent file access flags.
  /// Strings accepted: "r" (read-only), "r+" (read-write, preserving existing
  /// content), "w" (read-write, destroying existing files).
  static unsigned _str_to_flags(const std::string &flags_str) {
    if (flags_str == "r") return H5F_ACC_RDONLY;
    if (flags_str == "r+") return H5F_ACC_RDWR;
    if (flags_str == "w") return H5F_ACC_TRUNC;
    throw Exception("File::_str_to_flags", "Invalid access flag: " + flags_str);
    return 0;
  }
};

}  // namespace HDF5
