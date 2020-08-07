# OBS Plugin

## CMake Configuration

At the moment, the following entries must be defined to configure CMake:
-`obsPath`: The path to the obs-studio repo, used as a hint to find LibObs
-`LibObs_DIR`: The path to the built obs.lib file. Used by find_package to run the config files

In theory, only one of these should be required, yet it appears that both are necessary anyway because something is broken outside of my control.
