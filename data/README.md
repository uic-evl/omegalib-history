The `data` directory contains core assets and scripts used by omegalib. Some brief information about what's in here:
- `cubemaps` contains some cube-mapped environments that can be used as cyclops skyboxes.
- `cyclops` contains all the basic shaders used by cyclops (in the `common` directory), plus some models and textures used for testing.
- `evl` contains configuration files used for display systems at the Electronic Visualization Lab. Here you can find good examples of how to set up tiled displays and cluster display systems. For instance, `lyra-xinerama.cfg` is the configuration files used for CAVE2.
- `fonts` contains some default fonts used by the menu system and omegalib console.
- `menu_sounds` contains the default sounds used by the menu system.
- `porthole` stores the resources used by the porthole HTML5 interface
- `system` contains some basic configuration files. `desktop.cfg` is commonly used to run omegalib on laptops.
- `default.cfg` is used to select which system configuration to use
- `default_init.py` is the default initialization script used by `orun`. It sets up the basic menu common to all applications. The default initialization script can be changed in the configuration file.