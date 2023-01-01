#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>
namespace luna {
namespace gfx {
template<typename T>
class MappedBuffer;

template<typename T>
class Vector;

class MemoryBuffer;
enum class MemoryType {
      Vertex,
      Index,
      GPUOptimal,
      CPUVisible,
      General,
      Unknown,
};

// Raw untyped GPU memory. 
class MemoryBuffer {
  public:
    MemoryBuffer() {this->m_handle = -1; this->m_size = 0; this->m_type = MemoryType::Unknown;}
    MemoryBuffer(int gpu, std::size_t size, MemoryType type = MemoryType::General);
    ~MemoryBuffer();
    MemoryBuffer(MemoryBuffer&& mv) {*this = std::move(mv);};
    MemoryBuffer(const MemoryBuffer& cpy) = delete;
    auto unmap() -> void;
    auto flush() -> void;
    template<typename T>
    auto map(T** ptr) -> void {
      this->map_impl(reinterpret_cast<void**>(ptr)); // reinterpret_cast so we can get an opaque ptr to map to.
    }

    template<typename T>
    auto upload(const T* ptr, std::size_t amt) -> void {
      upload_data_impl(reinterpret_cast<const unsigned char*>(ptr), sizeof(T) * amt);
    }
    
    // Helper function to return a container that is STL-like but points to the mapped buffer of this object.
    // Note: Only works if this buffer is mappable.
    template<typename T>
    auto get_mapped_container() -> MappedBuffer<T> {
      auto m = MappedBuffer<T>();
      this->map(&m.begin_);
      m.end_ = m.begin_ + (this->size() / sizeof(T));
      m.m_handle = this->m_handle;
      return m;
    }

    auto gpu() const -> int;

    template<typename T>
    auto upload(const T* ptr) -> void { upload_data_impl(reinterpret_cast<const unsigned char*>(ptr), this->size());}
    [[nodiscard]] inline auto type() const {return this->m_type;}
    [[nodiscard]] inline auto size() const {return this->m_size;}
    [[nodiscard]] inline auto handle() const -> std::int32_t {return this->m_handle;}
    
    auto operator=(MemoryBuffer&& mv) -> MemoryBuffer& {
      this->m_handle = mv.m_handle;
      this->m_type = mv.m_type;
      this->m_size = mv.m_size;
      mv.m_handle = -1;
      return *this;
    }

    auto operator=(const MemoryBuffer& cpy) -> MemoryBuffer& = delete;  
  private:
    auto map_impl(void** ptr) -> void;
    auto upload_data_impl(const unsigned char* in_data, std::size_t num_bytes) -> void;
    std::int32_t m_handle;
    MemoryType m_type;
    std::size_t m_size;
};

// Typed GPU memory. Prioritize using this for all data.
template<typename T>
class Vector {
public:
  Vector() {};
  Vector(int gpu, std::size_t count, MemoryType type = MemoryType::General) {
    this->m_data = std::move(MemoryBuffer(gpu, sizeof(T) * count, type));
  }
  Vector(Vector&& mv) = default;
  Vector(const Vector& cpy) = delete;
  ~Vector() = default;
  auto operator=(const Vector& cpy) -> Vector& = delete;
  auto operator=(Vector&& mv) -> Vector& = default  ;
  [[nodiscard]] inline auto size() const -> std::size_t {return this->m_data.size() / sizeof(T);}
  [[nodiscard]] inline auto get_mapped_container() -> MappedBuffer<T> {return this->m_data.get_mapped_container<T>();}
  inline auto flush() -> void {this->m_data.flush();}
  inline auto upload(const T* ptr, std::size_t amt) -> void {this->m_data.upload(ptr, amt);}
  inline auto upload(const T* ptr) -> void {this->m_data.upload(ptr, this->size());}
  inline auto upload(T data) -> void {this->m_data.upload(&data, &data);}
  inline auto map(T** ptr) -> void {this->m_data.map(ptr);}
  inline auto unmap() -> void {this->m_data.unmap();}
  inline auto type() const {return this->m_data.type();}
  inline auto resize(std::size_t new_amt) -> void {this->resize(new_amt, this->type());}
  inline auto resize(std::size_t new_amt, MemoryType new_type) -> void {
    auto gpu = m_data.gpu();
    this->m_data = std::move(MemoryBuffer(gpu, sizeof(T) * new_amt, new_type));
  }
  auto buffer() const -> const MemoryBuffer& {return this->m_data;}
  auto buffer() -> MemoryBuffer& {return this->m_data;}
  inline auto handle() const -> std::int32_t {return this->m_data.handle();}
private:
  MemoryBuffer m_data;
};

template<typename T>
class MappedBuffer {
public:
  MappedBuffer() {begin_ = nullptr; end_ = nullptr;}
  MappedBuffer(MappedBuffer&& mv) {*this = std::move(mv);}
  MappedBuffer(const MappedBuffer& cpy) = delete;
  ~MappedBuffer() {if(this->m_handle >= 0) unmap_mapped_buffer(this->m_handle);}
  auto operator=(const MappedBuffer& cpy) -> MappedBuffer& = delete;
  auto operator=(MappedBuffer&& mv)->MappedBuffer & {
    this->m_handle = mv.m_handle;
    this->begin_ = mv.begin_;
    this->end_ = mv.end_;
    mv.m_handle = -1;
    return *this;
  };

  auto size() const -> std::size_t {return end_ - begin_;}
  auto begin() -> T* {return begin_;}
  auto end() -> T* {return end_;}
  auto begin() const -> const T* {return begin_;}
  auto end() const -> const T* {return end_;}
  auto operator[](std::size_t idx) -> T& {return *(begin_ + idx);}
  auto operator[](std::size_t idx) const -> const T& {return *(begin_ + idx);}
  auto data() const -> const T* {return this->begin_;}
  auto data() -> T* {return this->begin_;}
private:
  friend class MemoryBuffer;
  friend void unmap_mapped_buffer(std::int32_t handle);
  std::int32_t m_handle;
  T* begin_;
  T* end_;
};
}
}