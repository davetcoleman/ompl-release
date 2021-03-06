/*
 [auto_generated]
 boost/numeric/odeint/stepper/stepper_categories.hpp

 [begin_description]
 Definition of all stepper categories.
 [end_description]

 Copyright 2009-2011 Karsten Ahnert
 Copyright 2009-2011 Mario Mulansky

 Distributed under the Boost Software License, Version 1.0.
 (See accompanying file LICENSE_1_0.txt or
 copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef OMPLEXT_BOOST_NUMERIC_ODEINT_STEPPER_STEPPER_CATEGORIES_HPP_INCLUDED
#define OMPLEXT_BOOST_NUMERIC_ODEINT_STEPPER_STEPPER_CATEGORIES_HPP_INCLUDED


namespace boost {
namespace numeric {
namespace omplext_odeint {


/*
 * Tags to specify stepper types
 *
 * These tags are used by integrate() to choose which integration method is used
 */

struct stepper_tag {};
// struct explicit_stepper_tag : stepper_tag {};
// struct implicit_stepper_tag : stepper_tag {};


struct error_stepper_tag : stepper_tag {};
struct explicit_error_stepper_tag : error_stepper_tag {};
struct explicit_error_stepper_fsal_tag : error_stepper_tag {};

struct controlled_stepper_tag {};
struct explicit_controlled_stepper_tag : controlled_stepper_tag {};
struct explicit_controlled_stepper_fsal_tag : controlled_stepper_tag {};

struct dense_output_stepper_tag {};



} // odeint
} // numeric
} // boost


#endif // BOOST_NUMERIC_ODEINT_STEPPER_STEPPER_CATEGORIES_HPP_INCLUDED
