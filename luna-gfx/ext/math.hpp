#pragma once 
#include <cmath>
#include <numeric>
namespace luna {
template<typename T>
struct vec4_t {
  T x, y, z, w;

  vec4_t() = default;
  vec4_t(T r, T g, T b, T a) : x(r), y(g), z(b), w(a) {};

  constexpr auto length() const {return 4u;}
  [[nodiscard]] auto operator[](std::size_t index) const -> const T& {return *(reinterpret_cast<const T*>(this) + index);}
  [[nodiscard]] auto operator[](std::size_t index) -> T& {return *(reinterpret_cast<T*>(this) + index);}

  template<typename G>
  auto operator==(const G& cmp) const -> bool {
    if(this->x != cmp.x) return false;
    if(this->y != cmp.y) return false;
    if(this->z != cmp.z) return false;
    if(this->w != cmp.w) return false;
    return true;
  }

  auto operator-() -> vec4_t& {
    this->x = -x;
    this->y = -y;
    this->z = -z;
    this->w = -w;
    return *this;
  }

  auto operator+(const T& val) const -> vec4_t {
    auto tmp = vec4_t<T>();
    tmp = *this;
    tmp.x += val;
    tmp.y += val;
    tmp.z += val;
    tmp.w += val;
    return tmp;
  }

  template<typename G>
  auto operator+(const G& val) const -> vec4_t {
    auto tmp = vec4_t<T>();
    tmp = *this;
    tmp.x += val.x;
    tmp.y += val.y;
    tmp.z += val.z;
    tmp.w += val.w;
    return tmp;
  }

  template<typename G>
  auto operator*(const G& val) const -> vec4_t {
    auto tmp = vec4_t<T>();
    tmp = *this;
    tmp.x *= val.x;
    tmp.y *= val.y;
    tmp.z *= val.z;
    tmp.w *= val.w;
    return tmp;
  }

  auto operator*(T val) const -> vec4_t {
    auto tmp = vec4_t<T>();
    tmp = *this;
    tmp.x *= val;
    tmp.y *= val;
    tmp.z *= val;
    tmp.w *= val;
    return tmp;
  }

  template<typename G>
  auto operator!=(const G& cmp) const -> bool {
    return !(*this == cmp);
  }

  // We can become any type.
  template<typename G>
  auto operator=(const G& cpy) -> vec4_t& {
    this->x = cpy.x;
    this->y = cpy.y;
    this->z = cpy.z;
    this->w = cpy.w;
    return *this;
  }
};

template<typename T>
struct vec3_t {
  T x, y, z;
  constexpr vec3_t() = default;
  constexpr vec3_t(T r, T g, T b) : x(r), y(g), z(b) {};

  constexpr auto length() const {return 3u;}
  [[nodiscard]] auto operator[](std::size_t index) const -> const T& {return *(reinterpret_cast<const T*>(this) + index);}
  [[nodiscard]] auto operator[](std::size_t index) -> T& {return *(reinterpret_cast<T*>(this) + index);}

  template<typename G>
  auto operator==(const G& cmp) const -> bool {
    if(this->x != cmp.x) return false;
    if(this->y != cmp.y) return false;
    if(this->z != cmp.z) return false;
    return true;
  }

  template<typename G>
  auto operator*(const G& val) -> vec3_t {
    auto tmp = vec3_t<T>();
    tmp = *this;
    tmp.x *= val.x;
    tmp.y *= val.y;
    tmp.z *= val.z;
    return tmp;
  }

  auto operator*(T val) -> vec3_t {
    auto tmp = vec3_t<T>();
    tmp = *this;
    tmp.x *= val;
    tmp.y *= val;
    tmp.z *= val;
    return tmp;
  }

    auto operator+(const T& val) -> vec3_t {
    auto tmp = vec3_t<T>();
    tmp = *this;
    tmp.x += val;
    tmp.y += val;
    tmp.z += val;
    return tmp;
  }

  template<typename G>
  auto operator+(const G& val) -> vec3_t {
    auto tmp = vec3_t<T>();
    tmp = *this;
    tmp.x += val.x;
    tmp.y += val.y;
    tmp.z += val.z;
    return tmp;
  }

  auto operator-() -> vec3_t {
    auto tmp = vec3_t<T>();
    tmp.x = -x;
    tmp.y = -y;
    tmp.z = -z;
    return *this;
  }

  template<typename G>
  auto operator!=(const G& cmp) const -> bool {
    return !(*this == cmp);
  }

  // We can become any type.
  template<typename G>
  auto operator=(const G& cpy) -> vec3_t& {
    this->x = cpy.x;
    this->y = cpy.y;
    this->z = cpy.z;
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
  auto operator*(const G& val) -> vec2_t {
    auto tmp = vec2_t<T>();
    tmp = *this;
    tmp.x *= val.x;
    tmp.y *= val.y;
    return tmp;
  }

  auto operator*(T val) -> vec2_t {
    auto tmp = vec2_t<T>();
    tmp = *this;
    tmp.x *= val;
    tmp.y *= val;
    return tmp;
  }

    auto operator+(const T& val) -> vec2_t {
    auto tmp = vec3_t<T>();
    tmp = *this;
    tmp.x += val;
    tmp.y += val;
    return tmp;
  }

  template<typename G>
  auto operator+(const G& val) -> vec2_t {
    auto tmp = vec3_t<T>();
    tmp = *this;
    tmp.x += val.x;
    tmp.y += val.y;
    return tmp;
  }

  auto operator-() -> vec2_t {
    auto tmp = vec2_t<T>();
    tmp.x = -x;
    tmp.y = -y;
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
    matrix[3] = {0, 0, 0, t};
    matrix[2] = {0, 0, t, 0};
    matrix[1] = {0, t, 0, 0};
    matrix[0] = {t, 0, 0, 0};
  }

  ~mat4() = default;
  
  auto operator[](std::size_t index) const -> const vec4_t<float>& {return matrix[index];}
  auto operator[](std::size_t index) -> vec4_t<float>& {return matrix[index];}

  template<typename T>
  auto operator==(const T& cmp) const -> bool {
    for(auto x = 0; x < 4; ++x) {
      for(auto y = 0; y < 4; ++y) {
        if(std::fabs(matrix[x][y] - cmp[x][y]) > 0.001) return false;
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


inline auto operator*(const mat4& a, const mat4& b) -> mat4 {
  auto ret = mat4();
  auto get_row = [&a](std::size_t index) {
    auto row = vec4_t<float>();
    row.x = a[0][index];
    row.y = a[1][index];
    row.z = a[2][index];
    row.w = a[3][index];
    return row;
  };

  auto set_row = [](auto& out, vec4_t<float> vec, std::size_t index) {
    out[0][index] = vec.x;
    out[1][index] = vec.y;
    out[2][index] = vec.z;
    out[3][index] = vec.w;
  };

  auto r1 = get_row(0);
  auto r2 = get_row(1);
  auto r3 = get_row(2);
  auto r4 = get_row(3);
  auto c1 = b[0];
  auto c2 = b[1];
  auto c3 = b[2];
  auto c4 = b[3];
  set_row(ret, {dot(r1, c1), dot(r1, c2), dot(r1, c3), dot(r1, c4)}, 0);
  set_row(ret, {dot(r2, c1), dot(r2, c2), dot(r2, c3), dot(r2, c4)}, 1);
  set_row(ret, {dot(r3, c1), dot(r3, c2), dot(r3, c3), dot(r3, c4)}, 2);
  set_row(ret, {dot(r4, c1), dot(r4, c2), dot(r4, c3), dot(r4, c4)}, 3);
  return ret;
}

template<typename T>
auto perspective(T fovy, T aspect, T znear, T zfar) -> mat4
{
	T const tan_half_fov_y = std::tan(fovy / static_cast<T>(2));
	auto res = mat4(static_cast<T>(0));
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

template<typename G>
auto translate(const mat4& t, const G& g) -> mat4 {
	auto result = t;
	result[3] = t[0] * g[0] + t[1] * g[1] + t[2] * g[2] + t[3];
	return result;
}

template<typename G>
auto scale(const mat4& t, const G& g) -> mat4 {
  auto result = t;
	result[0] = t[0] * g[0];
	result[1] = t[1] * g[1];
	result[2] = t[2] * g[2];
	result[3] = t[3];
	return result;
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