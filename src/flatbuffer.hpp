#ifndef GODOT_FLATBUFFERS_EXTENSION_FLATBUFFER_HPP
#define GODOT_FLATBUFFERS_EXTENSION_FLATBUFFER_HPP

#include <godot_cpp/classes/ref_counted.hpp>

#include "flatbuffers/flatbuffers.h"
#include "flatbufferverifier.hpp"
#include "godot_cpp/variant/variant_internal.hpp"
#include "utils.hpp"


namespace godot_flatbuffers {
class FlatBuffer final : public godot::RefCounted {
  GDCLASS( FlatBuffer, RefCounted ) // NOLINT(*-use-auto)

  typedef int32_t  soffset_t;
  typedef uint16_t voffset_t;
  typedef uint32_t uoffset_t;

  // This is actually a PackedByteArray, but because of gdextension
  // we need to keep our data as a Variant to avoid value copies.
  godot::Variant variant;

  // This is a pointer to the variant, to allow modification of the data
  // across the plugin boundary, we need to pass by Variant,
  const godot::PackedByteArray* fb_bytes;

  // Starting position of the buffer.
  // because we're using packed byte arrays, i need the offset into the buffer
    // where we begin.
  int64_t fb_start{};

protected:

  static void _bind_methods();

public:
#ifdef DEBUG
  [[nodiscard]] godot::String get_memory_address() const;
#endif

  // Get and Set of properties
  [[nodiscard]]
  auto get_bytes() const -> godot::Variant{ return variant; }
  auto set_bytes(const godot::Variant &new_var )  -> void {
    fb_bytes = godot::VariantInternal::get_byte_array( &new_var );
    this->variant = new_var;
  }

  [[nodiscard]]
  auto get_start() const -> int64_t { return fb_start; }
  auto set_start( const int64_t start_ ) -> void { fb_start = start_; }

  // Field offset and position
  [[nodiscard]] int64_t get_field_offset( int64_t vtable_offset ) const;

  [[nodiscard]] int64_t get_field_start( int64_t vtable_offset ) const;

  // Array/Vector offset and position
  [[nodiscard]] int64_t get_array_size( int64_t vtable_offset ) const;

  [[nodiscard]] int64_t get_array_element_start( int64_t array_start, int64_t idx ) const;

  auto overwrite_bytes( godot::Variant source, int from, int dest, int size ) const -> godot::Error;

  // Template to simplify encoding godot types into bytes at start_
  // Very simple, performs a raw memcpy to the byte array after checking it has enough room.
  // Specialisations exist in the cpp file
  template< typename GType >
  auto encode_gtype( const int64_t start_, const GType &value ) -> void {
    ERR_FAIL_INDEX_EDMSG(start_ - 1 + sizeof(GType), fb_bytes->size(),
      godot::vformat( "Not enough room in the buffer to encode object\n"
        "buf(%d)[%d] !<- [%d]", fb_bytes->size(), start_, sizeof(GType) ));
    const auto mem = const_cast<godot::PackedByteArray*>(fb_bytes)->ptrw() + start_;
    memcpy( mem , &value, sizeof(GType)  );
  }

  // Template to simplify decoding godot data types from bytes
  // Very simple, checks we have enough data in the array and returns the bytes interpreted as the type
  // Specialisations exist in the cpp file
  template< typename GType > [[nodiscard]]
  auto decode_gtype( const int64_t start_ ) const -> GType {
    assert(start_ + sizeof( GType ) <= bytes->size() );
    const auto p = const_cast< uint8_t * >(fb_bytes->ptr() + start_);
    return *reinterpret_cast< GType * >(p);
  }

  // Template to simplify getting the type from
  template< typename GType > [[nodiscard]]
  auto get_gtype( const int64_t voffset ) const -> GType {
    const uoffset_t field_offset = get_field_offset( voffset );
    if( not field_offset) return {};
    const uoffset_t field_start = fb_start + field_offset;
    return decode_gtype<GType>( field_start );
  }

  // template for struct array access
  // Arrays are vectors of uint32 indexes pointing to the resultant object.
  template< typename GType > [[nodiscard]]
  auto at_gtype( const int64_t voffset, const uint32_t index ) const -> GType {
    // Starting with getting the array
    const uoffset_t array_offset = get_field_start( voffset );
    if( not array_offset) return {};

    // now using the index, get the data, for a POD array, the object is inline.
    const uint64_t array_data    = array_offset + 4;
    const uint64_t element_start = array_data + index * sizeof( GType );

    return decode_gtype<GType>( element_start );
  }

    //MARK:


  //MARK: Verification
  // │__   __       _  __ _         _   _
  // │\ \ / /__ _ _(_)/ _(_)__ __ _| |_(_)___ _ _
  // │ \ V / -_) '_| |  _| / _/ _` |  _| / _ \ ' \
  // │  \_/\___|_| |_|_| |_\__\__,_|\__|_\___/_||_|
  // ╰───────────────────────────────────────────────

  // Verify the vtable of this table.
  // Call this once per table, followed by VerifyField once per field.
  bool verify_table_start(const FlatBufferVerifier *verifier) const {
    return verifier->verify_table_start(fb_bytes->ptr()+fb_start);
  }


    template <typename T>
    bool verify_field( const FlatBufferVerifier *verifier, const voffset_t field, const size_t align ) const
    {
      // Calling GetOptionalFieldOffset should be safe now thanks to VerifyTable().
      const auto field_offset = get_optional_field_offset(field);

      return !field_offset ||
        verifier->verify_field<T>(fb_bytes->ptr()+fb_start, field_offset, align);
    }


  const uint8_t* get_vtable() const
  {
    return fb_bytes->ptr() + fb_start - flatbuffers::ReadScalar<soffset_t>(fb_bytes->ptr() + fb_start);
  }


  // This gets the field offset for any of the functions below it, or 0
  // if the field was not present.
  voffset_t get_optional_field_offset(const voffset_t field) const {
    // The vtable offset is always at the start.
    const auto vtable = get_vtable();
    // The first element is the size of the vtable (fields + type id + itself).
    const auto vtsize = flatbuffers::ReadScalar<voffset_t>(vtable);
    // If the field we're accessing is outside the vtable, we're reading older
    // data, so it's the same as if the offset was 0 (not present).
    return field < vtsize ? flatbuffers::ReadScalar<voffset_t>(vtable + field) : 0;
  }

  // VerifyFieldRequired
  // VerifyOffset
  // VerifyOffsetRequired
  // VerifyOffset64
  // VerifyOffset64Required
  // VerifyStringWithDefault
  // VerifyVectorWithDefault
  // VerifyVector64WithDefault
};
} //namespace godot_flatbuffers

#endif //GODOT_FLATBUFFERS_EXTENSION_FLATBUFFER_HPP
