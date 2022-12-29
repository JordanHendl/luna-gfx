#pragma once 
#include <cmath>
namespace luna {
template<typename T>
struct vec4_t {
  vec4_t() = default;
  vec4_t(T r, T g, T b, T a) : values{r, g, b, a} {};

  union {
    T x, y, z, w;
    T values[4];
  };
  constexpr auto length() const {return 4u;}
  [[nodiscard]] auto operator[](std::size_t index) const -> const T& {return values[index];}
  [[nodiscard]] auto operator[](std::size_t index) -> T& {return values[index];}

  template<typename G>
  auto operator==(const G& cmp) const -> bool {
    for(auto i = 0u; i < length(); ++i) {
      if(values[i] != cmp[i]) return false;
    }
    return true;
  }

  template<typename G>
  auto operator*(const G& val) -> vec4_t& {
    for(auto index = 0u; index < length(); ++index) {
      this->values[index] *= val[index];
    }
    return *this;
  }

  auto operator*(T val) -> vec4_t& {
    for(auto index = 0u; index < length(); ++index) {
      this->values[index] *= val;
    }
    return *this;
  }

  template<typename G>
  auto operator!=(const G& cmp) const -> bool {
    return !(*this == cmp);
  }

  // We can become any type.
  template<typename G>
  auto operator=(const G& cpy) -> vec4_t& {
    for(auto i = 0u; i < length(); ++i) {
      values[i] = cpy[i];
    }
    return *this;
  }
};

template<typename T>
struct vec3_t {
  union {
    T x, y, z;
    T values[3];
  };

  vec3_t() = default;
  vec3_t(T r, T g, T b) : values{r, g, b} {};

  constexpr auto length() const {return 3u;}
  [[nodiscard]] auto operator[](std::size_t index) const -> const T& {return values[index];}
  [[nodiscard]] auto operator[](std::size_t index) -> T& {return values[index];}

  template<typename G>
  auto operator==(const G& cmp) const -> bool {
    for(auto i = 0u; i < length(); ++i) {
      if(values[i] != cmp[i]) return false;
    }
    return true;
  }

  template<typename G>
  auto operator*(const G& val) -> vec3_t& {
    for(auto index = 0u; index < length(); ++index) {
      this->values[index] *= val[index];
    }
    return *this;
  }

  auto operator*(T val) -> vec3_t& {
    for(auto index = 0u; index < length(); ++index) {
      this->values[index] *= val;
    }
    return *this;
  }

  template<typename G>
  auto operator!=(const G& cmp) const -> bool {
    return !(*this == cmp);
  }

  // We can become any type.
  template<typename G>
  auto operator=(const G& cpy) -> vec3_t& {
    for(auto i = 0u; i < length(); ++i) {
      values[i] = cpy[i];
    }
    return *this;
  }
};

template<typename T>
struct vec2_t {
  vec2_t() = default;
  vec2_t(T r, T g) : values{r, g} {};

  union {
    T x, y;
    T values[2];
  };

  constexpr auto length() const {return 2u;}
  [[nodiscard]] auto operator[](std::size_t index) const -> const T& {return values[index];}
  [[nodiscard]] auto operator[](std::size_t index) -> T& {return values[index];}

  template<typename G>
  auto operator==(const G& cmp) const -> bool {
    for(auto i = 0u; i < length(); ++i) {
      if(values[i] != cmp[i]) return false;
    }
    return true;
  }

  template<typename G>
  auto operator*(const G& val) -> vec2_t& {
    for(auto index = 0u; index < length(); ++index) {
      this->values[index] *= val[index];
    }
    return *this;
  }

  auto operator*(T val) -> vec2_t& {
    for(auto index = 0u; index < length(); ++index) {
      this->values[index] *= val;
    }
    return *this;
  }

  template<typename G>
  auto operator!=(const G& cmp) const -> bool {
    return !(*this == cmp);
  }

  // We can become any type.
  template<typename G>
  auto operator=(const G& cpy) -> vec2_t& {
    for(auto i = 0u; i < length(); ++i) {
      values[i] = cpy[i];
    }
    return *this;
  }
};

template<typename T>
auto dot(T a, T b) -> float {
  static_assert(a.length() == b.length());
  auto ret = 0.0f;
  for(auto i = 0u; i < a.length(); i++) {
    ret += a[i] * b[i];
  }
  return ret;
}

struct mat4 {
  mat4(float t = 1.0f) {
    matrix[0] = {t, 0, 0, 0};
    matrix[1] = {0, t, 0, 0};
    matrix[2] = {0, 0, t, 0};
    matrix[3] = {0, 0, 0, t};
  }

  ~mat4() = default;
  
  auto operator[](std::size_t index) const -> const vec4_t<float>& {return matrix[index];}
  auto operator[](std::size_t index) -> vec4_t<float>& {return matrix[index];}

  template<typename T>
  auto operator==(const T& cmp) const -> bool {
    for(auto x = 0; x < 4; ++x) {
      for(auto y = 0; y < 4; ++y) {
        if(matrix[x][y] != cmp[x][y]) return false;
      }
    }
    return true;
  }

  template<typename G>
  auto operator!=(const G& cmp) const -> bool {
    return !(*this == cmp);
  }

  template<typename T>
  auto operator=(const T& cpy) -> mat4& {
    for(auto x = 0; x < 4; ++x) {
      for(auto y = 0; y < 4; ++y) {
        matrix[x][y] = cpy[x][y];
      }
    }
    return *this;
  }

  private:
    vec4_t<float> matrix[4];
};

template<typename T>
auto perspective(T fovy, T aspect, T znear, T zfar) -> mat4
{
	T const tan_half_fov_y = std::tan(fovy / static_cast<T>(2));
	mat4 res(static_cast<T>(0));
	res[0][0] = static_cast<T>(1) / (aspect * tan_half_fov_y);
	res[1][1] = static_cast<T>(1) / (tan_half_fov_y);
	res[2][2] = - (zfar + znear) / (zfar - znear);
	res[2][3] = - static_cast<T>(1);
	res[3][2] = - (static_cast<T>(2) * zfar * znear) / (zfar - znear);
	return res;
}

template<typename T>
auto normalize(T val) -> void {
  const auto d = dot(val, val);
  const auto reverse_sqrt = 1 / std::sqrt(d);
  return val * reverse_sqrt;
}

template<typename T>
auto to_radians(T degrees) -> float{
  constexpr auto cDegreesToRadians = 0.01745329251994329576923690768489;
  return degrees * cDegreesToRadians;
}

using vec4 = vec4_t<float>;
using vec3 = vec3_t<float>;
using vec2 = vec2_t<float>;

static_assert(sizeof(mat4) == sizeof(float) * 16);
static_assert(sizeof(vec4) == sizeof(float) * 4);
static_assert(sizeof(vec3) == sizeof(float) * 3);
static_assert(sizeof(vec2) == sizeof(float) * 2);
}