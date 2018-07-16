#pragma once

#include "Object.h"

namespace HDF5 {

/// Abstract data type.
class DataType : public Object {
 public:
  DataType() = default;
  DataType(hid_t type_id) : Object(type_id) {}

  /// Try to close datatype.
  virtual ~DataType() { destruct(); }

  /// Close datatype.
  virtual void close() override {
    if (get_type(get_id()) != H5I_DATATYPE) return;
    if (H5Tclose(get_id()) < 0) throw Exception("DataType::close");
    invalidate();
  }

  /// Get size of datatype in bytes.
  std::size_t get_size() const { return H5Tget_size(get_id()); }

  /// Compare to another datatype.
  bool operator==(const DataType &type) const {
    return H5Tequal(get_id(), type.get_id());
  }
  bool operator!=(const DataType &type) const { return !(*this == type); }
};

/// Predefined data types.
class PredType : public DataType {
 protected:
  PredType(hid_t type_id) : DataType(type_id) {}

 public:
  /// Predefined data types.
  static const PredType &NATIVE_CHAR();

  static const PredType &NATIVE_UINT8();
  static const PredType &NATIVE_UINT16();
  static const PredType &NATIVE_UINT32();
  static const PredType &NATIVE_UINT64();

  static const PredType &NATIVE_INT();
  static const PredType &NATIVE_INT64();

  static const PredType &NATIVE_FLOAT();
  static const PredType &NATIVE_DOUBLE();

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

/// Associates a HDF5 native type to a C type.
#define DEFINE_NATIVE_TYPE(htype, ctype)               \
  inline const PredType &PredType::NATIVE_##htype() {  \
    static PredType type(H5Tcopy(H5T_NATIVE_##htype)); \
    return type;                                       \
  }                                                    \
                                                       \
  template <>                                          \
  inline const PredType &PredType::get<ctype>() {      \
    return NATIVE_##htype();                           \
  }                                                    \
                                                       \
  class PredType  // this is just to require a semicolon after the macro

DEFINE_NATIVE_TYPE(CHAR, char);
DEFINE_NATIVE_TYPE(UINT8, uint8_t);
DEFINE_NATIVE_TYPE(UINT16, uint16_t);
DEFINE_NATIVE_TYPE(UINT32, uint32_t);
DEFINE_NATIVE_TYPE(UINT64, uint64_t);
DEFINE_NATIVE_TYPE(INT, int);
DEFINE_NATIVE_TYPE(INT64, int64_t);
DEFINE_NATIVE_TYPE(DOUBLE, double);
DEFINE_NATIVE_TYPE(FLOAT, float);

#undef DEFINE_NATIVE_TYPE

inline hid_t PredType::_create_vlen_string() {
  hid_t type_id = H5Tcopy(H5T_C_S1);
  H5Tset_size(type_id, H5T_VARIABLE);
  H5Tset_cset(type_id, H5T_CSET_UTF8);
  return type_id;
}

inline const PredType &PredType::STRING_UTF8_VLEN() {
  static PredType type(_create_vlen_string());
  return type;
}

template <>
inline const PredType &PredType::get<std::string>() {
  return STRING_UTF8_VLEN();
}

}  // namespace HDF5
