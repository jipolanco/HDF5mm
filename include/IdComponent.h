#pragma once

#include "Exception.h"

#include <string>

namespace HDF5 {

class File;

/// Manages a HDF5 object with a given identifier.
class IdComponent {
 public:
  /// Destructor, does nothing.
  virtual ~IdComponent() {}

  IdComponent(hid_t id) : id(id) {}

  /// Copy constructor. Increases reference count.
  IdComponent(const IdComponent &x) : id(x.id) {
    if (H5Iis_valid(id)) H5Iinc_ref(id);
  }

  /// Get object identifier.
  hid_t get_id() const { return id; }

  /// Get handle to File associated to this object.
  File get_file() const;

  /// Get reference count of this object (for debugging).
  int refcount() const { return refcount(id); }

 protected:
  /// HDF5 identifier.
  const hid_t id;

  /// Copy-assignment operator.
  IdComponent &operator=(const IdComponent &x) = delete;

  /// Close the object.
  ///
  /// Typically called by the destructor in derived classes.
  /// May throw an exception, as opposed to the destructor which is supposed to
  /// catch exceptions.
  ///
  /// Actually, this should never be called directly, since it is always called
  /// by the destructor, and we don't want to close the object more than once.
  virtual void close() = 0;

  /// Get reference count of an object (for debugging).
  static int refcount(hid_t id) { return H5Iget_ref(id); }

  /// Returns the HDF5 type of an object.
  static H5I_type_t get_type(hid_t id) { return H5Iget_type(id); }

  /// Check if an identifier is "valid". Wraps H5Iis_valid.
  static bool is_valid(hid_t id) {
    htri_t x = H5Iis_valid(id);
    if (x < 0) throw Exception("IdComponent::is_valid");
    return x > 0;
  }
};

}  // namespace HDF5
