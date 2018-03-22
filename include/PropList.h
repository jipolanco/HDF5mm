#pragma once

#include "DataSpace.h"  // dims_t
#include "IdComponent.h"

namespace HDF5 {

namespace PropList {

/// HDF5 property list.
class PropList : public IdComponent {
 protected:
  /// Close property list.
  void close() override {
    if (!is_valid(id)) return;
    if (H5Pclose(id) < 0) throw Exception("PropList::close");
  }

 public:
  /// Disable default constructor.
  PropList() = delete;

  /// Copy existing property list using its id.
  PropList(hid_t plist_id) : IdComponent(H5Pcopy(plist_id)) {}

  virtual ~PropList() {
    try {
      close();
    } catch (Exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  /// Default property list (`H5P_DEFAULT`).
  static const PropList &DEFAULT() {
    static PropList plist(H5P_DEFAULT);
    return plist;
  }
};

/// File access property list.
class FileAcc : public PropList {
 public:
  /// Create empty property list.
  FileAcc() : PropList(H5Pcreate(H5P_FILE_ACCESS)) {}

  static const FileAcc &DEFAULT() {
    static FileAcc plist(H5P_FILE_ACCESS_DEFAULT);
    return plist;
  }

#ifdef H5_HAVE_PARALLEL
  /// Set MPI IO parameters for parallel I/O.
  FileAcc &set_mpio(MPI_Comm comm, MPI_Info info = MPI_INFO_NULL) {
    H5Pset_fapl_mpio(this->id, comm, info);
    return *this;
  }
#endif  // H5_HAVE_PARALLEL

 protected:
  FileAcc(hid_t plist_id) : PropList(plist_id) {}
};

/// Dataset transfer property list.
class DSetXfer : public PropList {
 public:
  /// Create empty property list.
  DSetXfer() : PropList(H5Pcreate(H5P_DATASET_XFER)) {}

  static const DSetXfer &DEFAULT() {
    return static_cast<const DSetXfer &>(PropList::DEFAULT());
  }

#ifdef H5_HAVE_PARALLEL
  /// Set MPI transfer mode.
  /// Valid transfer modes are `H5FD_MPIO_INDEPENDENT` (default) and
  /// `H5FD_MPIO_COLLECTIVE`.
  DSetXfer &set_mpio(H5FD_mpio_xfer_t xfer_mode) {
    H5Pset_dxpl_mpio(this->id, xfer_mode);
    return *this;
  }

  /// Set MPI transfer mode to collective.
  DSetXfer &set_mpio_collective() {
    return set_mpio(H5FD_MPIO_COLLECTIVE);
  }

  /// Set MPI transfer mode to independent.
  DSetXfer &set_mpio_independent() {
    return set_mpio(H5FD_MPIO_INDEPENDENT);
  }

  /// Checks the actual IO mode used in a dataset write operation.
  ///
  /// Possible return values are:
  ///
  ///     H5D_MPIO_NO_COLLECTIVE
  ///     H5D_MPIO_CHUNK_INDEPENDENT
  ///     H5D_MPIO_CHUNK_COLLECTIVE
  ///     H5D_MPIO_CHUNK_MIXED
  ///     H5D_MPIO_CONTIGUOUS_COLLECTIVE
  ///
  /// NOTE: requires HDF5 >= 1.8.8.
  ///
  H5D_mpio_actual_io_mode_t get_mpio_actual_io_mode() const {
    H5D_mpio_actual_io_mode_t mode;
    if (H5Pget_mpio_actual_io_mode(this->id, &mode) < 0)
      throw Exception("DSetXferPropList::H5D_mpio_actual_io_mode_t");
    return mode;
  }
#endif

};

/// Dataset creation property list.
class DSetCreat : public PropList {
 public:
  /// Copy existing property list using its id.
  DSetCreat(hid_t plist_id) : PropList(plist_id) {}

  /// Create empty property list.
  DSetCreat() : PropList(H5P_DATASET_CREATE_DEFAULT) {}

  /// Default property list (`H5P_DATASET_CREATE_DEFAULT`).
  static DSetCreat &DEFAULT() {
    static DSetCreat plist;
    return plist;
  }

  /// Set chunk size.
  DSetCreat &set_chunk(int ndims, const hsize_t *dim) {
    H5Pset_chunk(id, ndims, dim);
    return *this;
  }

  DSetCreat &set_chunk(const dims_t &dims) {
    return set_chunk(dims.size(), dims.data());
  }

  template <size_t N>
  DSetCreat &set_chunk(const adims_t<N> &dims) {
    return set_chunk(dims.size(), dims.data());
  }

  /// Set shuffle filter.
  DSetCreat &set_shuffle() {
    H5Pset_shuffle(id);
    return *this;
  }

  /// Set deflate (gzip) compression filter.
  /// Compression level must be between 0 and 9.
  DSetCreat &set_deflate(unsigned int level) {
    H5Pset_deflate(id, level);
    return *this;
  }
};

}  // namespace PropList
}  // namespace HDF5
