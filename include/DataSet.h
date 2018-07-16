#pragma once

#include "AbstractDataSet.h"
#include "Object.h"
#include "PropList.h"

namespace HDF5 {

class DataSet : public Object, public AbstractDataSet {
 public:
  /// Default constructor.
  DataSet() = default;

  /// Construct from existing DataSet id.
  DataSet(hid_t dset_id) : Object(dset_id) {}

  /// Try to close DataSet.
  ~DataSet() { destruct(); }

  /// Close dataset.
  void close() override {
    if (H5Dclose(get_id()) < 0) throw Exception("DataSet::close");
    invalidate();
  }

  DataType get_datatype() const override { return DataType(H5Dget_type(get_id())); }

  DataSpace get_dataspace() const override {
    return DataSpace(H5Dget_space(get_id()));
  }

  /// Write dataset data.
  template <typename T>
  DataSet &write(
      const T *buf, const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) {
    return write(buf, PredType::get<T>(), mem_space, file_space, xfer_plist);
  }

  /// Write from reference.
  ///
  /// Note that the function is disabled if T is a pointer (in that case the
  /// write(const T *buf) function should be called instead).
  template <typename T>
  typename std::enable_if<!std::is_pointer<T>::value, DataSet &>::type
  write(
      const T &buf, const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) {
    return write(&buf, mem_space, file_space, xfer_plist);
  }

  /// Write data from std::vector.
  template <typename T>
  DataSet &write(
      const std::vector<T> &buf, const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) {
    return write(buf.data(), PredType::get<T>(), mem_space, file_space,
                 xfer_plist);
  }

  /// Write std::string.
  DataSet &write(
      const std::string &buf, const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) {
    const char *c_str = buf.c_str();
    return write(&c_str, PredType::STRING_UTF8_VLEN(), mem_space, file_space,
                 xfer_plist);
  }

  DataSet &write(
      const void *buf, const DataType &mem_type,
      const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) {
    herr_t status = H5Dwrite(get_id(), mem_type.get_id(), mem_space.get_id(),
                             file_space.get_id(), xfer_plist.get_id(), buf);
    if (status < 0) throw Exception("DataSet::write");
    return *this;
  }

  /// Read dataset data.
  template <typename T>
  const DataSet &read(
      T *buf, const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) const {
    return read(buf, PredType::get<T>(), mem_space, file_space, xfer_plist);
  }

  /// Read into object reference.
  template <typename T>
  typename std::enable_if<!std::is_pointer<T>::value, const DataSet &>::type
  read(T &buf, const DataSpace &mem_space = DataSpace::ALL(),
       const DataSpace &file_space = DataSpace::ALL(),
       const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) const {
    return read(&buf, mem_space, file_space, xfer_plist);
  }

  /// Load data into std::vector.
  template <typename T>
  const DataSet &read(
      std::vector<T> &buf, const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) const {
    // Determine number of points from dataspace.
    // If mem_space is ALL, use the dataspace associated to this dataset.
    auto &space = (mem_space.get_id() == DataSpace::ALL().get_id())
                      ? get_dataspace()
                      : mem_space;
    size_t N = space.get_select_npoints();
    buf.resize(N);
    return read(buf.data(), PredType::get<T>(), mem_space, file_space,
                xfer_plist);
  }

  /// Load data into std::string.
  const DataSet &read(
      std::string &buf, const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) const;

  const DataSet &read(
      void *buf, const DataType &mem_type,
      const DataSpace &mem_space = DataSpace::ALL(),
      const DataSpace &file_space = DataSpace::ALL(),
      const PropList::DSetXfer &xfer_plist = PropList::DSetXfer::DEFAULT()) const {
    herr_t status = H5Dread(get_id(), mem_type.get_id(), mem_space.get_id(),
                            file_space.get_id(), xfer_plist.get_id(), buf);
    if (status < 0) throw Exception("DataSet::read");
    return *this;
  }

  /// Get copy of dataset creation property list.
  PropList::DSetCreat get_create_plist() const {
    hid_t id = H5Dget_create_plist(get_id());
    if (id < 0) throw Exception("DataSet::get_create_plist");
    return id;
  }
};

}  // namespace HDF5

// Function implementation.
namespace HDF5 {

inline const DataSet &DataSet::read(std::string &buf,
                                    const DataSpace &mem_space,
                                    const DataSpace &file_space,
                                    const PropList::DSetXfer &xfer_plist) const {
  auto dtype = get_datatype();
  if (H5Tis_variable_str(dtype.get_id())) {
    // Load to C-style string, then create copy. Note that HDF5 allocates the
    // required data, and we need to free it manually later with
    // H5free_memory.
    char *s;
    read(&s, dtype, mem_space, file_space, xfer_plist);
    buf = s;
    H5free_memory(s);
  } else {
    size_t N = dtype.get_size();
    std::vector<char> s(N + 1);
    read(s.data(), dtype, mem_space, file_space, xfer_plist);
    buf = s.data();
  }
  return *this;
}

}  // namespace HDF5
