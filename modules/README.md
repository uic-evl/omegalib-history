This directory contains omegalib modules. Modules are a way to extend omegalib through C++ or python, and can be imported into omegalib python scripts with the standard `import` statement. C++ Modules are compiled automatically during the omegalib build process. Python modules do not require compilation, you just need to copy them directly into your application directory, or place them in one of the python module search paths.

To add a C++ module to the omegalib build, modify the `CMakeLists.txt` file in this directory.

`templateModule` is a good place to start if you want to create your own C++ module. `navigator` and `wandaid` are good examples of Python modules.

FOr more information on how modules work read this wiki page: https://github.com/febret/omegalib/wiki/ExtendingOmegalib
