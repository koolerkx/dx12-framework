#include "Math.h"

#include <cassert>
#include <cmath>
#include <limits>

using namespace DirectX;

namespace Math {

// Validation helpers
bool IsFinite(float value) {
  return std::isfinite(value);
}

bool IsFinite(const Vector2& v) {
  return std::isfinite(v.x) && std::isfinite(v.y);
}

bool IsFinite(const Vector3& v) {
  return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

bool IsFinite(const Vector4& v) {
  return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) && std::isfinite(v.w);
}

bool IsFinite(const Matrix3& m) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (!std::isfinite(m.m[i][j])) return false;
    }
  }
  return true;
}

bool IsFinite(const Matrix4& m) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (!std::isfinite(m.m[i][j])) return false;
    }
  }
  return true;
}

bool IsFinite(const Quaternion& q) {
  return std::isfinite(q.x) && std::isfinite(q.y) && std::isfinite(q.z) && std::isfinite(q.w);
}

void AssertFinite(float value) {
  assert(IsFinite(value) && "Value is NaN or Inf");
}

void AssertFinite(const Vector2& v) {
  assert(IsFinite(v) && "Vector2 contains NaN or Inf");
}

void AssertFinite(const Vector3& v) {
  assert(IsFinite(v) && "Vector3 contains NaN or Inf");
}

void AssertFinite(const Vector4& v) {
  assert(IsFinite(v) && "Vector4 contains NaN or Inf");
}

void AssertFinite(const Matrix3& m) {
  assert(IsFinite(m) && "Matrix3 contains NaN or Inf");
}

void AssertFinite(const Matrix4& m) {
  assert(IsFinite(m) && "Matrix4 contains NaN or Inf");
}

void AssertFinite(const Quaternion& q) {
  assert(IsFinite(q) && "Quaternion contains NaN or Inf");
}

// Utility Functions
bool NearZero(float val, float epsilon) {
  return std::abs(val) < epsilon;
}

bool NearEqual(float a, float b, float epsilon) {
  return std::abs(a - b) < epsilon;
}

float Clamp(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

// t is not clamped; values outside [0,1] extrapolate
float Lerp(float a, float b, float t) {
  return a + t * (b - a);
}

float LerpClamped(float a, float b, float t) {
  return Lerp(a, b, Clamp(t, 0.0f, 1.0f));
}

float Min(float a, float b) {
  return (a < b) ? a : b;
}

float Max(float a, float b) {
  return (a > b) ? a : b;
}

float Abs(float value) {
  return std::abs(value);
}

float Sin(float angle) {
  return std::sin(angle);
}

float Cos(float angle) {
  return std::cos(angle);
}

float Tan(float angle) {
  return std::tan(angle);
}

float Asin(float value) {
  return std::asin(value);
}

float Acos(float value) {
  return std::acos(value);
}

float Atan2(float y, float x) {
  return std::atan2(y, x);
}

float Sqrt(float value) {
  return std::sqrt(value);
}

// Vector2 Implementation
const Vector2 Vector2::Zero = Vector2(0.0f, 0.0f);
const Vector2 Vector2::One = Vector2(1.0f, 1.0f);
const Vector2 Vector2::UnitX = Vector2(1.0f, 0.0f);
const Vector2 Vector2::UnitY = Vector2(0.0f, 1.0f);

Vector2::Vector2() : XMFLOAT2(0.0f, 0.0f) {
}

Vector2::Vector2(float x, float y) : XMFLOAT2(x, y) {
}

Vector2::Vector2(const XMFLOAT2& v) : XMFLOAT2(v) {
}

Vector2::Vector2(FXMVECTOR v) {
  XMStoreFloat2(this, v);
}

Vector2::operator XMVECTOR() const {
  return XMLoadFloat2(this);
}

Vector2 Vector2::operator-() const {
  return Vector2(-x, -y);
}

Vector2& Vector2::operator+=(const Vector2& v) {
  x += v.x;
  y += v.y;
  return *this;
}

Vector2& Vector2::operator-=(const Vector2& v) {
  x -= v.x;
  y -= v.y;
  return *this;
}

Vector2& Vector2::operator*=(float s) {
  x *= s;
  y *= s;
  return *this;
}

Vector2& Vector2::operator/=(float s) {
  float inv = 1.0f / s;
  x *= inv;
  y *= inv;
  return *this;
}

float& Vector2::operator[](int i) {
  return (&x)[i];
}

const float& Vector2::operator[](int i) const {
  return (&x)[i];
}

float Vector2::Length() const {
  return XMVectorGetX(XMVector2Length(*this));
}

float Vector2::LengthSquared() const {
  return XMVectorGetX(XMVector2LengthSq(*this));
}

float Vector2::Dot(const Vector2& v) const {
  return XMVectorGetX(XMVector2Dot(*this, v));
}

float Vector2::Cross(const Vector2& v) const {
  return XMVectorGetX(XMVector2Cross(*this, v));
}

void Vector2::Normalize() {
  XMVECTOR n = XMVector2Normalize(*this);
  XMStoreFloat2(this, n);
}

Vector2 Vector2::Normalized() const {
  return Vector2(XMVector2Normalize(*this));
}

Vector2 Vector2::Perp() const {
  return Vector2(-y, x);
}

float Vector2::Distance(const Vector2& a, const Vector2& b) {
  return (b - a).Length();
}

float Vector2::DistanceSquared(const Vector2& a, const Vector2& b) {
  return (b - a).LengthSquared();
}

// t is not clamped; values outside [0,1] extrapolate
Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, float t) {
  return Vector2(XMVectorLerp(a, b, t));
}

Vector2 Vector2::LerpClamped(const Vector2& a, const Vector2& b, float t) {
  return Lerp(a, b, Clamp(t, 0.0f, 1.0f));
}

Vector2 Vector2::Min(const Vector2& a, const Vector2& b) {
  return Vector2(XMVectorMin(a, b));
}

Vector2 Vector2::Max(const Vector2& a, const Vector2& b) {
  return Vector2(XMVectorMax(a, b));
}

Vector2 operator+(Vector2 a, const Vector2& b) {
  return a += b;
}

Vector2 operator-(Vector2 a, const Vector2& b) {
  return a -= b;
}

Vector2 operator*(Vector2 v, float s) {
  return v *= s;
}

Vector2 operator*(float s, Vector2 v) {
  return v *= s;
}

Vector2 operator/(Vector2 v, float s) {
  return v /= s;
}

bool operator==(const Vector2& a, const Vector2& b) {
  return XMVector2Equal(a, b);
}

bool operator!=(const Vector2& a, const Vector2& b) {
  return !XMVector2Equal(a, b);
}

// Vector3 Implementation
const Vector3 Vector3::Zero = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::One = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::UnitX = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UnitY = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::UnitZ = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::Forward = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::Right = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Up = Vector3(0.0f, 1.0f, 0.0f);

Vector3::Vector3() : XMFLOAT3(0.0f, 0.0f, 0.0f) {
}

Vector3::Vector3(float x, float y, float z) : XMFLOAT3(x, y, z) {
}

Vector3::Vector3(const XMFLOAT3& v) : XMFLOAT3(v) {
}

Vector3::Vector3(FXMVECTOR v) {
  XMStoreFloat3(this, v);
}

Vector3::operator XMVECTOR() const {
  return XMLoadFloat3(this);
}

Vector3 Vector3::operator-() const {
  return Vector3(-x, -y, -z);
}

Vector3& Vector3::operator+=(const Vector3& v) {
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

Vector3& Vector3::operator-=(const Vector3& v) {
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

Vector3& Vector3::operator*=(float s) {
  x *= s;
  y *= s;
  z *= s;
  return *this;
}

Vector3& Vector3::operator/=(float s) {
  float inv = 1.0f / s;
  x *= inv;
  y *= inv;
  z *= inv;
  return *this;
}

float& Vector3::operator[](int i) {
  return (&x)[i];
}

const float& Vector3::operator[](int i) const {
  return (&x)[i];
}

float Vector3::Length() const {
  return XMVectorGetX(XMVector3Length(*this));
}

float Vector3::LengthSquared() const {
  return XMVectorGetX(XMVector3LengthSq(*this));
}

float Vector3::Dot(const Vector3& v) const {
  return XMVectorGetX(XMVector3Dot(*this, v));
}

Vector3 Vector3::Cross(const Vector3& v) const {
  return Vector3(XMVector3Cross(*this, v));
}

void Vector3::Normalize() {
  XMVECTOR n = XMVector3Normalize(*this);
  XMStoreFloat3(this, n);
}

Vector3 Vector3::Normalized() const {
  return Vector3(XMVector3Normalize(*this));
}

Vector3 Vector3::ProjectOnto(const Vector3& v) const {
  float dotVV = v.Dot(v);
  if (NearZero(dotVV)) return Vector3::Zero;
  return v * (Dot(v) / dotVV);
}

Vector3 Vector3::Reflect(const Vector3& normal) const {
  return Vector3(XMVector3Reflect(*this, normal));
}

float Vector3::Distance(const Vector3& a, const Vector3& b) {
  return (b - a).Length();
}

float Vector3::DistanceSquared(const Vector3& a, const Vector3& b) {
  return (b - a).LengthSquared();
}

float Vector3::Angle(const Vector3& a, const Vector3& b) {
  float dot = a.Normalized().Dot(b.Normalized());
  dot = Clamp(dot, -1.0f, 1.0f);
  return Acos(dot);
}

// t is not clamped; values outside [0,1] extrapolate
Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, float t) {
  return Vector3(XMVectorLerp(a, b, t));
}

Vector3 Vector3::LerpClamped(const Vector3& a, const Vector3& b, float t) {
  return Lerp(a, b, Clamp(t, 0.0f, 1.0f));
}

Vector3 Vector3::Slerp(const Vector3& a, const Vector3& b, float t) {
  float dot = a.Normalized().Dot(b.Normalized());
  dot = Clamp(dot, -1.0f, 1.0f);

  float theta = Acos(dot);
  float sinTheta = Sin(theta);

  if (NearZero(sinTheta)) {
    return Lerp(a, b, t);
  }

  float wa = Sin((1.0f - t) * theta) / sinTheta;
  float wb = Sin(t * theta) / sinTheta;

  return a * wa + b * wb;
}

Vector3 Vector3::Min(const Vector3& a, const Vector3& b) {
  return Vector3(XMVectorMin(a, b));
}

Vector3 Vector3::Max(const Vector3& a, const Vector3& b) {
  return Vector3(XMVectorMax(a, b));
}

Vector2 Vector3::xy() const {
  return Vector2(x, y);
}

Vector2 Vector3::xz() const {
  return Vector2(x, z);
}

Vector2 Vector3::yz() const {
  return Vector2(y, z);
}

Vector3 Vector3::xxx() const {
  return Vector3(x, x, x);
}

Vector3 Vector3::yyy() const {
  return Vector3(y, y, y);
}

Vector3 Vector3::zzz() const {
  return Vector3(z, z, z);
}

void Vector3::CreateOrthonormalBasis(const Vector3& normal, Vector3& outTangent, Vector3& outBitangent) {
  Vector3 n = normal.Normalized();

  // Choose a vector not parallel to n
  Vector3 perp = (Abs(n.x) < 0.9f) ? Vector3::UnitX : Vector3::UnitY;

  outTangent = n.Cross(perp).Normalized();
  outBitangent = n.Cross(outTangent);
}

Vector3 operator+(Vector3 a, const Vector3& b) {
  return a += b;
}

Vector3 operator-(Vector3 a, const Vector3& b) {
  return a -= b;
}

Vector3 operator*(Vector3 v, float s) {
  return v *= s;
}

Vector3 operator*(float s, Vector3 v) {
  return v *= s;
}

Vector3 operator/(Vector3 v, float s) {
  return v /= s;
}

bool operator==(const Vector3& a, const Vector3& b) {
  return XMVector3Equal(a, b);
}

bool operator!=(const Vector3& a, const Vector3& b) {
  return !XMVector3Equal(a, b);
}

// Vector4 Implementation
const Vector4 Vector4::Zero = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::One = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 Vector4::UnitX = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::UnitY = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::UnitZ = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
const Vector4 Vector4::UnitW = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

Vector4::Vector4() : XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) {
}

Vector4::Vector4(float x, float y, float z, float w) : XMFLOAT4(x, y, z, w) {
}

Vector4::Vector4(const Vector3& v, float w) : XMFLOAT4(v.x, v.y, v.z, w) {
}

Vector4::Vector4(const XMFLOAT4& v) : XMFLOAT4(v) {
}

Vector4::Vector4(FXMVECTOR v) {
  XMStoreFloat4(this, v);
}

Vector4::operator XMVECTOR() const {
  return XMLoadFloat4(this);
}

Vector4 Vector4::operator-() const {
  return Vector4(-x, -y, -z, -w);
}

Vector4& Vector4::operator+=(const Vector4& v) {
  x += v.x;
  y += v.y;
  z += v.z;
  w += v.w;
  return *this;
}

Vector4& Vector4::operator-=(const Vector4& v) {
  x -= v.x;
  y -= v.y;
  z -= v.z;
  w -= v.w;
  return *this;
}

Vector4& Vector4::operator*=(float s) {
  x *= s;
  y *= s;
  z *= s;
  w *= s;
  return *this;
}

Vector4& Vector4::operator/=(float s) {
  float inv = 1.0f / s;
  x *= inv;
  y *= inv;
  z *= inv;
  w *= inv;
  return *this;
}

float& Vector4::operator[](int i) {
  return (&x)[i];
}

const float& Vector4::operator[](int i) const {
  return (&x)[i];
}

float Vector4::Length() const {
  return XMVectorGetX(XMVector4Length(*this));
}

float Vector4::LengthSquared() const {
  return XMVectorGetX(XMVector4LengthSq(*this));
}

float Vector4::Dot(const Vector4& v) const {
  return XMVectorGetX(XMVector4Dot(*this, v));
}

void Vector4::Normalize() {
  XMVECTOR n = XMVector4Normalize(*this);
  XMStoreFloat4(this, n);
}

Vector4 Vector4::Normalized() const {
  return Vector4(XMVector4Normalize(*this));
}

Vector3 Vector4::xyz() const {
  return Vector3(x, y, z);
}

Vector2 Vector4::xy() const {
  return Vector2(x, y);
}

Vector2 Vector4::xz() const {
  return Vector2(x, z);
}

Vector2 Vector4::yz() const {
  return Vector2(y, z);
}

Vector4 Vector4::xxxx() const {
  return Vector4(x, x, x, x);
}

Vector4 Vector4::yyyy() const {
  return Vector4(y, y, y, y);
}

Vector4 Vector4::zzzz() const {
  return Vector4(z, z, z, z);
}

Vector4 Vector4::wwww() const {
  return Vector4(w, w, w, w);
}

// t is not clamped; values outside [0,1] extrapolate
Vector4 Vector4::Lerp(const Vector4& a, const Vector4& b, float t) {
  return Vector4(XMVectorLerp(a, b, t));
}

Vector4 Vector4::LerpClamped(const Vector4& a, const Vector4& b, float t) {
  return Lerp(a, b, Clamp(t, 0.0f, 1.0f));
}

Vector4 operator+(Vector4 a, const Vector4& b) {
  return a += b;
}

Vector4 operator-(Vector4 a, const Vector4& b) {
  return a -= b;
}

Vector4 operator*(Vector4 v, float s) {
  return v *= s;
}

Vector4 operator*(float s, Vector4 v) {
  return v *= s;
}

Vector4 operator/(Vector4 v, float s) {
  return v /= s;
}

bool operator==(const Vector4& a, const Vector4& b) {
  return XMVector4Equal(a, b);
}

bool operator!=(const Vector4& a, const Vector4& b) {
  return !XMVector4Equal(a, b);
}

// Matrix3 Implementation
const Matrix3 Matrix3::Identity = []() {
  Matrix3 m;
  m._11 = 1.0f;
  m._12 = 0.0f;
  m._13 = 0.0f;
  m._21 = 0.0f;
  m._22 = 1.0f;
  m._23 = 0.0f;
  m._31 = 0.0f;
  m._32 = 0.0f;
  m._33 = 1.0f;
  return m;
}();

Matrix3::Matrix3() : XMFLOAT3X3() {
  _11 = 1.0f;
  _12 = 0.0f;
  _13 = 0.0f;
  _21 = 0.0f;
  _22 = 1.0f;
  _23 = 0.0f;
  _31 = 0.0f;
  _32 = 0.0f;
  _33 = 1.0f;
}

Matrix3::Matrix3(const XMFLOAT3X3& m) : XMFLOAT3X3(m) {
}

Matrix3::Matrix3(FXMMATRIX m) {
  XMFLOAT4X4 temp;
  XMStoreFloat4x4(&temp, m);
  _11 = temp._11;
  _12 = temp._12;
  _13 = temp._13;
  _21 = temp._21;
  _22 = temp._22;
  _23 = temp._23;
  _31 = temp._31;
  _32 = temp._32;
  _33 = temp._33;
}

Matrix3::operator XMMATRIX() const {
  return XMMATRIX(_11, _12, _13, 0.0f, _21, _22, _23, 0.0f, _31, _32, _33, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

float& Matrix3::operator()(int row, int col) {
  return m[row][col];
}

const float& Matrix3::operator()(int row, int col) const {
  return m[row][col];
}

Matrix3& Matrix3::operator*=(const Matrix3& other) {
  *this = *this * other;
  return *this;
}

Matrix3 Matrix3::operator*(const Matrix3& other) const {
  return Matrix3(XMMatrixMultiply(*this, other));
}

Vector3 Matrix3::operator*(const Vector3& v) const {
  return Vector3(XMVector3TransformNormal(v, *this));
}

Matrix3 Matrix3::Transposed() const {
  Matrix3 result;
  result._11 = _11;
  result._12 = _21;
  result._13 = _31;
  result._21 = _12;
  result._22 = _22;
  result._23 = _32;
  result._31 = _13;
  result._32 = _23;
  result._33 = _33;
  return result;
}

Matrix3 Matrix3::Inverted() const {
  XMMATRIX mat = *this;
  XMVECTOR det;
  XMMATRIX inv = XMMatrixInverse(&det, mat);
  return Matrix3(inv);
}

float Matrix3::Determinant() const {
  return XMVectorGetX(XMMatrixDeterminant(*this));
}

Matrix3 Matrix3::CreateRotationX(float radians) {
  return Matrix3(XMMatrixRotationX(radians));
}

Matrix3 Matrix3::CreateRotationY(float radians) {
  return Matrix3(XMMatrixRotationY(radians));
}

Matrix3 Matrix3::CreateRotationZ(float radians) {
  return Matrix3(XMMatrixRotationZ(radians));
}

Matrix3 Matrix3::CreateRotation(float angle) {
  return CreateRotationZ(angle);
}

Matrix3 Matrix3::CreateScale(float scale) {
  Matrix3 result;
  result._11 = scale;
  result._22 = scale;
  result._33 = scale;
  return result;
}

Matrix3 Matrix3::CreateScale(float sx, float sy, float sz) {
  Matrix3 result;
  result._11 = sx;
  result._22 = sy;
  result._33 = sz;
  return result;
}

Matrix3 Matrix3::CreateFromQuaternion(const Quaternion& q) {
  return Matrix3(XMMatrixRotationQuaternion(q));
}

// Matrix4 Implementation
const Matrix4 Matrix4::Identity = []() {
  Matrix4 m;
  XMStoreFloat4x4(&m, XMMatrixIdentity());
  return m;
}();

Matrix4::Matrix4() {
  XMStoreFloat4x4(this, XMMatrixIdentity());
}

Matrix4::Matrix4(const XMFLOAT4X4& m) : XMFLOAT4X4(m) {
}

Matrix4::Matrix4(FXMMATRIX m) {
  XMStoreFloat4x4(this, m);
}

Matrix4::operator XMMATRIX() const {
  return XMLoadFloat4x4(this);
}

float& Matrix4::operator()(int row, int col) {
  return m[row][col];
}

const float& Matrix4::operator()(int row, int col) const {
  return m[row][col];
}

Matrix4& Matrix4::operator*=(const Matrix4& other) {
  XMStoreFloat4x4(this, XMMatrixMultiply(*this, other));
  return *this;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
  return Matrix4(XMMatrixMultiply(*this, other));
}

Vector3 Matrix4::TransformPoint(const Vector3& point) const {
  return Vector3(XMVector3TransformCoord(point, *this));
}

Vector3 Matrix4::TransformVector(const Vector3& vector) const {
  return Vector3(XMVector3TransformNormal(vector, *this));
}

Vector3 Matrix4::TransformNormal(const Vector3& normal) const {
  XMVECTOR det;
  XMMATRIX invTrans = XMMatrixTranspose(XMMatrixInverse(&det, *this));
  return Vector3(XMVector3TransformNormal(normal, invTrans));
}

Matrix4 Matrix4::Transposed() const {
  return Matrix4(XMMatrixTranspose(*this));
}

Matrix4 Matrix4::Inverted() const {
  XMVECTOR det;
  return Matrix4(XMMatrixInverse(&det, *this));
}

float Matrix4::Determinant() const {
  return XMVectorGetX(XMMatrixDeterminant(*this));
}

Vector4 Matrix4::GetRow(int row) const {
  const float* m = &_11;
  int i = row * 4;
  return Vector4(m[i], m[i + 1], m[i + 2], m[i + 3]);
}

Vector4 Matrix4::GetColumn(int col) const {
  const float* m = &_11;
  return Vector4(m[col], m[col + 4], m[col + 8], m[col + 12]);
}

Vector3 Matrix4::GetTranslation() const {
  return GetRow(3).xyz();
}

Vector3 Matrix4::GetScale() const {
  return Vector3(GetRow(0).xyz().Length(), GetRow(1).xyz().Length(), GetRow(2).xyz().Length());
}

Quaternion Matrix4::GetRotation() const {
  return Quaternion(XMQuaternionRotationMatrix(*this));
}

Matrix4 Matrix4::CreateTranslation(const Vector3& position) {
  return Matrix4(XMMatrixTranslation(position.x, position.y, position.z));
}

Matrix4 Matrix4::CreateScale(float scale) {
  return Matrix4(XMMatrixScaling(scale, scale, scale));
}

Matrix4 Matrix4::CreateScale(const Vector3& scale) {
  return Matrix4(XMMatrixScaling(scale.x, scale.y, scale.z));
}

Matrix4 Matrix4::CreateRotationX(float radians) {
  return Matrix4(XMMatrixRotationX(radians));
}

Matrix4 Matrix4::CreateRotationY(float radians) {
  return Matrix4(XMMatrixRotationY(radians));
}

Matrix4 Matrix4::CreateRotationZ(float radians) {
  return Matrix4(XMMatrixRotationZ(radians));
}

Matrix4 Matrix4::CreateFromQuaternion(const Quaternion& q) {
  return Matrix4(XMMatrixRotationQuaternion(q));
}

Matrix4 Matrix4::CreateFromTRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) {
  XMMATRIX s = XMMatrixScaling(scale.x, scale.y, scale.z);
  XMMATRIX r = XMMatrixRotationQuaternion(rotation);
  XMMATRIX t = XMMatrixTranslation(translation.x, translation.y, translation.z);
  return Matrix4(s * r * t);
}

Matrix4 Matrix4::CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
  return Matrix4(XMMatrixLookAtLH(eye, target, up));
}

Matrix4 Matrix4::CreatePerspectiveFOV(float fovY, float aspectRatio, float nearZ, float farZ) {
  return Matrix4(XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ));
}

Matrix4 Matrix4::CreateOrthographic(float width, float height, float nearZ, float farZ) {
  return Matrix4(XMMatrixOrthographicLH(width, height, nearZ, farZ));
}

Matrix4 Matrix4::CreateOrthographicOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ) {
  return Matrix4(XMMatrixOrthographicOffCenterLH(left, right, bottom, top, nearZ, farZ));
}

Matrix4 Matrix4::FaceTo(const Vector3& from, const Vector3& to, const Vector3& upHint) {
  Vector3 diff = to - from;
  if (diff.LengthSquared() < 0.0001f) return Identity;

  Vector3 forward = diff.Normalized();
  Vector3 up = upHint;
  if (Abs(forward.Dot(up)) > 0.999f) {
    up = Vector3(0.0f, 0.0f, 1.0f);
  }
  Vector3 right = up.Cross(forward).Normalized();
  Vector3 correctedUp = forward.Cross(right);

  Matrix4 m;
  m._11 = right.x;
  m._12 = right.y;
  m._13 = right.z;
  m._14 = 0.0f;
  m._21 = correctedUp.x;
  m._22 = correctedUp.y;
  m._23 = correctedUp.z;
  m._24 = 0.0f;
  m._31 = forward.x;
  m._32 = forward.y;
  m._33 = forward.z;
  m._34 = 0.0f;
  m._41 = 0.0f;
  m._42 = 0.0f;
  m._43 = 0.0f;
  m._44 = 1.0f;
  return m;
}

// Quaternion Implementation
const Quaternion Quaternion::Identity = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion::Quaternion() : XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) {
}

Quaternion::Quaternion(float x, float y, float z, float w) : XMFLOAT4(x, y, z, w) {
}

Quaternion::Quaternion(const XMFLOAT4& q) : XMFLOAT4(q) {
}

Quaternion::Quaternion(FXMVECTOR q) {
  XMStoreFloat4(this, q);
}

Quaternion::operator XMVECTOR() const {
  return XMLoadFloat4(this);
}

Quaternion Quaternion::operator*(const Quaternion& q) const {
  return Quaternion(XMQuaternionMultiply(*this, q));
}

Quaternion& Quaternion::operator*=(const Quaternion& q) {
  XMStoreFloat4(this, XMQuaternionMultiply(*this, q));
  return *this;
}

float Quaternion::Length() const {
  return XMVectorGetX(XMQuaternionLength(*this));
}

float Quaternion::LengthSquared() const {
  return XMVectorGetX(XMQuaternionLengthSq(*this));
}

void Quaternion::Normalize() {
  XMStoreFloat4(this, XMQuaternionNormalize(*this));
}

Quaternion Quaternion::Normalized() const {
  return Quaternion(XMQuaternionNormalize(*this));
}

Quaternion Quaternion::Conjugate() const {
  return Quaternion(XMQuaternionConjugate(*this));
}

Quaternion Quaternion::Inverse() const {
  return Quaternion(XMQuaternionInverse(*this));
}

float Quaternion::Dot(const Quaternion& q) const {
  return XMVectorGetX(XMQuaternionDot(*this, q));
}

Vector3 Quaternion::RotateVector(const Vector3& v) const {
  return Vector3(XMVector3Rotate(v, *this));
}

Matrix4 Quaternion::ToMatrix() const {
  return Matrix4(XMMatrixRotationQuaternion(*this));
}

void Quaternion::ToAxisAngle(Vector3& outAxis, float& outAngle) const {
  XMVECTOR axis;
  XMQuaternionToAxisAngle(&axis, &outAngle, *this);
  outAxis = Vector3(axis);
}

Vector3 Quaternion::ToEulerAngles() const {
  // Decompose for intrinsic ZXY convention matching XMQuaternionRotationRollPitchYaw
  // R = Ry(yaw) * Rx(pitch) * Rz(roll)
  // R[1][2] = -sin(pitch) → pitch = asin(2(wx - yz))
  float sinp = 2.0f * (w * x - y * z);
  float pitch = (Abs(sinp) >= 1.0f) ? std::copysign(PiOver2, sinp) : Asin(sinp);

  // R[0][2] / R[2][2] → yaw = atan2(2(xz + wy), 1 - 2(x² + y²))
  float yaw = Atan2(2.0f * (x * z + w * y), 1.0f - 2.0f * (x * x + y * y));

  // R[1][0] / R[1][1] → roll = atan2(2(xy + wz), 1 - 2(x² + z²))
  float roll = Atan2(2.0f * (x * y + w * z), 1.0f - 2.0f * (x * x + z * z));

  return Vector3(pitch, yaw, roll);
}

Quaternion Quaternion::CreateFromAxisAngle(const Vector3& axis, float angle) {
  return Quaternion(XMQuaternionRotationAxis(axis, angle));
}

Quaternion Quaternion::CreateFromEulerAngles(float pitch, float yaw, float roll) {
  return Quaternion(XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
}

Quaternion Quaternion::CreateFromRotationMatrix(const Matrix4& m) {
  return Quaternion(XMQuaternionRotationMatrix(m));
}

Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float t) {
  Quaternion result(XMQuaternionSlerp(a, b, t));
  result.Normalize();
  return result;
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t) {
  return Quaternion(XMQuaternionSlerp(a, b, t));
}

Quaternion Quaternion::LookRotation(const Vector3& forward, const Vector3& up) {
  Vector3 f = forward.Normalized();
  Vector3 r = up.Cross(f).Normalized();
  Vector3 u = f.Cross(r);

  Matrix4 m;
  m._11 = r.x;
  m._12 = r.y;
  m._13 = r.z;
  m._14 = 0.0f;
  m._21 = u.x;
  m._22 = u.y;
  m._23 = u.z;
  m._24 = 0.0f;
  m._31 = f.x;
  m._32 = f.y;
  m._33 = f.z;
  m._34 = 0.0f;
  m._41 = 0.0f;
  m._42 = 0.0f;
  m._43 = 0.0f;
  m._44 = 1.0f;

  return CreateFromRotationMatrix(m);
}

Quaternion Quaternion::FromToRotation(const Vector3& from, const Vector3& to) {
  Vector3 f = from.Normalized();
  Vector3 t = to.Normalized();

  float dot = f.Dot(t);

  if (dot > 0.9999f) {
    return Quaternion::Identity;
  }

  if (dot < -0.9999f) {
    Vector3 axis = Vector3::UnitX.Cross(f);
    if (axis.LengthSquared() < 0.0001f) {
      axis = Vector3::UnitY.Cross(f);
    }
    axis.Normalize();
    return CreateFromAxisAngle(axis, Pi);
  }

  Vector3 axis = f.Cross(t);
  float s = Sqrt((1.0f + dot) * 2.0f);
  float invS = 1.0f / s;
  Quaternion q = Quaternion(axis.x * invS, axis.y * invS, axis.z * invS, s * 0.5f);
  q.Normalize();
  return q;
}

bool operator==(const Quaternion& a, const Quaternion& b) {
  return XMQuaternionEqual(a, b);
}

bool operator!=(const Quaternion& a, const Quaternion& b) {
  return !XMQuaternionEqual(a, b);
}

// Ray Implementation
Ray::Ray() : origin(Vector3::Zero), direction(Vector3::Forward) {
}

Ray::Ray(const Vector3& origin, const Vector3& direction) : origin(origin), direction(direction.Normalized()) {
}

Vector3 Ray::PointAt(float t) const {
  return origin + direction * t;
}

// Plane Implementation
Plane::Plane() : normal(Vector3::Up), d(0.0f) {
}

Plane::Plane(const Vector3& normal, float d) : normal(normal.Normalized()), d(d) {
}

Plane::Plane(const Vector3& a, const Vector3& b, const Vector3& c) {
  Vector3 ab = b - a;
  Vector3 ac = c - a;
  normal = ab.Cross(ac).Normalized();
  d = -normal.Dot(a);
}

float Plane::SignedDistance(const Vector3& point) const {
  return normal.Dot(point) + d;
}

// Sphere Implementation
Sphere::Sphere() : center(Vector3::Zero), radius(1.0f) {
}

Sphere::Sphere(const Vector3& center, float radius) : center(center), radius(radius) {
}

bool Sphere::Contains(const Vector3& point) const {
  return Vector3::DistanceSquared(center, point) <= radius * radius;
}

bool Sphere::Intersects(const Sphere& other) const {
  float distSq = Vector3::DistanceSquared(center, other.center);
  float radiusSum = radius + other.radius;
  return distSq <= radiusSum * radiusSum;
}

// AABB Implementation
AABB::AABB() : min(Vector3::Zero), max(Vector3::Zero) {
}

// Auto-flips to ensure min <= max
AABB::AABB(const Vector3& min, const Vector3& max) : min(Vector3::Min(min, max)), max(Vector3::Max(min, max)) {
}

Vector3 AABB::GetCenter() const {
  return (min + max) * 0.5f;
}

Vector3 AABB::GetExtents() const {
  return (max - min) * 0.5f;
}

bool AABB::Contains(const Vector3& point) const {
  return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z;
}

bool AABB::Intersects(const AABB& other) const {
  return min.x <= other.max.x && max.x >= other.min.x && min.y <= other.max.y && max.y >= other.min.y && min.z <= other.max.z &&
         max.z >= other.min.z;
}

bool AABB::Intersects(const Sphere& sphere) const {
  Vector3 closest = Vector3::Max(min, Vector3::Min(sphere.center, max));
  return Vector3::DistanceSquared(closest, sphere.center) <= sphere.radius * sphere.radius;
}

void AABB::Encapsulate(const Vector3& point) {
  min = Vector3::Min(min, point);
  max = Vector3::Max(max, point);
}

void AABB::Encapsulate(const AABB& other) {
  min = Vector3::Min(min, other.min);
  max = Vector3::Max(max, other.max);
}

// Circle Implementation
Circle::Circle() : center(Vector2::Zero), radius(0.0f) {
}

Circle::Circle(const Vector2& center, float radius) : center(center), radius(radius) {
}

bool Circle::Contains(const Vector2& point) const {
  return Vector2::DistanceSquared(center, point) <= radius * radius;
}

bool Circle::Intersects(const Circle& other) const {
  float radiusSum = radius + other.radius;
  return Vector2::DistanceSquared(center, other.center) <= radiusSum * radiusSum;
}

// Rect Implementation
Rect::Rect() : min(Vector2::Zero), max(Vector2::Zero) {
}

Rect::Rect(const Vector2& min, const Vector2& max) : min(Vector2::Min(min, max)), max(Vector2::Max(min, max)) {
}

Vector2 Rect::GetCenter() const {
  return (min + max) * 0.5f;
}

Vector2 Rect::GetExtents() const {
  return (max - min) * 0.5f;
}

bool Rect::Contains(const Vector2& point) const {
  return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
}

bool Rect::Intersects(const Rect& other) const {
  return min.x <= other.max.x && max.x >= other.min.x && min.y <= other.max.y && max.y >= other.min.y;
}

bool Rect::Intersects(const Circle& circle) const {
  float closestX = Clamp(circle.center.x, min.x, max.x);
  float closestY = Clamp(circle.center.y, min.y, max.y);
  Vector2 closest(closestX, closestY);
  return Vector2::DistanceSquared(closest, circle.center) <= circle.radius * circle.radius;
}

// Capsule helpers
namespace {

Vector3 ClosestPointOnSegment(const Vector3& a, const Vector3& b, const Vector3& p) {
  Vector3 ab = b - a;
  float abLenSq = ab.LengthSquared();
  if (NearZero(abLenSq, 0.000001f)) return a;
  float t = Clamp((p - a).Dot(ab) / abLenSq, 0.0f, 1.0f);
  return a + ab * t;
}

float SegmentDistanceSquared(const Vector3& p1, const Vector3& q1, const Vector3& p2, const Vector3& q2) {
  Vector3 d1 = q1 - p1;
  Vector3 d2 = q2 - p2;
  Vector3 r = p1 - p2;

  float a = d1.LengthSquared();
  float e = d2.LengthSquared();
  float f = d2.Dot(r);

  // Both segments degenerate to points
  if (NearZero(a, 0.000001f) && NearZero(e, 0.000001f)) {
    return (p1 - p2).LengthSquared();
  }

  float s, t;

  // First segment degenerates to a point
  if (NearZero(a, 0.000001f)) {
    s = 0.0f;
    t = Clamp(f / e, 0.0f, 1.0f);
  } else {
    float c = d1.Dot(r);
    // Second segment degenerates to a point
    if (NearZero(e, 0.000001f)) {
      t = 0.0f;
      s = Clamp(-c / a, 0.0f, 1.0f);
    } else {
      float b = d1.Dot(d2);
      float denom = a * e - b * b;

      if (!NearZero(denom, 0.000001f)) {
        s = Clamp((b * f - c * e) / denom, 0.0f, 1.0f);
      } else {
        s = 0.0f;
      }

      t = (b * s + f) / e;

      if (t < 0.0f) {
        t = 0.0f;
        s = Clamp(-c / a, 0.0f, 1.0f);
      } else if (t > 1.0f) {
        t = 1.0f;
        s = Clamp((b - c) / a, 0.0f, 1.0f);
      }
    }
  }

  Vector3 closest1 = p1 + d1 * s;
  Vector3 closest2 = p2 + d2 * t;
  return (closest1 - closest2).LengthSquared();
}

}  // namespace

// Capsule Implementation
Capsule::Capsule() : start(Vector3::Zero), end(Vector3::Zero), radius(0.0f) {
}

Capsule::Capsule(const Vector3& start, const Vector3& end, float radius) : start(start), end(end), radius(radius) {
}

bool Capsule::Contains(const Vector3& point) const {
  Vector3 closest = ClosestPointOnSegment(start, end, point);
  return Vector3::DistanceSquared(closest, point) <= radius * radius;
}

bool Capsule::Intersects(const Sphere& sphere) const {
  Vector3 closest = ClosestPointOnSegment(start, end, sphere.center);
  float radiusSum = radius + sphere.radius;
  return Vector3::DistanceSquared(closest, sphere.center) <= radiusSum * radiusSum;
}

bool Capsule::Intersects(const Capsule& other) const {
  float distSq = SegmentDistanceSquared(start, end, other.start, other.end);
  float radiusSum = radius + other.radius;
  return distSq <= radiusSum * radiusSum;
}

// Intersection Tests
bool Intersects(const Ray& ray, const Plane& plane, float& outDistance) {
  float denom = plane.normal.Dot(ray.direction);
  if (NearZero(denom)) {
    return false;
  }

  outDistance = -(plane.normal.Dot(ray.origin) + plane.d) / denom;
  return outDistance >= 0.0f;
}

bool Intersects(const Ray& ray, const Sphere& sphere, float& outDistance) {
  Vector3 oc = ray.origin - sphere.center;
  float a = ray.direction.Dot(ray.direction);
  float b = 2.0f * oc.Dot(ray.direction);
  float c = oc.Dot(oc) - sphere.radius * sphere.radius;

  float discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f) {
    return false;
  }

  outDistance = (-b - Sqrt(discriminant)) / (2.0f * a);
  if (outDistance < 0.0f) {
    outDistance = (-b + Sqrt(discriminant)) / (2.0f * a);
  }

  return outDistance >= 0.0f;
}

bool Intersects(const Ray& ray, const AABB& box, float& outDistance) {
  float tmin = 0.0f;
  float tmax = std::numeric_limits<float>::max();

  for (int i = 0; i < 3; ++i) {
    if (NearZero(ray.direction[i])) {
      if (ray.origin[i] < box.min[i] || ray.origin[i] > box.max[i]) {
        return false;
      }
    } else {
      float invD = 1.0f / ray.direction[i];
      float t1 = (box.min[i] - ray.origin[i]) * invD;
      float t2 = (box.max[i] - ray.origin[i]) * invD;

      if (t1 > t2) std::swap(t1, t2);

      tmin = Math::Max(tmin, t1);
      tmax = Math::Min(tmax, t2);

      if (tmin > tmax) {
        return false;
      }
    }
  }

  outDistance = tmin;
  return true;
}

// Signed intersection - returns negative outDistance if ray origin is inside the sphere
bool IntersectsSigned(const Ray& ray, const Sphere& sphere, float& outDistance) {
  Vector3 oc = ray.origin - sphere.center;
  float a = ray.direction.Dot(ray.direction);
  float b = 2.0f * oc.Dot(ray.direction);
  float c = oc.Dot(oc) - sphere.radius * sphere.radius;
  float discriminant = b * b - 4.0f * a * c;

  if (discriminant < 0.0f) return false;

  // Always return near intersection (may be negative if inside)
  outDistance = (-b - Sqrt(discriminant)) / (2.0f * a);
  return true;
}

// Signed intersection - returns negative outDistance if ray origin is inside the AABB
bool IntersectsSigned(const Ray& ray, const AABB& box, float& outDistance) {
  float tmin = -std::numeric_limits<float>::max();
  float tmax = std::numeric_limits<float>::max();

  for (int i = 0; i < 3; ++i) {
    if (NearZero(ray.direction[i])) {
      if (ray.origin[i] < box.min[i] || ray.origin[i] > box.max[i]) {
        return false;
      }
    } else {
      float invD = 1.0f / ray.direction[i];
      float t1 = (box.min[i] - ray.origin[i]) * invD;
      float t2 = (box.max[i] - ray.origin[i]) * invD;
      if (t1 > t2) std::swap(t1, t2);
      tmin = Max(tmin, t1);
      tmax = Min(tmax, t2);
      if (tmin > tmax) return false;
    }
  }

  outDistance = tmin;  // May be negative if inside
  return true;
}

}  // namespace Math
