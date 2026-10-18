# Metakit Tips

Metakit behaviors that have caught us out in this codebase, with the correction each one implies. Applies to the vendored copy under `metakit/`.

## Sorted and Filtered Views Silently Drop Writes

`SortOn`, `SortOnReverse`, `Sort`, `Select` and `SelectRange` all return *derived* views: a permutation over the view they were called on, not a copy of its rows. Their sequences are `c4_FilterSeq`s (`c4_SortSeq` subclasses it without adding any mutators), and every `c4_FilterSeq` mutator — `Set`, `InsertAt`, `RemoveAt`, `SetSize` — is just `d4_assert(0)` (`metakit/src/derived.cpp:249-263`).

`d4_assert` compiles to nothing unless `q4_CHECK` is defined (`metakit/src/header.h:119-122`). Only Metakit's MSVC debug configuration defines it (`metakit/src/msvc.h:33-34`). PortaBase builds Metakit with `unix/configure` on every platform, without it. So a write through a derived view neither takes effect nor reports an error:

```cpp
c4_View sorted = data.SortOn(Id);
for (int i = 0; i < sorted.GetSize(); i++) {
    Id (sorted[i]) = i;   // silently does nothing
}
```

Reading through a derived view is fine. To write, map each row back to the base view with `GetIndexOf` and write there. Collect all the indices *before* writing any of them: a derived view stays attached to its base through change notification, so writing to the base can re-sort or re-filter the derived view under a loop that is still walking it.

```cpp
c4_View sorted = data.SortOn(Id);
int size = sorted.GetSize();
int *indices = new int[size];
int i;
for (i = 0; i < size; i++) {
    indices[i] = data.GetIndexOf(sorted[i]);
}
for (i = 0; i < size; i++) {
    Id (data[indices[i]]) = i;
}
delete[] indices;
```

`Database::compressRowIds` (`src/database.cpp`) is the worked example. It used the first pattern from 2011 until ROW-DB-011 fixed it, so every bulk delete in that period left the surviving row IDs unchanged. That was the origin of the non-consecutive row IDs in real files (`docs/intent/row-lifecycle/row-lifecycle-design.md`, Resolved item 2).

A sweep of `src/` on 2026-10-17 found no other writes through a derived view. It looked for property assignments, `c4_RowRef`s taken from derived views, and mutator calls. The `Condition` filters `Add` rows to a `Clone()`, which is a standalone view, not to a `Select` result.
