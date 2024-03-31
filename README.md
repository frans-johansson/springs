# Springs
Very rudimentary mass-spring-damper system simulation in C, visualized with Raylib.

The source code includes a default example of a cloth-like material simulation with mouse interactivity for pulling and cutting.

## Dependencies
Raylib needs to be available on your system for linking if you want to build the main binary.

## Controls
- `PERIOD`: Toggles a "wind" applying a constant force from the left direction.
- `LEFT MOUSE BUTTON`: When hovering over a node, click and drag to move the node. Otherwise, click and drag to "cut" the node connections (i.e., the springs).
- `SPACE`: Pause and unpause the simulation.
- `RETURN`: Reset the default cloth example.
