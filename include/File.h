#pragma once

#include "Group.h"
#include "PropList.h"

namespace HDF5 {

class File : public Group {
 protected:
  /// Close file.
  virtual void close() override {
    if (H5Fclose(id) < 0) throw Exception("File::close");
  }

 public:
  /// Construct from existing file id.
  File(hid_t file_id) : Group(file_id) {}

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
    hid_t id;
    if (flags & (H5F_ACC_TRUNC | H5F_ACC_EXCL | H5F_ACC_CREAT)) {
      id = H5Fcreate(name, flags, H5P_DEFAULT, fapl.id);
      if (id < 0)
        throw Exception("File::_open_or_create", "Error creating new file.");
    } else {
      id = H5Fopen(name, flags, fapl.id);
      if (id < 0)
        throw Exception("File::_open_or_create",
                        "Error opening existing file.");
    }
    return id;
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

inline File IdComponent::get_file() const {
  hid_t id = H5Iget_file_id(this->id);
  if (id < 0) throw Exception("IdComponent::get_file");
  return id;
}

// Note: this is defined here because it calls get_file().
inline Group Object::parent() const {
  std::string name = this->name();
  size_t n = name.rfind('/');  // find last occurence of '/'
  File ff = get_file();
  if (n == 0) return ff.open_group("/");
  return ff.open_group(name.substr(0, n));
}

}  // namespace HDF5
