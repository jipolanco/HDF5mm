#pragma once

#include "Exception.h"

#include <string>

namespace HDF5 {

class File;

/// Manages a HDF5 object with a given identifier.
class IdComponent {
 private:
  /// HDF5 identifier.
  hid_t _id;

 public:
  /// Invalid identifier.
  /// `H5I_INVALID_HID` is defined as a macro H5Ipublic.h, equal to -1.
  static constexpr hid_t INVALID_HID = H5I_INVALID_HID;

  /// Destructor, does nothing.
  virtual ~IdComponent() {}

  /// Default constructor.
  /// Initialises the object with an invalid id.
  IdComponent() : IdComponent(INVALID_HID) {}

  /// Construct from existing id.
  IdComponent(hid_t id) : _id(id) {}

  /// Copy constructor. Increases reference count.
  IdComponent(const IdComponent &x) : _id(x._id) {
    if (is_valid(_id)) H5Iinc_ref(_id);
  }

  /// Assign from another IdComponent.
  ///
  /// If the current id is valid, its reference count is decreased.
  /// This effectively closes the object if its count reaches zero.
  IdComponent &operator=(const IdComponent &x) {
    if (x._id == _id) return *this;
    // Decrease reference count before replacing the current id.
    if (is_valid(_id)) H5Idec_ref(_id);
    _id = x._id;
    // Increase reference count of new id.
    if (is_valid(_id)) H5Iinc_ref(_id);
    return *this;
  }

  /// Get object identifier.
  hid_t get_id() const { return _id; }

  /// Get handle to File associated to this object.
  File get_file() const;

  /// Get reference count of this object (for debugging).
  int refcount() const { return refcount(_id); }

  /// Check if the object identifier is valid.
  bool is_valid() const { return is_valid(_id); }

  /// Close the object.
  ///
  /// Typically called by the destructor in derived classes, but can also be
  /// called manually.
  ///
  /// May throw an exception, as opposed to the destructor which is supposed
  /// to catch exceptions.
  virtual void close() = 0;

 protected:
  /// Get reference count of an object (for debugging).
  static int refcount(hid_t id) { return H5Iget_ref(id); }

  /// Should be called by the destructor of derived classes.
  void destruct() noexcept {
    // Note that this is comparison is not exactly the same as calling
    // is_valid(). An invalid identifier may have values other than INVALID_HID.
    if (_id == INVALID_HID) return;
    try {
      close();
    } catch (Exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  /// Assign the object an invalid id.
  ///
  /// Should be called by derived classes after closing the object (after a
  /// `H5?_close` call), to make sure that the object is not closed again by
  /// the destructor.
  ///
  /// Note that the reference count is *not* decreased, since that is done by
  /// the respective `H5?_close` function.
  void invalidate() { _id = INVALID_HID; }

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
