#ifndef __TR1_WRAPPER_HPP__
#define __TR1_WRAPPER_HPP__

//  C++ Defines.
#ifdef OT_USE_TR1
#undef OT_USE_TR1
#endif
#if !defined(_MSC_VER) && defined(OPENTXS_CXX03_TR1)
#define OT_USE_TR1
#endif

#ifndef OT_USE_TR1
#define _SharedPtr std::shared_ptr
#else
#define _SharedPtr std::tr1::shared_ptr
#endif

#endif //__TR1_WRAPPER_HPP__
