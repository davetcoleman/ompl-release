/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2010, Rice University
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Rice University nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ioan Sucan */

#include "ompl/base/spaces/SO3StateSpace.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include "ompl/tools/config/MagicConstants.h"
#include <boost/math/constants/constants.hpp>

static const double MAX_QUATERNION_NORM_ERROR = 1e-9;

/// @cond IGNORE
namespace ompl
{
    namespace base
    {
        static inline void computeAxisAngle(SO3StateSpace::StateType &q, double ax, double ay, double az, double angle)
        {
            double norm = sqrt(ax * ax + ay * ay + az * az);
            if (norm < MAX_QUATERNION_NORM_ERROR)
                q.setIdentity();
            else
            {
                double s = sin(angle / 2.0);
                q.x = s * ax / norm;
                q.y = s * ay / norm;
                q.z = s * az / norm;
                q.w = cos(angle / 2.0);
            }
        }

        /* Standard quaternion multiplication: q = q0 * q1 */
        static inline void quaternionProduct(SO3StateSpace::StateType &q, const SO3StateSpace::StateType& q0, const SO3StateSpace::StateType& q1)
        {
            q.x = q0.w*q1.x + q0.x*q1.w + q0.y*q1.z - q0.z*q1.y;
            q.y = q0.w*q1.y + q0.y*q1.w + q0.z*q1.x - q0.x*q1.z;
            q.z = q0.w*q1.z + q0.z*q1.w + q0.x*q1.y - q0.y*q1.x;
            q.w = q0.w*q1.w - q0.x*q1.x - q0.y*q1.y - q0.z*q1.z;
        }
    }
}
/// @endcond

void ompl::base::SO3StateSpace::StateType::setAxisAngle(double ax, double ay, double az, double angle)
{
    computeAxisAngle(*this, ax, ay, az, angle);
}

void ompl::base::SO3StateSpace::StateType::setIdentity(void)
{
    x = y = z = 0.0;
    w = 1.0;
}

void ompl::base::SO3StateSampler::sampleUniform(State *state)
{
    rng_.quaternion(&state->as<SO3StateSpace::StateType>()->x);
}

void ompl::base::SO3StateSampler::sampleUniformNear(State *state, const State *near, const double distance)
{
    if (distance >= .25 * boost::math::constants::pi<double>())
    {
        sampleUniform(state);
        return;
    }
    double d = rng_.uniform01();
    SO3StateSpace::StateType q,
        *qs = static_cast<SO3StateSpace::StateType*>(state);
    const SO3StateSpace::StateType *qnear = static_cast<const SO3StateSpace::StateType*>(near);
    computeAxisAngle(q, rng_.gaussian01(), rng_.gaussian01(), rng_.gaussian01(), 2.*pow(d,1./3.)*distance);
    quaternionProduct(*qs, *qnear, q);
}

void ompl::base::SO3StateSampler::sampleGaussian(State *state, const State * mean, const double stdDev)
{
    // CDF of N(0, 1.17) at -pi/4 is approx. .25, so there's .25 probability
    // weight in each tail. Since the maximum distance in SO(3) is pi/2, we're
    // essentially as likely to sample a state within distance [0, pi/4] as
    // within distance [pi/4, pi/2]. With most weight in the tails (that wrap
    // around in case of quaternions) we might as well sample uniformly.
    if (stdDev > 1.17)
    {
        sampleUniform(state);
        return;
    }
    double x = rng_.gaussian(0, stdDev), y = rng_.gaussian(0, stdDev), z = rng_.gaussian(0, stdDev),
        theta = std::sqrt(x*x + y*y + z*z);
    if (theta < std::numeric_limits<double>::epsilon())
        space_->copyState(state, mean);
    else
    {
        SO3StateSpace::StateType q,
            *qs = static_cast<SO3StateSpace::StateType*>(state);
        const SO3StateSpace::StateType *qmu = static_cast<const SO3StateSpace::StateType*>(mean);
        double s = sin(theta / 2) / theta;
        q.w = cos(theta / 2);
        q.x = s * x;
        q.y = s * y;
        q.z = s * z;
        quaternionProduct(*qs, *qmu, q);
    }
}

unsigned int ompl::base::SO3StateSpace::getDimension(void) const
{
    return 3;
}

double ompl::base::SO3StateSpace::getMaximumExtent(void) const
{
    return .5 * boost::math::constants::pi<double>();
}

double ompl::base::SO3StateSpace::norm(const StateType *state) const
{
    double nrmSqr = state->x * state->x + state->y * state->y + state->z * state->z + state->w * state->w;
    return (fabs(nrmSqr - 1.0) > std::numeric_limits<double>::epsilon()) ? sqrt(nrmSqr) : 1.0;
}

void ompl::base::SO3StateSpace::enforceBounds(State *state) const
{
    StateType *qstate = static_cast<StateType*>(state);
    double nrm = norm(qstate);
    if (fabs(nrm) < MAX_QUATERNION_NORM_ERROR)
        qstate->setIdentity();
    else if (fabs(nrm - 1.0) > MAX_QUATERNION_NORM_ERROR)
    {
        qstate->x /= nrm;
        qstate->y /= nrm;
        qstate->z /= nrm;
        qstate->w /= nrm;
    }
}

bool ompl::base::SO3StateSpace::satisfiesBounds(const State *state) const
{
    return fabs(norm(static_cast<const StateType*>(state)) - 1.0) < MAX_QUATERNION_NORM_ERROR;
}

void ompl::base::SO3StateSpace::copyState(State *destination, const State *source) const
{
    const StateType *qsource = static_cast<const StateType*>(source);
    StateType *qdestination = static_cast<StateType*>(destination);
    qdestination->x = qsource->x;
    qdestination->y = qsource->y;
    qdestination->z = qsource->z;
    qdestination->w = qsource->w;
}

unsigned int ompl::base::SO3StateSpace::getSerializationLength(void) const
{
    return sizeof(double) * 4;
}

void ompl::base::SO3StateSpace::serialize(void *serialization, const State *state) const
{
    memcpy(serialization, &state->as<StateType>()->x, sizeof(double) * 4);
}

void ompl::base::SO3StateSpace::deserialize(State *state, const void *serialization) const
{
    memcpy(&state->as<StateType>()->x, serialization, sizeof(double) * 4);
}

/// @cond IGNORE

/*
Based on code from :

Copyright (c) 2003-2006 Gino van den Bergen / Erwin Coumans  http://continuousphysics.com/Bullet/
*/
namespace ompl
{
    namespace base
    {
        static inline double arcLength(const State *state1, const State *state2)
        {
            const SO3StateSpace::StateType *qs1 = static_cast<const SO3StateSpace::StateType*>(state1);
            const SO3StateSpace::StateType *qs2 = static_cast<const SO3StateSpace::StateType*>(state2);
            double dq = fabs(qs1->x * qs2->x + qs1->y * qs2->y + qs1->z * qs2->z + qs1->w * qs2->w);
            if (dq > 1.0 - MAX_QUATERNION_NORM_ERROR)
                return 0.0;
            else
                return acos(dq);
        }
    }
}
/// @endcond

double ompl::base::SO3StateSpace::distance(const State *state1, const State *state2) const
{
    return arcLength(state1, state2);
}

bool ompl::base::SO3StateSpace::equalStates(const State *state1, const State *state2) const
{
    return arcLength(state1, state2) < std::numeric_limits<double>::epsilon();
}

/*
Based on code from :

Copyright (c) 2003-2006 Gino van den Bergen / Erwin Coumans  http://continuousphysics.com/Bullet/
*/
void ompl::base::SO3StateSpace::interpolate(const State *from, const State *to, const double t, State *state) const
{
    assert(fabs(norm(static_cast<const StateType*>(from)) - 1.0) < MAX_QUATERNION_NORM_ERROR);
    assert(fabs(norm(static_cast<const StateType*>(to)) - 1.0) < MAX_QUATERNION_NORM_ERROR);

    double theta = arcLength(from, to);
    if (theta > std::numeric_limits<double>::epsilon())
    {
        double d = 1.0 / sin(theta);
        double s0 = sin((1.0 - t) * theta);
        double s1 = sin(t * theta);

        const StateType *qs1 = static_cast<const StateType*>(from);
        const StateType *qs2 = static_cast<const StateType*>(to);
        StateType       *qr  = static_cast<StateType*>(state);
        double dq = qs1->x * qs2->x + qs1->y * qs2->y + qs1->z * qs2->z + qs1->w * qs2->w;
        if (dq < 0)  // Take care of long angle case see http://en.wikipedia.org/wiki/Slerp
            s1 = -s1;

        qr->x = (qs1->x * s0 + qs2->x * s1) * d;
        qr->y = (qs1->y * s0 + qs2->y * s1) * d;
        qr->z = (qs1->z * s0 + qs2->z * s1) * d;
        qr->w = (qs1->w * s0 + qs2->w * s1) * d;
    }
    else
    {
        if (state != from)
            copyState(state, from);
    }
}

ompl::base::StateSamplerPtr ompl::base::SO3StateSpace::allocDefaultStateSampler(void) const
{
    return StateSamplerPtr(new SO3StateSampler(this));
}

ompl::base::State* ompl::base::SO3StateSpace::allocState(void) const
{
    return new StateType();
}

void ompl::base::SO3StateSpace::freeState(State *state) const
{
    delete static_cast<StateType*>(state);
}

void ompl::base::SO3StateSpace::registerProjections(void)
{
    class SO3DefaultProjection : public ProjectionEvaluator
    {
    public:

        SO3DefaultProjection(const StateSpace *space) : ProjectionEvaluator(space)
        {
        }

        virtual unsigned int getDimension(void) const
        {
            return 3;
        }

        virtual void defaultCellSizes(void)
        {
            cellSizes_.resize(3);
            cellSizes_[0] = boost::math::constants::pi<double>() / magic::PROJECTION_DIMENSION_SPLITS;
            cellSizes_[1] = boost::math::constants::pi<double>() / magic::PROJECTION_DIMENSION_SPLITS;
            cellSizes_[2] = boost::math::constants::pi<double>() / magic::PROJECTION_DIMENSION_SPLITS;
        }

        virtual void project(const State *state, EuclideanProjection &projection) const
        {
            projection(0) = state->as<SO3StateSpace::StateType>()->x;
            projection(1) = state->as<SO3StateSpace::StateType>()->y;
            projection(2) = state->as<SO3StateSpace::StateType>()->z;
        }
    };

    registerDefaultProjection(ProjectionEvaluatorPtr(dynamic_cast<ProjectionEvaluator*>(new SO3DefaultProjection(this))));
}

double* ompl::base::SO3StateSpace::getValueAddressAtIndex(State *state, const unsigned int index) const
{
    return index < 4 ? &(state->as<StateType>()->x) + index : NULL;
}

void ompl::base::SO3StateSpace::printState(const State *state, std::ostream &out) const
{
    out << "SO3State [";
    if (state)
    {
        const StateType *qstate = static_cast<const StateType*>(state);
        out << qstate->x << " " << qstate->y << " " << qstate->z << " " << qstate->w;
    }
    else
        out << "NULL";
    out << ']' << std::endl;
}

void ompl::base::SO3StateSpace::printSettings(std::ostream &out) const
{
    out << "SO(3) state space '" << getName() << "' (represented using quaternions)" << std::endl;
}
