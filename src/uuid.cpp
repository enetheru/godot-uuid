#include "uuid.hpp"

#include <concepts>
#include <type_traits>

#include "godot_cpp/variant/variant_internal.hpp"
#include "utils.hpp"

using namespace godot_flatbuffers;

using godot::ClassDB;
using godot::HashingContext;
using godot::RandomNumberGenerator;
using godot::Ref;
using godot::D_METHOD;

constexpr uuids::uuid nil_stduuid{};
const UUIDv4::UUID nil_uuidv4{};

constexpr uuids::uuid max_stduuid = uuids::uuid::from_string(MAX_UUID).value();

// MARK: Static

bool UUID::initialised = false;

// RNG Sources
std::mt19937 UUID::mt_rng{};

Ref< RandomNumberGenerator > UUID::godot_rng{};

// UUID generators
uuids::uuid_name_generator UUID::stduuid_dung{ nil_stduuid }; // Default Unique Name Generator (DUNG)
uuids::uuid_name_generator stduuid_dung{ nil_stduuid }; // Default Unique Name Generator (DUNG)

std::unique_ptr< uuids::basic_uuid_random_generator< RandomNumberGenerator > >
        UUID::stduuid_burg{}; // Basic UUID Random Generator (BURG)

UUIDv4::UUIDGenerator< std::mt19937_64 > UUID::uuidv4_generator{};

uuids::uuid_random_generator UUID::stduuid_urg{ mt_rng }; // UUID Random Generator (URG)

// MARK: Endianness

// ───────────────────────────────────────────────
//  Pick implementation depending on C++ standard
// ───────────────────────────────────────────────

#if __cplusplus >= 202002L
// C++20 or later → use concepts

template< typename T >
concept ByteLike = std::same_as< std::remove_cv_t< T >, uint8_t > || std::same_as< std::remove_cv_t< T >, std::byte > ||
                   std::same_as< std::remove_cv_t< T >, char >; // ← added for const char*


template< ByteLike T >
static uint32_t bytes_to_uint32_be(const T *bytes) noexcept {
  // Always treat the byte as unsigned to avoid sign-extension
  // on platforms where plain char is signed.
  const uint8_t b0 = static_cast< uint8_t >(bytes[ 0 ]);
  const uint8_t b1 = static_cast< uint8_t >(bytes[ 1 ]);
  const uint8_t b2 = static_cast< uint8_t >(bytes[ 2 ]);
  const uint8_t b3 = static_cast< uint8_t >(bytes[ 3 ]);

  return (static_cast< uint32_t >(b0) << 24) | (static_cast< uint32_t >(b1) << 16) |
         (static_cast< uint32_t >(b2) << 8) | static_cast< uint32_t >(b3);
}

#else
// C++17 / C++14 / C++11 fallback — SFINAE

template<
        typename T, typename = std::enable_if_t<
                            std::is_same_v< std::remove_cv_t< T >, uint8_t > ||
                            std::is_same_v< std::remove_cv_t< T >, std::byte > ||
                            std::is_same_v< std::remove_cv_t< T >, char > // ← added for const char*
                            > >
static uint32_t bytes_to_uint32_be(const T *bytes) noexcept {
  const uint8_t b0 = static_cast< uint8_t >(bytes[ 0 ]);
  const uint8_t b1 = static_cast< uint8_t >(bytes[ 1 ]);
  const uint8_t b2 = static_cast< uint8_t >(bytes[ 2 ]);
  const uint8_t b3 = static_cast< uint8_t >(bytes[ 3 ]);

  return (static_cast< uint32_t >(b0) << 24) | (static_cast< uint32_t >(b1) << 16) |
         (static_cast< uint32_t >(b2) << 8) | static_cast< uint32_t >(b3);
}
#endif


static void uint32_to_bytes_be(const uint32_t value, uint8_t *bytes) {
  bytes[ 0 ] = static_cast< uint8_t >(value >> 24);
  bytes[ 1 ] = static_cast< uint8_t >(value >> 16);
  bytes[ 2 ] = static_cast< uint8_t >(value >> 8);
  bytes[ 3 ] = static_cast< uint8_t >(value);
}

// MARK: to_stduuid


static std::optional<uuids::uuid> to_stduuid(const Variant &from) {
  switch( from.get_type() ) {
    case Variant::STRING: {
      const auto from_str = static_cast<String>(from);
      return uuids::uuid::from_string(from_str.utf8().ptr());
    }
    case Variant::VECTOR4I: {
      const auto from_vec = static_cast<Vector4i>(from);
      std::array< uint8_t, 16 > bytes;
      uint32_to_bytes_be(static_cast< uint32_t >(from_vec.x), bytes.data() + 0);
      uint32_to_bytes_be(static_cast< uint32_t >(from_vec.y), bytes.data() + 4);
      uint32_to_bytes_be(static_cast< uint32_t >(from_vec.z), bytes.data() + 8);
      uint32_to_bytes_be(static_cast< uint32_t >(from_vec.w), bytes.data() + 12);
      return uuids::uuid{ bytes.begin(), bytes.end() };
    }
    case Variant::PACKED_BYTE_ARRAY: {
      const auto from_bytes = godot::VariantInternal::get_byte_array(&from);
      ERR_FAIL_COND_V_MSG(from_bytes->size() != 16, {}, godot::vformat("size(%d) of provided byte array is not 16.", from_bytes->size()));
      auto retval = uuids::uuid();
      memcpy( &retval, from_bytes->ptr(), 16 );
      return retval;
    }
    default:
      ERR_FAIL_V_MSG({}, godot::vformat(
        "conversion to stduuid: type(%s) is not supported, must be String, Vector4i, or PackedByteArray",
        godot::Variant::get_type_name(from.get_type()) ));
  }
}

// MARK: to_uuidv4
//  The difficulty and advantage with the UUIDv4 lib is that there is no error checking here.
//  So we'd better be sure we are handing it valid data.

// static UUIDv4::UUID to_uuidv4(const Variant &from) {
//   switch( from.get_type() ) {
//     case Variant::STRING: {
//       const auto from_str = static_cast<String>(from);
//       ERR_FAIL_COND_V_MSG(from_str.length() > 16, nil_uuidv4,
//         godot::vformat("provided uuid String is not 16 bytes long"));
//       return UUIDv4::UUID(from_str.utf8().ptr());
//     }
//     case Variant::VECTOR4I: {
//       const auto from_vec = static_cast<Vector4i>(from);
//       uint8_t bytes[ 16 ];
//       std::memcpy(bytes, &from_vec, 16);
//       return UUIDv4::UUID(bytes);
//     }
//     case Variant::PACKED_BYTE_ARRAY: {
//       const auto from_bytes = static_cast<PackedByteArray>(from);
//       ERR_FAIL_COND_V_MSG(from_bytes.size() > 16, nil_uuidv4,
//         godot::vformat("provided uuid PackedByteArray is not 16 bytes long"));
//       return UUIDv4::UUID(from_bytes.ptr());
//     }
//     default:
//       ERR_FAIL_V_MSG(nil_uuidv4, godot::vformat(
//         "conversion to stduuid: type(%s) is not supported, must be String, Vector4i, or PackedByteArray",
//         godot::Variant::get_type_name(from.get_type()) ));
//   }
// }


// MARK: to_String

static String to_string(const uuids::uuid uuid) {
  return String(uuids::to_string(uuid).c_str());
}


static String to_string(const UUIDv4::UUID &from) {
  return String(from.str().c_str());
}


static String to_string(const PackedByteArray &bytes) {
  if( bytes.size() != 16 ) {
    return "";
  }
  const auto uuid = UUIDv4::UUID(bytes.ptr());
  return ::to_string(uuid);
}


// MARK: to_PackedByteArray
//  static PackedByteArray to_bytes(const uuids::uuid& uuid) {
//      PackedByteArray bytes;
//      bytes.resize(16);
//      const auto data = uuid.as_bytes();
//      std::copy(std::cbegin(data), std::cend(data), bytes.ptrw());
//      return bytes;
//  }


static PackedByteArray to_bytes(const uuids::uuid &uuid) {
  PackedByteArray bytes;
  bytes.resize(16);

  const auto src = uuid.as_bytes(); // span<const std::byte, 16>
  uint8_t   *dst = bytes.ptrw();    // unsigned char*

  std::memcpy(dst, src.data(), 16);

  return bytes;
}


static PackedByteArray to_bytes(const UUIDv4::UUID &uuid) {
  PackedByteArray bytes;
  bytes.resize(16);
  const auto data = uuid.bytes();
  std::ranges::copy(data, bytes.ptrw());
  return bytes;
}


// MARK: to_Vector4i
static Vector4i to_vector4i(const Variant &uuid) {
  PackedByteArray pba;
  switch( uuid.get_type() ) {
    case Variant::STRING: {
      pba = static_cast<String>(uuid).to_utf8_buffer();
      break;
    }
    case Variant::PACKED_BYTE_ARRAY: {
      pba = uuid;
      break;
    }
    default:
      ERR_FAIL_V_MSG({}, godot::vformat(
        "conversion to Vector4i from '%s' is not supported, must be String or PackedByteArray",
        godot::Variant::get_type_name(uuid.get_type()) ));
  }
  return Vector4i(
          pba.decode_u32(0),
          pba.decode_u32(4),
          pba.decode_u32(8),
          pba.decode_u32(12));
}

static Vector4i to_vector4i(const uuids::uuid &uuid) {
  const auto span = uuid.as_bytes();
  return Vector4i(
          static_cast< int32_t >(bytes_to_uint32_be(span.data() + 0)),
          static_cast< int32_t >(bytes_to_uint32_be(span.data() + 4)),
          static_cast< int32_t >(bytes_to_uint32_be(span.data() + 8)),
          static_cast< int32_t >(bytes_to_uint32_be(span.data() + 12)));
}


static Vector4i to_vector4i(const UUIDv4::UUID &uuid) {
  Vector4i           v;
  const std::string &bytes = uuid.bytes();
  std::memcpy(&v, bytes.data(), sizeof(Vector4i));
  return v;
}

// MARK: Bind

void UUID::_bind_methods() {
  // Constants
  ClassDB::bind_static_method("UUID", D_METHOD("get_nil_uuid"), &UUID::get_nil_uuid);
  ClassDB::bind_static_method("UUID", D_METHOD("get_max_uuid"), &UUID::get_max_uuid);
  ClassDB::bind_static_method("UUID", D_METHOD("get_namespace_dns"), &UUID::get_namespace_dns);
  ClassDB::bind_static_method("UUID", D_METHOD("get_namespace_url"), &UUID::get_namespace_url);
  ClassDB::bind_static_method("UUID", D_METHOD("get_namespace_oid"), &UUID::get_namespace_oid);
  ClassDB::bind_static_method("UUID", D_METHOD("get_namespace_x500"), &UUID::get_namespace_x500);

  // Enums
  ClassDB::bind_integer_constant("UUID", "Version", "NONE", 0);
  ClassDB::bind_integer_constant("UUID", "Version", "TIME_BASED", 1);
  ClassDB::bind_integer_constant("UUID", "Version", "DCE_SECURITY", 2);
  ClassDB::bind_integer_constant("UUID", "Version", "NAME_BASED_MD5", 3);
  ClassDB::bind_integer_constant("UUID", "Version", "RANDOM_NUMBER_BASED", 4);
  ClassDB::bind_integer_constant("UUID", "Version", "NAME_BASED_SHA1", 5);

  // Hash
  ClassDB::bind_static_method("UUID", D_METHOD("hash_uuid", "uuid_str"), &UUID::hash_uuid);

  // Creators v3
  ClassDB::bind_static_method(
          "UUID", D_METHOD("create_v3_godot_string", "name", "namespace_uuid"), &UUID::create_v3_godot_string,
          DEFVAL(String(NIL_UUID)));
  ClassDB::bind_static_method(
          "UUID", D_METHOD("create_v3_godot_bytes", "name", "namespace_uuid"), &UUID::create_v3_godot_bytes,
          DEFVAL(String(NIL_UUID)));
  ClassDB::bind_static_method(
        "UUID", D_METHOD("create_v3_godot_vector4i", "name", "namespace_uuid"), &UUID::create_v3_godot_vector4i,
        DEFVAL(String(NIL_UUID)));

  // v4
  ClassDB::bind_static_method("UUID", D_METHOD("create_v4_stduuid_string"), &UUID::create_v4_stduuid_string);
  ClassDB::bind_static_method("UUID", D_METHOD("create_v4_stduuid_bytes"), &UUID::create_v4_stduuid_bytes);
  ClassDB::bind_static_method("UUID", D_METHOD("create_v4_stduuid_vector4i"), &UUID::create_v4_stduuid_vector4i);

  ClassDB::bind_static_method("UUID", D_METHOD("create_v4_uuidv4_string"), &UUID::create_v4_uuidv4_string);
  ClassDB::bind_static_method("UUID", D_METHOD("create_v4_uuidv4_bytes"), &UUID::create_v4_uuidv4_bytes);
  ClassDB::bind_static_method("UUID", D_METHOD("create_v4_uuidv4_vector4i"), &UUID::create_v4_uuidv4_vector4i);

  // v5
  ClassDB::bind_static_method(
          "UUID", D_METHOD("create_v5_stduuid_string", "seed", "namespace_uuid"), &UUID::create_v5_stduuid_string,
          DEFVAL(String(NIL_UUID)));
  ClassDB::bind_static_method(
          "UUID", D_METHOD("create_v5_stduuid_bytes", "seed", "namespace_uuid"), &UUID::create_v5_stduuid_bytes,
          DEFVAL(String(NIL_UUID)));
  ClassDB::bind_static_method(
        "UUID", D_METHOD("create_v5_stduuid_vector4i", "seed", "namespace_uuid"), &UUID::create_v5_stduuid_vector4i,
        DEFVAL(String(NIL_UUID)));

  // Conversions
  ClassDB::bind_static_method("UUID", D_METHOD("to_string_from_variant", "uuid"), &UUID::to_string_from_variant);
  ClassDB::bind_static_method("UUID", D_METHOD("to_bytes_from_variant", "uuid"), &UUID::to_bytes_from_variant);
  ClassDB::bind_static_method("UUID", D_METHOD("to_vector4i_from_variant", "uuid"), &UUID::to_vector4i_from_variant);

  // Validation
  ClassDB::bind_static_method("UUID", D_METHOD("is_nil", "uuid"), &UUID::is_nil);
  ClassDB::bind_static_method("UUID", D_METHOD("is_valid", "uuid"), &UUID::is_valid);

  ClassDB::bind_static_method("UUID", D_METHOD("get_uuid_version", "uuid"), &UUID::get_uuid_version);
  ClassDB::bind_static_method("UUID", D_METHOD("get_uuid_variant", "uuid"), &UUID::get_uuid_variant);

  ClassDB::bind_static_method("UUID", D_METHOD("equals", "uuid", "uuid"), &UUID::equals);

  // Variant map
  ClassDB::bind_method(D_METHOD("assign_value", "uuid", "value"), &UUID::assign_value);
  ClassDB::bind_method(D_METHOD("get_value", "uuid", "default_value"), &UUID::get_value, DEFVAL(Variant()));
  ClassDB::bind_method(D_METHOD("has_key", "uuid"), &UUID::has_key);
  ClassDB::bind_method(D_METHOD("has_value", "value"), &UUID::has_value);
  ClassDB::bind_method(D_METHOD("erase_key", "uuid"), &UUID::erase_key);
  ClassDB::bind_method(D_METHOD("erase_value", "value"), &UUID::erase_value);
  ClassDB::bind_method(D_METHOD("clear"), &UUID::clear);
  ClassDB::bind_method(D_METHOD("get_map_size"), &UUID::get_map_size);
  ClassDB::bind_method(D_METHOD("get_keys_as_strings"), &UUID::get_keys_as_strings);
  ClassDB::bind_method(D_METHOD("get_keys_as_bytes"), &UUID::get_keys_as_bytes);
  ClassDB::bind_method(D_METHOD("get_keys_as_vector4i"), &UUID::get_keys_as_vector4i);
  ClassDB::bind_method(D_METHOD("get_values"), &UUID::get_values);
}

// MARK: Generation v3

static PackedByteArray create_v3_bytes(const String &seed, const Variant &namespace_uuid) {
  //validate namespace first
  const auto opt = to_stduuid(namespace_uuid);
  ERR_FAIL_COND_V_MSG(!opt.has_value(), {}, "invalid namespace_uuid provided");

  const auto ns_bytes = to_bytes(opt.value());
  ERR_FAIL_COND_V_MSG(ns_bytes.is_empty(), {}, "converted byte array is empty");

  const PackedByteArray seed_bytes = seed.to_utf8_buffer();

  Ref< HashingContext > ctx;
  ctx.instantiate();
  ctx->start(HashingContext::HASH_MD5);
  ctx->update(ns_bytes);
  ctx->update(seed_bytes);
  PackedByteArray hash = ctx->finish();
  ERR_FAIL_COND_V_MSG(hash.size() != 16, {}, "Error when generating v3 uuid: size is not 16.");

  // add versiona and variant information.
  hash[ 6 ] = (hash[ 6 ] & 0x0F) | 0x30; // Version 3 (MD5)
  hash[ 8 ] = (hash[ 8 ] & 0x3F) | 0x80; // Variant 1
  return hash;
}

String UUID::create_v3_godot_string(const String &seed, const Variant &namespace_uuid) {
  return ::to_string(create_v3_bytes(seed, namespace_uuid));
}


PackedByteArray UUID::create_v3_godot_bytes(const String &seed, const Variant &namespace_uuid) {
  return create_v3_bytes(seed, namespace_uuid);
}


Vector4i UUID::create_v3_godot_vector4i(const String &seed, const Variant &namespace_uuid) {
  return ::to_vector4i(create_v3_bytes(seed, namespace_uuid));
}

// MARK: Generation v4

String UUID::create_v4_stduuid_string() {
  return ::to_string(stduuid_urg());
}


PackedByteArray UUID::create_v4_stduuid_bytes() {
  return ::to_bytes(stduuid_urg());
}


Vector4i        UUID::create_v4_stduuid_vector4i() {
  return ::to_vector4i(stduuid_urg());
}


String UUID::create_v4_uuidv4_string() {
  return ::to_string(uuidv4_generator.getUUID());
}


PackedByteArray UUID::create_v4_uuidv4_bytes() {
  return ::to_bytes(uuidv4_generator.getUUID());
}


Vector4i        UUID::create_v4_uuidv4_vector4i() {
  return ::to_vector4i(uuidv4_generator.getUUID());
}

// MARK: Generation v5

static std::optional<uuids::uuid> create_v5_stduuid(const String &seed, const Variant &namespace_uuid) {
  // verify the namespace first
  const auto opt = ::to_stduuid(namespace_uuid);
  if( ! opt.has_value() ) return {};
  const auto ns_uuid = opt.value();

  // namespace can be a nil value.
  if( ns_uuid == nil_stduuid ) {
    return stduuid_dung(seed.utf8().ptr());
  }
  auto    ns_ung = uuids::uuid_name_generator{ ns_uuid };
  return  ns_ung(seed.utf8().ptr());
}

String UUID::create_v5_stduuid_string(const String &seed, const Variant &namespace_uuid) {
  const auto opt = create_v5_stduuid(seed, namespace_uuid);
  return opt.has_value() ? ::to_string(opt.value()) : String();
}


PackedByteArray UUID::create_v5_stduuid_bytes(const String &seed, const Variant &namespace_uuid) {
  const auto opt = create_v5_stduuid(seed, namespace_uuid);
  return opt.has_value() ? ::to_bytes(opt.value()) : PackedByteArray();
}


Vector4i UUID::create_v5_stduuid_vector4i(const String &seed, const Variant &namespace_uuid) {
  const auto opt = create_v5_stduuid(seed, namespace_uuid);
  return opt.has_value() ? ::to_vector4i(opt.value()) : Vector4i();
}

// MARK: Conversion

PackedByteArray
UUID::to_bytes_from_variant(const Variant &uuid) {
  const auto opt = to_stduuid(uuid);
  return opt.has_value() ? ::to_bytes(opt.value()) : PackedByteArray();
}


String
UUID::to_string_from_variant(const Variant &uuid) {
  const auto opt = to_stduuid(uuid);
  return opt.has_value() ? ::to_string(opt.value()) : String();
}


Vector4i
UUID::to_vector4i_from_variant(const Variant &uuid) {
  const auto opt = to_stduuid(uuid);
  return opt.has_value() ? ::to_vector4i(opt.value()) : Vector4i();
}


// MARK: Checks

int64_t UUID::hash_uuid(const String &uuid_str) {
  const auto opt = to_stduuid(uuid_str);
  ERR_FAIL_COND_V_MSG(!opt.has_value(), 0, "invalid uuid provided");
  //FIXME Investigate what an appropriate return value for an error.
  return static_cast< int64_t >(std::hash< uuids::uuid >{}(opt.value() ));
}


bool UUID::is_nil(const Variant &uuid) {
  const auto opt = to_stduuid(uuid);
  return opt.has_value() ? opt.value() == nil_stduuid : false;
}


bool UUID::is_valid(const Variant &uuid) {
  return to_stduuid(uuid).has_value();
}


bool UUID::equals(const Variant &uuid_a, const Variant &uuid_b) {
  const auto uuid = to_stduuid(uuid_a);
  if(!uuid.has_value()) return false;
  return to_stduuid(uuid_a) == to_stduuid(uuid_b);
}


int UUID::get_uuid_version(const Variant &uuid) {
  const auto opt = to_stduuid(uuid);
  ERR_FAIL_COND_V_MSG(!opt.has_value(), -1, "invalid uuid provided");
  return static_cast< int >( opt.value().version());
}


int UUID::get_uuid_variant(const Variant &uuid) {
  const auto opt = to_stduuid(uuid);
  ERR_FAIL_COND_V_MSG(!opt.has_value(), -1, "invalid uuid provided");
  return static_cast< int >( opt.value().variant());
}

// MARK: Association

bool UUID::assign_value(const Variant &uuid, const Variant &value) {
  const auto opt = to_stduuid(uuid);
  ERR_FAIL_COND_V_MSG(!opt.has_value(), false, "invalid uuid provided");
  _variant_map[ opt.value() ] = value;
  return true;
}


bool UUID::has_key(const Variant &uuid) const {
  const auto opt = to_stduuid(uuid);
  ERR_FAIL_COND_V_MSG(!opt.has_value(), false, "invalid uuid provided");
  return _variant_map.contains(opt.value());
}


bool UUID::has_value(const Variant &variant) const {
  for( const auto &value : _variant_map | std::views::values ) {
    if( value == variant) {
      return true;
    }
  }
  return false;
}


int UUID::get_map_size() const {
  return static_cast< int >(_variant_map.size());
}


Variant UUID::get_value(const Variant &uuid, const Variant &default_value) const {
  const auto opt = to_stduuid(uuid);
  ERR_FAIL_COND_V_MSG(!opt.has_value(), Variant(), "invalid uuid provided");
  const auto it = _variant_map.find(opt.value());
  return it != _variant_map.end() ? it->second : default_value;
}


PackedStringArray UUID::get_keys_as_strings() const {
  PackedStringArray keys;
  for( const auto &key : _variant_map | std::views::keys ) {
    keys.append(::to_string(key));
  }
  return keys;
}


PackedByteArray UUID::get_keys_as_bytes() const {
  PackedByteArray keys;
  for( const auto &key : _variant_map | std::views::keys ) {
    keys.append_array(to_bytes(key));
  }
  return keys;
}


Array UUID::get_keys_as_vector4i() const {
  Array keys;
  for( const auto &key : _variant_map | std::views::keys ) {
    keys.append(to_vector4i(key));
  }
  return keys;
}


Array UUID::get_values() const {
  Array values;
  for( const auto &key : _variant_map | std::views::values ) {
    values.append(to_vector4i(key));
  }
  return values;
}


bool UUID::erase_key(const Variant &uuid) {
  const auto opt = to_stduuid(uuid);
  ERR_FAIL_COND_V_MSG(!opt.has_value(), false, "invalid uuid provided");
  return _variant_map.erase(opt.value()) > 0;
}


bool UUID::erase_value(const Variant &variant) {
  for( const auto &[key,value] : _variant_map) {
    if(value == variant) {
      _variant_map.erase(key);
      return true;
    }
  }
  return false;
}


void UUID::clear() {
  _variant_map.clear();
}
