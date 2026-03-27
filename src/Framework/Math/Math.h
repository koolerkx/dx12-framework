#pragma once
#include <DirectXMath.h>

#include <numbers>

namespace Math {

// Constants & Utility Functions
[[maybe_unused]] constexpr float Pi = std::numbers::pi_v<float>;
[[maybe_unused]] constexpr float TwoPi = 2 * Pi;
[[maybe_unused]] constexpr float PiOver2 = Pi * 0.5f;
[[maybe_unused]] constexpr float PiOver4 = Pi * 0.25f;

constexpr float ToRadians(float degrees) {
  return degrees * Pi / 180.0f;
}
constexpr float ToDegrees(float radians) {
  return radians * 180.0f / Pi;
}

// Floating point comparison (epsilon = 0.001f default)
bool NearZero(float val, float epsilon = 0.001f);
bool NearEqual(float a, float b, float epsilon = 0.001f);

// Clamping & Lerp
float Clamp(float value, float min, float max);
// t is not clamped; values outside [0,1] extrapolate
float Lerp(float a, float b, float t);
float LerpClamped(float a, float b, float t);

// Min/Max
float Min(float a, float b);
float Max(float a, float b);
float Abs(float value);

// Trig wrappers (for consistency)
float Sin(float angle);
float Cos(float angle);
float Tan(float angle);
float Asin(float value);
float Acos(float value);
float Atan2(float y, float x);
float Sqrt(float value);

// Forward declarations for validation helpers
struct Vector2;
struct Vector3;
struct Vector4;
struct Matrix3;
struct Matrix4;
struct Quaternion;

// Validation helpers - returns true if all components are finite (not NaN or Inf)
bool IsFinite(float value);
bool IsFinite(const Vector2& v);
bool IsFinite(const Vector3& v);
bool IsFinite(const Vector4& v);
bool IsFinite(const Matrix3& m);
bool IsFinite(const Matrix4& m);
bool IsFinite(const Quaternion& q);

// Debug assertions - asserts if value contains NaN or Inf
void AssertFinite(float value);
void AssertFinite(const Vector2& v);
void AssertFinite(const Vector3& v);
void AssertFinite(const Vector4& v);
void AssertFinite(const Matrix3& m);
void AssertFinite(const Matrix4& m);
void AssertFinite(const Quaternion& q);

// Vector2 - 2D Vector
struct Vector2 : public DirectX::XMFLOAT2 {
  // Constructors
  Vector2();
  Vector2(float x, float y);
  explicit Vector2(const DirectX::XMFLOAT2& v);
  explicit Vector2(DirectX::FXMVECTOR v);

  // Static constants
  static const Vector2 Zero;
  static const Vector2 One;
  static const Vector2 UnitX;
  static const Vector2 UnitY;

  // Conversion to XMVECTOR for DirectXMath interop
  operator DirectX::XMVECTOR() const;

  // Arithmetic operators
  Vector2 operator-() const;
  Vector2& operator+=(const Vector2& v);
  Vector2& operator-=(const Vector2& v);
  Vector2& operator*=(float s);
  // Returns NaN/Inf if s == 0
  Vector2& operator/=(float s);

  // Element access
  float& operator[](int i);
  const float& operator[](int i) const;

  // Geometric operations
  float Length() const;
  float LengthSquared() const;
  float Dot(const Vector2& v) const;
  float Cross(const Vector2& v) const;

  void Normalize();
  // Returns NaN if zero-length
  Vector2 Normalized() const;

  Vector2 Perp() const;

  // Static utilities
  static float Distance(const Vector2& a, const Vector2& b);
  static float DistanceSquared(const Vector2& a, const Vector2& b);
  // t is not clamped; values outside [0,1] extrapolate
  static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);
  static Vector2 LerpClamped(const Vector2& a, const Vector2& b, float t);
  static Vector2 Min(const Vector2& a, const Vector2& b);
  static Vector2 Max(const Vector2& a, const Vector2& b);
};

// Global operators
Vector2 operator+(Vector2 a, const Vector2& b);
Vector2 operator-(Vector2 a, const Vector2& b);
Vector2 operator*(Vector2 v, float s);
Vector2 operator*(float s, Vector2 v);
Vector2 operator/(Vector2 v, float s);
bool operator==(const Vector2& a, const Vector2& b);
bool operator!=(const Vector2& a, const Vector2& b);

// Vector3 - 3D Vector
struct Vector3 : public DirectX::XMFLOAT3 {
  // Constructors
  Vector3();
  Vector3(float x, float y, float z);
  explicit Vector3(const DirectX::XMFLOAT3& v);
  explicit Vector3(DirectX::FXMVECTOR v);

  // Static constants
  static const Vector3 Zero;
  static const Vector3 One;
  static const Vector3 UnitX;
  static const Vector3 UnitY;
  static const Vector3 UnitZ;
  static const Vector3 Forward;
  static const Vector3 Right;
  static const Vector3 Up;

  // Conversion
  operator DirectX::XMVECTOR() const;

  // Arithmetic operators
  Vector3 operator-() const;
  Vector3& operator+=(const Vector3& v);
  Vector3& operator-=(const Vector3& v);
  Vector3& operator*=(float s);
  // Returns NaN/Inf if s == 0
  Vector3& operator/=(float s);

  // Element access
  float& operator[](int i);
  const float& operator[](int i) const;

  // Geometric operations
  float Length() const;
  float LengthSquared() const;
  float Dot(const Vector3& v) const;
  Vector3 Cross(const Vector3& v) const;

  void Normalize();
  // Returns NaN if zero-length
  Vector3 Normalized() const;

  // Swizzle accessors
  Vector2 xy() const;
  Vector2 xz() const;
  Vector2 yz() const;
  Vector3 xxx() const;
  Vector3 yyy() const;
  Vector3 zzz() const;

  // Projection & Reflection
  Vector3 ProjectOnto(const Vector3& v) const;
  Vector3 Reflect(const Vector3& normal) const;

  // Static utilities
  static float Distance(const Vector3& a, const Vector3& b);
  static float DistanceSquared(const Vector3& a, const Vector3& b);
  static float Angle(const Vector3& a, const Vector3& b);
  // t is not clamped; values outside [0,1] extrapolate
  static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);
  static Vector3 LerpClamped(const Vector3& a, const Vector3& b, float t);
  static Vector3 Slerp(const Vector3& a, const Vector3& b, float t);
  static Vector3 Min(const Vector3& a, const Vector3& b);
  static Vector3 Max(const Vector3& a, const Vector3& b);

  // Orthonormal basis generation
  static void CreateOrthonormalBasis(const Vector3& normal, Vector3& outTangent, Vector3& outBitangent);
};

// Global operators
Vector3 operator+(Vector3 a, const Vector3& b);
Vector3 operator-(Vector3 a, const Vector3& b);
Vector3 operator*(Vector3 v, float s);
Vector3 operator*(float s, Vector3 v);
Vector3 operator/(Vector3 v, float s);
bool operator==(const Vector3& a, const Vector3& b);
bool operator!=(const Vector3& a, const Vector3& b);

// Vector4 - 4D Vector
struct Vector4 : public DirectX::XMFLOAT4 {
  // Constructors
  Vector4();
  Vector4(float x, float y, float z, float w);
  Vector4(const Vector3& v, float w);
  explicit Vector4(const DirectX::XMFLOAT4& v);
  explicit Vector4(DirectX::FXMVECTOR v);

  // Static constants
  static const Vector4 Zero;
  static const Vector4 One;
  static const Vector4 UnitX;
  static const Vector4 UnitY;
  static const Vector4 UnitZ;
  static const Vector4 UnitW;

  // Conversion
  operator DirectX::XMVECTOR() const;

  // Arithmetic operators
  Vector4 operator-() const;
  Vector4& operator+=(const Vector4& v);
  Vector4& operator-=(const Vector4& v);
  Vector4& operator*=(float s);
  // Returns NaN/Inf if s == 0
  Vector4& operator/=(float s);

  // Element access
  float& operator[](int i);
  const float& operator[](int i) const;

  // Geometric operations
  float Length() const;
  float LengthSquared() const;
  float Dot(const Vector4& v) const;

  void Normalize();
  // Returns NaN if zero-length
  Vector4 Normalized() const;

  // Swizzle accessors
  Vector3 xyz() const;
  Vector2 xy() const;
  Vector2 xz() const;
  Vector2 yz() const;
  Vector4 xxxx() const;
  Vector4 yyyy() const;
  Vector4 zzzz() const;
  Vector4 wwww() const;

  // Static utilities
  // t is not clamped; values outside [0,1] extrapolate
  static Vector4 Lerp(const Vector4& a, const Vector4& b, float t);
  static Vector4 LerpClamped(const Vector4& a, const Vector4& b, float t);
};

// Global operators
Vector4 operator+(Vector4 a, const Vector4& b);
Vector4 operator-(Vector4 a, const Vector4& b);
Vector4 operator*(Vector4 v, float s);
Vector4 operator*(float s, Vector4 v);
Vector4 operator/(Vector4 v, float s);
bool operator==(const Vector4& a, const Vector4& b);
bool operator!=(const Vector4& a, const Vector4& b);

// Forward declarations
struct Quaternion;

// Matrix3 - 3x3 Matrix (Rotation, Scale, 2D Transforms)
struct Matrix3 : public DirectX::XMFLOAT3X3 {
  // Constructors
  Matrix3();
  explicit Matrix3(const DirectX::XMFLOAT3X3& m);
  explicit Matrix3(DirectX::FXMMATRIX m);

  // Static constants
  static const Matrix3 Identity;

  // Conversion
  operator DirectX::XMMATRIX() const;

  // Accessors
  float& operator()(int row, int col);
  const float& operator()(int row, int col) const;

  // Matrix operations
  Matrix3& operator*=(const Matrix3& m);
  Matrix3 operator*(const Matrix3& m) const;

  Vector3 operator*(const Vector3& v) const;

  Matrix3 Transposed() const;
  // Returns NaN matrix if singular (det == 0)
  Matrix3 Inverted() const;
  float Determinant() const;

  // Static creation functions
  static Matrix3 CreateRotationX(float radians);
  static Matrix3 CreateRotationY(float radians);
  static Matrix3 CreateRotationZ(float radians);
  static Matrix3 CreateRotation(float angle);
  static Matrix3 CreateScale(float scale);
  static Matrix3 CreateScale(float sx, float sy, float sz);
  static Matrix3 CreateFromQuaternion(const Quaternion& q);
};

// Matrix4 - 4x4 Matrix (3D Transforms, View, Projection)
struct Matrix4 : public DirectX::XMFLOAT4X4 {
  // Constructors
  Matrix4();
  explicit Matrix4(const DirectX::XMFLOAT4X4& m);
  explicit Matrix4(DirectX::FXMMATRIX m);

  // Static constants
  static const Matrix4 Identity;

  // Conversion
  operator DirectX::XMMATRIX() const;

  // Accessors
  float& operator()(int row, int col);
  const float& operator()(int row, int col) const;

  // Matrix operations
  Matrix4& operator*=(const Matrix4& m);
  Matrix4 operator*(const Matrix4& m) const;

  Vector3 TransformPoint(const Vector3& point) const;
  Vector3 TransformVector(const Vector3& vector) const;
  Vector3 TransformNormal(const Vector3& normal) const;

  Matrix4 Transposed() const;
  // Returns NaN matrix if singular (det == 0)
  Matrix4 Inverted() const;
  float Determinant() const;

  // Row/Column access
  Vector4 GetRow(int row) const;
  Vector4 GetColumn(int col) const;

  // Extract components
  Vector3 GetTranslation() const;
  Vector3 GetScale() const;
  Quaternion GetRotation() const;

  // Static creation functions - Transform matrices
  static Matrix4 CreateTranslation(const Vector3& position);
  static Matrix4 CreateScale(float scale);
  static Matrix4 CreateScale(const Vector3& scale);
  static Matrix4 CreateRotationX(float radians);
  static Matrix4 CreateRotationY(float radians);
  static Matrix4 CreateRotationZ(float radians);
  static Matrix4 CreateFromQuaternion(const Quaternion& q);

  // TRS composition
  static Matrix4 CreateFromTRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);

  // View matrix
  static Matrix4 CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up);

  // Projection matrices
  static Matrix4 CreatePerspectiveFOV(float fovY, float aspectRatio, float nearZ, float farZ);
  static Matrix4 CreateOrthographic(float width, float height, float nearZ, float farZ);
  static Matrix4 CreateOrthographicOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ);

  static Matrix4 FaceTo(const Vector3& from, const Vector3& to, const Vector3& upHint);
};

// Quaternion - Rotation representation
struct Quaternion : public DirectX::XMFLOAT4 {
  // Constructors
  Quaternion();
  Quaternion(float x, float y, float z, float w);
  explicit Quaternion(const DirectX::XMFLOAT4& q);
  explicit Quaternion(DirectX::FXMVECTOR q);

  // Static constants
  static const Quaternion Identity;

  // Conversion
  operator DirectX::XMVECTOR() const;

  // Quaternion operations
  Quaternion operator*(const Quaternion& q) const;
  Quaternion& operator*=(const Quaternion& q);

  float Length() const;
  float LengthSquared() const;
  void Normalize();
  Quaternion Normalized() const;

  Quaternion Conjugate() const;
  Quaternion Inverse() const;

  float Dot(const Quaternion& q) const;

  // Apply rotation to vector
  Vector3 RotateVector(const Vector3& v) const;

  // Conversions
  Matrix4 ToMatrix() const;
  void ToAxisAngle(Vector3& outAxis, float& outAngle) const;
  Vector3 ToEulerAngles() const;

  // Static creation
  // Returns NaN quaternion if axis is zero-length
  static Quaternion CreateFromAxisAngle(const Vector3& axis, float angle);
  static Quaternion CreateFromEulerAngles(float pitch, float yaw, float roll);
  static Quaternion CreateFromRotationMatrix(const Matrix4& m);

  // Interpolation
  static Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t);
  static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);

  // Utilities
  static Quaternion LookRotation(const Vector3& forward, const Vector3& up);
  static Quaternion FromToRotation(const Vector3& from, const Vector3& to);
};

bool operator==(const Quaternion& a, const Quaternion& b);
bool operator!=(const Quaternion& a, const Quaternion& b);

// 2D Collision Primitives
struct Circle {
  Vector2 center;
  float radius;

  Circle();
  Circle(const Vector2& center, float radius);

  bool Contains(const Vector2& point) const;
  bool Intersects(const Circle& other) const;
};

struct Rect {
  Vector2 min;
  Vector2 max;

  Rect();
  Rect(const Vector2& min, const Vector2& max);

  Vector2 GetCenter() const;
  Vector2 GetExtents() const;

  bool Contains(const Vector2& point) const;
  bool Intersects(const Rect& other) const;
  bool Intersects(const Circle& circle) const;
};

// 3D Collision Primitives
struct Ray {
  Vector3 origin;
  Vector3 direction;

  Ray();
  Ray(const Vector3& origin, const Vector3& direction);

  Vector3 PointAt(float t) const;
};

struct Plane {
  Vector3 normal;
  float d;

  Plane();
  Plane(const Vector3& normal, float d);
  // normal is NaN if points are collinear
  Plane(const Vector3& a, const Vector3& b, const Vector3& c);

  float SignedDistance(const Vector3& point) const;
};

struct Sphere {
  Vector3 center;
  float radius;

  Sphere();
  Sphere(const Vector3& center, float radius);

  bool Contains(const Vector3& point) const;
  bool Intersects(const Sphere& other) const;
};

struct AABB {
  Vector3 min;
  Vector3 max;

  AABB();
  AABB(const Vector3& min, const Vector3& max);

  Vector3 GetCenter() const;
  Vector3 GetExtents() const;

  bool Contains(const Vector3& point) const;
  bool Intersects(const AABB& other) const;
  bool Intersects(const Sphere& sphere) const;

  void Encapsulate(const Vector3& point);
  void Encapsulate(const AABB& other);

  static AABB Inverted();
};

struct Capsule {
  Vector3 start;
  Vector3 end;
  float radius;

  Capsule();
  Capsule(const Vector3& start, const Vector3& end, float radius);

  bool Contains(const Vector3& point) const;
  bool Intersects(const Sphere& sphere) const;
  bool Intersects(const Capsule& other) const;
};

struct Frustum {
  Plane planes[6];

  static Frustum FromViewProjection(const Matrix4& view_proj);
  bool Intersects(const AABB& box) const;
};

AABB TransformAABB(const AABB& local, const Matrix4& transform);

// Intersection tests
// Returns t >= 0; use IntersectsSigned for negative t when inside
bool Intersects(const Ray& ray, const Plane& plane, float& outDistance);
bool Intersects(const Ray& ray, const Sphere& sphere, float& outDistance);
bool Intersects(const Ray& ray, const AABB& box, float& outDistance);

// Signed intersection tests - returns negative outDistance if ray origin is inside the primitive
bool IntersectsSigned(const Ray& ray, const Sphere& sphere, float& outDistance);
bool IntersectsSigned(const Ray& ray, const AABB& box, float& outDistance);

}  // namespace Math
