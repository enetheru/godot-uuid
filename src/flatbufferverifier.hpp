#ifndef GODOT_FLATBUFFERS_FLATBUFFERVERIFIER_HPP
#define GODOT_FLATBUFFERS_FLATBUFFERVERIFIER_HPP

#include <memory>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "flatbuffers/verifier.h"
#include "godot_cpp/variant/variant_internal.hpp"

namespace godot_flatbuffers {

  /*
   * There are a few factors that went in to my descision to re-implement the verifier
   * 1. changing the data - i dont want to have to allocate a new object for
   *    every buffer that needs verifying.
   * 2. We cannot derive the size of a field using template specialisation.
   */
class FlatBufferVerifier final : public godot::RefCounted {
  GDCLASS( FlatBufferVerifier, RefCounted ) // NOLINT(*-use-auto)

  using uoffset_t = flatbuffers::uoffset_t;
  using soffset_t = flatbuffers::soffset_t;
  using voffset_t = flatbuffers::voffset_t;

  std::unique_ptr<flatbuffers::Verifier> verifier;

protected:
  static void _bind_methods(){
    using namespace godot;

    ClassDB::bind_method( D_METHOD( "end_table"), &FlatBufferVerifier::end_table );
    ClassDB::bind_method( D_METHOD( "set_buffer", "byte_array"), &FlatBufferVerifier::set_buffer );

  }

public:
  explicit FlatBufferVerifier() {}

  // This extremely volatile, the pointers to the memory will be invalid if the array is changed.
  // I think it should be OK, it just makes it harder to test
  void set_buffer( const godot::PackedByteArray &byte_array )
  {
    verifier = std::make_unique< flatbuffers::Verifier >( byte_array.ptr(), byte_array.size());
  }

  // Central location where any verification failures register.

  // // Central location where any verification failures register.
  // bool Check(const bool ok) const {
  //   // clang-format off
  //   #ifdef FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
  //     if (opts_.assert) { FLATBUFFERS_ASSERT(ok); }
  //   #endif
  //   // clang-format on
  //   if (TrackVerifierBufferSize) {
  //     if (!ok) {
  //       upper_bound_ = 0;
  //     }
  //   }
  //   return ok;
  // }
  //
  // // Verify any range within the buffer.
  // bool Verify(const size_t elem, const size_t elem_len) const {
  //   if (TrackVerifierBufferSize) {
  //     auto upper_bound = elem + elem_len;
  //     if (upper_bound_ < upper_bound) {
  //       upper_bound_ = upper_bound;
  //     }
  //   }
  //   return Check(elem_len < size_ && elem <= size_ - elem_len);
  // }
  //
  // bool VerifyAlignment(const size_t elem, const size_t align) const {
  //   return Check((elem & (align - 1)) == 0 || !opts_.check_alignment);
  // }
  //
  // // Verify a range indicated by sizeof(T).
  // template <typename T>
  // bool Verify(const size_t elem) const {
  //   return VerifyAlignment(elem, sizeof(T)) && Verify(elem, sizeof(T));
  // }
  //
  // bool VerifyFromPointer(const uint8_t* const p, const size_t len) {
  //   return Verify(static_cast<size_t>(p - buf_), len);
  // }
  //
  // // Verify relative to a known-good base pointer.
  // bool VerifyFieldStruct(const uint8_t* const base, const voffset_t elem_off,
  //                        const size_t elem_len, const size_t align) const {
  //   const auto f = static_cast<size_t>(base - buf_) + elem_off;
  //   return VerifyAlignment(f, align) && Verify(f, elem_len);
  // }
  //
  template <typename T>
  bool verify_field(const uint8_t* const base, const voffset_t elem_off,
                   const size_t align) const
  {
    return verifier->VerifyField<T>(base, elem_off, align);
  }
  //
  // // Verify a pointer (may be NULL) of a table type.
  // template <typename T>
  // bool VerifyTable(const T* const table) {
  //   return !table || table->Verify(*this);
  // }
  //
  // // Verify a pointer (may be NULL) of any vector type.
  // template <int&..., typename T, typename LenT>
  // bool VerifyVector(const Vector<T, LenT>* const vec) const {
  //   return !vec || VerifyVectorOrString<LenT>(
  //                      reinterpret_cast<const uint8_t*>(vec), sizeof(T));
  // }
  //
  // // Verify a pointer (may be NULL) of a vector to struct.
  // template <int&..., typename T, typename LenT>
  // bool VerifyVector(const Vector<const T*, LenT>* const vec) const {
  //   return VerifyVector(reinterpret_cast<const Vector<T, LenT>*>(vec));
  // }
  //
  // // Verify a pointer (may be NULL) to string.
  // bool VerifyString(const String* const str) const {
  //   size_t end;
  //   return !str || (VerifyVectorOrString<uoffset_t>(
  //                       reinterpret_cast<const uint8_t*>(str), 1, &end) &&
  //                   Verify(end, 1) &&           // Must have terminator
  //                   Check(buf_[end] == '\0'));  // Terminating byte must be 0.
  // }
  //
  // // Common code between vectors and strings.
  // template <typename LenT = uoffset_t>
  // bool VerifyVectorOrString(const uint8_t* const vec, const size_t elem_size,
  //                           size_t* const end = nullptr) const {
  //   const auto vec_offset = static_cast<size_t>(vec - buf_);
  //   // Check we can read the size field.
  //   if (!Verify<LenT>(vec_offset)) return false;
  //   // Check the whole array. If this is a string, the byte past the array must
  //   // be 0.
  //   const LenT size = ReadScalar<LenT>(vec);
  //   const auto max_elems = opts_.max_size / elem_size;
  //   if (!Check(size < max_elems))
  //     return false;  // Protect against byte_size overflowing.
  //   const auto byte_size = sizeof(LenT) + elem_size * size;
  //   if (end) *end = vec_offset + byte_size;
  //   return Verify(vec_offset, byte_size);
  // }
  //
  // // Special case for string contents, after the above has been called.
  // bool VerifyVectorOfStrings(const Vector<Offset<String>>* const vec) const {
  //   if (vec) {
  //     for (uoffset_t i = 0; i < vec->size(); i++) {
  //       if (!VerifyString(vec->Get(i))) return false;
  //     }
  //   }
  //   return true;
  // }
  //
  // // Special case for table contents, after the above has been called.
  // template <typename T>
  // bool VerifyVectorOfTables(const Vector<Offset<T>>* const vec) {
  //   if (vec) {
  //     for (uoffset_t i = 0; i < vec->size(); i++) {
  //       if (!vec->Get(i)->Verify(*this)) return false;
  //     }
  //   }
  //   return true;
  // }
  //

  bool verify_table_start(const uint8_t* const table) const
  {
    return verifier->VerifyTableStart(table);
  }

  //
  // template <typename T>
  // bool VerifyBufferFromStart(const char* const identifier, const size_t start) {
  //   // Buffers have to be of some size to be valid. The reason it is a runtime
  //   // check instead of static_assert, is that nested flatbuffers go through
  //   // this call and their size is determined at runtime.
  //   if (!Check(size_ >= FLATBUFFERS_MIN_BUFFER_SIZE)) return false;
  //
  //   // If an identifier is provided, check that we have a buffer
  //   if (identifier && !Check((size_ >= 2 * sizeof(flatbuffers::uoffset_t) &&
  //                             BufferHasIdentifier(buf_ + start, identifier)))) {
  //     return false;
  //   }
  //
  //   // Call T::Verify, which must be in the generated code for this type.
  //   const auto o = VerifyOffset<uoffset_t>(start);
  //   if (!Check(o != 0)) return false;
  //   if (!(reinterpret_cast<const T*>(buf_ + start + o)->Verify(*this))) {
  //     return false;
  //   }
  //   if (TrackVerifierBufferSize) {
  //     if (GetComputedSize() == 0) return false;
  //   }
  //   return true;
  // }
  //
  // template <typename T, int&..., typename SizeT>
  // bool VerifyNestedFlatBuffer(const Vector<uint8_t, SizeT>* const buf,
  //                             const char* const identifier) {
  //   // Caller opted out of this.
  //   if (!opts_.check_nested_flatbuffers) return true;
  //
  //   // An empty buffer is OK as it indicates not present.
  //   if (!buf) return true;
  //
  //   // If there is a nested buffer, it must be greater than the min size.
  //   if (!Check(buf->size() >= FLATBUFFERS_MIN_BUFFER_SIZE)) return false;
  //
  //   VerifierTemplate<TrackVerifierBufferSize> nested_verifier(
  //       buf->data(), buf->size(), opts_);
  //   return nested_verifier.VerifyBuffer<T>(identifier);
  // }
  //
  // // Verify this whole buffer, starting with root type T.
  // template <typename T>
  // bool VerifyBuffer() {
  //   return VerifyBuffer<T>(nullptr);
  // }
  //
  // template <typename T>
  // bool VerifyBuffer(const char* const identifier) {
  //   return VerifyBufferFromStart<T>(identifier, 0);
  // }
  //
  // template <typename T, typename SizeT = uoffset_t>
  // bool VerifySizePrefixedBuffer(const char* const identifier) {
  //   return Verify<SizeT>(0U) &&
  //          // Ensure the prefixed size is within the bounds of the provided
  //          // length.
  //          Check(ReadScalar<SizeT>(buf_) + sizeof(SizeT) <= size_) &&
  //          VerifyBufferFromStart<T>(identifier, sizeof(SizeT));
  // }
  //
  // template <typename OffsetT = uoffset_t, typename SOffsetT = soffset_t>
  // size_t VerifyOffset(const size_t start) const {
  //   if (!Verify<OffsetT>(start)) return 0;
  //   const auto o = ReadScalar<OffsetT>(buf_ + start);
  //   // May not point to itself.
  //   if (!Check(o != 0)) return 0;
  //   // Can't wrap around larger than the max size.
  //   if (!Check(static_cast<SOffsetT>(o) >= 0)) return 0;
  //   // Must be inside the buffer to create a pointer from it (pointer outside
  //   // buffer is UB).
  //   if (!Verify(start + o, 1)) return 0;
  //   return o;
  // }
  //
  // template <typename OffsetT = uoffset_t>
  // size_t VerifyOffset(const uint8_t* const base, const voffset_t start) const {
  //   return VerifyOffset<OffsetT>(static_cast<size_t>(base - buf_) + start);
  // }
  //
  // // Called at the start of a table to increase counters measuring data
  // // structure depth and amount, and possibly bails out with false if limits set
  // // by the constructor have been hit. Needs to be balanced with EndTable().
  // bool VerifyComplexity() {
  //   depth_++;
  //   num_tables_++;
  //   return Check(depth_ <= opts_.max_depth && num_tables_ <= opts_.max_tables);
  // }
  //
  // Called at the end of a table to pop the depth count.
  bool end_table() {
    return verifier->EndTable();
  }
  //
  // // Returns the message size in bytes.
  // //
  // // This should only be called after first calling VerifyBuffer or
  // // VerifySizePrefixedBuffer.
  // //
  // // This method should only be called for VerifierTemplate instances
  // // where the TrackVerifierBufferSize template parameter is true,
  // // i.e. for SizeVerifier.  For instances where TrackVerifierBufferSize
  // // is false, this fails at runtime or returns zero.
  // size_t GetComputedSize() const {
  //   if (TrackVerifierBufferSize) {
  //     uintptr_t size = upper_bound_;
  //     // Align the size to uoffset_t
  //     size = (size - 1 + sizeof(uoffset_t)) & ~(sizeof(uoffset_t) - 1);
  //     return (size > size_) ? 0 : size;
  //   }
  //   // Must use SizeVerifier, or (deprecated) turn on
  //   // FLATBUFFERS_TRACK_VERIFIER_BUFFER_SIZE, for this to work.
  //   (void)upper_bound_;
  //   FLATBUFFERS_ASSERT(false);
  //   return 0;
  // }
  //
  // std::vector<uint8_t>* GetFlexReuseTracker() { return flex_reuse_tracker_; }
  //
  // void SetFlexReuseTracker(std::vector<uint8_t>* const rt) {
  //   flex_reuse_tracker_ = rt;
  // }


};

}
#endif //GODOT_FLATBUFFERS_FLATBUFFERVERIFIER_HPP
