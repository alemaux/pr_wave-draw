/****************************************************************************

 Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.

 This file is part of the QGLViewer library version 2.7.2.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License 
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain 
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/

#ifndef QGLVIEWER_QUATERNION_H
#define QGLVIEWER_QUATERNION_H

#include "vec.h"
#include <iostream>
#include <math.h>

namespace qglviewer {
/*! \brief The Quaternion class represents 3D rotations and orientations.
        \class Quaternion quaternion.h QGLViewer/quaternion.h

        The Quaternion is an appropriate (although not very intuitive)
   representation for 3D rotations and orientations. Many tools are provided to
   ease the definition of a Quaternion: see constructors, setAxisAngle(),
   setFromRotationMatrix(), setFromRotatedBasis().

        You can apply the rotation represented by the Quaternion to 3D points
   using rotate() and inverseRotate(). See also the Frame class that represents
   a coordinate system and provides other conversion functions like
   Frame::coordinatesOf() and Frame::transformOf().

        You can apply the Quaternion \c q rotation to the OpenGL matrices using:
        \code glMultMatrixd(q.matrix()); 
        // equivalent to glRotate(q.angle()*180.0/M_PI, q.axis().x, q.axis().y, q.axis().z); \endcode

        Quaternion is part of the \c qglviewer namespace, specify \c
   qglviewer::Quaternion or use the qglviewer namespace: 
  \code using namespace qglviewer; \endcode

        <h3>Internal representation</h3>

        The internal representation of a Quaternion corresponding to a rotation
   around axis \c axis, with an angle \c alpha is made of four qreals (i.e.
   doubles) q[i]: \code {q[0],q[1],q[2]} = sin(alpha/2) *
   {axis[0],axis[1],axis[2]} q[3] = cos(alpha/2) \endcode

        Note that certain implementations place the cosine term in first
   position (instead of last here).

        The Quaternion is always normalized, so that its inverse() is actually
   its conjugate.

        See also the Vec and Frame classes' documentations.
        \nosubgrouping */
class QGLVIEWER_EXPORT Quaternion {
public:
  /*! @name Defining a Quaternion */
  //@{
  /*! Default constructor, builds an identity rotation. */
  Quaternion() {
    q[0] = q[1] = q[2] = 0.0;
    q[3] = 1.0;
  }

  /*! Constructor from rotation axis (non null) and angle (in radians). See also
   * setAxisAngle(). */
  Quaternion(const Vec &axis, qreal angle) { setAxisAngle(axis, angle); }

  Quaternion(const Vec &from, const Vec &to);

  /*! Constructor from the four values of a Quaternion. First three values are
     axis*sin(angle/2) and last one is cos(angle/2).

          \attention The identity Quaternion is Quaternion(0,0,0,1) and \e not
     Quaternion(0,0,0,0) (which is not unitary). The default Quaternion()
     creates such identity Quaternion. */
  Quaternion(qreal q0, qreal q1, qreal q2, qreal q3) {
    q[0] = q0;
    q[1] = q1;
    q[2] = q2;
    q[3] = q3;
  }

  /*! Copy constructor. */
  Quaternion(const Quaternion &Q) {
    for (int i = 0; i < 4; ++i)
      q[i] = Q.q[i];
  }

  /*! Equal operator. */
  Quaternion &operator=(const Quaternion &Q) {
    for (int i = 0; i < 4; ++i)
      q[i] = Q.q[i];
    return (*this);
  }

  /*! Sets the Quaternion as a rotation of axis \p axis and angle \p angle (in
     radians).

          \p axis does not need to be normalized. A null \p axis will result in
     an identity Quaternion. */
  void setAxisAngle(const Vec &axis, qreal angle) {
    const qreal norm = axis.norm();
    if (norm < 1E-8) {
      // Null rotation
      q[0] = 0.0;
      q[1] = 0.0;
      q[2] = 0.0;
      q[3] = 1.0;
    } else {
      const qreal sin_half_angle = sin(angle / 2.0);
      q[0] = sin_half_angle * axis[0] / norm;
      q[1] = sin_half_angle * axis[1] / norm;
      q[2] = sin_half_angle * axis[2] / norm;
      q[3] = cos(angle / 2.0);
    }
  }

  /*! Sets the Quaternion value. See the Quaternion(qreal, qreal, qreal, qreal)
   * constructor documentation. */
  void setValue(qreal q0, qreal q1, qreal q2, qreal q3) {
    q[0] = q0;
    q[1] = q1;
    q[2] = q2;
    q[3] = q3;
  }

#ifndef DOXYGEN
  void setFromRotatedBase(const Vec &X, const Vec &Y, const Vec &Z);
#endif
  void setFromRotationMatrix(const qreal m[3][3]);
  void setFromRotatedBasis(const Vec &X, const Vec &Y, const Vec &Z);
  //@}

  /*! @name Accessing values */
  //@{
  Vec axis() const;
  qreal angle() const;
  void getAxisAngle(Vec &axis, qreal &angle) const;

  /*! Bracket operator, with a constant return value. \p i must range in [0..3].
   * See the Quaternion(qreal, qreal, qreal, qreal) documentation. */
  qreal operator[](int i) const { return q[i]; }

  /*! Bracket operator returning an l-value. \p i must range in [0..3]. See the
   * Quaternion(qreal, qreal, qreal, qreal) documentation. */
  qreal &operator[](int i) { return q[i]; }
  //@}

  /*! @name Rotation computations */
  //@{
  /*! Returns the composition of the \p a and \p b rotations.

          The order is important. When applied to a Vec \c v (see
     operator*(const Quaternion&, const Vec&) and rotate()) the resulting
     Quaternion acts as if \p b was applied first and then \p a was applied.
     This is obvious since the image \c v' of \p v by the composited rotation
     satisfies: \code v'= (a*b) * v = a * (b*v) \endcode

          Note that a*b usually differs from b*a.

          \attention For efficiency reasons, the resulting Quaternion is not
     normalized. Use normalize() in case of numerical drift with small rotation
     composition. */
  friend Quaternion operator*(const Quaternion &a, const Quaternion &b) {
    return Quaternion(
        a.q[3] * b.q[0] + b.q[3] * a.q[0] + a.q[1] * b.q[2] - a.q[2] * b.q[1],
        a.q[3] * b.q[1] + b.q[3] * a.q[1] + a.q[2] * b.q[0] - a.q[0] * b.q[2],
        a.q[3] * b.q[2] + b.q[3] * a.q[2] + a.q[0] * b.q[1] - a.q[1] * b.q[0],
        a.q[3] * b.q[3] - b.q[0] * a.q[0] - a.q[1] * b.q[1] - a.q[2] * b.q[2]);
  }

  /*! Quaternion rotation is composed with \p q.

          See operator*(), since this is equivalent to \c this = \c this * \p q.

          \note For efficiency reasons, the resulting Quaternion is not
     normalized. You may normalize() it after each application in case of
     numerical drift. */
  Quaternion &operator*=(const Quaternion &q) {
    *this = (*this) * q;
    return *this;
  }

  /*! Returns the image of \p v by the rotation \p q.

          Same as q.rotate(v). See rotate() and inverseRotate(). */
  friend Vec operator*(const Quaternion &q, const Vec &v) {
    return q.rotate(v);
  }

  Vec rotate(const Vec &v) const;
  Vec inverseRotate(const Vec &v) const;
  //@}

  /*! @name Inversion */
  //@{
  /*! Returns the inverse Quaternion (inverse rotation).

          Result has a negated axis() direction and the same angle(). A
     composition (see operator*()) of a Quaternion and its inverse() results in
     an identity function.

          Use invert() to actually modify the Quaternion. */
  Quaternion inverse() const { return Quaternion(-q[0], -q[1], -q[2], q[3]); }

  /*! Inverses the Quaternion (same rotation angle(), but negated axis()).

          See also inverse(). */
  void invert() {
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
  }

  /*! Negates all the coefficients of the Quaternion.

          This results in an other representation of the \e same rotation
     (opposite rotation angle, but with a negated axis direction: the two cancel
     out). However, note that the results of axis() and angle() are unchanged
     after a call to this method since angle() always returns a value in [0,pi].

          This method is mainly useful for Quaternion interpolation, so that the
     spherical interpolation takes the shortest path on the unit sphere. See
     slerp() for details. */
  void negate() {
    invert();
    q[3] = -q[3];
  }

  /*! Normalizes the Quaternion coefficients.

          This method should not need to be called since we only deal with unit
     Quaternions. This is however useful to prevent numerical drifts, especially
     with small rotational increments. See also normalized(). */
  qreal normalize() {
    const qreal norm =
        sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    for (int i = 0; i < 4; ++i)
      q[i] /= norm;
    return norm;
  }

  /*! Returns a normalized version of the Quaternion.

          See also normalize(). */
  Quaternion normalized() const {
    qreal Q[4];
    const qreal norm =
        sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    for (int i = 0; i < 4; ++i)
      Q[i] = q[i] / norm;
    return Quaternion(Q[0], Q[1], Q[2], Q[3]);
  }
  //@}

  /*! @name Associated matrix */
  //@{
  const GLdouble *matrix() const;
  void getMatrix(GLdouble m[4][4]) const;
  void getMatrix(GLdouble m[16]) const;

  void getRotationMatrix(qreal m[3][3]) const;

  const GLdouble *inverseMatrix() const;
  void getInverseMatrix(GLdouble m[4][4]) const;
  void getInverseMatrix(GLdouble m[16]) const;

  void getInverseRotationMatrix(qreal m[3][3]) const;
  //@}

  /*! @name Slerp interpolation */
  //@{
  static Quaternion slerp(const Quaternion &a, const Quaternion &b, qreal t,
                          bool allowFlip = true);
  static Quaternion squad(const Quaternion &a, const Quaternion &tgA,
                          const Quaternion &tgB, const Quaternion &b, qreal t);
  /*! Returns the "dot" product of \p a and \p b: a[0]*b[0] + a[1]*b[1] +
   * a[2]*b[2] + a[3]*b[3]. */
  static qreal dot(const Quaternion &a, const Quaternion &b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
  }

  Quaternion log();
  Quaternion exp();
  static Quaternion lnDif(const Quaternion &a, const Quaternion &b);
  static Quaternion squadTangent(const Quaternion &before,
                                 const Quaternion &center,
                                 const Quaternion &after);
  //@}

  /*! @name Random Quaternion */
  //@{
  static Quaternion randomQuaternion();
  //@}

  /*! @name XML representation */
  //@{
  explicit Quaternion(const QDomElement &element);
  QDomElement domElement(const QString &name, QDomDocument &document) const;
  void initFromDOMElement(const QDomElement &element);
//@}

#ifdef DOXYGEN
  /*! @name Output stream */
  //@{
  /*! Output stream operator. Enables debugging code like:
          \code
          Quaternion rot(...);
          cout << "Rotation=" << rot << endl;
          \endcode */
  std::ostream &operator<<(std::ostream &o, const qglviewer::Vec &);
//@}
#endif

private:
  /*! The internal data representation is private, use operator[] to access
   * values. */
  qreal q[4];
};

} // namespace qglviewer

std::ostream &operator<<(std::ostream &o, const qglviewer::Quaternion &);

#endif // QGLVIEWER_QUATERNION_H
