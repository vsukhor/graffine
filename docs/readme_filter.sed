# Doxygen input filter for README.md (see FILTER_PATTERNS in doxyfile.in).
# Doxygen does not support the <picture> element, which GitHub uses to switch
# the logo with the color scheme: replace the block with the light-scheme logo,
# found by Doxygen in IMAGE_PATH. \image html, unlike a Markdown image, emits
# the image for HTML only, so that Breathe shows it once.
/<picture>/,/<\/picture>/c\
\\image html graffine-logo_1.png "graffine logo" width=50%
