![logo](https://code.google.com/p/omegalib/logo?cct=1370642046) omegalib
========
A creative framework for virtual reality and cluster-driven display systems.

Wiki page here: https://github.com/febret/omegalib/wiki

omegalib is a middleware designed to ease the development of applications on virtual reality and immersive systems. Its main features are:
 * Support for hybrid systems, presenting high definition 2D and 3D content on the same display surface
 * A C++ and Python API. Applications can be developed as standalone executables in C++ or as scripts in Python. Omegalib also supports mixed native/script applications with user-defined C++ modules that can be exposed to Python through a simple declarative interface.
 * Runtime application switching: applications can be reloaded or swapped without restarting the display system.
 * Display system scalability: omegalib runs on desktop machines, multi-GPU workstations driving tiled displays, and cluster systems like the 36-machine, 72-display CAVE2.
 * Web interface control and pixel streaming to HTML5 clients
 * Experimental SAGE integration
 * A customizable 2D / 3D widgets library
 * Support for a wide range of input peripherals (controllers, motion capture systems, touch surfaces, speech and brain interfaces), through the [https://github.com/febret/omicron Omicron toolkit]
 * Extendable integration with third party higher level toolkits like vtk and Open Scene Graph.
 * Sound playback through a supercollider-based sound server. Omegalib takes care of synchronizing sound assets between the application and sound machines of a VR installation. The sound server is scalable (it runs on laptop stereo speakers or on the 22 channel CAVE2 audio system) and can be customized using the supercollider scripting language.
Altough omegalib is designed for VR systems, it can be used to develop applications on standard desktop systems, leveraging the power of multiple GPU units when available.
The omegalib source distribution also includes all required main dependencies and *builds out-of-the-box* on Windows, Linux (32 and 64 bit) and OSX 10.7 or higher.
