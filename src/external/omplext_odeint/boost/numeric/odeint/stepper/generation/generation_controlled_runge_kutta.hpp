/*
 [auto_generated]
 boost/numeric/odeint/stepper/generation/generation_controlled_runge_kutta.hpp

 [begin_description]
 Specialization of the controller factory for the controlled_runge_kutta class.
 [end_description]

 Copyright 2009-2011 Karsten Ahnert
 Copyright 2009-2011 Mario Mulansky

 Distributed under the Boost Software License, Version 1.0.
 (See accompanying file LICENSE_1_0.txt or
 copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef OMPLEXT_BOOST_NUMERIC_ODEINT_STEPPER_GENERATION_GENERATION_CONTROLLED_RUNGE_KUTTA_HPP_INCLUDED
#define OMPLEXT_BOOST_NUMERIC_ODEINT_STEPPER_GENERATION_GENERATION_CONTROLLED_RUNGE_KUTTA_HPP_INCLUDED

#include <omplext_odeint/boost/numeric/odeint/stepper/controlled_runge_kutta.hpp>
#include <omplext_odeint/boost/numeric/odeint/stepper/generation/make_controlled.hpp>


namespace boost {
namespace numeric {
namespace omplext_odeint {


// controller factory for controlled_runge_kutta
template< class Stepper >
struct controller_factory< Stepper , controlled_runge_kutta< Stepper > >
{
    typedef Stepper stepper_type;
    typedef controlled_runge_kutta< stepper_type > controller_type;
    typedef typename controller_type::error_checker_type error_checker_type;
    typedef typename stepper_type::value_type value_type;

    controller_type operator()( const value_type &abs_error , const value_type &rel_error , const stepper_type &stepper )
    {
        return controller_type( error_checker_type( abs_error , rel_error ) , stepper );
    }
};




} // odeint
} // numeric
} // boost


#endif // BOOST_NUMERIC_ODEINT_STEPPER_GENERATION_GENERATION_CONTROLLED_RUNGE_KUTTA_HPP_INCLUDED
