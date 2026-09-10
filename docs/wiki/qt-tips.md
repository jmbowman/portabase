# Qt Tips

Qt behaviors that have caught us out in this codebase, with the correction each one implies. Qt 5.15 unless noted.

## Implicit Sharing: Cheap to Pass, Expensive to Touch

Qt's containers (`QList`, `QMap`, `QString`, `QByteArray`, …) are implicitly shared. Copying one — passing it by value, returning it, assigning it — only bumps a reference count. The deep copy is deferred until something *mutates* the shared data, and it happens on the first call to a **non-const** member.

The trap is that the mutating call often does not look like one:

```cpp
bool isPermutation(QList<int> values)      // free so far: just a refcount bump
{
    std::sort(values.begin(), values.end());  // non-const begin() detaches: full copy
    ...
}
```

`std::sort` needs mutable iterators, so it picks the non-const `begin()` overload, and that detaches. The cost is a full duplication of the caller's list on every call, invisible at the call site.

Two consequences worth internalising:

- **Passing by const reference is not by itself a fix.** If the function still needs to mutate what it was given, it has to copy — the allocation moves, it does not disappear. The fix is to find an algorithm that does not mutate, or to have the caller own the mutation once instead of paying for it per call.
- **`const` on the local matters.** `const QList<int> &v` or a `const` local calls the const overloads of `begin()`/`end()`/`operator[]`, which never detach. A non-const local silently can.

Prefer `at(i)` over `operator[]` on a non-const container for the same reason: `at()` is const-only and never detaches.

For the concrete case this came from — replacing a sort-based permutation check with a non-mutating bitset pass — see the "Permutation checking" row in `docs/intent/integrity-check/integrity-check-design.md`'s Decisions & Alternatives table.
