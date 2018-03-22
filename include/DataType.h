#pragma once

#include "Object.h"

namespace HDF5 {

/// Abstract data type.
class DataType : public Object {
 public:
  DataType(hid_t type_id) : Object(type_id) {}

  /// Try to close datatype.
  virtual ~DataType() {
    try {
      close();
    } catch (Exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  /// Close datatype.
  virtual void close() override {
    if (get_type(id) != H5I_DATATYPE) return;
    if (H5Tclose(id) < 0) throw Exception("DataType::close");
  }

  /// Get size of datatype in bytes.
  std::size_t get_size() const { return H5Tget_size(id); }

  /// Compare to another datatype.
  bool operator==(const DataType &type) const { return H5Tequal(id, type.id); }
  bool operator!=(const DataType &type) const { return !(*this == type); }
};

/// Predefined data types.
class PredType : public DataType {
 protected:
  PredType(hid_t type_id) : DataType(type_id) {}

 public:
  /// Predefined data types.
  static const PredType &NATIVE_CHAR();
  static const PredType &NATIVE_INT();
  static const PredType &NATIVE_INT64();
  static const PredType &NATIVE_UINT64();
  static const PredType &NATIVE_DOUBLE();
  static const PredType &NATIVE_FLOAT();

  /// Variable-length string with UTF8 encoding.
  static const PredType &STRING_UTF8_VLEN();

  /// Get reference to a datatype associated to a C++ type.
  template <typename T>
  static const PredType &get();

  /// Get reference to a datatype from an object instance.
  template <typename T>
  static const PredType &from(const T &) {
    return get<T>();
  }

  /// Get reference to a datatype from a std::vector instance.
  template <typename T>
  static const PredType &from(const std::vector<T> &) {
    return get<T>();
  }

 private:

  /// Create variable-length string datatype with UTF8 encoding.
  static hid_t _create_vlen_string();
};

}  // namespace HDF5

// Function definitions.
namespace HDF5 {

inline hid_t PredType::_create_vlen_string() {
  hid_t type_id = H5Tcopy(H5T_C_S1);
  H5Tset_size(type_id, H5T_VARIABLE);
  H5Tset_cset(type_id, H5T_CSET_UTF8);
  return type_id;
}

inline const PredType &PredType::NATIVE_CHAR() {
  static PredType type(H5Tcopy(H5T_NATIVE_CHAR));
  return type;
}

inline const PredType &PredType::NATIVE_INT() {
  static PredType type(H5Tcopy(H5T_NATIVE_INT));
  return type;
}

inline const PredType &PredType::NATIVE_INT64() {
  static PredType type(H5Tcopy(H5T_NATIVE_INT64));
  return type;
}

inline const PredType &PredType::NATIVE_UINT64() {
  static PredType type(H5Tcopy(H5T_NATIVE_UINT64));
  return type;
}

inline const PredType &PredType::NATIVE_DOUBLE() {
  static PredType type(H5Tcopy(H5T_NATIVE_DOUBLE));
  return type;
}

inline const PredType &PredType::NATIVE_FLOAT() {
  static PredType type(H5Tcopy(H5T_NATIVE_FLOAT));
  return type;
}

inline const PredType &PredType::STRING_UTF8_VLEN() {
  static PredType type(_create_vlen_string());
  return type;
}

template <>
inline const PredType &PredType::get<char>() {
  return NATIVE_CHAR();
}

template <>
inline const PredType &PredType::get<int>() {
  return NATIVE_INT();
}

template <>
inline const PredType &PredType::get<int64_t>() {
  return NATIVE_INT64();
}

template <>
inline const PredType &PredType::get<uint64_t>() {
  return NATIVE_UINT64();
}

template <>
inline const PredType &PredType::get<double>() {
  return NATIVE_DOUBLE();
}

template <>
inline const PredType &PredType::get<float>() {
  return NATIVE_FLOAT();
}

template <>
inline const PredType &PredType::get<std::string>() {
  return STRING_UTF8_VLEN();
}

}  // namespace HDF5
