       ___
      /  /
   __/  /__
  /_    __/▟████████▛
   /  /   ▟██▛▀▀▀▀▀▀       ████████ ▟▙  ██   ▆█       ██████ ▟▙
  /  /   ▟██▛                 ██    ▝▘  ██ ▐████▌     ██     ▝▘ ▜█▙   ▟█▛ ▄▆██▆▄
 /  /   ▟██████▆▄             ██    ██  ██   ██       █████▌ ██  ▜█▙ ▟█▛ ▐██▄▄██▌
/   ╰-- ▀▀▀▀▀████▌            ██    ██  ██   ██       ██     ██   ▜███▛  ▐█▙  ▄▄
\     /      ▟███             ██    ██  ██   ▜██▛     ██     ██    ▜█▛    ▀████▀
 `---'    ▂▄████
    ▜█████████▀
      ▀▀▀▀▀▀

================================================================================

README

  The Tilt Five native development kit enables development of content for the
  Tilt Five platform.

GETTING STARTED

  API documentation is provided in the 'docs' directory. Refer to the content
  there for details on how to use the API.

  Once you're familiar with the API, developing for Tilt Five is as simple as
  including the appropriate header from the 'include' directory, and packaging
  the appropriate library from the 'lib' directory with your application.

  Native C headers are provided by including 'TiltFiveNative.h' (C11 required).
  To use the C++ wrapper, include 'TiltFiveNative.hpp' (C++11 required).

  The native library can be dynamically loaded or linked.

SAMPLES

  The `samples` directory in the NDK download contains three trivial clients
  exercising a range of functions of the NDK. The two files `sample.c` and
  `sample.cpp` use the C and C++ interface respectively.
  They have been tested with GCC 11.2.0, clang 13.0.0-2, and MSVC 19.30.30705,
  built as follows:

  - `gcc -std=c11 sample.c -I .. ../lib/linux64/libTiltFiveNative.so`
  - `g++ -std=c++11 sample.cpp -I .. ../lib/linux64/libTiltFiveNative.so -latomic`
  - `clang -std=c11 sample.c -I .. ../lib/linux64/libTiltFiveNative.so`
  - `clang++ -std=c++11 sample.cpp -I .. ../lib/linux64/libTiltFiveNative.so -latomic`
  - `cl /std:c11 sample.c /MD /I .. ..\lib\win64\TiltFiveNative.dll.if.lib`
  - `cl /std:c++14 sample.cpp /MD /EHsc /I .. ..\lib\win64\TiltFiveNative.dll.if.lib`

CONTACT

  Need support with your questions?
  Developing a game?
  Want to join our team?
  Reach out! We always want to meet fellow gamers!

  Developer Relations : devrel@tiltfive.com
  Careers             : jobs@tiltfive.com
  Press Relations     : press@tiltfive.com

WEBSITE

  Visit the Tilt Five website for the latest news and downloads:

        https://tiltfive.com


NOTICE

 Copyright (C) 2020-2022 Tilt Five, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
