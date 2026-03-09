#ifndef GODOT_FLATBUFFERS_FLATBUFFERBUILDER_HPP
#define GODOT_FLATBUFFERS_FLATBUFFERBUILDER_HPP

#include "flatbuffers/flatbuffer_builder.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot_flatbuffers {
class FlatBufferBuilder final : public godot::RefCounted {
  GDCLASS( FlatBufferBuilder, RefCounted ) // NOLINT(*-use-auto)

  std::unique_ptr< flatbuffers::FlatBufferBuilder > builder;

  using uoffset_t = flatbuffers::uoffset_t;
  using Offset    = flatbuffers::Offset< >;

protected:
  static FlatBufferBuilder *Create( int size ) {
    const auto fbb = memnew( FlatBufferBuilder( size ) );
    fbb->builder   = std::make_unique< flatbuffers::FlatBufferBuilder >( size );
    // TODO Use this 'flatbuffers::FlatBufferBuilderImpl(...)' to add a custom allocator so that we can create directly into a PackedByteArray
    //  This will make the need to copy the data after construction unnecessary
    return fbb;
  }

  static void _bind_methods();

public:
  explicit FlatBufferBuilder();

  explicit FlatBufferBuilder( int size );


  [[nodiscard]] int64_t GetSize() const { return builder->GetSize(); }

  [[nodiscard]] godot::PackedByteArray GetPackedByteArray() const;

  void Clear() const { builder->Clear(); }
  void Reset() const { builder->Reset(); }

  void Finish( uint32_t root ) const;

  [[nodiscard]] uoffset_t StartTable() const { return builder->StartTable(); }
  [[nodiscard]] uoffset_t EndTable( const uoffset_t start ) const { return builder->EndTable( start ); }

  // Add is used when the data is inline
  // Create is used when the data is offset.


  // Add Offsets
  void AddOffset( uint16_t voffset, uint64_t value ) const;

  // Add Arrays of bytes
  void AddBytes( uint16_t voffset, const godot::PackedByteArray &bytes ) const;


  // Vector of offsets
  [[nodiscard]] uoffset_t CreateVectorOffset( const godot::PackedInt32Array &array ) const;

  // Custom Class to Table Creators
  [[nodiscard]] uoffset_t CreateVectorTable( const godot::Array &array, const godot::Callable &constructor ) const;


  // The full range of godot types:

  // Add / Create Scalars
  // Scalars
  // bool
  // int64_t
  // int32_t
  // int16_t
  // int8_t
  // uint64_t
  // uint32_t
  // uint16_t
  // uint8_t
  // double
  // float
  template< typename in, typename out >
  void AddScalar( uint16_t voffset, in value ) { builder->AddElement< out >( voffset, value ); }

  template< typename in, typename out >
  void AddScalarDefault( uint16_t voffset, in value, in def ) { builder->AddElement< out >( voffset, value, def ); }


  // Add / Create Structs
  // Below are the list of godot types that are represented as structs in the flatbuffer object.
  // Vector2
  // Vector2i
  // Rect2
  // Rect2i
  // Vector3
  // Vector3i
  // Transform2D
  // Vector4
  // Vector4i
  // Plane
  // Quaternion
  // AABB
  // Basis
  // Transform3D
  // Projection
  // Color
  template< typename godot_type >
  void AddGodotStruct( uint16_t voffset, const godot_type &value ) { builder->AddStruct( voffset, &value ); }

  // When structs need to be added as nullable, or part of a union, such that they are accessed via an offset
  // Then we create
  template< typename godot_type >
  uoffset_t CreateGodotStruct( const godot_type &value ) { return builder->CreateStruct( &value ).o; }


  // String
  [[nodiscard]] uoffset_t CreateString( const godot::String &string ) const;

  // StringName
  // NodePath
  // godot::RID
  // ObjectID

  // Object *
  // Callable
  // Signal
  // Dictionary
  // Array

  // -> Vectors
  // PackedByteArray
  // PackedInt32Array
  // PackedInt64Array
  // PackedFloat32Array
  // PackedFloat64Array
  template< typename T >
  uoffset_t CreatePackedArray( const godot::Array &v ) {
    builder->StartVector< T >( v.size() );
    for( auto i = v.size(); i > 0; ) {
      builder->PushElement( static_cast< T >(v[ --i ]) );
    }
    return builder->EndVector( v.size() );
  }

  // PackedStringArray <-> VectorOfString
  [[nodiscard]] uoffset_t CreatePackedStringArray( const godot::PackedStringArray &value ) const;

  // PackedArray's are fairly easy to deal with as they are contiguous containers.
  // PackedVector2Array
  // PackedVector3Array
  // PackedColorArray
  // PackedVector4Array
  template<typename packed_type>
  [[nodiscard]] uoffset_t CreateVectorOfStructs( const packed_type &value ) const {
    return builder->CreateVectorOfStructs(&value, value.size() ).o;
  }

};

}  // namespace godot_flatbuffers
#endif //GODOT_FLATBUFFERS_FLATBUFFERBUILDER_HPP
