
## Using different string type

By default the object tree is built out of `std::string`, but you can influence this either globally (using the `--cpp-str-type` argument to `flatc`) or per field using the `cpp_str_type` attribute.

The type must support `T::c_str()`, `T::length()` and `T::empty()` as member functions.

Further, the type must be constructible from std::string, as by default a std::string instance is constructed and then used to initialize the custom string type. This behavior impedes efficient and zero-copy construction of custom string types; the `--cpp-str-flex-ctor` argument to `flatc` or the per field attribute `cpp_str_flex_ctor` can be used to change this behavior, so that the custom string type is constructed by passing the pointer and length of the FlatBuffers String. The custom string class will require a constructor in the following format: `custom_str_class(const char *, size_t)`. Please note that the character array is not guaranteed to be NULL terminated, you should always use the provided size to determine end of string.

```cpp
// REQUIRES
// T::c_str(), 
// T::length()
// T::empty()  

godot::String my_string;  
my_string.length(); //OK  
my_string.c_str(); // BROKEN  
my_string.is_empty(); //BROKEN  
  
godot::CharString my_charstring;  
my_charstring.length(); //OK  
my_charstring.c_str(); //BROKEN  
my_charstring.empty(); //BROKEN
```
## Storing maps / dictionaries in a FlatBuffer

FlatBuffers doesn't support maps natively, but there is support to emulate their behavior with vectors and binary search, which means you can have fast lookups directly from a FlatBuffer without having to unpack your data into a `std::map` or similar.

To use it: - Designate one of the fields in a table as they "key" field. You do this by setting the `key` attribute on this field, e.g. `name:string (key)`. You may only have one key field, and it must be of string or scalar type. - Write out tables of this type as usual, collect their offsets in an array or vector. - Instead of `CreateVector`, call `CreateVectorOfSortedTables`, which will first sort all offsets such that the tables they refer to are sorted by the key field, then serialize it. - Now when you're accessing the FlatBuffer, you can use `Vector::LookupByKey` instead of just `Vector::Get` to access elements of the vector, e.g.: `myvector->LookupByKey("Fred")`, which returns a pointer to the corresponding table type, or `nullptr` if not found. `LookupByKey` performs a binary search, so should have a similar speed to `std::map`, though may be faster because of better caching. `LookupByKey` only works if the vector has been sorted, it will likely not find elements if it hasn't been sorted.

## Access of untrusted buffers

The generated accessor functions access fields over offsets, which is very quick. These offsets are not verified at run-time, so a malformed buffer could cause a program to crash by accessing random memory.

When you're processing large amounts of data from a source you know (e.g. your own generated data on disk), this is acceptable, but when reading data from the network that can potentially have been modified by an attacker, this is undesirable.

For this reason, you can optionally use a buffer verifier before you access the data. This verifier will check all offsets, all sizes of fields, and null termination of strings to ensure that when a buffer is accessed, all reads will end up inside the buffer.

Each root type will have a verification function generated for it, e.g. for `Monster`, you can call:

    `Verifier verifier(buf, len);     bool ok = VerifyMonsterBuffer(verifier);`

if `ok` is true, the buffer is safe to read.

Besides untrusted data, this function may be useful to call in debug mode, as extra insurance against data being corrupted somewhere along the way.

While verifying a buffer isn't "free", it is typically faster than a full traversal (since any scalar data is not actually touched), and since it may cause the buffer to be brought into cache before reading, the actual overhead may be even lower than expected.

In specialized cases where a denial of service attack is possible, the verifier has two additional constructor arguments that allow you to limit the nesting depth and total amount of tables the verifier may encounter before declaring the buffer malformed. The default is `Verifier(buf, len, 64 /* max depth */, 1000000, /* max tables */)` which should be sufficient for most uses.