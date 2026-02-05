# Math Library for Games

A lightweight, high-performance math library built on top of DirectXMath, designed for game development and real-time graphics applications.

## Overview

### Features

- **DirectXMath Integration**: Leverages SIMD-optimized DirectXMath internally for performance
- **Intuitive API**: Clean C++ interface with operator overloads and fluent method chaining
- **Row-Major Convention**: Consistent with DirectX and Unreal Engine conventions
- **Complete Toolset**: Vectors, Matrices, Quaternions, and Collision Primitives

### Conventions

| Convention                | Description                                  |
| ------------------------- | -------------------------------------------- |
| **Coordinate System**     | Left-Handed (LH)                             |
| **Matrix Storage**        | Row-Major                                    |
| **Vector Type**           | Row Vector                                   |
| **Multiplication Order**  | `v' = v × M` (vector on left)                |
| **Transform Composition** | `M = S × R × T` (Scale → Rotate → Translate) |

### Memory Layout (Row-Major)

For a 4×4 matrix, elements are stored in memory as:

```
Index:  [0]  [1]  [2]  [3]  [4]  [5]  [6]  [7]  [8]  [9]  [10] [11] [12] [13] [14] [15]
Element: _11  _12  _13  _14  _21  _22  _23  _24  _31  _32  _33  _34  _41  _42  _43  _44
                                                                    ^^^^^^^^^^^^^^^^^^^
                                                                    Translation stored here
```

---

## API Reference

### Constants

| Constant  | Value        | Description     |
| --------- | ------------ | --------------- |
| `Pi`      | 3.1415926535 | $\pi$           |
| `TwoPi`   | 6.2831853071 | $2\pi$          |
| `PiOver2` | 1.5707963267 | $\frac{\pi}{2}$ |
| `PiOver4` | 0.7853981633 | $\frac{\pi}{4}$ |

---

### Utility Functions

#### Angle Conversion

```cpp
constexpr float ToRadians(float degrees);
constexpr float ToDegrees(float radians);
```

$$\text{radians} = \text{degrees} \times \frac{\pi}{180}$$

$$\text{degrees} = \text{radians} \times \frac{180}{\pi}$$

#### Comparison

```cpp
bool NearZero(float val, float epsilon = 0.001f);
bool NearEqual(float a, float b, float epsilon = 0.001f);
```

$$\text{NearZero}(x) = |x| < \epsilon$$

$$\text{NearEqual}(a, b) = |a - b| < \epsilon$$

#### Interpolation & Clamping

```cpp
float Clamp(float value, float min, float max);
float Lerp(float a, float b, float t);       // t is NOT clamped; values outside [0,1] extrapolate
float LerpClamped(float a, float b, float t); // t is clamped to [0,1]
```

$$\text{Clamp}(x, a, b) = \min(\max(x, a), b)$$

$$\text{Lerp}(a, b, t) = a + t(b - a) = (1-t)a + tb$$

$$\text{LerpClamped}(a, b, t) = \text{Lerp}(a, b, \text{Clamp}(t, 0, 1))$$

#### Validation Helpers

```cpp
bool IsFinite(float value);
bool IsFinite(const Vector2& v);
bool IsFinite(const Vector3& v);
bool IsFinite(const Vector4& v);
bool IsFinite(const Matrix3& m);
bool IsFinite(const Matrix4& m);
bool IsFinite(const Quaternion& q);

void AssertFinite(float value);
void AssertFinite(const Vector2& v);
// ... (same overloads for all types)
```

`IsFinite` returns `true` if all components are finite (not NaN or Inf). `AssertFinite` triggers a debug assertion on non-finite values.

#### Math Wrappers

```cpp
float Min(float a, float b);
float Max(float a, float b);
float Abs(float value);
float Sin(float angle);
float Cos(float angle);
float Tan(float angle);
float Asin(float value);
float Acos(float value);
float Atan2(float y, float x);
float Sqrt(float value);
```

---

### Vector2

2D vector for 2D graphics, UI, and texture coordinates.

#### Construction

```cpp
Vector2();                              // (0, 0)
Vector2(float x, float y);              // (x, y)
Vector2(const DirectX::XMFLOAT2& v);    // From XMFLOAT2
Vector2(DirectX::FXMVECTOR v);          // From XMVECTOR
```

#### Static Constants

| Constant         | Value    |
| ---------------- | -------- |
| `Vector2::Zero`  | $(0, 0)$ |
| `Vector2::One`   | $(1, 1)$ |
| `Vector2::UnitX` | $(1, 0)$ |
| `Vector2::UnitY` | $(0, 1)$ |

#### Operators

```cpp
Vector2 operator-() const;              // Negation
Vector2 operator+(const Vector2& v);    // Addition
Vector2 operator-(const Vector2& v);    // Subtraction
Vector2 operator*(float s);             // Scalar multiplication
Vector2 operator/(float s);             // Scalar division
float& operator[](int i);               // Element access
```

#### Methods

**Length**

```cpp
float Length() const;
float LengthSquared() const;
```

$$\|\mathbf{v}\| = \sqrt{x^2 + y^2}$$

$$\|\mathbf{v}\|^2 = x^2 + y^2$$

**Dot Product**

```cpp
float Dot(const Vector2& v) const;
```

$$\mathbf{a} \cdot \mathbf{b} = a_x b_x + a_y b_y = \|\mathbf{a}\| \|\mathbf{b}\| \cos\theta$$

**Cross Product (2D)**

```cpp
float Cross(const Vector2& v) const;
```

$$\mathbf{a} \times \mathbf{b} = a_x b_y - a_y b_x$$

Returns a scalar representing the signed area of the parallelogram.

**Normalization**

```cpp
void Normalize();
Vector2 Normalized() const;
```

$$\hat{\mathbf{v}} = \frac{\mathbf{v}}{\|\mathbf{v}\|}$$

**Perpendicular**

```cpp
Vector2 Perp() const;
```

$$\text{Perp}(x, y) = (-y, x)$$

Returns a vector rotated 90° counter-clockwise.

#### Static Methods

```cpp
static float Distance(const Vector2& a, const Vector2& b);
static float DistanceSquared(const Vector2& a, const Vector2& b);
static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);        // t NOT clamped
static Vector2 LerpClamped(const Vector2& a, const Vector2& b, float t); // t clamped to [0,1]
static Vector2 Min(const Vector2& a, const Vector2& b);
static Vector2 Max(const Vector2& a, const Vector2& b);
```

$$\text{Distance}(\mathbf{a}, \mathbf{b}) = \|\mathbf{b} - \mathbf{a}\|$$

---

### Vector3

3D vector for positions, directions, and colors.

#### Construction

```cpp
Vector3();                              // (0, 0, 0)
Vector3(float x, float y, float z);     // (x, y, z)
Vector3(const DirectX::XMFLOAT3& v);    // From XMFLOAT3
Vector3(DirectX::FXMVECTOR v);          // From XMVECTOR
```

#### Static Constants

| Constant           | Value       | Description            |
| ------------------ | ----------- | ---------------------- |
| `Vector3::Zero`    | $(0, 0, 0)$ | Origin                 |
| `Vector3::One`     | $(1, 1, 1)$ | Unit scale             |
| `Vector3::UnitX`   | $(1, 0, 0)$ | X-axis                 |
| `Vector3::UnitY`   | $(0, 1, 0)$ | Y-axis                 |
| `Vector3::UnitZ`   | $(0, 0, 1)$ | Z-axis                 |
| `Vector3::Forward` | $(0, 0, 1)$ | Forward direction (LH) |
| `Vector3::Right`   | $(1, 0, 0)$ | Right direction        |
| `Vector3::Up`      | $(0, 1, 0)$ | Up direction           |

#### Methods

**Dot Product**

```cpp
float Dot(const Vector3& v) const;
```

$$\mathbf{a} \cdot \mathbf{b} = a_x b_x + a_y b_y + a_z b_z$$

**Cross Product**

```cpp
Vector3 Cross(const Vector3& v) const;
```

$$\mathbf{a} \times \mathbf{b} = \begin{vmatrix} \mathbf{i} & \mathbf{j} & \mathbf{k} \\ a_x & a_y & a_z \\ b_x & b_y & b_z \end{vmatrix} = (a_y b_z - a_z b_y, a_z b_x - a_x b_z, a_x b_y - a_y b_x)$$

**Projection**

```cpp
Vector3 ProjectOnto(const Vector3& v) const;
```

$$\text{proj}_{\mathbf{v}}\mathbf{u} = \frac{\mathbf{u} \cdot \mathbf{v}}{\mathbf{v} \cdot \mathbf{v}} \mathbf{v}$$

**Reflection**

```cpp
Vector3 Reflect(const Vector3& normal) const;
```

$$\mathbf{r} = \mathbf{v} - 2(\mathbf{v} \cdot \mathbf{n})\mathbf{n}$$

Where $\mathbf{v}$ is the incident vector and $\mathbf{n}$ is the surface normal.

#### Static Methods

```cpp
static float Distance(const Vector3& a, const Vector3& b);
static float DistanceSquared(const Vector3& a, const Vector3& b);
static float Angle(const Vector3& a, const Vector3& b);
static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);        // t NOT clamped
static Vector3 LerpClamped(const Vector3& a, const Vector3& b, float t); // t clamped to [0,1]
static Vector3 Slerp(const Vector3& a, const Vector3& b, float t);
static Vector3 Min(const Vector3& a, const Vector3& b);
static Vector3 Max(const Vector3& a, const Vector3& b);
static void CreateOrthonormalBasis(const Vector3& normal, Vector3& outTangent, Vector3& outBitangent);
```

**Angle Between Vectors**

$$\theta = \arccos\left(\frac{\mathbf{a} \cdot \mathbf{b}}{\|\mathbf{a}\| \|\mathbf{b}\|}\right)$$

**Spherical Linear Interpolation (Slerp)**

$$\text{Slerp}(\mathbf{a}, \mathbf{b}, t) = \frac{\sin((1-t)\theta)}{\sin\theta}\mathbf{a} + \frac{\sin(t\theta)}{\sin\theta}\mathbf{b}$$

Where $\theta = \arccos(\hat{\mathbf{a}} \cdot \hat{\mathbf{b}})$

---

### Vector4

4D vector for homogeneous coordinates and RGBA colors.

#### Construction

```cpp
Vector4();                              // (0, 0, 0, 0)
Vector4(float x, float y, float z, float w);
Vector4(const Vector3& v, float w);     // Extend Vector3
Vector4(const DirectX::XMFLOAT4& v);
Vector4(DirectX::FXMVECTOR v);
```

#### Static Constants

| Constant         | Value          |
| ---------------- | -------------- |
| `Vector4::Zero`  | $(0, 0, 0, 0)$ |
| `Vector4::One`   | $(1, 1, 1, 1)$ |
| `Vector4::UnitX` | $(1, 0, 0, 0)$ |
| `Vector4::UnitY` | $(0, 1, 0, 0)$ |
| `Vector4::UnitZ` | $(0, 0, 1, 0)$ |
| `Vector4::UnitW` | $(0, 0, 0, 1)$ |

#### Methods

```cpp
float Length() const;
float LengthSquared() const;
float Dot(const Vector4& v) const;
void Normalize();
Vector4 Normalized() const;
Vector3 ToVector3() const;              // Drop w component
```

$$\|\mathbf{v}\| = \sqrt{x^2 + y^2 + z^2 + w^2}$$

#### Static Methods

```cpp
static Vector4 Lerp(const Vector4& a, const Vector4& b, float t);        // t NOT clamped
static Vector4 LerpClamped(const Vector4& a, const Vector4& b, float t); // t clamped to [0,1]
```

---

### Matrix3

3×3 matrix for rotations, scales, and 2D transformations.

#### Construction

```cpp
Matrix3();                              // Identity matrix
Matrix3(const DirectX::XMFLOAT3X3& m);
Matrix3(DirectX::FXMMATRIX m);
```

#### Static Constants

```cpp
static const Matrix3 Identity;
```

$$\mathbf{I}_3 = \begin{bmatrix} 1 & 0 & 0 \\ 0 & 1 & 0 \\ 0 & 0 & 1 \end{bmatrix}$$

#### Operators

```cpp
float& operator()(int row, int col);    // Element access
Matrix3 operator*(const Matrix3& m) const;
Vector3 operator*(const Vector3& v) const;
```

#### Methods

```cpp
Matrix3 Transposed() const;
Matrix3 Inverted() const;
float Determinant() const;
```

**Transpose**

$$\mathbf{M}^T_{ij} = \mathbf{M}_{ji}$$

**Determinant (3×3)**

$$\det(\mathbf{M}) = a(ei - fh) - b(di - fg) + c(dh - eg)$$

For matrix $\begin{bmatrix} a & b & c \\ d & e & f \\ g & h & i \end{bmatrix}$

#### Static Creation Methods

```cpp
static Matrix3 CreateRotationX(float radians);
static Matrix3 CreateRotationY(float radians);
static Matrix3 CreateRotationZ(float radians);
static Matrix3 CreateRotation(float angle);        // 2D rotation (around Z)
static Matrix3 CreateScale(float scale);
static Matrix3 CreateScale(float sx, float sy, float sz);
static Matrix3 CreateFromQuaternion(const Quaternion& q);
```

**Rotation Matrices**

$$\mathbf{R}_x(\theta) = \begin{bmatrix} 1 & 0 & 0 \\ 0 & \cos\theta & \sin\theta \\ 0 & -\sin\theta & \cos\theta \end{bmatrix}$$

$$\mathbf{R}_y(\theta) = \begin{bmatrix} \cos\theta & 0 & -\sin\theta \\ 0 & 1 & 0 \\ \sin\theta & 0 & \cos\theta \end{bmatrix}$$

$$\mathbf{R}_z(\theta) = \begin{bmatrix} \cos\theta & \sin\theta & 0 \\ -\sin\theta & \cos\theta & 0 \\ 0 & 0 & 1 \end{bmatrix}$$

**Scale Matrix**

$$\mathbf{S} = \begin{bmatrix} s_x & 0 & 0 \\ 0 & s_y & 0 \\ 0 & 0 & s_z \end{bmatrix}$$

---

### Matrix4

4×4 matrix for 3D transformations, view, and projection.

#### Construction

```cpp
Matrix4();                              // Identity matrix
Matrix4(const DirectX::XMFLOAT4X4& m);
Matrix4(DirectX::FXMMATRIX m);
```

#### Static Constants

```cpp
static const Matrix4 Identity;
```

$$\mathbf{I}_4 = \begin{bmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{bmatrix}$$

#### Operators

```cpp
float& operator()(int row, int col);
Matrix4 operator*(const Matrix4& m) const;
```

#### Transform Methods

```cpp
Vector3 TransformPoint(const Vector3& point) const;
Vector3 TransformVector(const Vector3& vector) const;
Vector3 TransformNormal(const Vector3& normal) const;
```

**TransformPoint** - Applies full transformation including translation (w=1):

$$\mathbf{p}' = \mathbf{p} \times \mathbf{M}$$

**TransformVector** - Ignores translation (w=0):

$$\mathbf{v}' = \mathbf{v} \times \mathbf{M}_{3\times3}$$

**TransformNormal** - Uses inverse-transpose for correct normal transformation:

$$\mathbf{n}' = \mathbf{n} \times (\mathbf{M}^{-1})^T$$

This is essential for non-uniform scaling to maintain perpendicularity.

#### Matrix Operations

```cpp
Matrix4 Transposed() const;
Matrix4 Inverted() const;
float Determinant() const;
```

#### Component Extraction

```cpp
Vector3 GetTranslation() const;
Vector3 GetScale() const;
Quaternion GetRotation() const;
```

#### Static Creation Methods - Transform

```cpp
static Matrix4 CreateTranslation(const Vector3& position);
static Matrix4 CreateScale(float scale);
static Matrix4 CreateScale(const Vector3& scale);
static Matrix4 CreateRotationX(float radians);
static Matrix4 CreateRotationY(float radians);
static Matrix4 CreateRotationZ(float radians);
static Matrix4 CreateFromQuaternion(const Quaternion& q);
static Matrix4 CreateFromTRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);
```

**Translation Matrix**

$$\mathbf{T} = \begin{bmatrix} 1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ t_x & t_y & t_z & 1 \end{bmatrix}$$

**TRS Composition**

$$\mathbf{M} = \mathbf{S} \times \mathbf{R} \times \mathbf{T}$$

The multiplication order ensures: Scale first → Rotate → Translate last.

#### Static Creation Methods - View

```cpp
static Matrix4 CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up);
```

Creates a left-handed view matrix.

$$\mathbf{V} = \begin{bmatrix} r_x & u_x & f_x & 0 \\ r_y & u_y & f_y & 0 \\ r_z & u_z & f_z & 0 \\ -\mathbf{r} \cdot \mathbf{eye} & -\mathbf{u} \cdot \mathbf{eye} & -\mathbf{f} \cdot \mathbf{eye} & 1 \end{bmatrix}$$

Where:
- $\mathbf{f} = \text{normalize}(\mathbf{target} - \mathbf{eye})$ (forward)
- $\mathbf{r} = \text{normalize}(\mathbf{up} \times \mathbf{f})$ (right)
- $\mathbf{u} = \mathbf{f} \times \mathbf{r}$ (up)

#### Static Creation Methods - Projection

```cpp
static Matrix4 CreatePerspectiveFOV(float fovY, float aspectRatio, float nearZ, float farZ);
static Matrix4 CreateOrthographic(float width, float height, float nearZ, float farZ);
```

**Perspective Projection (LH)**

$$\mathbf{P} = \begin{bmatrix} \frac{1}{\text{aspect} \cdot \tan(\frac{\text{fov}}{2})} & 0 & 0 & 0 \\ 0 & \frac{1}{\tan(\frac{\text{fov}}{2})} & 0 & 0 \\ 0 & 0 & \frac{f}{f-n} & 1 \\ 0 & 0 & \frac{-nf}{f-n} & 0 \end{bmatrix}$$

**Orthographic Projection (LH)**

$$\mathbf{P} = \begin{bmatrix} \frac{2}{w} & 0 & 0 & 0 \\ 0 & \frac{2}{h} & 0 & 0 \\ 0 & 0 & \frac{1}{f-n} & 0 \\ 0 & 0 & \frac{-n}{f-n} & 1 \end{bmatrix}$$

---

### Quaternion

Unit quaternion for rotation representation. Avoids gimbal lock and enables smooth interpolation.

#### Construction

```cpp
Quaternion();                           // Identity (0, 0, 0, 1)
Quaternion(float x, float y, float z, float w);
Quaternion(const DirectX::XMFLOAT4& q);
Quaternion(DirectX::FXMVECTOR q);
```

A quaternion is represented as:

$$\mathbf{q} = w + xi + yj + zk = (x, y, z, w)$$

Where $i^2 = j^2 = k^2 = ijk = -1$

#### Static Constants

```cpp
static const Quaternion Identity;       // (0, 0, 0, 1)
```

#### Operators

```cpp
Quaternion operator*(const Quaternion& q) const;  // Concatenate rotations
```

**Quaternion Multiplication (Hamilton Product)**

$$\mathbf{q}_1 \mathbf{q}_2 = (w_1 w_2 - \mathbf{v}_1 \cdot \mathbf{v}_2, w_1 \mathbf{v}_2 + w_2 \mathbf{v}_1 + \mathbf{v}_1 \times \mathbf{v}_2)$$

Where $\mathbf{v} = (x, y, z)$ is the vector part.

#### Methods

```cpp
float Length() const;
float LengthSquared() const;
void Normalize();
Quaternion Normalized() const;
Quaternion Conjugate() const;
Quaternion Inverse() const;
float Dot(const Quaternion& q) const;
```

**Conjugate**

$$\mathbf{q}^* = (w, -x, -y, -z)$$

**Inverse** (for unit quaternions, inverse equals conjugate)

$$\mathbf{q}^{-1} = \frac{\mathbf{q}^*}{\|\mathbf{q}\|^2}$$

**Dot Product**

$$\mathbf{q}_1 \cdot \mathbf{q}_2 = w_1 w_2 + x_1 x_2 + y_1 y_2 + z_1 z_2$$

#### Rotation Methods

```cpp
Vector3 RotateVector(const Vector3& v) const;
Matrix4 ToMatrix() const;
void ToAxisAngle(Vector3& outAxis, float& outAngle) const;
Vector3 ToEulerAngles() const;          // Returns (pitch, yaw, roll)
```

**Rotate Vector**

$$\mathbf{v}' = \mathbf{q} \mathbf{v} \mathbf{q}^*$$

Where $\mathbf{v}$ is treated as a pure quaternion $(0, v_x, v_y, v_z)$.

**Quaternion to Rotation Matrix**

$$\mathbf{R} = \begin{bmatrix} 1 - 2(y^2 + z^2) & 2(xy + wz) & 2(xz - wy) \\ 2(xy - wz) & 1 - 2(x^2 + z^2) & 2(yz + wx) \\ 2(xz + wy) & 2(yz - wx) & 1 - 2(x^2 + y^2) \end{bmatrix}$$

#### Static Creation Methods

```cpp
static Quaternion CreateFromAxisAngle(const Vector3& axis, float angle);
static Quaternion CreateFromEulerAngles(float pitch, float yaw, float roll);
static Quaternion CreateFromRotationMatrix(const Matrix4& m);
```

**Axis-Angle to Quaternion**

$$\mathbf{q} = \left(\sin\frac{\theta}{2} \cdot \mathbf{axis}, \cos\frac{\theta}{2}\right)$$

#### Interpolation

```cpp
static Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t);
static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);
```

**Spherical Linear Interpolation (Slerp)**

$$\text{Slerp}(\mathbf{q}_1, \mathbf{q}_2, t) = \frac{\sin((1-t)\theta)}{\sin\theta}\mathbf{q}_1 + \frac{\sin(t\theta)}{\sin\theta}\mathbf{q}_2$$

Where $\theta = \arccos(\mathbf{q}_1 \cdot \mathbf{q}_2)$

#### Utility Methods

```cpp
static Quaternion LookRotation(const Vector3& forward, const Vector3& up);
static Quaternion FromToRotation(const Vector3& from, const Vector3& to);
```

**FromToRotation** - Creates the shortest arc rotation from one direction to another.

---

### 2D Collision Primitives

#### Circle

```cpp
struct Circle {
    Vector2 center;
    float radius;

    Circle();                               // center=(0,0), radius=0
    Circle(const Vector2& center, float radius);
    bool Contains(const Vector2& point) const;
    bool Intersects(const Circle& other) const;
};
```

**Contains Point**

$$\|\mathbf{p} - \mathbf{c}\|^2 \leq r^2$$

**Circle-Circle Intersection**

$$\|\mathbf{c}_1 - \mathbf{c}_2\|^2 \leq (r_1 + r_2)^2$$

#### Rect (2D AABB)

```cpp
struct Rect {
    Vector2 min;
    Vector2 max;

    Rect();                                 // min=(0,0), max=(0,0)
    Rect(const Vector2& min, const Vector2& max); // Auto-flips if min > max
    Vector2 GetCenter() const;
    Vector2 GetExtents() const;
    bool Contains(const Vector2& point) const;
    bool Intersects(const Rect& other) const;
    bool Intersects(const Circle& circle) const;
};
```

> **Note:** Like AABB, the constructor automatically flips components so that `min <= max` per axis.

**Center and Extents**

$$\mathbf{center} = \frac{\mathbf{min} + \mathbf{max}}{2}$$

$$\mathbf{extents} = \frac{\mathbf{max} - \mathbf{min}}{2}$$

**Rect-Rect Intersection**

$$\text{intersects} = (\min_1 \leq \max_2) \land (\max_1 \geq \min_2) \text{ for both axes}$$

**Rect-Circle Intersection (Closest Point)**

Clamp the circle center to the rect bounds, then check distance:

$$\mathbf{closest} = \text{clamp}(\mathbf{c}, \mathbf{min}, \mathbf{max})$$

$$\|\mathbf{closest} - \mathbf{c}\|^2 \leq r^2$$

---

### 3D Collision Primitives

#### Ray

```cpp
struct Ray {
    Vector3 origin;
    Vector3 direction;  // Normalized

    Ray();
    Ray(const Vector3& origin, const Vector3& direction);
    Vector3 PointAt(float t) const;
};
```

$$\mathbf{P}(t) = \mathbf{origin} + t \cdot \mathbf{direction}$$

#### Plane

```cpp
struct Plane {
    Vector3 normal;
    float d;

    Plane();
    Plane(const Vector3& normal, float d);
    Plane(const Vector3& a, const Vector3& b, const Vector3& c);  // From 3 points
    float SignedDistance(const Vector3& point) const;
};
```

Plane equation: $\mathbf{n} \cdot \mathbf{p} + d = 0$

**Signed Distance**

$$\text{dist} = \mathbf{n} \cdot \mathbf{p} + d$$

Positive = front side, Negative = back side.

#### Sphere

```cpp
struct Sphere {
    Vector3 center;
    float radius;

    Sphere();
    Sphere(const Vector3& center, float radius);
    bool Contains(const Vector3& point) const;
    bool Intersects(const Sphere& other) const;
};
```

**Contains Point**

$$\|\mathbf{p} - \mathbf{c}\|^2 \leq r^2$$

**Sphere-Sphere Intersection**

$$\|\mathbf{c}_1 - \mathbf{c}_2\|^2 \leq (r_1 + r_2)^2$$

#### AABB (Axis-Aligned Bounding Box)

```cpp
struct AABB {
    Vector3 min;
    Vector3 max;

    AABB();
    AABB(const Vector3& min, const Vector3& max); // Auto-flips if min > max
    Vector3 GetCenter() const;
    Vector3 GetExtents() const;
    bool Contains(const Vector3& point) const;
    bool Intersects(const AABB& other) const;
    bool Intersects(const Sphere& sphere) const;
    void Encapsulate(const Vector3& point);
    void Encapsulate(const AABB& other);
};
```

> **Note:** The constructor automatically flips components so that `min <= max` per axis. Passing `AABB(Vector3(10,10,10), Vector3(0,0,0))` produces a valid box with `min=(0,0,0)` and `max=(10,10,10)`.

**Center and Extents**

$$\mathbf{center} = \frac{\mathbf{min} + \mathbf{max}}{2}$$

$$\mathbf{extents} = \frac{\mathbf{max} - \mathbf{min}}{2}$$

**AABB-AABB Intersection**

$$\text{intersects} = (\min_1 \leq \max_2) \land (\max_1 \geq \min_2) \text{ for all axes}$$

#### Capsule

```cpp
struct Capsule {
    Vector3 start;
    Vector3 end;
    float radius;

    Capsule();                              // start=(0,0,0), end=(0,0,0), radius=0
    Capsule(const Vector3& start, const Vector3& end, float radius);
    bool Contains(const Vector3& point) const;
    bool Intersects(const Sphere& sphere) const;
    bool Intersects(const Capsule& other) const;
};
```

A capsule is the Minkowski sum of a line segment and a sphere — the set of all points within `radius` distance of the segment from `start` to `end`.

**Closest Point on Segment**

$$t = \text{clamp}\left(\frac{(\mathbf{p} - \mathbf{a}) \cdot (\mathbf{b} - \mathbf{a})}{\|\mathbf{b} - \mathbf{a}\|^2}, 0, 1\right)$$

$$\mathbf{closest} = \mathbf{a} + t(\mathbf{b} - \mathbf{a})$$

**Contains Point**

$$\|\mathbf{closest} - \mathbf{p}\|^2 \leq r^2$$

**Capsule-Sphere Intersection**

$$\text{dist}^2(\text{segment}, \mathbf{c}_{\text{sphere}}) \leq (r_{\text{capsule}} + r_{\text{sphere}})^2$$

**Capsule-Capsule Intersection**

$$\text{dist}^2(\text{segment}_1, \text{segment}_2) \leq (r_1 + r_2)^2$$

Segment-to-segment distance uses the standard algorithm from *Real-Time Collision Detection* (Ericson), handling degenerate cases (point-point, point-segment, parallel segments).

---

### Intersection Tests

```cpp
// Standard intersection — outDistance >= 0 (clamped to front of ray)
bool Intersects(const Ray& ray, const Plane& plane, float& outDistance);
bool Intersects(const Ray& ray, const Sphere& sphere, float& outDistance);
bool Intersects(const Ray& ray, const AABB& box, float& outDistance);

// Signed intersection — outDistance may be negative if ray origin is inside the primitive
bool IntersectsSigned(const Ray& ray, const Sphere& sphere, float& outDistance);
bool IntersectsSigned(const Ray& ray, const AABB& box, float& outDistance);
```

`Intersects` always returns `outDistance >= 0`. When the ray origin is inside the primitive, it returns the exit point.

`IntersectsSigned` returns the near intersection which may be negative, useful for detecting whether the origin is inside a volume and computing penetration depth.

**Ray-Plane Intersection**

$$t = -\frac{\mathbf{n} \cdot \mathbf{o} + d}{\mathbf{n} \cdot \mathbf{d}}$$

Where $\mathbf{o}$ is ray origin and $\mathbf{d}$ is ray direction.

**Ray-Sphere Intersection**

Solve quadratic: $at^2 + bt + c = 0$

Where:
- $a = \mathbf{d} \cdot \mathbf{d}$
- $b = 2(\mathbf{o} - \mathbf{c}) \cdot \mathbf{d}$
- $c = (\mathbf{o} - \mathbf{c}) \cdot (\mathbf{o} - \mathbf{c}) - r^2$

$$t = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}$$

**Ray-AABB Intersection (Slab Method)**

For each axis, compute entry and exit times:

$$t_{\min} = \frac{\text{box}_{\min} - \mathbf{o}}{\mathbf{d}}, \quad t_{\max} = \frac{\text{box}_{\max} - \mathbf{o}}{\mathbf{d}}$$

Intersection occurs when $\max(t_{\min}) < \min(t_{\max})$ across all axes.

---

## Usage Examples

### Basic Vector Operations

```cpp
using namespace Math;

Vector3 a(1.0f, 2.0f, 3.0f);
Vector3 b(4.0f, 5.0f, 6.0f);

Vector3 sum = a + b;                    // (5, 7, 9)
float dot = a.Dot(b);                   // 32
Vector3 cross = a.Cross(b);             // (-3, 6, -3)
Vector3 normalized = a.Normalized();    // Unit vector
```

### Transform Composition

```cpp
// Create individual transforms
Matrix4 scale = Matrix4::CreateScale(2.0f);
Matrix4 rotation = Matrix4::CreateRotationY(ToRadians(45.0f));
Matrix4 translation = Matrix4::CreateTranslation(Vector3(10.0f, 0.0f, 0.0f));

// Compose: Scale -> Rotate -> Translate
Matrix4 world = scale * rotation * translation;

// Transform a point
Vector3 localPos(1.0f, 0.0f, 0.0f);
Vector3 worldPos = world.TransformPoint(localPos);
```

### Camera Setup

```cpp
Vector3 eye(0.0f, 5.0f, -10.0f);
Vector3 target(0.0f, 0.0f, 0.0f);
Vector3 up = Vector3::Up;

Matrix4 view = Matrix4::CreateLookAt(eye, target, up);
Matrix4 proj = Matrix4::CreatePerspectiveFOV(
    ToRadians(60.0f),   // FOV
    16.0f / 9.0f,       // Aspect ratio
    0.1f,               // Near plane
    1000.0f             // Far plane
);

Matrix4 viewProj = view * proj;
```

### Quaternion Rotation

```cpp
// Create rotation around Y-axis
Quaternion rotation = Quaternion::CreateFromAxisAngle(Vector3::UnitY, ToRadians(90.0f));

// Rotate a vector
Vector3 forward = Vector3::Forward;
Vector3 rotated = rotation.RotateVector(forward);  // Now pointing right

// Interpolate between rotations
Quaternion start = Quaternion::Identity;
Quaternion end = Quaternion::CreateFromAxisAngle(Vector3::UnitZ, ToRadians(180.0f));
Quaternion halfway = Quaternion::Slerp(start, end, 0.5f);  // 90 degrees
```

### Collision Detection

```cpp
// Ray casting
Ray ray(Vector3(0.0f, 10.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
Plane ground(Vector3::UnitY, 0.0f);

float distance;
if (Intersects(ray, ground, distance)) {
    Vector3 hitPoint = ray.PointAt(distance);  // (0, 0, 0)
}

// AABB intersection
AABB box1(Vector3(-1, -1, -1), Vector3(1, 1, 1));
AABB box2(Vector3(0, 0, 0), Vector3(2, 2, 2));

if (box1.Intersects(box2)) {
    // Boxes overlap
}

// 2D collision
Circle circle(Vector2(0.0f, 0.0f), 5.0f);
Rect rect(Vector2(-3.0f, -3.0f), Vector2(3.0f, 3.0f));

if (rect.Intersects(circle)) {
    // Rect and circle overlap
}

// 3D capsule collision
Capsule capsule(Vector3(0, 0, 0), Vector3(0, 10, 0), 2.0f);
Sphere sphere(Vector3(4, 5, 0), 3.0f);

if (capsule.Intersects(sphere)) {
    // Capsule and sphere overlap
}

// Detect ray origin inside a sphere
Sphere bigSphere(Vector3::Zero, 10.0f);
Ray ray2(Vector3::Zero, Vector3::UnitZ); // Origin at center
float signedDist;
if (IntersectsSigned(ray2, bigSphere, signedDist)) {
    // signedDist < 0 means origin is inside the sphere
}
```

---

## Edge Case Behaviors

The following edge cases produce degenerate (NaN/Inf) results by design rather than throwing exceptions:

| Operation                                      | Condition                  | Result                               |
| ---------------------------------------------- | -------------------------- | ------------------------------------ |
| `Vector2/3/4::operator/=(s)`                   | `s == 0`                   | NaN/Inf components                   |
| `Vector2/3/4::Normalized()`                    | Zero-length vector         | Implementation-defined (NaN or zero) |
| `Matrix3/4::Inverted()`                        | Singular matrix (det == 0) | NaN matrix                           |
| `Quaternion::CreateFromAxisAngle(axis, angle)` | Zero-length axis           | NaN quaternion (asserts in debug)    |
| `Plane(a, b, c)`                               | Collinear points           | Degenerate normal (NaN or zero)      |
| `Lerp(a, b, t)`                                | `t` outside [0,1]          | Extrapolates beyond `[a, b]`         |
| `LerpClamped(a, b, t)`                         | `t` outside [0,1]          | Clamped to `[a, b]`                  |

Use `IsFinite()` to check for degenerate results and `AssertFinite()` to catch them during development.
