# mycraft-vulkan

## Compiling

This project uses `vcpkg` to install dependencies. Make sure you already have
`vcpkg` and the Vulkan SDK (not required if the required tools are already
installed separately) set up, then choose one of the following:

### Debug Build

```bash
cd <project directory>
cmake --preset debug
cmake --build --preset debug
```

### Release Build

```bash
cd <project directory>
cmake --preset release
cmake --build --preset release
```

The presets use `clang++` by default.

## Development

After configuring the project, a `compile_commands.json` symlink will be created
in the project root, which allows it to be easily picked up by `clangd`.

### Guidelines

1. When you change the warning flags to disable for Clang in `.clangd` in
   `CompileFlags.Add`, remember to sync those changes to `CMakeLists.txt` in the
   `CLANG_WARNINGS` variable as well.

### Error Handling

This project uses Boost's
[LEAF](https://www.boost.org/doc/libs/latest/libs/leaf/doc/html/index.html) for
error handling instead of exceptions. It offers a middle ground between
exceptions and Rust's `Result`. Exceptions make error handling implicit, and it
becomes hard to tell which part of the code can throw errors and which part
handles them. Rust's `Result` makes error handling explicit, and functions
returning a `Result` must specify what type of error it can return.
`boost::leaf::result` makes error handling explicit, but doesn't require
functions to return a specific error type. Compare `Result<T, E>` with
`boost::leaf::result<T>`. The latter doesn't specify the error type.

This can be a good or a bad thing, though. It offers more flexibility over
`Result`, but makes it unclear what error type to handle if you call a function
returning a `boost::leaf::result`, since `boost::leaf::result` allows you to
attach an arbitrary number of error objects of arbitrary types. To make it more
sensible, this project applies following rules on top of LEAF:

1. Any function returning a `boost::leaf::result`, in the case of an error, must
   return an enum as an error object that specifies the type of error that
   occurred, called the *error enum variant*. The type of the error enum variant
   must be documented in the function's documentation, called the *error enum
   type*.

2. Because C++ doesn't have Rust-like enums that allow you to attach extra
   information along with an enum variant, if the enum error variant requires
   extra information to fully describe the error, there should be an *error info
   object* that gets returned along with that error enum variant in the
   `boost::leaf::result`. The type of the error info object is called its *error
   info type*, and is always named as `<error enum type name><error enum variant
   name>Info`. An error info object doesn't have to always accompany the error
   enum variant if the error enum variant only requires extra information
   sometimes.

3. There can be more than the above two error objects attached to a
   `boost::leaf::result` to return information not specific to any particular
   error type. For example, the `BOOST_LEAF_NEW_ERROR` macro always attaches an
   extra `e_source_location` object.

4. If a `boost::leaf::result` needs to wrap another `boost::leaf::result`, store
   the wrapped `boost::leaf::result` in a error info type, and document which
   function returns that wrapped `boost::leaf::result`, so users know how to
   handle it.

For example, if you have this function:

```cpp
auto read_file(const std::string &path) -> boost::leaf::result<File>;
```

It may return this error enum type:

```cpp
enum class FileReadError { FileNotFound, NoPermission };
```

`read_file` should document that `FileReadError` is the error enum type it
returns in case of an error.

If `FileReadError::FileNotFound` needs to describe which path segment doesn't
exist, it can use this error info type:

```cpp
struct FileReadErrorFileNotFoundInfo {
    std::string non_existent_path_segment;
};
```

Then, to handle all `FileReadError` variants, you can accept the type
`FileReadError` in the handler. To handle `FileReadError::FileNotFound`
specifically and also acquire its `FileReadErrorFileNotFoundInfo`, you can
accept `boost::leaf::match<FileReadError, FileReadError::FileNotFound>,
FileReadErrorFileNotFoundInfo` in the handler.
