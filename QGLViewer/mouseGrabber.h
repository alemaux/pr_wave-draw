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

#ifndef QGLVIEWER_MOUSE_GRABBER_H
#define QGLVIEWER_MOUSE_GRABBER_H

#include "config.h"

#include <QEvent>

class QGLViewer;

namespace qglviewer {
class Camera;

/*! \brief Abstract class for objects that grab mouse focus in a QGLViewer.
  \class MouseGrabber mouseGrabber.h QGLViewer/mouseGrabber.h

  MouseGrabber are objects which react to the mouse cursor, usually when it
  hovers over them. This abstract class only provides an interface for all these
  objects: their actual behavior has to be defined in a derived class.

  <h3>How does it work ?</h3>

  All the created MouseGrabber are grouped in a MouseGrabberPool(). The
  QGLViewers parse this pool, calling all the MouseGrabbers' checkIfGrabsMouse()
  methods that setGrabsMouse() if desired.

  When a MouseGrabber grabsMouse(), it becomes the QGLViewer::mouseGrabber().
  All the mouse events (mousePressEvent(), mouseReleaseEvent(),
  mouseMoveEvent(), mouseDoubleClickEvent() and wheelEvent()) are then
  transmitted to the QGLViewer::mouseGrabber() instead of being normally
  processed. This continues while grabsMouse() (updated using
  checkIfGrabsMouse()) returns \c true.

  If you want to (temporarily) disable a specific MouseGrabbers, you can remove
  it from this pool using removeFromMouseGrabberPool(). You can also disable a
  MouseGrabber in a specific QGLViewer using
  QGLViewer::setMouseGrabberIsEnabled().

  <h3>Implementation details</h3>

  In order to make MouseGrabber react to mouse events, mouse tracking has to be
  activated in the QGLViewer which wants to use MouseGrabbers: \code init() {
  setMouseTracking(true); } \endcode Call \c QOpenGLWidget::hasMouseTracking()
  to get the current state of this flag.

  The \p camera parameter of the different mouse event methods is a pointer to
  the QGLViewer::camera() of the QGLViewer that uses the MouseGrabber. It can be
  used to compute 2D to 3D coordinates conversion using
  Camera::projectedCoordinatesOf() and Camera::unprojectedCoordinatesOf().

  Very complex behaviors can be implemented using this framework: auto-selected
  objects (no need to press a key to use them), automatic drop-down menus, 3D
  GUI, spinners using the wheelEvent(), and whatever your imagination creates.
  See the <a href="../examples/mouseGrabber.html">mouseGrabber example</a> for
  an illustration.

  Note that ManipulatedFrame are MouseGrabber: see the <a
  href="../examples/keyFrames.html">keyFrame example</a> for an illustration.
  Every created ManipulatedFrame is hence present in the MouseGrabberPool()
  (note however that ManipulatedCameraFrame are not inserted).

  <h3>Example</h3>

  Here is for instance a draft version of a MovableObject class. Instances of
  these class can freely be moved on screen using the mouse, as movable
  post-it-like notes: \code class MovableObject : public MouseGrabber
  {
  public:
  MovableObject() : pos(0,0), moved(false) {}

  void checkIfGrabsMouse(int x, int y, const qglviewer::Camera* const)
  {
    // MovableObject is active in a region of 5 pixels around its pos.
    // May depend on the actual shape of the object. Customize as desired.
    // Once clicked (moved = true), it keeps grabbing mouse until button is
  released. setGrabsMouse( moved || ((pos-QPoint(x,y)).manhattanLength() < 5) );
  }

  void mousePressEvent( QMouseEvent* const e, Camera* const) { prevPos =
  e->pos(); moved = true; }

  void mouseMoveEvent(QMouseEvent* const e, const Camera* const)
  {
    if (moved)
    {
    // Add position delta to current pos
    pos += e->pos() - prevPos;
    prevPos = e->pos();
    }
  }

  void mouseReleaseEvent(QMouseEvent* const, Camera* const) { moved = false; }

  void draw()
  {
    // The object is drawn centered on its pos, with different possible aspects:
    if (grabsMouse())
    if (moved)
    // Object being moved, maybe a transparent display
    else
    // Object ready to be moved, maybe a highlighted visual feedback
    else
    // Normal display
  }

  private:
  QPoint pos, prevPos;
  bool moved;
  };
  \endcode
  Note that the different event callback methods are called only once the
  MouseGrabber grabsMouse(). \nosubgrouping */
class QGLVIEWER_EXPORT MouseGrabber {
#ifndef DOXYGEN
  friend class ::QGLViewer;
#endif

public:
  MouseGrabber();
  /*! Virtual destructor. Removes the MouseGrabber from the MouseGrabberPool().
   */
  virtual ~MouseGrabber() { MouseGrabber::MouseGrabberPool_.removeAll(this); }

  /*! @name Mouse grabbing detection */
  //@{
public:
  /*! Pure virtual method, called by the QGLViewers before they test if the
  MouseGrabber grabsMouse(). Should setGrabsMouse() according to the mouse
  position.

  This is the core method of the MouseGrabber. It has to be overloaded in your
  derived class. Its goal is to update the grabsMouse() flag according to the
  mouse and MouseGrabber current positions, using setGrabsMouse().

  grabsMouse() is usually set to \c true when the mouse cursor is close enough
  to the MouseGrabber position. It should also be set to \c false when the mouse
  cursor leaves this region in order to release the mouse focus.

  \p x and \p y are the mouse cursor coordinates (Qt coordinate system: (0,0)
  corresponds to the upper left corner).

  A typical implementation will look like:
  \code
  // (posX,posY) is the position of the MouseGrabber on screen.
  // Here, distance to mouse must be less than 10 pixels to activate the
  MouseGrabber. setGrabsMouse( sqrt((x-posX)*(x-posX) + (y-posY)*(y-posY)) <
  10); \endcode

  If the MouseGrabber position is defined in 3D, use the \p camera parameter,
  corresponding to the calling QGLViewer Camera. Project on screen and then
  compare the projected coordinates: \code Vec proj =
  camera->projectedCoordinatesOf(myMouseGrabber->frame()->position());
  setGrabsMouse((fabs(x-proj.x) < 5) && (fabs(y-proj.y) < 2)); // Rectangular
  region \endcode

  See examples in the <a href="#_details">detailed description</a> section and
  in the <a href="../examples/mouseGrabber.html">mouseGrabber example</a>. */
  virtual void checkIfGrabsMouse(int x, int y, const Camera *const camera) = 0;

  /*! Returns \c true when the MouseGrabber grabs the QGLViewer's mouse events.

  This flag is set with setGrabsMouse() by the checkIfGrabsMouse() method. */
  bool grabsMouse() const { return grabsMouse_; }

protected:
  /*! Sets the grabsMouse() flag. Normally used by checkIfGrabsMouse(). */
  void setGrabsMouse(bool grabs) { grabsMouse_ = grabs; }
  //@}

  /*! @name MouseGrabber pool */
  //@{
public:
  /*! Returns a list containing pointers to all the active MouseGrabbers.

  Used by the QGLViewer to parse all the MouseGrabbers and to check if any of
  them grabsMouse() using checkIfGrabsMouse().

  You should not have to directly use this list. Use
  removeFromMouseGrabberPool() and addInMouseGrabberPool() to modify this list.

  \attention This method returns a \c QPtrList<MouseGrabber> with Qt 3 and a \c
  QList<MouseGrabber> with Qt 2. */
  static const QList<MouseGrabber *> &MouseGrabberPool() {
    return MouseGrabber::MouseGrabberPool_;
  }

  /*! Returns \c true if the MouseGrabber is currently in the MouseGrabberPool()
  list.

  Default value is \c true. When set to \c false using
  removeFromMouseGrabberPool(), the QGLViewers no longer checkIfGrabsMouse() on
  this MouseGrabber. Use addInMouseGrabberPool() to insert it back. */
  bool isInMouseGrabberPool() const {
    return MouseGrabber::MouseGrabberPool_.contains(
        const_cast<MouseGrabber *>(this));
  }
  void addInMouseGrabberPool();
  void removeFromMouseGrabberPool();
  void clearMouseGrabberPool(bool autoDelete = false);
  //@}

  /*! @name Mouse event handlers */
  //@{
protected:
  /*! Callback method called when the MouseGrabber grabsMouse() and a mouse
  button is pressed.


  The MouseGrabber will typically start an action or change its state when a
  mouse button is pressed. mouseMoveEvent() (called at each mouse displacement)
  will then update the MouseGrabber accordingly and mouseReleaseEvent() (called
  when the mouse button is released) will terminate this action.

  Use the \p event QMouseEvent::state() and QMouseEvent::button() to test the
  keyboard and button state and possibly change the MouseGrabber behavior
  accordingly.

  See the <a href="#_details">detailed description section</a> and the <a
  href="../examples/mouseGrabber.html">mouseGrabber example</a> for examples.

  See the \c QOpenGLWidget::mousePressEvent() and the \c QMouseEvent
  documentations for details. */
  virtual void mousePressEvent(QMouseEvent *const event, Camera *const camera) {
    Q_UNUSED(event);
    Q_UNUSED(camera);
  }
  /*! Callback method called when the MouseGrabber grabsMouse() and a mouse
  button is double clicked.

  See the \c QOpenGLWidget::mouseDoubleClickEvent() and the \c QMouseEvent
  documentations for details. */
  virtual void mouseDoubleClickEvent(QMouseEvent *const event,
                                     Camera *const camera) {
    Q_UNUSED(event);
    Q_UNUSED(camera);
  }
  /*! Mouse release event callback method. See mousePressEvent(). */
  virtual void mouseReleaseEvent(QMouseEvent *const event,
                                 Camera *const camera) {
    Q_UNUSED(event);
    Q_UNUSED(camera);
  }
  /*! Callback method called when the MouseGrabber grabsMouse() and the mouse is
  moved while a button is pressed.

  This method will typically update the state of the MouseGrabber from the mouse
  displacement. See the mousePressEvent() documentation for details. */
  virtual void mouseMoveEvent(QMouseEvent *const event, Camera *const camera) {
    Q_UNUSED(event);
    Q_UNUSED(camera);
  }
  /*! Callback method called when the MouseGrabber grabsMouse() and the mouse
  wheel is used.

  See the \c QOpenGLWidget::wheelEvent() and the \c QWheelEvent documentations
  for details. */
  virtual void wheelEvent(QWheelEvent *const event, Camera *const camera) {
    Q_UNUSED(event);
    Q_UNUSED(camera);
  }
  //@}

private:
  // Copy constructor and opertor= are declared private and undefined
  // Prevents everyone from trying to use them
  MouseGrabber(const MouseGrabber &);
  MouseGrabber &operator=(const MouseGrabber &);

  bool grabsMouse_;

  // Q G L V i e w e r   p o o l
  static QList<MouseGrabber *> MouseGrabberPool_;
};

} // namespace qglviewer

#endif // QGLVIEWER_MOUSE_GRABBER_H
