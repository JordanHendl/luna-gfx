#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>
namespace luna {
namespace gfx {
template<typename T>
struct MappedBuffer {
public:
  MappedBuffer() {begin_ = nullptr; end_ = nullptr;}
  ~MappedBuffer() {this->unmap_func_();}
  auto size() -> std::size_t {return end_ - begin_;}
  auto begin() -> T* {return begin_;}
  auto end() -> T* {return end_;}
  auto operator[](std::size_t idx) -> T& {return *(begin_ + idx);}
  auto operator[](std::size_t idx) const -> const T& {return *(begin_ + idx);}
  auto data() -> T* {return this->begin_;}
private:
  friend class Buffer;
  std::function<void()> unmap_func_;
  T* begin_;
  T* end_;
};

  class Buffer {
    public:
      enum class Type {
        Vertex,
        Index,
        GPUOptimal,
        CPUVisible,
        General,
        Unknown,
      } ;

      Buffer() {this->m_handle = -1; this->m_size = 0; this->m_type = Type::Unknown;}
      Buffer(int gpu, std::size_t size, Buffer::Type type = Type::General);
      ~Buffer();

      Buffer(Buffer&& mv) {*this = std::move(mv);};
      Buffer(const Buffer& cpy) = delete;

      auto unmap() -> void;

      template<typename T>
      auto map(T** ptr) -> void {
        if(this->type() == Type::General || this->type() == Type::CPUVisible) {
          this->map_impl(reinterpret_cast<void**>(ptr)); // reinterpret_cast so we can get an opaque ptr to map to.
        }
      }

      template<typename T>
      auto upload_data(const T* ptr, std::size_t amt) -> void {
        upload_data_impl(reinterpret_cast<const unsigned char*>(ptr), sizeof(T) * amt);
      }

      template<typename T>
      auto get_mapped_container() -> MappedBuffer<T> {
        auto m = MappedBuffer<T>();
        this->map(&m.begin_);
        m.end_ = m.begin_ + (this->size() / sizeof(T));
        m.unmap_func_ = std::function<void()>(std::bind(&Buffer::unmap, this));
        return m;
      }

      template<typename T>
      auto upload_data(const T* ptr) -> void {
        upload_data_impl(reinterpret_cast<const unsigned char*>(ptr), sizeof(ptr) * this->size());
      }


      [[nodiscard]] auto type() const {return this->m_type;}
      [[nodiscard]] auto size() const {return this->m_size;}
      [[nodiscard]] inline auto handle() const -> std::int32_t {return this->m_handle;}
      
      auto operator=(Buffer&& mv) -> Buffer& {
        this->m_handle = mv.m_handle;
        this->m_type = mv.m_type;
        this->m_size = mv.m_size;
        mv.m_handle = -1;
        return *this;
      }

      auto operator=(const Buffer& cpy) -> Buffer& = delete;  
    private:
      auto map_impl(void** ptr) -> void;
      auto upload_data_impl(const unsigned char* in_data, std::size_t num_bytes) -> void;
      std::int32_t m_handle;
      Type m_type;
      std::size_t m_size;
  };
}
}