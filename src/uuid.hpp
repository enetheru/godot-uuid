#ifndef GODOT_FLATBUFFERS_EXTENSION_UUID_HPP
#define GODOT_FLATBUFFERS_EXTENSION_UUID_HPP

#include <godot_cpp/classes/random_number_generator.hpp>
#include "godot_cpp/classes/hashing_context.hpp"
#include "godot_cpp/core/class_db.hpp"

#include <ranges>
#include <unordered_map>

#include <uuid.h>
#include <uuid_v4.h>

namespace godot_flatbuffers {

using godot::Array;

using godot::RandomNumberGenerator;
using godot::RefCounted;
using godot::String;
using godot::StringName;
using godot::Variant;
using godot::Vector4i;
using godot::PackedByteArray;
using godot::PackedStringArray;

inline constexpr auto NIL_UUID  = "00000000-0000-0000-0000-000000000000";
inline constexpr auto MAX_UUID  = "ffffffff-ffff-ffff-ffff-ffffffffffff";
inline constexpr auto DNS_UUID  = "6ba7b810-9dad-11d1-80b4-00c04fd430c8";
inline constexpr auto URL_UUID  = "6ba7b811-9dad-11d1-80b4-00c04fd430c8";
inline constexpr auto OID_UUID  = "6ba7b812-9dad-11d1-80b4-00c04fd430c8";
inline constexpr auto x500_UUID = "6ba7b814-9dad-11d1-80b4-00c04fd430c8";

/**
 * UUID
 *
 * Utility class for generating, parsing, and handling RFC 4122 UUIDs.
 * Provides random (v4), name-based (v3 MD5, v5 SHA-1), and nil UUIDs.
 * Supports conversion between string, byte array, and Vector4i representations.
 * Includes a simple Variant dictionary keyed by UUID.
 *
 * @experimental This API surface may change in future versions.
 */
class UUID final : public RefCounted {
  GDCLASS(UUID, RefCounted);

  static bool initialised;
  // RNG Sources
  static godot::Ref< RandomNumberGenerator > godot_rng;
  // UUID generators
  static uuids::uuid_name_generator stduuid_dung; // Default Unique Name Generator (DUNG)
  static std::unique_ptr< uuids::basic_uuid_random_generator< RandomNumberGenerator > >
                                                  stduuid_burg; // Basic UUID Random Generator (BURG)
  std::unordered_map< uuids::uuid, Variant > _variant_map;

protected:
  static void _bind_methods();

public:
  // ───────────────────────────────────────────────
  //  Namespace constants (RFC 9562 recommended)
  // ───────────────────────────────────────────────
  static String get_nil_uuid() { return NIL_UUID; }
  static String get_max_uuid() { return MAX_UUID; }
  static String get_namespace_dns() { return DNS_UUID; }
  static String get_namespace_url() { return URL_UUID; }
  static String get_namespace_oid() { return OID_UUID; }
  static String get_namespace_x500() { return x500_UUID; }

  // ───────────────────────────────────────────────
  //  Hashing
  // ───────────────────────────────────────────────
  static int64_t hash_uuid(const String &uuid_str);

  // ───────────────────────────────────────────────
  //  Creators
  // ───────────────────────────────────────────────

  // v3 (Godot MD5 only)
  static String          create_v3_godot_string(const String &seed, const Variant &namespace_uuid = String(NIL_UUID));
  static PackedByteArray create_v3_godot_bytes(const String &seed, const Variant &namespace_uuid = String(NIL_UUID));
  static Vector4i        create_v3_godot_vector4i(const String &seed, const Variant &namespace_uuid = String(NIL_UUID));

  // String / Bytes wrappers (unchanged naming)
  static String          create_v4_stduuid_string();
  static PackedByteArray create_v4_stduuid_bytes();
  static Vector4i        create_v4_stduuid_vector4i();

  static String          create_v4_uuidv4_string();
  static PackedByteArray create_v4_uuidv4_bytes();
  static Vector4i        create_v4_uuidv4_vector4i();

  static String          create_v5_stduuid_string(const String &seed, const Variant &namespace_uuid = String(NIL_UUID));
  static PackedByteArray create_v5_stduuid_bytes(const String &seed, const Variant &namespace_uuid = String(NIL_UUID));
  static Vector4i        create_v5_stduuid_vector4i(const String &seed, const Variant &namespace_uuid = String(NIL_UUID));

  // ───────────────────────────────────────────────
  //  Conversion
  // ───────────────────────────────────────────────
  static PackedByteArray to_bytes_from_variant(const Variant &uuid);
  static String          to_string_from_variant(const Variant &uuid);
  static Vector4i        to_vector4i_from_variant(const Variant &uuid);

  // ───────────────────────────────────────────────
  //  Validation / Inspection
  // ───────────────────────────────────────────────
  static bool is_nil(const Variant &uuid);

  static bool is_valid(const Variant &uuid);
  static bool equals(const Variant &uuid_a, const Variant &uuid_b);

  static int  get_uuid_version(const Variant &uuid);
  static int  get_uuid_variant(const Variant &uuid);

  // ───────────────────────────────────────────────
  //  Variant map
  // ───────────────────────────────────────────────
  //    _      _    _
  //   /_\  __| |__| |
  //  / _ \/ _` / _` |
  // /_/ \_\__,_\__,_|
  bool assign_value(const Variant &uuid, const Variant &value); // true if uuid_str is valid, else false.

  //  _  _
  // | || |__ _ ___
  // | __ / _` (_-<
  // |_||_\__,_/__/

  bool has_key(const Variant &uuid) const;
  bool has_value(const Variant &variant) const;

  //   ___     _
  //  / __|___| |_
  // | (_ / -_)  _|
  //  \___\___|\__|
  int get_map_size() const;

  Variant get_value(const Variant &uuid, const Variant &default_value = Variant()) const;

  PackedStringArray get_keys_as_strings() const;
  PackedByteArray get_keys_as_bytes() const;
  Array get_keys_as_vector4i() const;

  Array get_values() const;

  //  ___
  // | _ \___ _ __
  // |   / -_) '  \
  // |_|_\___|_|_|_|
  bool  erase_key(const Variant &uuid);
  bool  erase_value(const Variant &value);

  //  ___      _
  // |   \ ___| |
  // | |) / -_) |
  // |___/\___|_|
  void  clear();
};
;
} // namespace godot_flatbuffers

#endif
