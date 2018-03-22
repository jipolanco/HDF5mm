#pragma once

#include "AbstractDataSet.h"
#include "Location.h"
#include "Object.h"

#include <type_traits>

namespace HDF5 {

/// HDF5 attribute.
class Attribute : public Location, public AbstractDataSet {
 protected:
  /// Close attribute (may throw exception).
  void close() override {
    if (H5Aclose(id) < 0) throw Exception("Attribute::close");
  }

 public:
  /// Construct from existing attribute id.
  Attribute(hid_t attr_id) : Location(attr_id) {}

  /// Get attribute name.
  std::string name() const {
    ssize_t size = H5Aget_name(this->id, 0, nullptr);
    if (size < 0)
      throw Exception("Attribute::name", "First call to H5Aget_name");
    std::vector<char> name(size + 1);
    if (H5Aget_name(this->id, name.size(), name.data()) < 0)
      throw Exception("Attribute::name", "Second call to H5Aget_name");
    return std::string(name.data());
  }

  /// Try to close attribute.
  ~Attribute() {
    try {
      close();
    } catch (Exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  DataType get_datatype() const override { return DataType(H5Aget_type(id)); }

  DataSpace get_dataspace() const override {
    return DataSpace(H5Aget_space(id));
  }

  /// Write attribute data.
  template <typename T>
  Attribute &write(const T *buf) {
    return write(buf, PredType::get<T>());
  }

  /// Write from reference.
  ///
  /// Note that the function is disabled if T is a pointer (in that case the
  /// write(const T *buf) function should be called instead).
  template <typename T>
  typename std::enable_if<!std::is_pointer<T>::value, Attribute &>::type
  write(const T &buf) {
    return write(&buf);
  }

  /// Write data from std::vector.
  template <typename T>
  Attribute &write(const std::vector<T> &buf) {
    return write(buf.data(), PredType::get<T>());
  }

  /// Write std::string.
  Attribute &write(const std::string &buf) {
    const char *c_str = buf.c_str();
    return write(&c_str, PredType::STRING_UTF8_VLEN());
  }

  Attribute &write(const void *buf, const DataType &mem_type) {
    if (H5Awrite(this->id, mem_type.id, buf) < 0)
      throw Exception("Attribute::write");
    return *this;
  }

  /// Read attribute data.
  template <typename T>
  const Attribute &read(T *buf) const {
    return read(buf, PredType::get<T>());
  }

  /// Read into object reference.
  template <typename T>
  typename std::enable_if<!std::is_pointer<T>::value, const Attribute &>::type
  read(T &buf) const {
    return read(&buf);
  }

  /// Load data into std::vector.
  template <typename T>
  const Attribute &read(std::vector<T> &buf) const {
    size_t N = get_dataspace().get_select_npoints();
    buf.resize(N);
    return read(buf.data(), PredType::get<T>());
  }

  /// Load data into std::string.
  const Attribute &read(std::string &buf) const;

  const Attribute &read(void *buf, const DataType &mem_type) const {
    if (H5Aread(this->id, mem_type.id, buf) < 0)
      throw Exception("Attribute::read");
    return *this;
  }
};

}  // namespace HDF5

// Function implementation.
namespace HDF5 {

inline const Attribute &Attribute::read(std::string &buf) const {
  auto dtype = get_datatype();
  if (H5Tis_variable_str(dtype.id)) {
    // Load to C-style string, then create copy. Note that HDF5 allocates the
    // required data, and we need to free it manually later with H5free_memory.
    char *s;
    read(&s, dtype);
    buf = s;
    H5free_memory(s);
  } else {
    size_t N = dtype.get_size();
    std::vector<char> s(N + 1);
    read(s.data(), dtype);
    buf = s.data();
  }
  return *this;
}

// Note: member functions from Object class are implemented here to avoid
// circular dependency issues.
inline Attribute Object::create_attribute(const char *name,
                                          const DataType &type,
                                          const DataSpace &space) {
  return Attribute(
      H5Acreate(this->id, name, type.id, space.id, H5P_DEFAULT, H5P_DEFAULT));
}

inline Attribute Object::create_attribute(const std::string &name,
                                          const DataType &type,
                                          const DataSpace &space) {
  return create_attribute(name.c_str(), type, space);
}

inline Attribute Object::open_attribute(const char *name) const {
  return Attribute(H5Aopen(this->id, name, H5P_DEFAULT));
}

inline Attribute Object::open_attribute(const std::string &name) const {
  return open_attribute(name.c_str());
}

template <typename T>
inline Attribute Object::write_attribute(const T &val, const std::string &name,
                                         const DataSpace &space) {
  static_assert(!std::is_pointer<T>::value, "Pointer types not yet supported.");
  return create_attribute(name, PredType::from(val), space).write(val);
}

template <typename T>
inline Attribute Object::write_attribute(const T &val,
                                         const std::string &name) {
  return write_attribute(val, name, DataSpace::from(val));
}

template <typename T>
inline T Object::read_attribute(const std::string &name) const {
  static_assert(!std::is_pointer<T>::value, "Pointer types not yet supported.");
  T val;
  open_attribute(name).read(val);
  return val;
}

}  // namespace HDF5
