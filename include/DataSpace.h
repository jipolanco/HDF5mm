#pragma once

#include "IdComponent.h"

#include <array>
#include <initializer_list>
#include <vector>

namespace HDF5 {

/// Type describing dataspace dimensions.
/// TODO define inside DataSpace
using dims_t = std::vector<hsize_t>;

/// Similar to dims_t, but based on a static-size array.
template <size_t N>
using adims_t = std::array<hsize_t, N>;

/// HDF5 dataspace.
class DataSpace : public IdComponent {
 protected:
  /// Close DataSpace.
  void close() override {
    if (get_type(id) != H5I_DATASPACE) return;
    if (H5Sclose(id) < 0) throw Exception("DataSpace::close");
  }

 public:
  /// Create scalar dataspace (`H5S_SCALAR`).
  DataSpace() : IdComponent(H5Screate(H5S_SCALAR)) {}

  /// Create simple dataspace (`H5S_SIMPLE`).
  DataSpace(int rank, const hsize_t *dims)
      : IdComponent(H5Screate_simple(rank, dims, nullptr)) {}

  /// Create simple dataspace.
  DataSpace(const dims_t &dims) : DataSpace(dims.size(), dims.data()) {}

  template <size_t N>
  DataSpace(const adims_t<N> &dims) : DataSpace(dims.size(), dims.data()) {}

  /// Create simple dataspace from initialiser list
  /// (e.g. {42, 4, 5} -> 3D dataspace).
  DataSpace(std::initializer_list<hsize_t> l)
      : DataSpace(l.size(), l.begin()) {}

  /// Create from existing DataSpace id.
  DataSpace(hid_t space_id) : IdComponent(space_id) {}

  /// Create from dataspace class (`H5S_SCALAR`, `H5S_SIMPLE` or `H5S_NULL`).
  DataSpace(H5S_class_t type) : DataSpace(H5Screate(type)) {}

  /// Try to close DataSpace.
  ~DataSpace() {
    try {
      close();
    } catch (Exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  /// Select the entire dataspace.
  DataSpace &select_all() {
    H5Sselect_all(id);
    return *this;
  }

  /// Resets the selection to include no elements.
  DataSpace &select_none() {
    H5Sselect_none(id);
    return *this;
  }

  /// Describes N-dimensional hyperslab of a dataset.
  template <size_t N>
  struct Hyperslab {
    adims_t<N> start;
    adims_t<N> stride;
    adims_t<N> count;
    adims_t<N> block;

    /// Intiialise hyperslab for dataset of Ndim dimensions with default values.
    Hyperslab() {
      start.fill(0);
      stride.fill(1);
      count.fill(1);
      block.fill(1);
    }
  };

  /// Select hyperslab using Hyperslab object.
  template <size_t N>
  DataSpace &select_hyperslab(const Hyperslab<N> &h,
                              H5S_seloper_t op = H5S_SELECT_SET) {
    H5Sselect_hyperslab(this->id, op, h.start.data(), h.stride.data(),
                        h.count.data(), h.block.data());
    return *this;
  }

  /// Select hyperslab using optional array pointers.
  DataSpace &select_hyperslab(H5S_seloper_t op, const hsize_t *count,
                              const hsize_t *start,
                              const hsize_t *stride = nullptr,
                              const hsize_t *block = nullptr) {
    H5Sselect_hyperslab(this->id, op, start, stride, count, block);
    return *this;
  }

  /// Returns the number of elements in the current selection.
  hssize_t get_select_npoints() const {
    auto N = H5Sget_select_npoints(id);
    if (N < 0) throw Exception("DataSpace::get_select_npoints");
    return N;
  }

  /// Returns number of dimensions of the dataspace (like Julia's `ndims`).
  int ndims() const { return H5Sget_simple_extent_ndims(id); }

  /// Returns dimensions of the dataspace (like Julia's `size`).
  dims_t size() const {
    dims_t dims(ndims());
    H5Sget_simple_extent_dims(id, dims.data(), nullptr);
    return dims;
  }

  /// Returns size of the dataspace along a single dimension.
  hsize_t size(int i) const {
    if (i < 0 || i >= ndims())
      throw Exception("DataSet::size(int)", "Invalid dimension index.");
    return size()[i];
  }

  /// Returns size of the dataspace along a single dimension.
  hsize_t operator[](int i) const { return size(i); }

  /// Returns number of elements of the dataspace (like Julia's `length`).
  hssize_t length() const { return H5Sget_simple_extent_npoints(id); }

  /// DataSpace describing the selection of a complete dataspace.
  static const DataSpace &ALL() {
    static DataSpace ALL(H5S_ALL);
    return ALL;
  }

  /// Infer dataspace from object instance.
  template <typename T>
  static DataSpace from(const T &) {
    return DataSpace();  // scalar dataspace
  }

  template <typename T>
  static DataSpace from(const std::vector<T> &x) {
    hsize_t N = x.size();
    return DataSpace(1, &N);  // 1D simple dataspace
  }
};

}  // namespace HDF5
