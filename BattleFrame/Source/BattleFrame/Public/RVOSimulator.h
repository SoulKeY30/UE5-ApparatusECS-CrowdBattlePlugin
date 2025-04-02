// Copyright 2023, EastFoxStudio. All Rights Reserved.

#ifndef RVO_RVO_SIMULATOR_H_
#define RVO_RVO_SIMULATOR_H_

//#include "Rvo2DLibrary.h"
 /**
  * \file       RVOSimulator.h
  * \brief      Contains the RVOSimulator class.
  */

#include <limits>

#include "RVOVector2.h"

namespace RVO {
	/**
	 * \brief       Error value.
	 *
	 * A value equal to the largest unsigned integer that is returned in case
	 * of an error by functions in RVO::RVOSimulator.
	 */
	const size_t RVO_ERROR = std::numeric_limits<size_t>::max();

	/**
	 * \brief      Defines a directed line.
	 */
	class Line {
	public:
		/**
		 * \brief     A point on the directed line.
		 */
		Vector2 point;

		/**
		 * \brief     The direction of the directed line.
		 */
		Vector2 direction;
	};
}

#endif /* RVO_RVO_SIMULATOR_H_ */
