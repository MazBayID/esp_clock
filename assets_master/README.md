# Assets Master

This folder is the intake area for original UI assets and their manifest.

Upload the original files and manifest here without renaming or converting them.
After reviewing the manifest, Neo Firmware will map each asset into the appropriate
runtime location under `firmware/NeoFirmware/src/assets/`.

The existing clock digit styles are already active firmware defaults and are kept
separately under `src/assets/default/digits/`. New uploads do not replace them
automatically.

Please keep source/design files here; firmware-ready generated headers or bitmaps
will be placed in the runtime asset folders after their dimensions, format, color
depth, and intended use have been checked.
