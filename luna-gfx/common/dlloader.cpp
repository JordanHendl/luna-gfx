#include "luna-gfx/common/dlloader.hpp"
#include "luna-gfx/error/error.hpp"

#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <exception>

#ifdef _WIN32
#include <windows.h>
typedef UINT(CALLBACK* LPFNDLLFUNC1)(DWORD, UINT);
using LibHandle = HINSTANCE;
using SymHandle = LPFNDLLFUNC1;

static inline LibHandle loadSharedObject(const char* input) {
  return LoadLibrary(input);
}

static inline const char* getError() {
  return "Windows Error handler not yet implemented.";
}

static inline SymHandle loadSymbol(LibHandle& handle, const char* symbol_name) {
  return reinterpret_cast<SymHandle>(GetProcAddress(handle, symbol_name));
}

static inline void releaseHandle(LibHandle handle) { FreeLibrary(handle); }

#elif __linux__
#include <dlfcn.h>
using LibHandle = void*;
using SymHandle = void*;
static inline LibHandle loadSharedObject(const char* input) {
  return dlopen(input, RTLD_LAZY);
}

static inline const char* getError() { return dlerror(); }

static inline SymHandle loadSymbol(LibHandle& handle, const char* symbol_name) {
  return dlsym(handle, symbol_name);
}

static inline void releaseHandle(LibHandle handle) { dlclose(handle); }

#endif

namespace luna {
namespace gfx {
struct Dlloader::DlloaderData {
  using FunctionMap = std::map<std::string, Dlloader::DL_FUNC>;
  using SharedLibraryMap = std::map<std::string, bool>;

  LibHandle handle;
  FunctionMap map;
  std::string soname;

  DlloaderData() { this->handle = NULL; }
  ~DlloaderData() {
    if (this->handle) {
      ::releaseHandle(this->handle);
      this->handle = nullptr;
    }
  }
};

Dlloader::Dlloader() { this->m_data = std::make_shared<Dlloader::DlloaderData>(); }

Dlloader::~Dlloader() {}

Dlloader::DL_FUNC Dlloader::symbol(const char* symbol_name) {
  Dlloader::DL_FUNC func;

  if (data().map.find(symbol_name) == data().map.end()) {
    func = reinterpret_cast<Dlloader::DL_FUNC>(
          loadSymbol(data().handle, symbol_name));
    LunaAssert(func, "Unable to load requested symbol ", symbol_name, " from the loaded module!");
    data().map.insert({symbol_name, func});
  }

  return data().map.at(symbol_name);
}

void Dlloader::load(const char* lib_path) {
  data().handle = loadSharedObject(lib_path);
  data().soname = lib_path;
  LunaAssert(data().handle, "Unable to load dynamic lib ", lib_path, ".", " Error: ", getError());
}

bool Dlloader::initialized() const { return data().handle != nullptr; }
void Dlloader::reset() {
  if (data().handle) ::releaseHandle(data().handle);
}

Dlloader::DlloaderData& Dlloader::data() { return *this->m_data; }

const Dlloader::DlloaderData& Dlloader::data() const { return *this->m_data; }
}  // namespace io
}  // namespace ohm
