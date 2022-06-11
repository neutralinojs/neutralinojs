// infoware - C++ System information Library
//
// Written in 2016-2020 by nabijaczleweli <nabijaczleweli@gmail.com> and ThePhD <phdofthehouse@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related
// and neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along with this software.
// If not, see <http://creativecommons.org/publicdomain/zero/1.0/>


#ifndef INFOWARE_USE_D3D
#ifndef INFOWARE_USE_OPENCL
#ifdef INFOWARE_USE_OPENGL


// OpenGL is literaly hitler to initialise a windowless context for, so no game.
// Leave it for someone smarter than me :G
//
// Here is guidance I received from my Graphics Lord, Elim Garak, on Discord:
// [5:44 PM] 🕴: Elim, my lord, plz tell me the simplest way of setting up a dummy OGL ctx so I can glGetString()
// [5:46 PM] Elim | iscicadabannedyet.com: You know how to acquire your function pointers, rite? For WGL, EGL, XGL shit etc(edited)
// [5:46 PM] Borgleader: glLoadGen >>>>> acquiring pointers manually
// [5:46 PM] Elim | iscicadabannedyet.com: or that
// [5:46 PM] 🕴: not really, no
// [5:46 PM] Elim | iscicadabannedyet.com: You just want to know how to get the dummy?
// [5:46 PM] Elim | iscicadabannedyet.com: oh
// [5:47 PM] 🕴: from what I've read it's impl-def land
// [5:47 PM] 🕴: OGL land is so fucking alien to me
// [5:48 PM] 🕴: from what I read I assume you use like WGL or some shit to make a ctx which allows you to call OGL-proper functions
// [5:48 PM] Elim | iscicadabannedyet.com: Basically, on Windows it's as trivial as wglCreateContext(GetDC(your_window_handle)
// [5:48 PM] Elim | iscicadabannedyet.com: WGL is the glue between the Windows window system and OGL
// [5:49 PM] 🕴: Can I use a dummy window for that?
// [5:49 PM] Elim | iscicadabannedyet.com: yes
// [5:49 PM] 🕴: cool
// [5:49 PM] 🕴: And what on non-Windows?
// [5:50 PM] Elim | iscicadabannedyet.com: On OS X, one doesn't need to jump through such hoops because the Apple lords integrated OpenGL 4.1 and below inside
//                                         of their system SDK
// [5:50 PM] Elim | iscicadabannedyet.com: As for lunix, it's EGL over there
// [5:50 PM] Elim | iscicadabannedyet.com: https://www.khronos.org/registry/egl/sdk/docs/man/html/eglCreateContext.xhtml
// [5:50 PM] 🕴: So I don't need to set up anything on OSX, it will work OOTB?(edited)
// [5:50 PM] Borgleader: except their drivers are shit so theres no point inusing it 😛
// [5:51 PM] Elim | iscicadabannedyet.com: It will work in the sense that you can get any core profile out of the box, yeah
// [5:51 PM] Elim | iscicadabannedyet.com: will it work practically as borgleader mentions
// [5:51 PM] Elim | iscicadabannedyet.com: hehehe, fun times ahead 😛(edited)
// [5:51 PM] 🕴: right, so a raw call on OSX, WGL wglCreateContext() w/dummy window on Windooze and EGL on Linux
// [5:52 PM] Elim | iscicadabannedyet.com: yeah, on lunix eglCreateContext attaches to the display
// [5:52 PM] 🕴: Elim is the greatest man of all time
// [5:52 PM] Elim | iscicadabannedyet.com: bby, write code and be quiet 😛
// [5:53 PM] Elim | iscicadabannedyet.com: but yeah, it's p retarded


#include "infoware/gpu.hpp"
#include <vector>


std::vector<iware::gpu::device_properties_t> iware::gpu::device_properties() {
	return {};
}


#endif
#endif
#endif
