#pragma once
#include <cstddef>
#include <cstdint>
namespace luna {
namespace gfx {
  class Buffer {
    public:
      enum class Type {
        Vertex,
        Index,
        ShaderStorage,
        CPUVisible,
        General,
        Unknown,
      } ;

      Buffer() {this->m_handle = -1; this->m_size = 0; this->m_type = Type::Unknown;}
      Buffer(int gpu, std::size_t size, Buffer::Type type = Type::General);
      ~Buffer();

      Buffer(Buffer&& mv) = default;
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
      auto upload_data(const T* ptr) -> void {
        upload_data_impl(reinterpret_cast<const unsigned char*>(ptr), sizeof(ptr) * this->size());
      }

      [[nodiscard]] auto type() const {return this->m_type;}
      [[nodiscard]] auto size() const {return this->m_size;}
      [[nodiscard]] inline auto handle() const -> std::int32_t {return this->m_handle;}
    private:
      auto map_impl(void** ptr) -> void;
      auto upload_data_impl(const unsigned char* in_data, std::size_t num_bytes) -> void;
      std::int32_t m_handle;
      Type m_type;
      std::size_t m_size;
  };
}
}