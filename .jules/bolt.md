## 2024-07-16 - Replace CRC32 bitwise loop with table-driven lookup
**Learning:** CRC32 calculation using a bitwise inner loop is a significant performance bottleneck when processing large payloads (like video streams). A 256-entry precomputed lookup table provides an immediate 4x speedup in C without sacrificing readability.
**Action:** Whenever a CRC or hashing algorithm is implemented in software without hardware acceleration, check if a lookup table or chunked approach can be used instead of a naive bit-by-bit loop.

## 2024-05-18 - [Pointer Aliasing in struct Arrays]
**Learning:** In C, dereferencing pointers nested in structures inside tight loops (like `i->index[k]` or `v->index.index[k]`) forces the compiler to defensively reload the base pointer on every iteration due to potential pointer aliasing. This significantly impedes memory copy and loop vectorization performance.
**Action:** Always extract pointers from structs into local `const` or restricted variables outside inner data packing/unpacking loops to give the compiler aliasing guarantees and enable better optimization.
