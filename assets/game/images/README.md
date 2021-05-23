# Images

The images are created by rasterizing their SVG versions in assets/web at
150x150.

The BMP file format makes them fairly large but any compression algorithm should
significantly cut down their size by more than half, making it unnecessary to
use another file format which would mean having to depend on a library like
SDL_image.
