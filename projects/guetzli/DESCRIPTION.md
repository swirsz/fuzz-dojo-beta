Use (Report link) to browse the source code and fuzz introspector report https://introspector.oss-fuzz.com/project-profile?project=guetzli

Guetzli is a JPEG encoder/image compression - 80% coverage, the single fuzz driver has large missing functionality for downsampling images.

preprocess_downsample.cc has a 0% coverage.  It also does not look like it does JPEG encoding as jpeg_data_encoder.cc also has 0% coverage.  Compare, heat map, and many miscellaneous functions are not covered.
