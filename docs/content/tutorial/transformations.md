# Applying Transformations

Every transformation is a functor constructed on a graph, and optionally a
[property processor](property_processors.md):

```cpp
trs::vertex_merger::From<Deg1, Deg1, G> merge {gr};
```

## Three ways to apply

Each functor checks its preconditions before modifying the graph, at a cost
independent of the graph size[^1], and offers three ways of applying it:

```cpp
merge(s1, s2);                     // (1)!

if (merge.is_applicable(s1, s2))   // (2)!
    ...

if (auto res = merge.try_apply(s1, s2); !res)   // (3)!
    std::cout << res.error().reason;
```

1.  **Call operator.** Checks, then applies. Throws
    `graffine::precondition_error` if the transformation is not applicable.
2.  **`is_applicable`.** Only checks; never modifies the graph.
3.  **`try_apply`.** Checks, then applies. Returns a `std::expected` holding
    the result, or a `Rejection` whose `reason` explains why it was not
    applicable. Does not throw or allocate on rejection.

A **rejected transformation leaves the graph unchanged**.

`try_apply` suits simulations that propose many candidate moves, most of which
may be rejected.

There is also `apply(...)`, which performs the transformation **without**
checks, for arguments already known to be applicable.

[^1]: Except for pulling from a degree 2 vertex, whose checks copy the path.

## Degree-specific and degree-independent functors

Most families come in two forms:

- **Degree-specific** templates, e.g. `vertex_merger::From<Deg1, Deg2, G>` or
  `vertex_split::To<Deg1, Deg1, G>`, which handle one combination of vertex
  degrees and accept slots or vertices.
- A **wrapper** `Functor<G>`, which inspects the degrees of its arguments and
  dispatches to the right specific functor, rejecting unsupported
  combinations.

```cpp
trs::vertex_merger::Functor<G> merge {gr};
merge.try_apply(v1, v2);   // any supported pair of degrees
```

## Results

Each family defines its result type `Res` as a `std::array` of pointers or
ids to the elements it produced, e.g. the merged vertex, the new edge or the
new component's `CmpId`. See the
[Transformation Catalogue](../api/transforms.md).

## Verbosity

With `GRAFFINE_FUNCTORS_VERBOSE` enabled (see
[Configuration Options](../build_install/configuration.md)), functors report
what they do through `graffine::msgr`.
