/*
    Copyright (C) 2001-2003 KSVG Team
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.


    This file includes excerpts from the Scalable Vector Graphics
    (SVG) 1.0 Specification (Proposed Recommendation)
    http://www.w3.org/TR/SVG

    Copyright � 2001 World Wide Web Consortium, (Massachusetts
    Institute of Technology, Institut National de Recherche en
    Informatique et en Automatique, Keio University).
    All Rights Reserved.
*/

#ifndef SVGSVGElement_H
#define SVGSVGElement_H

#include <dom/dom_string.h>
#include "SVGElement.h"
#include "SVGTests.h"
#include "SVGLangSpace.h"
#include "SVGExternalResourcesRequired.h"
#include "SVGStylable.h"
#include "SVGLocatable.h"
#include "SVGFitToViewBox.h"
#include "SVGZoomAndPan.h"

namespace KSVG
{
class SVGAnimatedLength;
class SVGPoint;
class SVGNumber;
class SVGAngle;
class SVGMatrix;
class SVGLength;
class SVGRect;
class SVGTransform;
class SVGViewSpec;
class SVGSVGElementImpl;

/**
 * A key interface definition is the SVGSVGElement interface, which is the
 * interface that corresponds to the 'svg' element. This interface contains
 * various miscellaneous commonly-used utility methods, such as matrix
 * operations and the ability to control the time of redraw on visual
 * rendering devices. SVGSVGElement extends ViewCSS and DocumentCSS to provide
 * access to the  computed values of properties and the override style sheet
 * as described in DOM2.
 *
 * For more information :
 * <a href="http://www.w3.org/TR/SVG/struct.html#NewDocument">
 * the 'svg' element</a>
 */
class SVGSVGElement :	public SVGElement,
 					 	public SVGTests,
					 	public SVGLangSpace,
					 	public SVGExternalResourcesRequired,
					 	public SVGStylable,
					 	public SVGLocatable,
					 	public SVGFitToViewBox,
						public SVGZoomAndPan
						//public events::EventTarget,
						//public events::DocumentEvent,
						//public css::ViewCSS,
						//public css::DocumentCSS
{
public:
	SVGSVGElement();
	SVGSVGElement(const SVGSVGElement &);
	SVGSVGElement &operator=(const SVGSVGElement &other);
	SVGSVGElement(SVGSVGElementImpl *);
	virtual ~SVGSVGElement();

	/**
	 * (Has no meaning or effect on outermost 'svg' elements.)
	 * The x-axis coordinate of one corner of the rectangular region into which an
	 * embedded 'svg' element is placed. If the attribute is not specified, the
	 * effect is as if a value of "0" were specified.
	 *
	 * This attribute is animatable.
	 *
	 * @return The x-axis coordinate of the viewport of the 'svg' element.
	 */
	SVGAnimatedLength x() const;

	/**
	 * (Has no meaning or effect on outermost 'svg' elements.)
	 * The x-axis coordinate of one corner of the rectangular region into which an
	 * embedded 'svg' element is placed. If the attribute is not specified, the
	 * effect is as if a value of "0" were  specified.
	 *
	 * This attribute is animatable.
	 *
	 * @return The y-axis coordinate of the viewport of the 'svg' element.
	 */
	SVGAnimatedLength y() const;

	/**
	 * For outermost 'svg' elements, the intrinsic width of the SVG document
	 * fragment. For embedded 'svg' elements, the width of the rectangular
	 * region into which the 'svg' element is placed.
	 * A negative value is an error (see <a
	 * href="http://www.w3.org/TR/SVG/implnote.html#ErrorProcessing"> Error
	 * processing  </a>).
	 * A value of zero disables rendering of the element. If the attribute is not
	 * specified, the effect is as if a value of "100%" were specified.
	 *
	 * This attribute is animatable.
	 *
	 * @return The width of the viewport of the 'svg' element.
	 */
	SVGAnimatedLength width() const;

	/**
	 * For outermost 'svg' elements, the intrinsic height of the SVG document
	 * fragment. For embedded 'svg' elements, the height of the rectangular region
	 * into which the 'svg' element is placed.
	 * A negative value is an error (see <a
	 * href="http://www.w3.org/TR/SVG/implnote.html#ErrorProcessing"> Error
	 * processing  </a>).
	 * A value of zero disables rendering of the element. If the attribute is not
	 * specified, the effect is as if a value of "100%" were specified.
	 *
	 * This attribute is animatable.
	 *
	 * @return The height of the viewport of the 'svg' element.
	 */
	SVGAnimatedLength height() const;

	/**
	 * The <code> contentScriptType </code> attribute identifies the default
	 * scripting language for the given document. This attribute sets the scripting
	 * language used to process the value strings in event attributes. The value
	 * content-type specifies a media type, per <a
	 * href="http://www.ietf.org/rfc/rfc2045.txt"> [RFC2045]</a>. The default value
	 * is "text/ecmascript".
	 *
	 * This attribute is not animatable.
	 *
	 * @exception
	 * NO_MODIFICATION_ALLOWED_ERR: Raised on an attempt to change the value of a
	 * readonly attribute.
	 */
	void setContentScriptType(const DOM::DOMString &);

	/**
	 * @return The value of the <code>contentScript</code> attribute on the given
	 * 'svg' element.
	 */
	DOM::DOMString contentScriptType() const;

	/**
	 * Identifies the default style sheet language for the given document. This
	 * attribute sets the style sheet language for the style attributes that are
	 * available on many elements. The value of the attribute consists of a media
	 * type, per <a href="http://www.ietf.org/rfc/rfc2045.txt">[RFC2045]</a>.
	 * The default value is "text/css".
	 */
	 void setContentStyleType(const DOM::DOMString &);

	/**
	 * @return The value of the <code>contentStyle</code> attribute on the given
	 * 'svg' element.
	 */
	DOM::DOMString contentStyleType() const;

	/**
	 * The position and size of the viewport (implicit or explicit) that
	 * corresponds to this 'svg' element. When the user agent is actually
	 * rendering the content,  then the position and size values represent the
	 * actual values when rendering.  The position and size values are unitless
	 * values in the coordinate system of  the parent element. If no parent element
	 * exists (i.e., 'svg' element represents  the root of the document tree), if
	 * this SVG document is embedded as part of  another document (e.g., via the
	 * HTML 'object' element), then the position and  size are unitless values in
	 * the coordinate system of the parent document. (If  the parent uses CSS or
	 * XSL layout, then unitless values represent pixel units  for the current CSS
	 * or XSL viewport, as described in the CSS2 specification.)  If the parent
	 * element does not have a coordinate system, then the user agent  should
	 * provide reasonable default values for this attribute.
	 *
	 * @return The viewport represented by a <code>SVGRect</rect>.
	 */
	SVGRect viewport() const;

	/**
	 * Size of a pixel units (as defined by CSS2) along the x-axis of the viewport,
	 * which represents a unit somewhere in the range of 70dpi to 120dpi, and, on
	 * systems that support this, might actually match the characteristics of the
	 * target medium. On systems where it is impossible to know the size of a
	 * pixel, a suitable default pixel size is provided.
	 *
	 * @return Corresponding size of a pixel unit along the x-axis of the viewport.
	 */
	float pixelUnitToMillimeterX() const;

	/**
	 * @return Corresponding size of a pixel unit along the y-axis of the viewport.
	 */
	float pixelUnitToMillimeterY() const;

	/**
	 * User interface (UI) events in DOM Level 2 indicate the screen positions at
	 * which the given UI event occurred. When the user agent actually knows the
	 * physical size of a "screen unit", this attribute will express that
	 * information; otherwise, user agents will provide a suitable default value
	 * such as .28mm.
	 *
	 * @return Corresponding size of a screen pixel along the x-axis of the
	 * viewport.
	 */
	 float screenPixelToMillimeterX() const;

	/**
	 * @return Corresponding size of a screen pixel along the y-axis of the
	 * viewport.
	 */
	float screenPixelToMillimeterY() const;

	/**
	 * The initial view (i.e., before magnification and panning) of the current
	 * innermost SVG document fragment can be either the "standard" view (i.e.,
	 * based on attributes on the 'svg' element such as fitBoxToViewport) or to a
	 * "custom" view (i.e., a hyperlink into a particular 'view' or other element -
	 * see <a href="http://www.w3.org/TR/SVG/linking.html#LinksIntoSVG"> Linking
	 * into SVG content: URI fragments and SVG views)</a>. If the initial view
	 * is the "standard" view, then this attribute is false. If the initial
	 * view is a "custom" view, then this attribute is true.
	 *
	 * @exception DOMException
	 *  NO_MODIFICATION_ALLOWED_ERR: Raised on an attempt to change the value of a
	 * readonly attribute.
	 */
	void setUseCurrentView(bool);

	/**
	 * @return Indicated whether the view is "custom" or not.
	 */
	bool useCurrentView() const;

	/**
	 * The definition of the initial view (i.e., before magnification and panning)
	 * of the current innermost SVG document fragment. The meaning depends on the
	 * situation:
	 *
	 * If the initial view was a "standard" view, then:
	 *
	 * the values for <code>viewBox</code>, <code>preserveAspectRatio</code> and
	 * <code>zoomAndPan</code> within <code>currentView</code> will match the
	 * values for the corresponding DOM attributes that  are on SVGSVGElement
	 * directly.
	 * the values for <code>transform</code> and <code>viewTarget</code> within
	 * <code>currentView</code> will be null.
	 *
	 * If the initial view was a link into a 'view' element, then:
	 *
	 * the values for <code>viewBox</code>, <code>preserveAspectRatio</code> and
	 * <code>zoomAndPan</code> within <code>currentView</code> will correspond to
	 * the corresponding attributes for the given 'view' element.
	 * the values for <code>transform</code> and <code>viewTarget</code> within
	 * <code>currentView </code>will be null
	 *
	 * If the initial view was a link into another element (i.e., other than a
	 *'view'), then:
	 *
	 * the values for <code>viewBox</code>, <code>preserveAspectRatio</code> and
	 * <code>zoomAndPan</code> within <code>currentView</code> will match the
	 * values for the corresponding DOM attributes that are on SVGSVGElement
	 * directly for the closest ancestor 'svg' element.
	 * the values for transform within <code>currentView</code> will be null.
	 * the <code>viewTarget</code> within <code>currentView</code> will represent
	 * the target of the link.
	 *
	 * If the initial view was a link into the SVG document fragment using an SVG
	 * view specification fragment identifier (i.e., #svgView(...)), then:
	 *
	 * the values for <code>viewBox</code>, <code>preserveAspectRatio</code>,
	 * <code>zoomAndPan</code>, <code>transform</code> and <code>viewTarget</code>
	 * within <code>currentView</code> will correspond to the values from the SVG
	 * view specification fragment identifier
	 *
	 * The object itself and its contents are both readonly.
	 */
	SVGViewSpec currentView() const;

	/**
	 * The currentScale attribute indicates the current scale factor relative to
	 * the initial view to take into account user magnification and panning
	 * operations, as described under
	 * <a href="http://www.w3.org/TR/SVG/interact.html#ZoomAndPanAttribute">
	 * Magnification and panning</a>. DOM attributes currentScale and
	 * currentTranslate are  equivalent to the 2x3 matrix [a b c d e f] =
	 * [currentScale 0 0 currentScale  currentTranslate.x currentTranslate.y].
	 * If "magnification" is enabled (i.e., zoomAndPan="magnify"), then the effect
	 * is as if an extra transformation were placed at the outermost level on the
	 * SVG document fragment (i.e., outside the outermost 'svg' element).
	 *
	 * @exception DOMException
	 * NO_MODIFICATION_ALLOWED_ERR: Raised on an attempt to change the value of a
	 * readonly attribute.
	 *
	 * @param The new value of the currentScale attribute.
	 */
	void setCurrentScale(float);

	/**
	 * @return The <code> currentScale </code> attribute value as described above.
	 */
	float currentScale() const;

	/**
	 * The corresponding translation factor that takes into account user
	 * "magnification".
	 *
	 * @return The translation factor represented by an <code> SVGPoint </code>.
	 */
	SVGPoint currentTranslate() const;

	/**
	 * Takes a time-out value which indicates that redraw shall not occur until:
	 *
	 * (a) the corresponding unsuspendRedraw(suspend_handle_id) call has been made
	 * (b) an unsuspendRedrawAll() call has been made
	 * (c) its timer has timed out.
	 *
	 * In environments that do not support interactivity (e.g., print media), then
	 * redraw shall not be suspended suspend_handle_id =
	 * suspendRedraw(max_wait_milliseconds) and  unsuspendRedraw(suspend_handle_id)
	 * must be packaged as balanced pairs. When you want to suspend redraw actions
	 * as a collection of SVG DOM changes occur, then precede the changes to the
	 * SVG DOM with a method call similar to suspend_handle_id =
	 * suspendRedraw(max_wait_milliseconds) and follow the changes with a method
	 * call similar to unsuspendRedraw(suspend_handle_id). Note that multiple
	 * suspendRedraw calls can be used at once and that each such method call is
	 * treated independently of  the other suspendRedraw method calls.
	 *
	 * @param max_wait_milliseconds The amount of time in milliseconds to hold off
	 * before redrawing the device.Values greater than 60 seconds will be
	 * truncated down to 60 seconds.
	 *
	 * @return A number which acts as a unique identifier for the given
	 * suspendRedraw() call. This value must be passed as the parameter to the
	 * corresponding unsuspendRedraw() method call.
	 */
	unsigned long suspendRedraw(unsigned long max_wait_milliseconds);

	/**
	 * Cancels a specified suspendRedraw() by providing a unique suspend_handle_id.
	 *
	 * @param suspend_handle_id A number which acts as a unique identifier for the
	 * desired suspendRedraw() call. The number supplied must be a value returned
	 * from a previous call to suspendRedraw().
	 */
	void unsuspendRedraw(unsigned long suspend_handle_id);

	/**
	 * Cancels all currently active suspendRedraw() method calls. This method is
	 * most useful at the very end of a set of SVG DOM calls to ensure that all
	 * pending suspendRedraw() method calls have been cancelled.
	 */
	void unsuspendRedrawAll();

	/**
	 * In rendering environments supporting interactivity, forces the user agent to
	 * immediately redraw all regions of the viewport that require updating.
	 */
	void forceRedraw();

	/**
	 * Suspends (i.e., pauses) all currently running animations that are defined
	 * within the SVG document fragment corresponding to this 'svg' element,
	 * causing the animation clock corresponding to this document fragment to stand
	 * still until it is paused.
	 */
	void pauseAnimations();

	/**
	 * Unsuspends (i.e., pauses) currently running animations that are defined
	 * within the SVG document fragment, causing the animation clock to continue
	 * from the time it was suspended.
	 */
	void unpauseAnimations();

	/**
	 * Returns true if this SVG Document fragment is in a paused state.
	 *
	 * @return Boolean indicating whether this SVG document fragment is in a paused
	 * state.
	 */
	bool animationsPaused();

	/**
	 * Returns the current time in seconds relative to the start time for the
	 * current SVG document fragment.
	 *
	 * @return The current time in seconds.
	 */
	float getCurrentTime();

	/**
	 * Adjusts the clock for this SVG document fragment, establishing a new current
	 * time.
	 *
	 * @param seconds The new current time in seconds relative to the start time
	 * for the current SVG document fragment.
	 */
	void setCurrentTime(float seconds);

	/**
	 * Returns the list of graphics elements whose rendered content intersects the
	 * supplied rectangle, honoring the 'pointer-events' property value on each
	 * candidate graphics element.
	 *
	 * @param rect The test rectangle. The values are in the initial coordinate
	 * system for the current 'svg' element.
	 * @param referenceElement If not null, then only return elements whose drawing
	 * order has them below the given reference element.
	 *
	 * @return A list of Elements whose content is intersects the supplied
	 * rectangle.
	 */
	DOM::NodeList getIntersectionList(const SVGRect &rect, const SVGElement
 &referenceElement);

	/**
	 * Returns the list of graphics elements whose rendered content is entirely
	 * contained within the supplied rectangle, honoring the 'pointer-events'
	 * property value on each candidate graphics element.
	 *
	 * @param rect The test rectangle. The values are in the initial coordinate
	 * system for the current 'svg' element.
	 * @param referenceElement If not null, then only return elements whose drawing
	 * order has them below the given reference element.
	 *
	 * @return A list of Elements whose content is enclosed by the supplied
	 * rectangle.
	 */
	DOM::NodeList getEnclosureList(const SVGRect &rect, const SVGElement
 &referenceElement);

	/**
	 * Returns true if the rendered content of the given element intersects the
	 * supplied rectangle, honoring the 'pointer-events' property value on each
	 * candidate graphics element.
	 *
	 * @param element The element on which to perform the given test.
	 * @param rect The test rectangle. The values are in the initial coordinate
	 * system for the current 'svg' element.
	 *
	 * @return True or false, depending on whether the given element intersects the
	 * supplied rectangle.
	 */
	bool checkIntersection(const SVGElement &element, const SVGRect &rect);

	/**
	 * Returns true if the rendered content of the given element is entirely
	 * contained within the supplied rectangle, honoring the 'pointer-events'
	 * property value on each candidate  graphics element.
	 *
	 * @param element The element on which to perform the given test.
	 * @param rect The test rectangle. The values are in the initial coordinate
	 * system for the current 'svg' element.
	 *
	 * @return True or false, depending on whether the given element is enclosed by
	 * the supplied rectangle.
	 */
	bool checkEnclosure(const SVGElement &element, const SVGRect &rect);

	/**
	 * Unselects any selected objects, including any selections of text
	 * strings and type-in bars.
	 */
	void deSelectAll();

	/**
	 * Creates an <code> SVGNumber </code> object outside of any document trees.
	 * The object is initialized to a value of zero.
	 *
	 * @return An <code> SVGNumber </code> object.
	 */
	SVGNumber createSVGNumber();

	/**
	 * Creates an <code> SVGLength </code> object outside of any document trees.
	 * The object is initialized to a value of zero user units.
	 *
	 * @return An <code> SVGLength </code> object.
	 */
	SVGLength createSVGLength();

	/**
	 * Creates an <code> SVGAngle </code> object outside of any document trees.
	 * The object is initialized to a value of zero degreesunitless).
	 *
	 * @return An <code> SVGAngle </code> object.
	 */
	SVGAngle createSVGAngle();

	/**
	 * Creates an <code> SVGPoint </code> object outside of any document trees.
	 * The object is initialized to the point (0,0) in the user coordinate system.
	 *
	 * @return An <code> SVGPoint </code> object.
	 */
	SVGPoint createSVGPoint();

	/**
	 * Creates an <code> SVGMatrix </code> object outside of any document trees.
	 * The object is initialized to a value of the identity matrix.
	 *
	 * @return An <code> SVGMatrix </code> object.
	 */
	SVGMatrix createSVGMatrix();

	/**
	 * Creates an <code> SVGRect </code> object outside of any document trees.
	 * The object is initialized such that all values are set to 0 user units.
	 *
	 * @return An <code> SVGRect </code> object.
	 */
	SVGRect createSVGRect();

	/**
	 * Creates an <code> SVGTransform </code> object outside of any document trees.
	 * The object is initialized to an identity matrix transform
	 * (SVG_TRANSFORM_MATRIX).
	 *
	 * @return An <code> SVGTransform </code> object.
	 */
	SVGTransform createSVGTransform();

	/**
	 * Creates an <code> SVGTransform </code> object outside of any document trees.
	 * The object is initialized to the given matrix transform (i.e.,
	 * SVG_TRANSFORM_MATRIX).
	 *
	 * @return An <code> SVGTransform </code> object.
	 */
	SVGTransform createSVGTransformFromMatrix(const SVGMatrix &matrix);

	/**
	 * Searches this SVG document fragment (i.e., the search is restricted to a
	 * subset of the document tree) for an <code> Element </code> whose id is given
	 * by elementId. If an <code> Element </code> is found, that <code> Element
	 * </code> is returned. If no such element exists, returns null. Behavior
	 * is not defined if more than one element has this id.
	 *
	 * @param elementId The unique id value for an element.
	 *
	 * @return The matching element.
	 */
	SVGElement getElementById(const DOM::DOMString &elementId);

	// Internal! - NOT PART OF THE SVG SPECIFICATION
	SVGSVGElementImpl *handle() const { return impl; }

private:
	SVGSVGElementImpl *impl;
};

}

#endif

// vim:ts=4:noet
