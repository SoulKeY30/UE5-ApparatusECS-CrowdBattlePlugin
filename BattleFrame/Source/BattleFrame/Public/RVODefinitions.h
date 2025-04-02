// Copyright 2023, EastFoxStudio. All Rights Reserved.

#ifndef RVO_DEFINITIONS_H_
#define RVO_DEFINITIONS_H_

 /**
  * \file       RVODefinitions.h
  * \brief      Contains functions and constants used in multiple classes.
  */

#include "RVOVector2.h"

  /**
   * \brief       A sufficiently small positive number.
   */
const float RVO_EPSILON = 0.00001f;

namespace RVO {
	//class Agent;
	//class Obstacle;
	//class RVOSimulator;

	/**
	 * \brief      Computes the squared distance from a line segment with the
	 *             specified endpoints to a specified point.
	 * \param      a               The first endpoint of the line segment.
	 * \param      b               The second endpoint of the line segment.
	 * \param      c               The point to which the squared distance is to
	 *                             be calculated.
	 * \return     The squared distance from the line segment to the point.
	 */
	inline float distSqPointLineSegment(const Vector2& a, const Vector2& b,
		const Vector2& c)
	{
		const float r = ((c - a) * (b - a)) / absSq(b - a);

		if (r < 0.0f) {
			return absSq(c - a);
		}
		else if (r > 1.0f) {
			return absSq(c - b);
		}
		else {
			return absSq(c - (a + r * (b - a)));
		}
	}

	/**
	 * \brief      Computes the signed distance from a line connecting the
	 *             specified points to a specified point.
	 * \param      a               The first point on the line.
	 * \param      b               The second point on the line.
	 * \param      c               The point to which the signed distance is to
	 *                             be calculated.
	 * \return     Positive when the point c lies to the left of the line ab.
	 */
	inline float leftOf(const Vector2& a, const Vector2& b, const Vector2& c)
	{
		return det(a - c, b - a);
	}

	/**
	 * \brief      Computes the square of a float.
	 * \param      a               The float to be squared.
	 * \return     The square of the float.
	 */
	inline float sqr(float a)
	{
		return a * a;
	}
}

#endif /* RVO_DEFINITIONS_H_ */
