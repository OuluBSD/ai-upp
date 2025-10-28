#pragma once
#ifndef _CtrlCore_t__h_
#define _CtrlCore_t__h_

#ifdef t_
#undef t_
#endif

#ifdef tt_
#undef tt_
#endif

#ifndef t_h
#define t_h

inline const char *t_(const char *s)  { return t_GetLngString(s); }
inline const char *tt_(const char *s) { return s; }

#endif

#endif