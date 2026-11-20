# A random dhwem3 fork

This is a random dhwem3 fork, here's some additions that may or may not work that well.

* Integrated Skin Deep's GLSL renderer
* Ported over required Doom 3 BFG hlsl shaders for use with renderer
* First person body view-ish
* Some other nonsense

## Building

Mostly the same as dhwem3, however on linux we'll use ninja as I find it's a little nicer to use.

``` shell
mkdir build
cd build
cmake ../neo -G 'Ninja'
ninja
```

