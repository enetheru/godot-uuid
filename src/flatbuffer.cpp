#include "flatbuffer.hpp"
#include <godot_cpp/core/class_db.hpp>

#include "utils.hpp"

#ifdef DEBUG
#include <utility>

godot::String FlatBuffer::get_memory_address() const {
  const auto i = reinterpret_cast< std::uintptr_t >(bytes->ptr());
  return godot::vformat( "%X", i );
}
#endif

#include <source_location>


#include "godot_cpp/variant/variant_internal.hpp"

namespace godot_flatbuffers {

// Get the size of the godot variant types as they are expressed in the
// flatbuffer.
size_t get_variant_struct_size(const godot::Variant::Type type ) {
  switch( type ) {
    case godot::Variant::VECTOR2:return sizeof(godot::Vector2);
    case godot::Variant::VECTOR2I:return sizeof(godot::Vector2i);
    case godot::Variant::RECT2:return sizeof(godot::Rect2);
    case godot::Variant::RECT2I:return sizeof(godot::Rect2i);
    case godot::Variant::VECTOR3:return sizeof(godot::Vector3);
    case godot::Variant::VECTOR3I:return sizeof(godot::Vector3i);
    case godot::Variant::TRANSFORM2D:return sizeof(godot::Transform2D);
    case godot::Variant::VECTOR4:return sizeof(godot::Vector4);
    case godot::Variant::VECTOR4I:return sizeof(godot::Vector4i);
    case godot::Variant::PLANE:return sizeof(godot::Plane);
    case godot::Variant::QUATERNION:return sizeof(godot::Quaternion);
    case godot::Variant::AABB:return sizeof(godot::AABB);
    case godot::Variant::BASIS:return sizeof(godot::Basis);
    case godot::Variant::TRANSFORM3D:return sizeof(godot::Transform3D);
    case godot::Variant::PROJECTION:return sizeof(godot::Projection);
    case godot::Variant::COLOR:return sizeof(godot::Color);
    default:;
  }
  return -1;
}


template< typename T > [[nodiscard]]
T decode_gtype2( const godot::PackedByteArray *bytes, const uint32_t start ) {
  assert(start + sizeof( T ) <= bytes->size() );
  const auto p = const_cast< uint8_t * >(bytes->ptr() + start);
  T t = *reinterpret_cast< T * >(p);
  return t;
}


template<typename C, typename E > [[nodiscard]]
auto decode_packed_string( const godot::PackedByteArray *bytes, const int64_t start ) -> C {
  const int num_elements = bytes->decode_u32( start );
  const int64_t data_start = start + sizeof(uint32_t);

  C packed_array;
  packed_array.resize(num_elements);
  for (int i = 0; i < num_elements; ++i) {
    const int64_t element_pos = data_start + i * sizeof(uint32_t);
    const uint32_t offset = bytes->decode_u32(element_pos);
    const int64_t string_pos = element_pos + offset;
    const uint32_t string_size = bytes->decode_u32(string_pos);
    packed_array.set(i, bytes->slice(string_pos + 4, string_pos + 4 + string_size).get_string_from_utf8());
  }
  return packed_array;
}

template<typename C, typename E> [[nodiscard]]
auto decode_packed_struct( const godot::PackedByteArray *bytes, const int64_t start ) -> C {
  const uint32_t num_elements = bytes->decode_u32(start);
  const int64_t data_start = start + sizeof(uint32_t);
  const size_t element_size = sizeof(E);

  C packed_array;
  packed_array.resize(num_elements);
  const uint8_t *src_ptr = bytes->ptr() + data_start;
  uint8_t *dest_ptr = reinterpret_cast<uint8_t *>(packed_array.ptrw());
  memcpy(dest_ptr, src_ptr, num_elements * element_size);

  return packed_array;
}


godot::PackedByteArray
slice_packed_bytes(const godot::PackedByteArray *bytes, uint32_t start, const size_t element_size) {
  const uint32_t num_elements = bytes->decode_u32(start);
  enetheru::print("bytes_size: %d", bytes->size());
  enetheru::print("start: %d", start);
  enetheru::print("num_elements: %d", num_elements);
  enetheru::print("end: %d", start + element_size * num_elements);
  start += sizeof(uint32_t);
  ERR_FAIL_COND_V_MSG(
    start + num_elements * element_size > bytes->size(),{},
    "decoded array bounds are larger than byte data.");
  return bytes->slice(start, start + num_elements * element_size);
}



// MARK: Specialisation
// ║ ___              _      _ _          _   _
// ║/ __|_ __  ___ __(_)__ _| (_)___ __ _| |_(_)___ _ _
// ║\__ \ '_ \/ -_) _| / _` | | (_-</ _` |  _| / _ \ ' \
// ║|___/ .__/\___\__|_\__,_|_|_/__/\__,_|\__|_\___/_||_|
// ╙────|_|──────────────────────────────────────────────

template<> [[nodiscard]]
auto FlatBuffer::decode_gtype( const int64_t start_ ) const -> godot::PackedByteArray {
  const int64_t size        = fb_bytes->decode_u32( start_ );
  const int64_t array_start = start_ + 4;
  return fb_bytes->slice( array_start, array_start + size );
}



// // Ony a few of these functions exist in 'godot_cpp\variant\packed_byte_array.hpp'
// DECODE_PACKED_GTYPE( PackedFloat32Array, float, to_float32_array )
// DECODE_PACKED_GTYPE( PackedFloat64Array, double, to_float64_array )
// DECODE_PACKED_GTYPE( PackedInt32Array, uint32_t, to_int32_array )
// DECODE_PACKED_GTYPE( PackedInt64Array, uint64_t, to_int64_array )


template<> [[nodiscard]]
auto FlatBuffer::decode_gtype( const int64_t start_ ) const -> godot::String{
  return fb_bytes->slice( start_ + 4, start_ + 4 + fb_bytes->decode_u32( start_ ) ).get_string_from_utf8();
}


template<> [[nodiscard]]
auto FlatBuffer::decode_gtype( const int64_t start_ ) const -> godot::PackedStringArray {
  const int64_t size = fb_bytes->decode_u32( start_ );
  const int64_t data = start_ + sizeof( uint32_t ); // NOLINT(*-narrowing-conversions)

  godot::PackedStringArray string_array;
  for( int i = 0; i < size; ++i ) {
    const int64_t  element = data + i * sizeof( uint32_t ); // NOLINT(*-narrowing-conversions)
    const uint32_t offset  = fb_bytes->decode_u32( element );
    string_array.append( decode_gtype<godot::String>( element + offset ) );
  }
  return string_array;
}

// MARK: Bind Methods
// ║ ___ _         _   __  __     _   _            _
// ║| _ |_)_ _  __| | |  \/  |___| |_| |_  ___  __| |___
// ║| _ \ | ' \/ _` | | |\/| / -_)  _| ' \/ _ \/ _` (_-<
// ║|___/_|_||_\__,_| |_|  |_\___|\__|_||_\___/\__,_/__/
// ╙───────────────────────────────────────────────────────────────────────────
void FlatBuffer::_bind_methods() {
  using godot::ClassDB;
  using godot::PropertyInfo;
  using godot::D_METHOD;

  //Debug
#ifdef DEBUG
  ClassDB::bind_method( D_METHOD( "get_memory_address" ), &FlatBuffer::get_memory_address );
#endif

  // TODO implement a static function get_root(Script, buffer) -> Object

  ClassDB::bind_method( D_METHOD( "set_bytes", "bytes" ), &FlatBuffer::set_bytes );
  ClassDB::bind_method( D_METHOD( "get_bytes" ), &FlatBuffer::get_bytes );
  ADD_PROPERTY( PropertyInfo(godot::Variant::PACKED_BYTE_ARRAY, "_fb_bytes"), "set_bytes", "get_bytes" );

  //Properties
  ClassDB::bind_method( D_METHOD( "set_start", "start" ), &FlatBuffer::set_start );
  ClassDB::bind_method( D_METHOD( "get_start" ), &FlatBuffer::get_start );
  ADD_PROPERTY( PropertyInfo(godot::Variant::INT, "_fb_start"), "set_start", "get_start" );

  // Field Access Helpers
  ClassDB::bind_method( D_METHOD( "get_field_offset", "vtable_pos" ), &FlatBuffer::get_field_offset );
  ClassDB::bind_method( D_METHOD( "get_inline_field_start", "vtable_pos" ), &FlatBuffer::get_inline_field_start );
  ClassDB::bind_method( D_METHOD( "get_offset_field_start", "vtable_pos" ), &FlatBuffer::get_offset_field_start );

  //Array Access Helpers
  ClassDB::bind_method( D_METHOD( "get_array_size", "vtable_offset" ), &FlatBuffer::get_array_size );
  ClassDB::bind_method( D_METHOD( "get_array_element_start", "array_start", "idx" ), &FlatBuffer::get_array_element_start );

  //Overwrite Bytes
  ClassDB::bind_method( D_METHOD( "overwrite_bytes", "source", "from", "to", "size" ), &FlatBuffer::overwrite_bytes );

  ClassDB::bind_method(
    D_METHOD( "encode_variant", "start_", "value", "expected_type" ),
    &FlatBuffer::encode_variant, godot::Variant::Type::VARIANT_MAX);

  ClassDB::bind_method(
    D_METHOD( "get_variant_at", "vt_enum", "index", "expected_type" ),
    &FlatBuffer::get_variant_at);

  ClassDB::bind_method(
    D_METHOD( "decode_variant", "start", "expected_type" ),
    &FlatBuffer::decode_variant);


  //MARK: Verification Bindings
  ClassDB::bind_method( D_METHOD( "verify_table_start", "verifier" ), &FlatBuffer::verify_table_start );

  // field verifiers
  ClassDB::bind_method( D_METHOD( "verify_field_u8", "verifier", "field", "align" ), &FlatBuffer::verify_field<uint8_t> );
  ClassDB::bind_method( D_METHOD( "verify_field_s8", "verifier", "field", "align" ), &FlatBuffer::verify_field<int8_t> );
  ClassDB::bind_method( D_METHOD( "verify_field_u16", "verifier", "field", "align" ), &FlatBuffer::verify_field<uint16_t> );
  ClassDB::bind_method( D_METHOD( "verify_field_s16", "verifier", "field", "align" ), &FlatBuffer::verify_field<int16_t> );
  ClassDB::bind_method( D_METHOD( "verify_field_u32", "verifier", "field", "align" ), &FlatBuffer::verify_field<uint32_t> );
  ClassDB::bind_method( D_METHOD( "verify_field_s32", "verifier", "field", "align" ), &FlatBuffer::verify_field<int32_t> );
  ClassDB::bind_method( D_METHOD( "verify_field_u64", "verifier", "field", "align" ), &FlatBuffer::verify_field<uint64_t> );
  ClassDB::bind_method( D_METHOD( "verify_field_s64", "verifier", "field", "align" ), &FlatBuffer::verify_field<int64_t> );
  ClassDB::bind_method( D_METHOD( "verify_field_float", "verifier", "field", "align" ), &FlatBuffer::verify_field<float> );
  ClassDB::bind_method( D_METHOD( "verify_field_double", "verifier", "field", "align" ), &FlatBuffer::verify_field<double> );
}

// MARK: Function Definitions
// ║ ___               ___       __
// ║| __|  _ _ _  __  |   \ ___ / _|___
// ║| _| || | ' \/ _| | |) / -_)  _(_-<
// ║|_| \_,_|_||_\__| |___/\___|_| /__/
// ╙───────────────────────────────────

auto FlatBuffer::overwrite_bytes(godot::Variant source, const int from, const int dest, const int size) const
        -> godot::Error {
  ERR_FAIL_COND_V_EDMSG(
          source.get_type() != godot::Variant::PACKED_BYTE_ARRAY, godot::ERR_INVALID_PARAMETER,
          "overwrite_bytes source must be a PackedByteArray");
  ERR_FAIL_INDEX_V_EDMSG(
          dest + size, fb_bytes->size(), godot::ERR_INVALID_PARAMETER, "Not enough memory for overwrite_bytes");

  const auto source_bytes = godot::VariantInternal::get_byte_array(&source)->ptr();
  const auto dest_bytes   = const_cast< godot::PackedByteArray * >(fb_bytes)->ptrw();
  memcpy(dest_bytes + dest, source_bytes + from, size);
  return godot::OK;
}

auto FlatBuffer::encode_variant(const int64_t start, const godot::Variant &value, godot::Variant::Type expected_type)
        -> void {
  if( expected_type == godot::Variant::Type::VARIANT_MAX ) {
    expected_type = value.get_type();
  } else {
    // TODO format the string below to show the types.
    ERR_FAIL_COND_MSG(expected_type != value.get_type(), "given type does not match expected type");
  }

  switch( expected_type ) {
    case godot::Variant::NIL:
      break;
    case godot::Variant::BOOL:
      break;
    case godot::Variant::INT:
      break;
    case godot::Variant::FLOAT:
      break;
    case godot::Variant::STRING: {
      const auto v = static_cast< godot::String >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::VECTOR2: {
      const auto v = static_cast< godot::Vector2 >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::VECTOR2I: {
      const auto v = static_cast< godot::Vector2i >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::RECT2: {
      const auto v = static_cast< godot::Rect2 >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::RECT2I: {
      const auto v = static_cast< godot::Rect2i >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::VECTOR3: {
      const auto v = static_cast< godot::Vector3 >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::VECTOR3I: {
      const auto v = static_cast< godot::Vector3i >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::TRANSFORM2D: {
      const auto v = static_cast< godot::Transform2D >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::VECTOR4: {
      const auto v = static_cast< godot::Vector4 >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::VECTOR4I: {
      const auto v = static_cast< godot::Vector4i >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::PLANE: {
      const auto v = static_cast< godot::Plane >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::QUATERNION: {
      const auto v = static_cast< godot::Quaternion >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::AABB: {
      const auto v = static_cast< godot::AABB >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::BASIS: {
      const auto v = static_cast< godot::Basis >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::TRANSFORM3D: {
      const auto v = static_cast< godot::Transform3D >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::PROJECTION: {
      const auto v = static_cast< godot::Projection >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::COLOR: {
      const auto v = static_cast< godot::Color >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::STRING_NAME: {
      // Convert to String first.
      const godot::String v = static_cast< godot::StringName >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::NODE_PATH: {
      // Convert to String first.
      const godot::String v = static_cast< godot::NodePath >(value);
      encode_gtype(start, v);
      break;
    }
    case godot::Variant::RID: {
      ERR_FAIL_MSG("Unsupported type(RID) given as value.");
    }
    case godot::Variant::OBJECT:
      break;
    case godot::Variant::CALLABLE: {
      ERR_FAIL_MSG("Unsupported type(RID) given as value.");
    }
    case godot::Variant::SIGNAL: {
      ERR_FAIL_MSG("Unsupported type(RID) given as value.");
    }
    case godot::Variant::DICTIONARY:
      break;
    case godot::Variant::ARRAY:
      break;
    case godot::Variant::PACKED_BYTE_ARRAY:
      break;
    case godot::Variant::PACKED_INT32_ARRAY:
      break;
    case godot::Variant::PACKED_INT64_ARRAY:
      break;
    case godot::Variant::PACKED_FLOAT32_ARRAY:
      break;
    case godot::Variant::PACKED_FLOAT64_ARRAY:
      break;
    case godot::Variant::PACKED_STRING_ARRAY:
      break;
    case godot::Variant::PACKED_VECTOR2_ARRAY:
      break;
    case godot::Variant::PACKED_VECTOR3_ARRAY:
      break;
    case godot::Variant::PACKED_COLOR_ARRAY:
      break;
    case godot::Variant::PACKED_VECTOR4_ARRAY:
      break;
    case godot::Variant::VARIANT_MAX:
      break;
    default:
      break;
  }
}


godot::Variant FlatBuffer::decode_variant(int64_t start, const godot::Variant::Type expected_type) const {
  // TODO format the string below to show the types.
  ERR_FAIL_COND_V_MSG(expected_type == godot::Variant::Type::VARIANT_MAX, nullptr, "An expected type is required.");

  switch( expected_type ) {
    case godot::Variant::NIL:
      return nullptr;
    case godot::Variant::BOOL:
      return static_cast< bool >(fb_bytes->decode_s8(start));
    case godot::Variant::INT:
      return fb_bytes->decode_s64(start);
    case godot::Variant::FLOAT:
      return fb_bytes->decode_double(start);
    case godot::Variant::STRING: {
      const int string_size = fb_bytes->decode_u32(start);
      start += sizeof(uint32_t);
      return fb_bytes->slice(start, start + string_size).get_string_from_utf8();
    }
    case godot::Variant::VECTOR2:
      return decode_gtype2< godot::Vector2 >(fb_bytes, start);
    case godot::Variant::VECTOR2I:
      return decode_gtype2< godot::Vector2i >(fb_bytes, start);
    case godot::Variant::RECT2:
      return decode_gtype2< godot::Rect2 >(fb_bytes, start);
    case godot::Variant::RECT2I:
      return decode_gtype2< godot::Rect2i >(fb_bytes, start);
    case godot::Variant::VECTOR3:
      return decode_gtype2< godot::Vector3 >(fb_bytes, start);
    case godot::Variant::VECTOR3I:
      return decode_gtype2< godot::Vector3i >(fb_bytes, start);
    case godot::Variant::TRANSFORM2D:
      return decode_gtype2< godot::Transform2D >(fb_bytes, start);
    case godot::Variant::VECTOR4:
      return decode_gtype2< godot::Vector4 >(fb_bytes, start);
    case godot::Variant::VECTOR4I:
      return decode_gtype2< godot::Vector4i >(fb_bytes, start);
    case godot::Variant::PLANE:
      return decode_gtype2< godot::Plane >(fb_bytes, start);
    case godot::Variant::QUATERNION:
      return decode_gtype2< godot::Quaternion >(fb_bytes, start);
    case godot::Variant::AABB:
      return decode_gtype2< godot::AABB >(fb_bytes, start);
    case godot::Variant::BASIS:
      return decode_gtype2< godot::Basis >(fb_bytes, start);
    case godot::Variant::TRANSFORM3D:
      return decode_gtype2< godot::Transform3D >(fb_bytes, start);
    case godot::Variant::PROJECTION:
      return decode_gtype2< godot::Projection >(fb_bytes, start);
    case godot::Variant::COLOR:
      return decode_gtype2< godot::Color >(fb_bytes, start);
    case godot::Variant::STRING_NAME: {
      const uint32_t string_size = fb_bytes->decode_u32(start);
      start += sizeof(uint32_t);
      const godot::String string = fb_bytes->slice(start, start + string_size).get_string_from_utf8();
      return godot::StringName(string);
    }
    case godot::Variant::NODE_PATH: {
      const uint32_t string_size = fb_bytes->decode_u32(start);
      start += sizeof(uint32_t);
      const godot::String string = fb_bytes->slice(start, start + string_size).get_string_from_utf8();
      return godot::NodePath(string);
    }
    case godot::Variant::RID: {
      ERR_FAIL_V_MSG(nullptr, "Unsupported type(RID) given as value.");
    }
    case godot::Variant::OBJECT:
      break;
    case godot::Variant::CALLABLE: {
      ERR_FAIL_V_MSG(nullptr, "Unsupported type(RID) given as value.");
    }
    case godot::Variant::SIGNAL: {
      ERR_FAIL_V_MSG(nullptr, "Unsupported type(RID) given as value.");
    }
    case godot::Variant::DICTIONARY:
      break;
    case godot::Variant::ARRAY:
      break;
    case godot::Variant::PACKED_BYTE_ARRAY:
      return slice_packed_bytes(fb_bytes, start, sizeof(uint8_t));
    case godot::Variant::PACKED_INT32_ARRAY:
      return slice_packed_bytes(fb_bytes, start, sizeof(int32_t)).to_int32_array();
    case godot::Variant::PACKED_INT64_ARRAY:
      return slice_packed_bytes(fb_bytes, start, sizeof(int64_t)).to_int64_array();
    case godot::Variant::PACKED_FLOAT32_ARRAY:
      return slice_packed_bytes(fb_bytes, start, sizeof(float)).to_float32_array();
    case godot::Variant::PACKED_FLOAT64_ARRAY:
      return slice_packed_bytes(fb_bytes, start, sizeof(double)).to_float64_array();
    case godot::Variant::PACKED_STRING_ARRAY:
      return decode_packed_string< godot::PackedStringArray, godot::String >(fb_bytes, start);
    case godot::Variant::PACKED_VECTOR2_ARRAY:
      return decode_packed_struct< godot::PackedVector2Array, godot::Vector2 >(fb_bytes, start);
    case godot::Variant::PACKED_VECTOR3_ARRAY:
      return decode_packed_struct< godot::PackedVector3Array, godot::Vector3 >(fb_bytes, start);
    case godot::Variant::PACKED_COLOR_ARRAY:
      return decode_packed_struct< godot::PackedColorArray, godot::Color >(fb_bytes, start);
    case godot::Variant::PACKED_VECTOR4_ARRAY:
      return decode_packed_struct< godot::PackedVector4Array, godot::Vector4 >(fb_bytes, start);
    default:;
  }
  return nullptr;
}


// Returns the offset to the field at the vtable position given.
int64_t FlatBuffer::get_field_offset(const int64_t vtable_pos) const {
  // vtable positions less than 4 are not valid.
  if( vtable_pos < 4 ) {
    return 0;
  }

  const int64_t vtable_start = fb_start - fb_bytes->decode_s32(fb_start);
  const int64_t vtable_size  = fb_bytes->decode_s16(vtable_start);
  // int64_t table_size = bytes->decode_s16( vtable_pos + 2 ); Unnecessary

  // The vtable_pos being outside the range is not an error,
  // it simply means that the element is not present in the table.
  if( vtable_pos >= vtable_size ) {
    return 0;
  }

  // decoding zero means that the field is not present.
  return fb_bytes->decode_s16(vtable_start + vtable_pos);
}


FlatBuffer::voffset_t FlatBuffer::get_optional_field_offset(const voffset_t vtable_pos) const {
  // The vtable offset is always at the start.
  const auto vtable = get_vtable();
  // The first element is the size of the vtable (fields + type id + itself).
  const auto vtsize = flatbuffers::ReadScalar< voffset_t >(vtable);
  // If the field we're accessing is outside the vtable, we're reading older
  // data, so it's the same as if the offset was 0 (not present).
  return vtable_pos < vtsize ? flatbuffers::ReadScalar< voffset_t >(vtable + vtable_pos) : 0;
}


// returns offset from the start of the buffer to the inline field.
int64_t FlatBuffer::get_inline_field_start(const int64_t vtable_pos) const {
  const int field_offset = get_field_offset(vtable_pos); // NOLINT(*-narrowing-conversions)
  if( ! field_offset )
    return 0;
  return fb_start + field_offset;
}

// returns offset from the start of the buffer to the offset field.
int64_t FlatBuffer::get_offset_field_start(const int64_t vtable_pos) const {
  const int field_offset = get_field_offset(vtable_pos); // NOLINT(*-narrowing-conversions)
  if( ! field_offset )
    return 0;
  return fb_start + field_offset + fb_bytes->decode_u32(fb_start + field_offset);
}


// Template to simplify getting the type from
godot::Variant FlatBuffer::get_variant(const int64_t field_start, const godot::Variant::Type expected_type) const {
  return decode_variant(field_start, expected_type);
}

// template for struct array access
godot::Variant FlatBuffer::get_variant_at(
        const int64_t vtable_pos, const uint32_t index, const godot::Variant::Type expected_type) const {
  const uoffset_t elements_start = get_offset_field_start(vtable_pos) + sizeof(uint32_t);
  if( not elements_start )
    return {};

  const uint64_t element_start = elements_start + index * get_variant_struct_size(expected_type);
  return decode_variant(element_start, expected_type);
}

int64_t FlatBuffer::get_array_size(const int64_t vtable_offset) const {
  const int64_t field_start = get_offset_field_start(vtable_offset);
  if( ! field_start )
    return 0;
  return fb_bytes->decode_u32(field_start);
}


int64_t FlatBuffer::get_array_element_start(const int64_t array_start, const int64_t idx) const {
#ifdef DEBUG_ENABLED
  [[maybe_unused]] const int64_t array_size = fb_bytes->decode_u32(array_start);
  assert(array_start < bytes->size() - 4);
  assert(idx < array_size);
  assert(array_start + array_size * 4 < bytes->size());
#endif

  const int64_t data    = array_start + 4;
  const int64_t element = data + idx * 4;
  const int64_t value   = fb_bytes->decode_u32(element);

  return element + value;
}


} // end namespace godot_flatbuffers
